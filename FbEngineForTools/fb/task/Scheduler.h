#pragma once

#include "fb/lang/IntTypes.h"
#include "ISchedulerTask.h"
#include "fb/lang/Const.h"
#include "fb/lang/Delegate.h"
#include "fb/lang/Megaton.h"

FB_PACKAGE1(task)

class SchedulerTaskData;
class ISchedulerTask;
class Scheduler;
struct SchedulerData;

// This is not going to (or even trying to) be the best scheduler ever, but to provide a reasonable task based interface which can be extended and possibly replaced later 
// with a faster (and more complex) solution - without completely breaking the client code.
// Tasks are recommended to be of "reasonable" size, meaning don't try to queue thousands of micro-tasks -- it is not going to be efficient, and scheduler will likely be a bottleneck.
// In other words, don't try to add a task for every single tiny thing you need to do, but rather add tasks where each do several tiny things instead.
// For efficiency, it's highly recommend to pack parameters/runtime data with provided data chunks.
// Avoid spamming multiple tasks which will then first obtain a lock to inspect what they will actually have to do, if anything.

// Note:
// Tasks are not threads, and as such there is a fundamental difference between them. You CAN NOT create tasks which will do a blocking wait while waiting for other tasks to finish.
// Not only is this extremely inefficient (preventing other tasks from running), but there is a good change you will create a deadlock. If the system is using a low amount of threads 
// (say, 1) your blocking task will prevent every other task from running - including the other task you are supposed to be waiting for. 
// For reliable results your tasks simply cannot block while waiting for results. If workload is shared over multiple tasks, have each of them do their work and keep a counter on how many 
// have already run. Once they are all finished, do the work on the last task to finish (or just spawn a another task to do it). You can also add all tasks to a dependency group, and
// wait for the dependency group tasks to finish and then do the final bit of work.
// It is also required that your threading logic will work without ANY worker threads. If worker threads are disabled from the scheduler it will run your tasks as you keep adding them,
// and this doesn't work with any blocking waits between main core and ANY threaded task.

// Note:
// If you need to wait a bunch of tasks to finish, it is extremely important to use dependency groups. Otherwise your only way to sync is to wait until ALL tasks have finished, 
// and that is extremely inefficient as you can never know what else is being executed at the same time. If you are running a small update along with the renderer, you will block the
// main thread for a very long time for no reason. Depency groups avoid these problems, and it will also give a hint to scheduler what are the most important tasks at the moment.
// This hint might or might not affect the actual scheduling order.
// As with any other ways of blocking, it is NOT ALLOWED to wait for dependencies from running tasks. In theory it might be possible, but it would add complexity for little or no gain.
// It is users responsibility to work out their code accordingly.
class Scheduler
{
	SchedulerData *data;
	FB_MEGATON_CLASS_DECL();

public:
	// Absolute maximum amount of threads that scheduler can use.
	// Note: Don't make any assumptions about this value (ie, it being small'ish or whatever), that's why it's intentionally 
	// defined to be relatively large compared to expected value it's probably going to be.
	// Note: These indices are NOT unique between TaskGroup's
	static const int MaxThreadAmount = 128;

	// Initialisation flags
	enum InitFlag
	{
		// Dev flag. Force assumed system core count to MaxThreadAmount. Extremely inefficient, but useful for debugging threading problems.
		InitFlagMaxThreads = 1,

		// Dev flags to disable various threads. All user code _has_to_ work correctly. 
		// If not, it has some implicit assumptions that have to be fixed!
		InitFlagDisableCriticalWorkerThreads = 2,
		InitFlagDisableForegroundWorkerThreads = 4,
		InitFlagDisableBackgroundWorkerThreads = 8,
		InitFlagDisableAllWorkerThreads = InitFlagDisableCriticalWorkerThreads | InitFlagDisableForegroundWorkerThreads | InitFlagDisableBackgroundWorkerThreads,
		InitFlagDisableRepeatableThread = 16,
		InitFlagDisableAllThreads = InitFlagDisableAllWorkerThreads | InitFlagDisableRepeatableThread,
		/* Use all logical processors. By default, spawns threads for physical processor count only. */
		InitFlagUseAllLogicalProcessors = 32
	};

