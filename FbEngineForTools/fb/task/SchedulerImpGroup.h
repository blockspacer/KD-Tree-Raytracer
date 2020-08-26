#ifndef FB_TASK_SCHEDULERIMPGROUP_H
#define FB_TASK_SCHEDULERIMPGROUP_H

#include "fb/container/PodVector.h"
#include "fb/container/Vector.h"
#include "fb/lang/CallStack.h"
#include "fb/lang/thread/IThreadEntry.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/lang/thread/Thread.h"
#include "fb/profiling/TaskProfiler.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/task/SchedulerImpSync.h"
#include "fb/task/SchedulerImpTask.h"
#include "fb/task/GetSystemThreadPriority.h"

// Implementation detail, don't use directly

FB_PACKAGE1(task)

#define FB_GROUP_DEPENDENCY_INDEX_MASK 0xffffff
#define FB_GROUP_DEPENDENCY_MASK_MASK  0xff000000
#define FB_GROUP_DEPENDENCY_MASK_SHIFT  24

#define FB_GROUP_DEPENDENCY_INDEX_GET(v) ((v) & FB_GROUP_DEPENDENCY_INDEX_MASK)
#define FB_GROUP_DEPENDENCY_MASK_GET(v) (((v) & FB_GROUP_DEPENDENCY_MASK_MASK) >> FB_GROUP_DEPENDENCY_MASK_SHIFT)
#define FB_GROUP_DEPENDENCY_MAKE(i, m) (i | (m << FB_GROUP_DEPENDENCY_MASK_SHIFT))

struct Group
{
	mutable Sync sync;
	TaskList tasks;

	// To make sure user can do a blocking wait only from 1 thread at the same time
	Mutex userBlockMutex;
	// To make sure user can do a blocking dependency wait only from 1 thread at the same time
	Mutex userDependencyMutex;

	struct ThreadEntry: public IThreadEntry
	{
		Group *self = nullptr;
		uint32_t threadIndex = 0;

		ThreadEntry()
		{
		}

		void entry()
		{
			self->threadEntry(threadIndex);
		}
	};

	// Thread stuff
	ThreadEntry **threadEntries;
	Thread **threads;
	uint32_t threadAmount; 
	bool threadQuit;

	// Execution state
	int executeCounter;
	// Blocking stuff
	Scheduler::DependencyGroupId userBlockingDependency;
	bool userBlocking;

	// Dependency stuff
	struct DependencyGroup
	{
		uint32_t executeCounter = 0;
		uint32_t queueCounter = 0;
		uint32_t activeMask = 1;
		bool isActive = false;

		// Possible after task
		ISchedulerTask *afterTask = nullptr;
		InternalTaskDataCopy afterTaskData;
		Scheduler::TaskPriority afterTaskPriority = Scheduler::TaskPriorityNormal;

		DependencyGroup()
		{
		}

		bool hasAfterTask() const
		{
			return afterTask != NULL;
		}

		void validateMask(uint32_t dependencyMask) const
		{
			// If this fails, you are using old dependency index which has been destroyed and/or replaced
			fb_assert(isActive && activeMask == dependencyMask);
		}

		uint32_t getTaskState() const
		{
			uint32_t taskState = 0;
			if (queueCounter)
				taskState |= TaskActiveStateQueue;
			if (executeCounter)
				taskState |= TaskActiveStateExecution;

			return taskState;
		}
	};
	PodVector<DependencyGroup> dependencyGroups;

	enum GroupPriority
	{
		GroupPriorityLow = 1,
		GroupPriorityNormal = 2,
		GroupPriorityHigh = 4
	};

	enum TaskActiveState
	{
		TaskActiveStateQueue = 1,
		TaskActiveStateExecution = 2
	};

	Scheduler *self;

