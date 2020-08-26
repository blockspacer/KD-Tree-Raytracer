#pragma once

#include "ISchedulerTask.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/MemoryFunctions.h"

FB_PACKAGE1(task)

class ISchedulerTask;

// Implementation detail of Scheduler.h. Exposed to outside only for testing purposes

struct InternalTaskDataCopy
{
	const void *taskDataType;
	uint32_t taskDataSize;
	char taskData[SchedulerTaskData::MaxDataBytes];

	InternalTaskDataCopy()
	:	taskDataType(0)
	,	taskDataSize(0)
	{
	}

	void copyFrom(const SchedulerTaskData &data)
	{
		if (!data.hasData())
			return;

		const char *dataPointer = 0;
		uint32_t dataSize = 0;
		const void *dataType = 0;
		data.getParam(dataPointer, dataSize, dataType);

		lang::MemCopy::copy(taskData, dataPointer, dataSize);
		taskDataSize = dataSize;
		taskDataType = dataType;
	}

	SchedulerTaskData getTaskData() const
	{
		return SchedulerTaskData((taskDataSize) ? taskData : NULL, taskDataSize, taskDataType);
	}

	void reset()
	{
		taskDataSize = 0;
		taskDataType = 0;
	}
};

struct Task
{
	// Double linked list
	Task *next = nullptr;
	Task *previous = nullptr;

	// Task data
	ISchedulerTask *task = nullptr;
	InternalTaskDataCopy taskData;

	// Dependencies
	Scheduler::DependencyGroupId dependency;

	uint64_t taskDebugCreationTime = 0;
	uint32_t taskDebugThreadId = 0;

	static Task *create()
	{
		void *ptr = lang::allocateFixed(sizeof(Task));
		return new (ptr) Task();
	}

	static void free(Task *ptr)
	{
		if(ptr)
			lang::freeFixed(ptr, sizeof(Task));
	}
};

struct TaskList
{
	Task *first;
	Task *last;
	int taskCounter;

	TaskList()
	:	first(0)
	,	last(0)
	,	taskCounter(0)
	{
		validate();
	}

	void pushFront(Task *t)
	{
		fb_assert(t && t->next == 0 && t->previous == 0);
		++taskCounter;

		t->next = first;
		if (first)
			first->previous = t;
		first = t;
		if (last == 0)
			last = t;

		validate();
	}

	void pushFront(Task *firstTask, Task *lastTask, uint32_t taskAmount)
	{
		fb_assert(firstTask && lastTask && lastTask->next == 0 && firstTask->previous == 0);
		taskCounter += taskAmount;

		lastTask->next = first;
		if(first)
			first->previous = lastTask;
		first = firstTask;
		if (last == nullptr)
			last = lastTask;

		validate();
	}

	void pushAfter(Scheduler::DependencyGroupId dependency, Task *task)
	{
		if (!first) // No tasks, push to front
		{
			pushFront(task);
			return;
		}

		fb_assert(task && task->next == 0 && task->previous == 0);

		Task *t = first;
		while (t)
		{
			if (t->dependency != dependency)
			{
				++taskCounter;

				task->next = t;
				task->previous = t->previous;
				t->previous = task;
				if (task->previous)
					task->previous->next = task;
				else
					first = task;

				validate(dependency);
				return;
			}
			
			t = t->next;
		}

		// No hits, meaning all tasks are given dependency -- push to back
		pushBack(task);
	}

	void pushAfter(Scheduler::DependencyGroupId dependency, Task *firstTask, Task *lastTask, uint32_t taskAmount)
	{
		if (!first) // No tasks, push to front
		{
			pushFront(firstTask, lastTask, taskAmount);
			return;
		}

		fb_assert(firstTask && lastTask && lastTask->next == 0 && firstTask->previous == 0);

		Task *t = first;
		while (t)
		{
			if (t->dependency != dependency)
			{
				taskCounter += taskAmount;

				lastTask->next = t;
				firstTask->previous = t->previous;
				t->previous = lastTask;
				if (firstTask->previous)
					firstTask->previous->next = firstTask;
				else
					first = firstTask;

				validate(dependency);
				return;
			}
			
			t = t->next;
		}

		// No hits, meaning all tasks are given dependency -- push to back
		pushBack(firstTask, lastTask, taskAmount);
	}

	void pushBack(Task *t)
	{
		fb_assert(t && t->next == 0 && t->previous == 0);
		++taskCounter;

		t->previous = last;
		if (last)
			last->next = t;
		last = t;
		if (first == 0)
			first = t;

		validate();
	}

	void pushBack(Task *firstTask, Task *lastTask, uint32_t taskAmount)
	{
		fb_assert(firstTask && lastTask && lastTask->next == 0 && firstTask->previous == 0);
		taskCounter += taskAmount;

		firstTask->previous = last;
		if (last)
			last->next = firstTask;
		last = lastTask;
		if (first == 0)
			first = firstTask;

		validate();
	}

