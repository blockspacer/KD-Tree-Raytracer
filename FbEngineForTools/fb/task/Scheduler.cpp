#include "Precompiled.h"
#include "fb/task/Scheduler.h"

#include "fb/lang/IntTypes.h"
#include "fb/task/SchedulerImpGroup.h"
#include "fb/task/SchedulerImpRepeatable.h"
#include "fb/task/SchedulerImpSync.h"
#include "fb/task/SchedulerImpTask.h"
#include "fb/task/SchedulerTaskArray.h"
#include "fb/sys/util/SystemInfoUtil.h"

FB_PACKAGE1(task)

FB_MEGATON_CLASS_IMPL(Scheduler);

FB_STACK_SET_CLASS(Scheduler);

struct SchedulerData
{
	Group criticalGroup;
	Group foregroundGroup;
	Group backgroundGroup;
	Repeatable repeatable;

	SchedulerData(uint32_t criticalThreads, uint32_t foregroundThreads, uint32_t backgroundThreads, bool useRepeatableThread, Scheduler *self)
	:	criticalGroup(criticalThreads, Group::GroupPriorityHigh, self)
	,	foregroundGroup(foregroundThreads, Group::GroupPriorityNormal, self)
	,	backgroundGroup(backgroundThreads, Group::GroupPriorityLow, self)
	,	repeatable(useRepeatableThread, self)
	{
	}

	~SchedulerData()
	{
	}

	Group &getGroup(Scheduler::TaskGroup taskGroup)
	{
		if (taskGroup == Scheduler::TaskGroupCritical)
			return criticalGroup;
		if (taskGroup == Scheduler::TaskGroupForeground)
			return foregroundGroup;
		if (taskGroup == Scheduler::TaskGroupBackground)
			return backgroundGroup;

		fb_assert(0 && !"Invalid taskGroup in scheduler.");
		return foregroundGroup;
	}

	Scheduler::DependencyGroupId createDependencyGroup(Scheduler::TaskGroup taskGroup)
	{
		Group &g = getGroup(taskGroup);
		return g.createDependencyGroup();
	}

	void freeDependencyGroup(Scheduler::TaskGroup taskGroup, Scheduler::DependencyGroupId dependencyGroupId)
	{
		Group &g = getGroup(taskGroup);
		g.freeDependencyGroup(dependencyGroupId);
	}

	void addTasks(Scheduler::TaskGroup taskGroup, Scheduler::DependencyGroupId dependencyGroupId, Scheduler::TaskPriority taskPriority, ISchedulerTask *task, SizeType taskAmount, const SchedulerTaskData *optionalData)
	{
		Group &g = getGroup(taskGroup);
		g.addTasks(dependencyGroupId, taskPriority, task, taskAmount, optionalData);
	}

	Scheduler::RepeatableTaskId addRepeatableTask(Scheduler::TaskGroup taskGroup, ISchedulerTask *task, Scheduler::TaskGranularity taskGranularity, uint32_t granularityParam, const SchedulerTaskData &optionalData)
	{
		return repeatable.addRepeatableTask(taskGroup, task, taskGranularity, granularityParam, optionalData);
	}

	void removeRepeatableTask(Scheduler::RepeatableTaskId repeatableTaskId)
	{
		repeatable.removeRepeatableTask(repeatableTaskId);
	}

	void waitForDependencies(Scheduler::TaskGroup taskGroup, Scheduler::DependencyGroupId dependencyGroupId)
	{
		Group &g = getGroup(taskGroup);
		g.waitForDependencies(dependencyGroupId);
	}