	// Parameter is a bit mask containing a combination of InitFlags. If overrideProcessorCount is larger than zero, 
	// that number is used instead of autodetected one. InitFlagUseAllLogicalProcessors will also be ignored.
	Scheduler(uint32_t flags = 0, SizeType overrideProcessorCount = 0);
	~Scheduler();

	// Supported task groups
	enum TaskGroup
	{
		// Critical tasks are run with higher than normal priority, more or less in the specified order they were added. 
		// Only execution guarantee is that queued tasks are finished before returning from updateCall() (or other ways to explicitly sync, such as dependency groups).
		// This is ment for low-latency, important tasks which don't spend a long time doing their work -- otherwise they'd prevent other critical tasks from running.
		TaskGroupCritical = 1,

		// Foregroup tasks are run with normal priority, more or less in the specified order they were added. 
		// Only execution guarantee is that queued tasks are finished before returning from updateCall() (or other ways to explicitly sync, such as dependency groups).
		TaskGroupForeground = 2,
		// Background tasks are run when time allows, more or less in the specified order they were added. 
		// No guarantees are made about when these tasks will finish, only that eventually they will. Even so, it is adviced that you avoid expensive blocking (such as file IO), 
		// as that will cause big delays to other background tasks. Use async API's whenever possible. It is a good guess that in normal scenario there will always be several background
		// threads running with low priority, even if the system CPU has only 1 or 2 cores. This allows background tasks to execute whenever the system can spare, and thus it is important
		// to avoid excessive blocking -- it will slow down other background tasks which could actually do some work.
		TaskGroupBackground = 4,

		// This is not a valid parameter for addTask() and others which will require a specific group. Only useful for certain functions supporting multiple groups.
		TaskGroupAll = 0xFFFF
	};

	// Supported priorities
	enum TaskPriority
	{
		// Task is pushed (more or less) to the back of the queue. 
		TaskPriorityNormal = 1,
		// Task is pushed (more or less) to the front of the queue, to be executed ASAP.
		TaskPriorityHigh = 2,
	};

	// For repeatable 
	enum TaskGranularity
	{
		// Additional integer specifies granularity in scheduler updates (run every N'th update).
		TaskGranularityUpdateBased = 1,
		// Additional integer specifies granularity in milliseconds (run every N'th millisecond).
		// While units are in milliseconds, in practice the actual timing is certainly not that accurate.
		// Do not base your own timing to these millisecond values, for example! Use a separate timer instead.
		TaskGranularityMillisecondBased = 2
	};

	// ------------------
	// General queries ->
	// ------------------

	// Does not include main thread or repeatable scheduler thread. 
	// Parameter is a bit mask specifying which groups to include.
	// While it's possible to use this for work balancing, it's recommended to create "nicely" sized tasks instead and let scheduler do the rest
	uint32_t getWorkerThreadAmount(uint32_t taskGroup = TaskGroupForeground) const;

	// -----------------------
	// Dependency interface ->
	// -----------------------

	// Note: 
	// DepencyGroupId is TaskGroup specific (to avoid using multiple locks)!
	typedef uint32_t DependencyGroupId;
	FB_CONST_POD(DependencyGroupId, InvalidDependencyGroupId, 0xFFFFFFFF);

	// Note: 
	// While dependency groups are more efficient than full blown blocking on everything, it is still a form of blocking. It will always introduce a bubble for the execution, 
	// and as such it is recommended to not spam a huge amount of dependencies. Instead, use it for making sure a large chunk of work is guaranteed to be finished.
	// It is always more efficient for tasks to spawn more tasks as they need, instead of waiting for explicit completion.