	// Create new task pointers

	/*
	Task *createTask()
	{
		validate();
		//return taskAllocator.getPointer();
	}

	// Delete task (it shouldn't be in the queue)
	void free(Task *t)
	{
		fb_assert(t);
		validate();
		//taskAllocator.freePointer(t);
	}
	*/

	// Remove task from the queue, don't delete it
	void remove(Task *t)
	{
		if (!t)
			return;

		--taskCounter;

		// Node update
		if (t->next)
		{
			fb_assert(t->next->previous == t);
			t->next->previous = t->previous;
		}
		if (t->previous)
		{
			fb_assert(t->previous->next == t);
			t->previous->next = t->next;
		}

		// Tree update
		if (first == t)
			first = t->next;
		if (last == t)
			last = t->previous;

		validate();
	}

	// pop task and remove it from the queue
	Task *popFront()
	{
		Task *t = first;
		remove(t);

		return t;
	}

	// pop task which we should run next
	Task *popNext()
	{
		// Just give the first one
		return popFront();
	}

	// pop task which we should run next
	Task *popNext(Scheduler::DependencyGroupId dependency)
	{
		Task *t = first;
		while (t)
		{
			if (t->dependency == dependency)
			{
				remove(t);
				return t;
			}

			t = t->next;
		}

		// No hits
		return 0;
	}

	uint32_t removeDependencies(Scheduler::DependencyGroupId dependency)
	{
		uint32_t result = 0;

		Task *t = first;
		while (t)
		{
			Task *next = t->next;
			if (t->dependency == dependency)
			{
				remove(t);
				Task::free(t);
				++result;
			}

			t = next;
		}

		return result;
	}

	// Sort based on dependencies
	bool stableSort(Scheduler::DependencyGroupId dependency)
	{
		validate();

		// Create 2 lists
		// One containing dependencies and the other containing all the others

		Task *dependFirst = 0;
		Task *dependLast = 0;
		Task *otherFirst = 0;
		Task *otherLast = 0;
		Task *t = first;
		while (t)
		{
			Task *storedNext = t->next;

			if (t->dependency == dependency)
			{
				if (!dependFirst)
				{
					// Empty list
					dependFirst = t;
					dependLast = t;
					t->previous = 0;
					t->next = 0;
				}
				else
				{
					// Append
					dependLast->next = t;
					t->previous = dependLast;
					dependLast = t;
				}
			}
			else
			{
				if (!otherFirst)
				{
					// Empty list
					otherFirst = t;
					otherLast = t;
					t->previous = 0;
					t->next = 0;
				}
				else
				{
					// Append
					otherLast->next = t;
					t->previous = otherLast;
					otherLast = t;
				}
			}

			t = storedNext;
		}

		// And merge results

		// This is the only case where we need to modify pointers.
		// Being empty, or only having one case means nothing would change anyway
		if (dependFirst && otherFirst)
		{
			dependLast->next = otherFirst;
			otherFirst->previous = dependLast;
			dependFirst->previous = 0;
			otherLast->next = 0;

			first = dependFirst;
			last = otherLast;
		}

		validate(dependency);

		// Since we walk the whole list, let the user know if there actually are any matching tasks
		return dependFirst != 0;
	}

	bool hasTasks() const
	{
		return first != 0;
	}

	void validate(Scheduler::DependencyGroupId dependency = Scheduler::getInvalidDependencyGroupId()) const
	{
		#ifdef FB_SCHEDULER_ENABLE_INTERNAL_ASSERTS
			// Check invariants

			// Make sure linked list is in valid state (next & previous pointers in sync)
			// If asked, make sure dependencies are in the right order
			// .. And that counters are fine

			Task *t = first;
			Task *p = 0;

			int firstDependedIndex = -1;
			int firstOtherIndex = -1;
			int counter = 0;

			while (t)
			{
				fb_assert(t->previous == p);
				fb_assert(!p || (p->next == t));
				
				if (dependency != Scheduler::getInvalidDependencyGroupId())
				{
					if (t->dependency == dependency)
					{
						fb_assert(firstOtherIndex == -1);

						if (firstDependedIndex == -1)
							firstDependedIndex = counter;
					}
					else
					{
						fb_assert(counter > firstDependedIndex);

						if (firstOtherIndex == -1)
							firstOtherIndex = counter;
					}
				}

				p = t;
				t = t->next;
				++counter;
			}

			fb_assert(counter == taskCounter);
			fb_assert((firstOtherIndex == -1 || firstDependedIndex == -1) || (firstOtherIndex > firstDependedIndex));
		#endif
	}
};

FB_END_PACKAGE1()