	void blockUntilCompleted(uint32_t taskGroups)
	{
		// Make sure we don't have update thread spamming more tasks
		repeatable.enableTasks(false);

		// If another user thread keeps on adding tasks during the block, this might not catch them properly.
		// You can't really guarantee a state with no tasks left on every taskGroup, as we are checking them serially.
		// At least it was mostly-no-tasks left when we exit :).
		// Another thread adding tasks during blocking is not a well defined situation anyway.

		for (;;)
		{
			// First execute everything that's there
			if (taskGroups & Scheduler::TaskGroupCritical)
				criticalGroup.blockUntilCompleted();
			if (taskGroups & Scheduler::TaskGroupForeground)
				foregroundGroup.blockUntilCompleted();
			if (taskGroups & Scheduler::TaskGroupBackground)
				backgroundGroup.blockUntilCompleted();

			// And make sure it's all done now
			//  -> Lower priority tasks didn't spawn any higher priority tasks

			bool tasksLeft = false;
			if (taskGroups & Scheduler::TaskGroupCritical)
				tasksLeft |= criticalGroup.isCurrentlyWorking();
			if (taskGroups & Scheduler::TaskGroupForeground)
				tasksLeft |= foregroundGroup.isCurrentlyWorking();
			if (taskGroups & Scheduler::TaskGroupBackground)
				tasksLeft |= backgroundGroup.isCurrentlyWorking();

			// All good
			if (!tasksLeft)
				break;
		}

		// And re-enable the update thread
		repeatable.enableTasks(true);
	}

	void completeAllAndStopReceiving(uint32_t taskGroups)
	{
		// Make sure we don't have update thread spamming more tasks
		repeatable.enableTasks(false);

		// If another user thread keeps on adding tasks during the block, this might not catch them properly.
		// You can't really guarantee a state with no tasks left on every taskGroup, as we are checking them serially.
		// At least it was mostly-no-tasks left when we exit :).
		// Another thread adding tasks during blocking is not a well defined situation anyway.

		for (;;)
		{
			// First execute everything that's there
			if (taskGroups & Scheduler::TaskGroupCritical)
				criticalGroup.blockUntilCompleted();
			if (taskGroups & Scheduler::TaskGroupForeground)
				foregroundGroup.blockUntilCompleted();
			if (taskGroups & Scheduler::TaskGroupBackground)
				backgroundGroup.blockUntilCompleted();

			// And make sure it's all done now
			//  -> Lower priority tasks didn't spawn any higher priority tasks

			bool tasksLeft = false;
			if (taskGroups & Scheduler::TaskGroupCritical)
				tasksLeft |= criticalGroup.isCurrentlyWorking();
			if (taskGroups & Scheduler::TaskGroupForeground)
				tasksLeft |= foregroundGroup.isCurrentlyWorking();
			if (taskGroups & Scheduler::TaskGroupBackground)
				tasksLeft |= backgroundGroup.isCurrentlyWorking();

			// All good
			if (!tasksLeft)
				break;
		}
	}

	void updateCall()
	{
		// Make sure we have everything completed from these groups
		blockUntilCompleted(Scheduler::TaskGroupCritical | Scheduler::TaskGroupForeground);

		// Do update after blocking, so that repeatable tasks start working after the big blocking phase
		repeatable.updateCall();
	}
};

Scheduler::Scheduler(uint32_t flags, SizeType overrideProcessorCount)
:	data(NULL)
{
	// Figure out how to initialise our implementation.
	// Let's assume we are running in at least a dual core processor (so we always use at least some threading)

	SizeType systemCores = (flags & InitFlagUseAllLogicalProcessors) == 0 ? sys::util::SystemInfoUtil::getConcurrentThreadCountHint() : sys::util::SystemInfoUtil::getLogicalThreadCount();
	if (overrideProcessorCount > 0)
		systemCores = overrideProcessorCount;
	
	// Let's go crazy -- horribly inefficient for more than debugging
	if (flags & InitFlagMaxThreads)
		systemCores = MaxThreadAmount;

	// Make sure indices always have space for user calling thread index (0)
	if (systemCores >= MaxThreadAmount)
		systemCores = MaxThreadAmount;

	#if FB_EDITOR_ENABLED == FB_FALSE
		systemCores = lang::min(systemCores, 8u);
	#endif

	// Always only one critical thread
	uint32_t criticalThreads = 1;

	// Always at least this amount of these threads
	uint32_t foregroundThreads = 1;
	uint32_t backgroundThreads = 3;

	// Actually, let's do just that
	// If we only have 1 worker thread, it will only run during user blocking. 
	// Then again, not much point having 2 threads fighting for the same core
	if (systemCores > foregroundThreads)
		foregroundThreads = systemCores;

	// One for each core (if we have enough cores)
	if (systemCores > backgroundThreads)
		backgroundThreads = systemCores;

	// Always use a thread for scheduling repeatable tasks
	bool useRepeatableThread = true;

	// Thread override flags for development. Client code must not depend on any actual amount. 
	// If it breaks (more than added latency kind of way), fix your code. Not these flags.
	if (flags & InitFlagDisableCriticalWorkerThreads)
		criticalThreads = 0;
	if (flags & InitFlagDisableForegroundWorkerThreads)
		foregroundThreads = 0;
	if (flags & InitFlagDisableBackgroundWorkerThreads)
		backgroundThreads = 0;

	data = new SchedulerData(criticalThreads, foregroundThreads, backgroundThreads, useRepeatableThread, this);
}

