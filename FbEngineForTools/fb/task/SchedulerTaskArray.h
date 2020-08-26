#ifndef FB_TASK_SCHEDULERTASKARRAY_H
#define FB_TASK_SCHEDULERTASKARRAY_H

#include "fb/lang/IntTypes.h"
#include "ISchedulerTask.h"
#include "Scheduler.h"

FB_PACKAGE1(task)

// Scoped helper for sending several instanced tasks to scheduler.
// Grouping increases efficiency due to reduced locking
template<uint32_t CacheSize, typename CacheParamType>
class SchedulerTaskArray
{
	Scheduler *scheduler;
	Scheduler::TaskGroup taskGroup;
	Scheduler::DependencyGroupId dependency;
	Scheduler::TaskPriority priority;
	ISchedulerTask *task;

	CacheParamType dataCache[CacheSize];
	uint32_t dataCacheEntries;

public:
	SchedulerTaskArray(Scheduler *scheduler_, Scheduler::TaskGroup taskGroup_, Scheduler::DependencyGroupId dependency_, Scheduler::TaskPriority priority_, ISchedulerTask *task_)
	:	scheduler(scheduler_)
	,	taskGroup(taskGroup_)
	,	dependency(dependency_)
	,	priority(priority_)
	,	task(task_)
	,	dataCacheEntries(0)
	{
	}

	SchedulerTaskArray(Scheduler *scheduler_, Scheduler::TaskGroup taskGroup_, Scheduler::TaskPriority priority_, ISchedulerTask *task_)
	:	scheduler(scheduler_)
	,	taskGroup(taskGroup_)
	,	dependency(Scheduler::getInvalidDependencyGroupId())
	,	priority(priority_)
	,	task(task_)
	,	dataCacheEntries(0)
	{
	}

	~SchedulerTaskArray()
	{
		if (dataCacheEntries)
			push();
	}

	void push()
	{
		fb_assert(dataCacheEntries <= CacheSize);
		if (dataCacheEntries)
		{
			SchedulerTaskData localCache[CacheSize];
			for (uint32_t i = 0, iend = dataCacheEntries; i < iend; ++i)
				localCache[i] = task::SchedulerTaskData(dataCache[i]);

			scheduler->addTasks(taskGroup, dependency, priority, task, dataCacheEntries, localCache);
			dataCacheEntries = 0;
		}
	}

	void addTask(const CacheParamType &data)
	{
		dataCache[dataCacheEntries++] = data;
		if (dataCacheEntries == CacheSize)
			push();
	}

	CacheParamType popTask() 
	{
		fb_assert(dataCacheEntries > 0);
		--dataCacheEntries;
		return dataCache[dataCacheEntries];
	}

	uint32_t getCacheSize() const { return dataCacheEntries; }
};


FB_END_PACKAGE1()

#endif