	Group(uint32_t threadAmount_, uint32_t priority, Scheduler *scheduler)
	:	threadEntries(0)
	,	threads(0)
	,	threadAmount(threadAmount_)
	,	threadQuit(false)
	,	executeCounter(0)
	,	userBlockingDependency(Scheduler::getInvalidDependencyGroupId())
	,	userBlocking(false)
	,	self(scheduler)
	{
		dependencyGroups.reserve(256);

		// Prepare syncing
		sync.init(threadAmount);
		// And lock just in case
		SyncGuard g(sync);

		// Create thread entries
		threadEntries = new ThreadEntry* [threadAmount];
		threads = new Thread* [threadAmount];
		for (uint32_t i = 0; i < threadAmount; ++i)
		{
			threadEntries[i] = new ThreadEntry();
			threadEntries[i]->self = this;
			threadEntries[i]->threadIndex = 1 + i;
		}

		StringRef threadName = "Scheduler::Worker (undefined)";
		uint32_t threadPriority = 0;
		if (priority == GroupPriorityLow)
		{
			threadName = "SchedulerWorker, low";
			threadPriority = GetSystemThreadPriority::getSystemThreadPriority(-2);
		}
		else if (priority == GroupPriorityNormal)
		{
			threadName = "SchedulerWorker, normal";
			threadPriority = GetSystemThreadPriority::getSystemThreadPriority(0);
		}
		else if (priority == GroupPriorityHigh)
		{
			threadName = "SchedulerWorker, high";
			threadPriority = GetSystemThreadPriority::getSystemThreadPriority(1);
		}

		// Create the actual threads

		for (uint32_t i = 0; i < threadAmount; ++i)
		{
			// First foreground thread has -1 priority compared to normal.
			// Otherwise we'd have too many threads for the amount of cores
			StaticString localName = StaticString::createFromAnyString(threadName);
			uint32_t localPriority = threadPriority;

			// This hack doesn't really work with manually managed affinities and priorities	
			if (priority == GroupPriorityNormal && i == 0)
			{
				localName = StaticString("SchedulerWorker, normal-1");
				localPriority = GetSystemThreadPriority::getSystemThreadPriority(-1);
			}

			threads[i] = Thread::createNewThread(threadEntries[i], localName);
			Thread::setPriority(threads[i], localPriority);
		}
	}

	~Group()
	{
		if (threadAmount)
		{
			// Signal quit
			{
				SyncGuard g(sync);
				threadQuit = true;

				sync.signalWorkForThreads(threadAmount);
			}

			// Join threads
			for (uint32_t i = 0; i < threadAmount; ++i)
			{
				if (threads[i])
				{
					Thread::joinThread(threads[i]);
					delete threadEntries[i];
				}
			}

			// And delete
			delete[] threads;
			delete[] threadEntries;
			threads = 0;
			threadEntries = 0;
		}
	}

	uint32_t getTaskStateNoMutex(Scheduler::DependencyGroupId dependency)
	{
		uint32_t dependencyIndex = FB_GROUP_DEPENDENCY_INDEX_GET(dependency);
		uint32_t dependencyMask = FB_GROUP_DEPENDENCY_MASK_GET(dependency);
		DependencyGroup &dg = dependencyGroups[dependencyIndex];
		dg.validateMask(dependencyMask);

		return dg.getTaskState();
	}

	Scheduler::DependencyGroupId createDependencyGroup()
	{
		SyncGuard g(sync);

		uint32_t createIndex = Scheduler::getInvalidDependencyGroupId();
		for (uint32_t i = 0; i < dependencyGroups.getSize(); ++i)
		{
			DependencyGroup &dg = dependencyGroups[i];
			if (!dg.isActive)
			{
				createIndex = i;
				break;
			}
		}

		// Create new
		if (createIndex == Scheduler::getInvalidDependencyGroupId())
		{
			createIndex = dependencyGroups.getSize();
			dependencyGroups.resize(createIndex + 1);
		}

		DependencyGroup &dg = dependencyGroups[createIndex];
		fb_assert(!dg.isActive && dg.executeCounter == 0 && dg.queueCounter == 0);

		dg.isActive = true;
		++dg.activeMask;
		if (dg.activeMask >= 255)
			dg.activeMask = 1;

		Scheduler::DependencyGroupId id = FB_GROUP_DEPENDENCY_MAKE(createIndex, dg.activeMask);
		return id;
	}