	// Create new dependency group id. You can add tasks to it, and use it to check the state of dependended tasks.
	// Cost of creating a dependency groups is comparable to addTask() to the scheduler (ie, not too much). Still, it's recommended to avoid creating dependency groups on the fly,
	// and use precreated ones instead.
	DependencyGroupId createDependencyGroup(TaskGroup taskGroup);
	// Free up a dependency group. 
	// freeDependencyGroup() will try to remove given dependencyGroupId tasks from task queue instead of actually executing them. 
	// Only guarantee is that no given dependency group tasks will be executing after this call returns. 
	// If you need to make sure that all tasks have actually been executed, you need to explicitly call waitForDependencies() prior to this call.
	// Cost of freeing dependency group is comparable to addTask() +  waitForDependencies() on already running tasks.
	void freeDependencyGroup(TaskGroup taskGroup, DependencyGroupId dependencyGroupId);

	// -----------------
	// Task interface ->
	// -----------------

	// Note: Scheduler does NOT take ownership of the given task pointers. It is caller's responsibility to manage lifetime. 
	// It is possible, for example, to use static instances as task pointers. It is also possible for task to delete itself before returning from run().
	// As there are no guarantees of execution time, it is callers responsibility to make sure tasks have finished running before they are being deleted.
	// Recommended way of doing this is to use depency groups.

	// Note:
	// Tasks are to be executed "at some point". No guarantees are made of when exactly this is, other than what is specified for update/blocking/waiting methods.

	// Optional data stored in SchedulerTaskData must be smaller or equal to SchedulerTaskData::MaxDataBytes. It is recommended to always use static sized data to 
	// guarantee staying within the limits. Optional data will be copied internally, so user can free up the pointer after this call returns.
	// It is possible that the task will actually start running before this call returns.
	void addTask(TaskGroup taskGroup, TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData = SchedulerTaskData());