Scheduler::~Scheduler()
{
	completeAllAndStopReceiving(fb::task::Scheduler::TaskGroupAll);
	delete data;
}

uint32_t Scheduler::getWorkerThreadAmount(uint32_t taskGroup) const
{
	uint32_t result = 0;
	if (taskGroup & TaskGroupCritical)
		result += data->criticalGroup.threadAmount;
	if (taskGroup & TaskGroupForeground)
		result += data->foregroundGroup.threadAmount;
	if (taskGroup & TaskGroupBackground)
		result += data->backgroundGroup.threadAmount;

	return result;
}

Scheduler::DependencyGroupId Scheduler::createDependencyGroup(TaskGroup taskGroup)
{
	return data->createDependencyGroup(taskGroup);
}

void Scheduler::freeDependencyGroup(TaskGroup taskGroup, DependencyGroupId dependencyGroupId)
{
	data->freeDependencyGroup(taskGroup, dependencyGroupId);
}

void Scheduler::addTask(TaskGroup taskGroup, TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData)
{
	fb_assert(task);
	data->addTasks(taskGroup, Scheduler::getInvalidDependencyGroupId(), taskPriority, task, 1, &optionalData);
}

void Scheduler::addTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData)
{
	fb_assert(task);
	data->addTasks(taskGroup, dependencyGroupId, taskPriority, task, 1, &optionalData);
}

void Scheduler::addTasks(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, ISchedulerTask *task, SizeType taskAmount, const SchedulerTaskData *optionalDatas)
{
	fb_assert(task && taskAmount);
	fb_assert(taskAmount == 1 || optionalDatas); // It makes no sense to add same task multiple times without any data to go with it
	data->addTasks(taskGroup, dependencyGroupId, taskPriority, task, taskAmount, optionalDatas);
}

void Scheduler::addTask(TaskGroup taskGroup, TaskPriority taskPriority, const char * taskNameString, SchedulerTaskDelegateType schedulerTaskDelegate)
{
	addTasks(taskGroup, Scheduler::getInvalidDependencyGroupId(), taskPriority, &taskNameString, &schedulerTaskDelegate, 1u);
}

void Scheduler::addTasks(TaskGroup taskGroup, TaskPriority taskPriority, const char** taskNameString, SchedulerTaskDelegateType *schedulerTaskDelegates, uint32_t taskAmount)
{
	addTasks(taskGroup, Scheduler::getInvalidDependencyGroupId(), taskPriority, taskNameString, schedulerTaskDelegates, taskAmount);
}

namespace
{
struct DelegateTask : public task::ISchedulerTask
{
	struct Params
	{
		Params() : taskNameString(NULL) { }
		Params(const DelegateCallData& delegateData, const char* taskNameString)
			: delegateData(delegateData)
			, taskNameString(taskNameString)
		{
		}

		DelegateCallData delegateData;
		const char* taskNameString;

		FB_SCHEDULER_TASK_PARAM_TYPE();
	};

	const char *getStaticTaskNameString() const
	{
		return "Scheduler delegate task";
	}

	virtual void run(const task::SchedulerTaskData &data, task::Scheduler &scheduler)
	{
		Params p;
		data.getParam(p);
		Scheduler::SchedulerTaskDelegateType& schedulerDelegate = static_cast<Scheduler::SchedulerTaskDelegateType&>(p.delegateData);
		FB_ZONE(p.taskNameString);
		schedulerDelegate();
	}
};
}

void Scheduler::addTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, const char * taskNameString, SchedulerTaskDelegateType schedulerTaskDelegate)
{
	addTasks(taskGroup, dependencyGroupId, taskPriority, &taskNameString, &schedulerTaskDelegate, 1u);
}