	void freeDependencyGroup(Scheduler::DependencyGroupId dependencyGroupId)
	{
		uint32_t dependencyIndex = FB_GROUP_DEPENDENCY_INDEX_GET(dependencyGroupId);
		uint32_t dependencyMask = FB_GROUP_DEPENDENCY_MASK_GET(dependencyGroupId);

		// First, remove queued tasks with given dependency
		{
			SyncGuard g(sync);
			uint32_t removeQueueCount = tasks.removeDependencies(dependencyGroupId);

			// As we removed everything from the queue, we also need to clear the counters
			DependencyGroup &dg = dependencyGroups[dependencyIndex];
			dg.queueCounter -= removeQueueCount;

			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
				fb_assert(dg.queueCounter == 0);
			#endif			
		}

		// Make sure already executing tasks have finished
		waitForDependencies(dependencyGroupId);

		// And delete the group
		{
			SyncGuard g(sync);

			DependencyGroup &dg = dependencyGroups[dependencyIndex];
			dg.validateMask(dependencyMask);

			dg.isActive = false;
			fb_assert(dg.executeCounter == 0);
			fb_assert(dg.queueCounter == 0);
		}
	}

	void addTasksImpNoMutex(Scheduler::DependencyGroupId dependencyGroupId, Scheduler::TaskPriority taskPriority, ISchedulerTask *task, SizeType taskAmount, const SchedulerTaskData *optionalDatas)
	{
		FB_ZONE_FUNC();
		// Task independent stuff
		bool taskPushFront = false;
		bool taskPushAfterDependency = false;
		bool taskPushBack = false;

		if (dependencyGroupId != Scheduler::getInvalidDependencyGroupId())
		{
			uint32_t dependencyIndex = FB_GROUP_DEPENDENCY_INDEX_GET(dependencyGroupId);
			uint32_t dependencyMask = FB_GROUP_DEPENDENCY_MASK_GET(dependencyGroupId);
			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
				fb_assert(dependencyIndex < dependencyGroups.getSize());
			#endif

			DependencyGroup &dg = dependencyGroups[dependencyIndex];
			dg.validateMask(dependencyMask);
			dg.queueCounter += taskAmount;
		}

		if (userBlockingDependency != Scheduler::getInvalidDependencyGroupId()) // Are we are currently blocking on any dependency
		{
			if (dependencyGroupId == userBlockingDependency) // Are we are blocking on our own dependency
			{
				if (taskPriority == Scheduler::TaskPriorityHigh) // High go to front
					taskPushFront = true;
				else
					taskPushAfterDependency = true; // Others after exising (dependency) tasks
			}
			else // Are we blocking on some other dependency
			{
				if (taskPriority == Scheduler::TaskPriorityHigh) // High go after blocking dependency
					taskPushAfterDependency = true;
				else
					taskPushBack = true; // Others right at the back
			}
		}
		else // No active dependency
		{
			if (taskPriority == Scheduler::TaskPriorityHigh) // Criticals go to front
				taskPushFront = true;
			else
				taskPushBack = true; // Others right at the back
		}

		// Per task stuff
		for (SizeType i = 0; i < taskAmount; ++i)
		{
			FB_ZONE("for(taskAmount)");
			Task *t = Task::create();
			fb_assert(t);
			t->task = task;
			t->dependency = dependencyGroupId;
			t->taskDebugCreationTime = profiling::TaskProfiler::getTimeStamp();
			t->taskDebugThreadId = profiling::TaskProfiler::getThreadId();

			if (optionalDatas)
				t->taskData.copyFrom(optionalDatas[i]);

			if (taskPushFront)
				tasks.pushFront(t);
			else if (taskPushBack)
				tasks.pushBack(t);
			else if (taskPushAfterDependency)
				tasks.pushAfter(userBlockingDependency, t);
			else
			{
				fb_assert(0 && !"Task variables out of sync.");
			}
		}
	}