	// Extended version of addTask which allows specifying a dependency group id.
	void addTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData = SchedulerTaskData());

	// Full blown version which allows adding multiple tasks (same task pointer but different data) with one call 
	void addTasks(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, ISchedulerTask *task, SizeType taskAmount, const SchedulerTaskData *optionalDatas = nullptr);

	/* Delegate version. Can be called without implementing a SchedulerTask */
	typedef Delegate<void(void) > SchedulerTaskDelegateType;
	void addTask(TaskGroup taskGroup, TaskPriority taskPriority, const char* taskNameString, SchedulerTaskDelegateType schedulerTaskDelegate);
	void addTasks(TaskGroup taskGroup, TaskPriority taskPriority, const char** taskNameStrings, SchedulerTaskDelegateType *schedulerTaskDelegates, uint32_t taskAmount);

	/* Extended version which allows specifying a dependency group id */
	void addTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, const char* taskNameString, SchedulerTaskDelegateType schedulerTaskDelegate);
	void addTasks(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, const char** taskNameStrings, SchedulerTaskDelegateType *schedulerTaskDelegates, uint32_t taskAmount);

	// Set task which will be run after all currently existing dependencyGroupId tasks, if any. Only one is supported, as having several would either be ill defined, or inefficient.
	// Therefore, use only for dependency groups you can be sure have none set (and not finished) already. 
	// There is a common pattern which does something like:
	//   spawn N tasks to a dependencyGroup
	//   block until dependency is done
	//   sort or otherwise use the results of spawned tasks
	// It requires either spawned tasks to keep track of execution and spawn sort task manually, or blocking from the main thread and then doing the sorting (or spawning the task). 
	// With this call, final task can be added to the scheduler and it will be automatically added to the queue once prior tasks have completed. Less manual work, and more efficient as well
	// since scheduler can do this pretty much for free (no need for manual syncing, blocking, and/or pointless main thread wake-ups).
	void setAfterDepedencyTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, ISchedulerTask *task, const SchedulerTaskData &optionalData = SchedulerTaskData());

	/* Delegate version. Can be called without implementing a SchedulerTask */
	void setAfterDepedencyTask(TaskGroup taskGroup, DependencyGroupId dependencyGroupId, TaskPriority taskPriority, const char * taskNameString, SchedulerTaskDelegateType schedulerTaskDelegate);

	// -------------------
	// Repeatable tasks ->
	// -------------------

	typedef uint32_t RepeatableTaskId;
	FB_CONST_POD(RepeatableTaskId, InvalidRepeatableTaskID, 0xFFFFFFFF);

	// Note:
	// Repeatable tasks work by scheduler automagically adding given task to execute queue at specified intervals. This does not mean that that will be executed instantly, 
	// as it will be run just like any other task added to the queue "at some point". However, repeatable tasks are always pushed to the front of the task queue for lower latency.
	// System will automatically prevent queuing the same task multiple times -- if it is already queued/being executed, it will not be added there again to avoid choking the scheduler.

	// User must store returned RepeatableTaskId, as this is the only way to stop repeating task from running.
	// It is possible that the task will actually start running for the first time before this call returns.
	RepeatableTaskId addRepeatableTask(TaskGroup taskGroup, ISchedulerTask *task, TaskGranularity taskGranularity, uint32_t granularityParam, const SchedulerTaskData &optionalData = SchedulerTaskData());

	// Remove repeatable task from execution. 
	// It is possible that repeatable task will still be executed during this call, before scheduler was able to prevent that. 
	// It is only guaranteed that no given repeatable tasks will be executed after this call returns.
	void removeRepeatableTask(RepeatableTaskId repeatableTaskId);

	// -----------------------------
	// Waiting/blocking interface ->
	// -----------------------------

	// Note: 
	// While all scheduler calls are guaranteed to be thread safe, syncing is not efficient if done simultaneously from several user threads. 
	// Only one user thread has access to waitForDependencies() at any given time (per taskGroup), and others will be blocked while waiting.
	// Same applies to blockUntilCompleted().

	// Wait for all dependencies with given id to be completed. This is highly recommended compared to blockUntilCompleted(), as it will only block a specific dependency group.
	// Best possible effort is done to prioritise given dependency group tasks over all other tasks.
	// Note: While no tasks in given group/dependecy were queued/running when call decided to return, 
	//       nothing prevents scheduler/other threads adding more tasks before the actual control resumes to the caller.
	void waitForDependencies(TaskGroup taskGroup, DependencyGroupId dependencyGroupId); 

	// Block calling thread until all tasks are completed. It will also execute tasks spawned by run tasks, but while blocking no repeatable tasks will be executed.
	// It is EXTREMELY inefficient to call this often, as it will wait for every single task to be completed.
	// It is HIGHLY recommended to only block on specific dependency group instead.
	// Parameter is a bit mask specifying which task groups to include.
	// Note: While no tasks were queued/running when call decided to return, 
	//       nothing prevents scheduler/other threads adding more tasks before the actual control resumes to the caller.
	void blockUntilCompleted(uint32_t taskGroups = TaskGroupForeground);

	void completeAllAndStopReceiving(uint32_t taskGroups = TaskGroupAll);

	// -------------------
	// Update interface ->
	// -------------------

	// Update function to be called once per desired granularity (frame / gametick or somesuch).
	// Along with normal update, it does internal sync comparable to blockUntilCompleted(TaskGroupCritical | TaskGroupForeground)
	// Note: While no tasks were queued/running when method decided to return, 
	//       nothing prevents scheduler/other threads adding more tasks before the actual control resumes to the caller.
	// This is potentially very expensive, and in game context you most likely will want to do this just before updating input for the next frame.
	void updateCall();

	/* For debugging. Do not use for production code */
	void suspendAllThreads();
	void resumeAllThreads();
};

FB_END_PACKAGE1()

FB_MEGATON_GLOBAL_GETTER(task, Scheduler)