void Scheduler::addTasks(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, const char ** taskNameString, SchedulerTaskDelegateType *schedulerTaskDelegates, uint32_t taskAmount)
{
	static DelegateTask delegateTask;

	// Share client batcher code
	task::SchedulerTaskArray<32, DelegateTask::Params> taskArray(this, taskGroup, dependencyGroupId, taskPriority, &delegateTask);
	for(uint32_t i = 0; i < taskAmount; ++i)
	{
		DelegateTask::Params params(schedulerTaskDelegates[i], taskNameString[i]);
		taskArray.addTask(params);
	}

	taskArray.push();

	/*
	DelegateTask::Params params(schedulerTaskDelegate, taskNameString);
	fb_static_assert(sizeof(DelegateTask::Params) <= SchedulerTaskData::MaxDataBytes);
	addTask(taskGroup, dependencyGroupId, taskPriority, &delegateTask, params);
	*/
}

void Scheduler::setAfterDepedencyTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData)
{
	Group &g = data->getGroup(taskGroup);
	g.setAfterDependencyTask(dependencyGroupId, taskPriority, task, optionalData);
}

void Scheduler::setAfterDepedencyTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, const char * taskNameString, SchedulerTaskDelegateType schedulerTaskDelegate)
{
	static DelegateTask delegateTask;
	DelegateTask::Params params(schedulerTaskDelegate, taskNameString);
	fb_static_assert(sizeof(DelegateTask::Params) <= SchedulerTaskData::MaxDataBytes);
	Group &g = data->getGroup(taskGroup);
	g.setAfterDependencyTask(dependencyGroupId, taskPriority, &delegateTask, params);
}

Scheduler::RepeatableTaskId Scheduler::addRepeatableTask(TaskGroup taskGroup, ISchedulerTask *task, TaskGranularity taskGranularity, uint32_t granularityParam, const SchedulerTaskData &optionalData)
{
	// 0 would run task every single time repeatable scheduler is updated -- making it very, very unpredictable and implementation dependent
	fb_assert(granularityParam > 0);
	fb_assert(task);

	return data->addRepeatableTask(taskGroup, task, taskGranularity, granularityParam, optionalData);
}

void Scheduler::removeRepeatableTask(RepeatableTaskId repeatableTaskId)
{
	data->removeRepeatableTask(repeatableTaskId);
}

void Scheduler::waitForDependencies(TaskGroup taskGroup, DependencyGroupId dependencyGroupId)
{
	FB_STACK_METHOD();
	data->waitForDependencies(taskGroup, dependencyGroupId);
}

void Scheduler::blockUntilCompleted(uint32_t taskGroups)
{
	FB_STACK_METHOD();
	data->blockUntilCompleted(taskGroups);
}

void Scheduler::completeAllAndStopReceiving(uint32_t taskGroups)
{
	FB_STACK_METHOD();
	data->completeAllAndStopReceiving(taskGroups);
}

void Scheduler::updateCall()
{
	data->updateCall();
}

lang::AtomicUInt32 nextSchedulerTypeThingy;
uint32_t SchedulerTaskData::getNextSchedulerTypeThingy()
{
	return (1<<31) | lang::atomicIncRelaxed(nextSchedulerTypeThingy);
}

void Scheduler::suspendAllThreads()
{
	for (SizeType i = 0; i < data->criticalGroup.threadAmount; ++i)
		Thread::suspend(data->criticalGroup.threads[i]);

	for (SizeType i = 0; i < data->foregroundGroup.threadAmount; ++i)
		Thread::suspend(data->foregroundGroup.threads[i]);

	for (SizeType i = 0; i < data->backgroundGroup.threadAmount; ++i)
		Thread::suspend(data->backgroundGroup.threads[i]);

	Thread::suspend(data->repeatable.thread);
}

void Scheduler::resumeAllThreads()
{
	for (SizeType i = 0; i < data->criticalGroup.threadAmount; ++i)
		Thread::resume(data->criticalGroup.threads[i]);

	for (SizeType i = 0; i < data->foregroundGroup.threadAmount; ++i)
		Thread::resume(data->foregroundGroup.threads[i]);

	for (SizeType i = 0; i < data->backgroundGroup.threadAmount; ++i)
		Thread::resume(data->backgroundGroup.threads[i]);

	Thread::resume(data->repeatable.thread);
}

FB_END_PACKAGE1()