	void addTasksImpNoMutex(Scheduler::DependencyGroupId dependencyGroupId, Scheduler::TaskPriority taskPriority, Task *firstTask, Task *lastTask, SizeType taskAmount)
	{
		FB_ZONE_FUNC();
		// Task independent stuff
		bool taskPushFront = false;
		bool taskPushAfterDependency = false;
		bool taskPushBack = false;

		if (dependencyGroupId != Scheduler::getInvalidDependencyGroupId())
		{
			uint32_t dependencyIndex = FB_GROUP_DEPENDENCY_INDEX_GET(dependencyGroupId);
			uint32_t dependencyMask = FB_GROUP_DEPENDENCY_MASK_GET(dependencyGroupId);
			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
				fb_assert(dependencyIndex < dependencyGroups.getSize());
			#endif

			DependencyGroup &dg = dependencyGroups[dependencyIndex];
			dg.validateMask(dependencyMask);
			dg.queueCounter += taskAmount;
		}

		if (userBlockingDependency != Scheduler::getInvalidDependencyGroupId()) // Are we are currently blocking on any dependency
		{
			if (dependencyGroupId == userBlockingDependency) // Are we are blocking on our own dependency
			{
				if (taskPriority == Scheduler::TaskPriorityHigh) // High go to front
					taskPushFront = true;
				else
					taskPushAfterDependency = true; // Others after exising (dependency) tasks
			}
			else // Are we blocking on some other dependency
			{
				if (taskPriority == Scheduler::TaskPriorityHigh) // High go after blocking dependency
					taskPushAfterDependency = true;
				else
					taskPushBack = true; // Others right at the back
			}
		}
		else // No active dependency
		{
			if (taskPriority == Scheduler::TaskPriorityHigh) // Criticals go to front
				taskPushFront = true;
			else
				taskPushBack = true; // Others right at the back
		}

		if (taskPushFront)
			tasks.pushFront(firstTask, lastTask, taskAmount);
		else if (taskPushBack)
			tasks.pushBack(firstTask, lastTask, taskAmount);
		else if (taskPushAfterDependency)
			tasks.pushAfter(userBlockingDependency, firstTask, lastTask, taskAmount);
		else
		{
			fb_assert(0 && !"Task variables out of sync.");
		}
	}

	void addTasks(Scheduler::DependencyGroupId dependencyGroupId, Scheduler::TaskPriority taskPriority, ISchedulerTask *task, SizeType taskAmount, const SchedulerTaskData *optionalDatas)
	{
		FB_ZONE("addSchedulerTasks");

		// Create local linked list before poking around the scheduler group internals
		Task *firstTask = nullptr;
		Task *lastTask = nullptr;
		{
			FB_ZONE("createLocalList");

			uint64_t time = profiling::TaskProfiler::getTimeStamp();
			uint32_t threadId = profiling::TaskProfiler::getThreadId();
			for(uint32_t i = 0; i < taskAmount; ++i)
			{
				Task *t = Task::create();
				t->task = task;
				t->dependency = dependencyGroupId;
				t->taskDebugCreationTime = time;
				t->taskDebugThreadId = threadId;
				if (optionalDatas)
					t->taskData.copyFrom(optionalDatas[i]);

				if(lastTask)
				{
					lastTask->next = t;
					t->previous = lastTask;
					lastTask = t;
				}
				else
				{
					firstTask = t;
					lastTask = t;
				}
			}
		}

		#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
			if(firstTask != lastTask)
			{
				fb_assert(firstTask->next != nullptr);
				fb_assert(firstTask->previous == nullptr);
				fb_assert(lastTask->next == nullptr);
				fb_assert(lastTask->previous != nullptr);
				Task *current = firstTask;
				while(current)
				{
					if(!current->next)
					{
						fb_assert(current == lastTask);
						break;
					}

					fb_assert(current->next != nullptr);
					fb_assert(current == current->next->previous);
					current = current->next;
				}
			}
			else
			{
				fb_assert(firstTask->next == nullptr);
				fb_assert(firstTask->previous == nullptr);
				fb_assert(lastTask->next == nullptr);
				fb_assert(lastTask->previous == nullptr);
			}
		#endif

		{
			FB_ZONE_ENTER("SyncGuard g(sync)", profiling::ZoneBlock);
			SyncGuard g(sync);
			FB_ZONE_EXIT();
			addTasksImpNoMutex(dependencyGroupId, taskPriority, firstTask, lastTask, taskAmount);
		}

		{
			FB_ZONE("SignalWorkThreads");
			sync.signalWorkForThreads(taskAmount);
		}
	}

