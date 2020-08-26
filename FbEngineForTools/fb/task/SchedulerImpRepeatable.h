#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/time/RunningSystemTimer.h"

#include "fb/profiling/ZoneProfiler.h"

FB_PACKAGE1(task)

// Implementation detail of Scheduler.h. Isolated here for easier readability.

#define FB_SCHEDULER_REPEATABLE_GRANULARITY_MS 2

struct Repeatable: public IThreadEntry
{
	Scheduler *self = nullptr;
	Thread *thread = nullptr;
	bool isQuitting = false;
	bool enableTaskUpdate = true;
	bool enableUpdateThread = false;

	// For timing
	uint32_t updateCounter = 0;

	// We can assume(tm) that there is low contention. 
	// We shouldn't be modifying list of repeatable tasks very often.
	Mutex mutex;
	lang::RunningSystemTimer timer;

	// Wrapper for repeatable task which also control information about their execution
	struct RepeatableTask
	{
		// Timing -- only touched by update logic and thus these are NOT protected by internal mutex.
		// Only accessed with general Repeatable mutex.
		uint32_t lastUpdateTimingValue = 0;
		uint32_t timingType = Scheduler::TaskGranularityMillisecondBased;
		uint32_t timingGranularity = 1;
		uint32_t dependencyGroup = Scheduler::getInvalidDependencyGroupId();

		// User task info -- we don't touch these after the init
		ISchedulerTask *task = nullptr;
		char taskData[SchedulerTaskData::MaxDataBytes];
		uint32_t taskDataSize = 0;
		const void *taskDataType = nullptr;
		Scheduler::TaskGroup taskGroup = Scheduler::TaskGroupForeground;

		// Execution info -- protected by internal mutex
		Mutex taskMutex;
		bool isQueued = false;
		bool isRunning = false;
		// We are no longer enabled after user wants to remove the task
		bool isEnabled = true;

		RepeatableTask()
		{
		}

		~RepeatableTask()
		{
			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
			taskMutex.enter();

			fb_assert(!isEnabled);
			fb_assert(!isQueued);
			fb_assert(!isRunning);

			taskMutex.leave();
			#endif
		}

		// Lock task for queue if possible (not already in queue, or somesuch).
		// If shouldBeDeleted is set to true, task has been previously deleted and no longer active -> delete it instead!
		void queueIfPossible(Scheduler &scheduler, task::ISchedulerTask *wrapperTask, bool &shouldBeDeleted) 
		{
			shouldBeDeleted = false;

			taskMutex.enter();
			{
				if (!isEnabled && !isQueued && !isRunning)
				{
					// So ask to be deleted
					shouldBeDeleted = true;
				}

				if (!shouldBeDeleted && !isQueued && !isRunning)
				{
					TaskRunWrapper::Param p;
					p.t = this;
					scheduler.addTask(taskGroup, dependencyGroup, Scheduler::TaskPriorityHigh, wrapperTask, SchedulerTaskData(p));

					isQueued = true;
				}
			}
			taskMutex.leave();
		}

		// Remove task from execution
		void setForDelete(Scheduler &scheduler)
		{
			bool shouldWait = false;

			taskMutex.enter();
			{
				// If we are not actually enabled, user is trying to do a double delete.
				fb_assert(isEnabled);

				isEnabled = false;
				shouldWait = isRunning;
			}
			taskMutex.leave();

			// Do the waiting outside of the mutex, as otherwise we could block queueIfPossible() for a very long time.
			// In turn, this could block whole repeatable scheduler thread for unspecified amount of time.

			// If the task was already executing, wait for it to complete
			// There is no need to wait for queued tasks, as those will just execute a no-op and never go to running state
			if (shouldWait)
				scheduler.waitForDependencies(taskGroup, dependencyGroup);

			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
				taskMutex.enter();
				fb_assert(!isRunning);
				taskMutex.leave();
			#endif
		}

		bool preRunImp()
		{
			bool shouldRun = true;

			taskMutex.enter();
			{
				shouldRun = isEnabled;
				fb_assert(!isRunning);

				if (shouldRun)
				{
					isRunning = true;
				}

				isQueued = false;
			}
			taskMutex.leave();

			return shouldRun;
		};

		void postRunImp()
		{
			taskMutex.enter();
			{
				isQueued = false;
				isRunning = false;
			}
			taskMutex.leave();
		};

		void run(task::Scheduler &scheduler)
		{
			if (!preRunImp())
			{
				// We have been disabled, probably because user removed us from execution
				// --> Can't access user task anymore
				return;
			}

			// Run the actual user task
			SchedulerTaskData runData = (taskDataSize) ? SchedulerTaskData(taskData, taskDataSize, taskDataType) : SchedulerTaskData();

			FB_ZONE(task->getStaticTaskNameString());
			FB_STACK_CUSTOM(task->getStaticTaskNameString());
			task->run(runData, scheduler);

			// And flag us being ready for another run
			postRunImp();
		}

		static RepeatableTask *create()
		{
			void *ptr = lang::allocateFixed(sizeof(RepeatableTask));
			return new (ptr) RepeatableTask();
		}

		static void free(RepeatableTask *ptr)
		{
			if(ptr)
				lang::freeFixed(ptr, sizeof(RepeatableTask));
		}
	};

	struct TaskRunWrapper: public ISchedulerTask
	{
		struct Param
		{
			RepeatableTask *t = nullptr;
			FB_SCHEDULER_TASK_PARAM_TYPE();
		};

		const char *getStaticTaskNameString() const
		{
			return "RepeatableTaskWrapper";
		}