	void setAfterDependencyTask(Scheduler::DependencyGroupId dependencyGroupId, Scheduler::TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData)
	{
		fb_assert(dependencyGroupId != Scheduler::getInvalidDependencyGroupId());
		uint32_t dependencyIndex = FB_GROUP_DEPENDENCY_INDEX_GET(dependencyGroupId);
		uint32_t dependencyMask = FB_GROUP_DEPENDENCY_MASK_GET(dependencyGroupId);

		bool shouldSignalWork = false;
		{
			FB_ZONE_ENTER("SyncGuard g(sync)", profiling::ZoneBlock);
			SyncGuard g(sync);
			FB_ZONE_EXIT();

			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
				fb_assert(dependencyIndex < dependencyGroups.getSize());
			#endif

			DependencyGroup &dg = dependencyGroups[dependencyIndex];
			dg.validateMask(dependencyMask);
			fb_assert(!dg.hasAfterTask());

			if (dg.executeCounter == 0 && dg.queueCounter == 0)
			{
				// If there are no tasks in the queue, spawn it instantly.
				// This can easily happen as there is a delay between adding the work tasks, and setting the after dependency task

				addTasksImpNoMutex(dependencyGroupId, taskPriority, task, 1, &optionalData);
				shouldSignalWork = true;
				#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
					fb_assert(!dg.hasAfterTask());
				#endif
			}
			else
			{
				dg.afterTask = task;
				dg.afterTaskData.copyFrom(optionalData);
				dg.afterTaskPriority = taskPriority;

				#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
					fb_assert(dg.hasAfterTask());
				#endif
			}
		}

		if (shouldSignalWork)
			sync.signalWorkForThreads(1);
	}

	// Note: 
	// This form of executeTask() will do mutex _twice_ for each task. 
	// For efficiency, it would best if end bookkeeping lock could also fetch another task if needed/wanted.

	// State was true, but might not be after returning
	uint32_t executeTask(uint32_t threadIndex)
	{
		Task *t = 0;
		uint32_t currentDependency = Scheduler::getInvalidDependencyGroupId();
		uint32_t dependencyIndex = 0;
		uint32_t dependencyMask = 0;

		// Find work -- sync point
		{
			FB_ZONE("executeTask - Find work");
			FB_ZONE_ENTER("SyncGuard g(sync);", profiling::ZoneBlock);
			SyncGuard g(sync);
			FB_ZONE_EXIT();

			t = tasks.popNext();

			// Execution state
			if (t)
			{
				FB_ZONE("Execution state");
				++executeCounter;
				currentDependency = t->dependency;

				// Update state
				if (currentDependency != Scheduler::getInvalidDependencyGroupId())
				{
					dependencyIndex = FB_GROUP_DEPENDENCY_INDEX_GET(currentDependency);
					dependencyMask = FB_GROUP_DEPENDENCY_MASK_GET(currentDependency);
					DependencyGroup &dg = dependencyGroups[dependencyIndex];
					dg.validateMask(dependencyMask);

					--dg.queueCounter;
					++dg.executeCounter;
				}
			}
		}

		// We should only come when we have some work!
		//fb_assert(t);

		// Nothing to do
		if (!t)
			return false;

		FB_ZONE("executeTask");
		profiling::TaskProfiler::taskStarted(t->taskDebugCreationTime, t->taskDebugThreadId, t->dependency);
		uint64_t taskStartTime = profiling::TaskProfiler::getTimeStamp();

		// Call user code to do the actual work
		{
			FB_ZONE(t->task->getStaticTaskNameString());
			fb_assert(self != NULL);
			t->task->run(t->taskData.getTaskData(), *self);
		}

		profiling::TaskProfiler::taskDone(taskStartTime, t->dependency);
		Task::free(t);
		t = nullptr;

		// Bookkeeping -- sync point
		uint32_t taskState = 0;
		bool shouldSignalWork = false;
		{
			FB_ZONE("SyncGuard g(sync);", profiling::ZoneBlock);
			SyncGuard g(sync);

			// Execution state
			--executeCounter;

			if (currentDependency != Scheduler::getInvalidDependencyGroupId())
			{
				DependencyGroup &dg = dependencyGroups[dependencyIndex];
				dg.validateMask(dependencyMask);
				--dg.executeCounter;

				// If we have a after task, add it here before we release the mutex, or possibly wake up the user dependency.
				// Otherwise we'd get a race.
				if (dg.executeCounter == 0 && dg.queueCounter == 0)
				{
					if (dg.hasAfterTask())
					{
						SchedulerTaskData taskData = dg.afterTaskData.getTaskData();
						addTasksImpNoMutex(currentDependency, dg.afterTaskPriority, dg.afterTask, 1, &taskData);
						shouldSignalWork = true;

						dg.afterTask = 0;
						dg.afterTaskData.reset();
						#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
							fb_assert(dg.queueCounter == 1 && dg.executeCounter == 0);
							fb_assert(!dg.hasAfterTask());
						#endif
					}
				}

				if (currentDependency == userBlockingDependency && dg.executeCounter == 0 && dg.queueCounter == 0)
				{
					userBlockingDependency = Scheduler::getInvalidDependencyGroupId();
					sync.resumeUserDependencyBlockThread();
				}
			}

			if (tasks.hasTasks())
				taskState |= TaskActiveStateQueue;
			if (executeCounter > 0)
				taskState |= TaskActiveStateExecution;

			// If we are finished with everything, wake up the user thread
			if (taskState == 0 && userBlocking)
			{
				userBlocking = false;
				sync.resumeUserGeneralBlockThread();
			}
		}

		if (shouldSignalWork)
			sync.signalWorkForThreads(1);

		return taskState;
	}

	bool isCurrentlyWorking() const
	{
		SyncGuard g(sync);
		return tasks.hasTasks() || (executeCounter > 0);
	}