		void run(const SchedulerTaskData &data, task::Scheduler &scheduler)
		{
			Param p;
			data.getParam(p);

			fb_assert(p.t);
			p.t->run(scheduler);
		}
	};

	PodVector<RepeatableTask*> tasks;
	TaskRunWrapper wrapperTask;
	
	Repeatable(bool enableUpdateThread_, Scheduler *self_)
	:	self(self_)
	,	enableUpdateThread(enableUpdateThread_)
	{
		if (enableUpdateThread)
		{
			FB_STATIC_CONST_STRING(threadName, "Scheduler repeatable thread");
			thread = Thread::createNewThread(this, threadName);
		}
	}

	~Repeatable()
	{
		mutex.enter();
		isQuitting = true;
		mutex.leave();

		if (thread)
			Thread::joinThread(thread);
	}

	uint32_t getDifference(uint32_t start, uint32_t end) const
	{
		// Handle wrap-around just to be safe
		if (start <= end)
			return end - start;

		// Assume end wrapped around once
		uint32_t result = (0xFFFFFFFF - start) + end;
		return result;
	}

	void taskUpdateLogic()
	{
		// Assume we are in mutex
		
		if (!enableTaskUpdate)
			return;

		// Actual update
		uint32_t currentTime = uint32_t(timer.getTimeInMilliseconds());

		uint32_t taskAmount = tasks.getSize();
		for (uint32_t i = 0; i < taskAmount; ++i)
		{
			RepeatableTask *FB_RESTRICT t = tasks[i];
			if (!t)
				continue;

			bool shouldQueue = false;
			if (t->timingType == Scheduler::TaskGranularityUpdateBased)
			{
				if (getDifference(t->lastUpdateTimingValue, updateCounter) >= t->timingGranularity)
				{
					t->lastUpdateTimingValue = updateCounter;
					shouldQueue = true;
				}
			}
			else
			{
				if (getDifference(t->lastUpdateTimingValue, currentTime) >= t->timingGranularity)
				{
					t->lastUpdateTimingValue = currentTime;
					shouldQueue = true;
				}
			}

			if (shouldQueue)
			{
				// Put it to queue unless it's there already
				bool shouldBeDeleted = false;
				t->queueIfPossible(*self, &wrapperTask, shouldBeDeleted);

				// Delete instead, if user wants to get rid of (and it's not actually running)
				if (shouldBeDeleted)
				{
					tasks[i] = 0;
					RepeatableTask::free(t);
					t = nullptr;
				}
			}
		}
	}

	void entry()
	{
		for (;;)
		{
			Thread::sleep(FB_SCHEDULER_REPEATABLE_GRANULARITY_MS);
			mutex.enter();

			taskUpdateLogic();
			if (isQuitting)
			{
				mutex.leave();
				break;
			}

			mutex.leave();
		}
	}

	// Be very careful with this!
	void enableTasks(bool enable) 
	{
		mutex.enter();
		enableTaskUpdate = enable;
		mutex.leave();
	}

	void updateCall()
	{
		mutex.enter();

		++updateCounter;

		// Make sure tasks are eventually executed, even if the update thread is disabled
		// Don't run taskUpdateLogic() otherwise -- we could, but that could theoretically double high priority update thread execution time
		// It's also nice to that execution time has some variance, otherwise client code might depend on exact timings
		if (!enableUpdateThread)
			taskUpdateLogic();

		mutex.leave();
	}

	Scheduler::RepeatableTaskId addRepeatableTask(Scheduler::TaskGroup taskGroup, ISchedulerTask *task, Scheduler::TaskGranularity taskGranularity, uint32_t granularityParam, const SchedulerTaskData &optionalData)
	{
		Scheduler::RepeatableTaskId id = Scheduler::getInvalidRepeatableTaskID();
		mutex.enter();

		// Initialise
		Repeatable::RepeatableTask *t = RepeatableTask::create();
		t->timingType = taskGranularity;
		t->timingGranularity = granularityParam;
		t->dependencyGroup = self->createDependencyGroup(taskGroup);
		t->task = task;
		t->taskGroup = taskGroup;
		if (optionalData.hasData())
		{
			const char *dataPointer = 0;
			uint32_t dataSize = 0;
			const void *dataType = 0;
			optionalData.getParam(dataPointer, dataSize, dataType);

			lang::MemCopy::copy(t->taskData, dataPointer, dataSize);
			t->taskDataSize = dataSize;
			t->taskDataType = dataType;
		}

		// Find a place to store it
		{
			uint32_t taskAmount = tasks.getSize();
			for (uint32_t i = 0; i < taskAmount; ++i)
			{
				if (tasks[i] == 0)
				{
					id = i;
					tasks[i] = t;
					break;
				}
			}

			if (id == Scheduler::getInvalidRepeatableTaskID())
			{
				id = tasks.getSize();
				tasks.pushBack(t);
			}
		}
		
		mutex.leave();
		fb_assert(id != Scheduler::getInvalidRepeatableTaskID());
		return id;
	}

	void removeRepeatableTask(Scheduler::RepeatableTaskId repeatableTaskId)
	{
		// We don't need a global mutex for this, as that would be a very bad idea. 
		// We would block all repeatable tasks for possibly very long time, as we might have to wait for the task to actually finish executing.

		fb_assert(repeatableTaskId != Scheduler::getInvalidRepeatableTaskID());
		uint32_t taskIndex = repeatableTaskId;
		fb_assert(tasks.getSize() > taskIndex);
		RepeatableTask *t = tasks[taskIndex];
		
		// Make sure it won't be run again.
		// taskUpdateLogic() will take care of actual delete
		fb_assert(t);
		t->setForDelete(*self);
	}
};

FB_END_PACKAGE1()