	void waitForDependencies(Scheduler::DependencyGroupId dependencyGroupId)
	{
		// ToDo:
		// Allowing only 1 user thread at the time to wait for dependencies.
		// Easy, but inefficient if multiple users threads would like to wait for different dependencies
		userDependencyMutex.enter();
		
		if (threadAmount > 0)
		{
			// Inform others what we are blocking on
			bool shouldBlock = false;
			{
				SyncGuard g(sync);
				uint32_t originalState = getTaskStateNoMutex(dependencyGroupId);

				// Only block if we have actual tasks running while in this mutex
				// Otherwise there might not be anyone to actually wake us up!
				if (originalState)
				{
					// Only sort if we actually have something to do in the queue
					if (originalState & TaskActiveStateQueue)
					{
						FB_UNUSED_NAMED_VAR(bool, result) = tasks.stableSort(dependencyGroupId);
						
						#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
							// Make sure we are in sync
							fb_assert(result);
						#endif
					}

					#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
						// Make sure we haven't leaked the state earlier
						fb_assert(userBlockingDependency == Scheduler::getInvalidDependencyGroupId());
					#endif

					userBlockingDependency = dependencyGroupId;
					shouldBlock = true;
				}
			}

			if (shouldBlock)
			{
				FB_ZONE("sync.blockUserDependencyBlockThread()", profiling::ZoneBlock);
				uint64_t waitStartedTimeStamp = profiling::TaskProfiler::getTimeStamp();

				// Worker threads are responsible for waking us up
				sync.blockUserDependencyBlockThread();

				#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
				{
					SyncGuard g(sync);
					DependencyGroup &dg = dependencyGroups[FB_GROUP_DEPENDENCY_INDEX_GET(dependencyGroupId)];
					uint32_t state = getTaskStateNoMutex(dependencyGroupId);

					// Make sure the flag was reset (meaning someone actually freed us). If not, we woke up unintentionally.
					fb_assert(userBlockingDependency == Scheduler::getInvalidDependencyGroupId());

					// Technically another user thread could have put a dependency task before we come here, but this is handy development assert anyway
					fb_assert(state == 0);
				}
				#endif


				profiling::TaskProfiler::finishedWaitingForTaskDependency(waitStartedTimeStamp, dependencyGroupId);
			}
		}
		else
		{
			// Enter user block mutex as we need threadIndex 0 for task execution
			// Not the most efficient solution, but no threads is a development feature anyway
			userBlockMutex.enter();

			for (;;)
			{
				uint32_t taskState = executeTask(0);
				if (!taskState)
					break;

				// Above is enough for when no tasks are left, but having no dependency tasks left is enough for us
				// We could return this info from executeTask() with some optional parameters, but mutex efficiency in no-threads (development) case is a bit of a moot point
				{
					SyncGuard g(sync);
					uint32_t dependencyState = getTaskStateNoMutex(dependencyGroupId);
					if (!dependencyState)
						break;
				}
			}

			userBlockMutex.leave();
		}

		userDependencyMutex.leave();
	}

	void blockUntilCompleted()
	{
		// If user is trying to do blocking from multiple threads at the same time, they will block on mutex while the first thread goes idle.
		// Note: From efficiency point of view, it might be best to try a mutex, and if it fails tell that to the caller. 
		// If user is blocking on multiple groups, other ones are most likely free. It would give slightly better task execution efficiency.
		// However, let's not make things more complicated (yet) by optimising for corner cases.
		userBlockMutex.enter();

		if (threadAmount > 0)
		{
			// Inform others that what we are blocking
			{
				SyncGuard g(sync);

				// Don't block if there is no work -- we'd deadlock as no one would come to wake us up!
				if (!tasks.hasTasks())
				{
					userBlockMutex.leave();
					return;
				}

				userBlocking = true;
			}

			// Worker threads are responsible for waking us up
			// Even if they finish and set wake up before we reach the actual block, it is guaranteed to work 
			// (ie, just don't block)
			sync.blockUserGeneralBlockThread();

			#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
			{
				SyncGuard g(sync);
				// Make sure someone woke us up
				fb_assert(userBlocking == false);

				// These asserts are questionable in case another thread just add some tasks
				//fb_assert(executeCounter == 0);
				//userBlocking = false;
			}
			#endif
		}
		else
		{
			// If there are no threads available, we have to do this manually.
			// Use a separate mutex to keep possible other client thread out. If we are supposed to use no threads, then that's what we are going to do.
			// Also, we need this to guarantee unique threadIndex without extra bookkeeping.
			for (;;)
			{
				// Execute a task from the user thread (0)
				uint32_t taskState = executeTask(0);

				// Break if done
				if (!taskState)
					break;
			}
		}

		userBlockMutex.leave();
	}

	void threadEntry(uint32_t threadIndex)
	{
		FB_STACK_CUSTOM("SchedulerWorkerThreadEntry");

		for (;;)
		{
			// Wait for work
			sync.blockThreadUntilMoreWork();

			// Execute a task when we are woken up
			executeTask(threadIndex);

			// Are we dying?
			if (threadQuit)
				break;
		}
	}
};

FB_END_PACKAGE1()

#endif
