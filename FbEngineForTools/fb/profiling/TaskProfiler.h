#pragma once

FB_DECLARE_TEMPLATED_STRUCT0(PodVector)

FB_PACKAGE1(profiling)

struct TaskCreationDump
{
	double createdTime;
	double startedTime;
	uint32_t creationThread;
	uint32_t runningThread;
	uint32_t dependencyId;
};

struct TaskCompletionDump
{
	double startedTime;
	double finishedTime;
	uint32_t runningThread;
	uint32_t dependencyId;
};

struct TaskWaitingDump
{
	double startedWaitingTime;
	double finishedWaitingTime;
	uint32_t waitingThread;
	uint32_t dependencyId;
};

struct TaskProfiler
{
	static uint64_t getTimeStamp();
	static uint32_t getThreadId();
	static void taskStarted(uint64_t creationTimeStamp, uint32_t creationThread, uint32_t dependencyId);
	static void taskDone(uint64_t startTimeStamp, uint32_t dependencyId);
	static void finishedWaitingForTaskDependency(uint64_t startTimeStamp, uint32_t dependencyId);

	static void dumpTaskCreations(uint64_t nowTimestamp, PodVector<TaskCreationDump> &result);
	static void dumpTaskCompletions(uint64_t nowTimestamp, PodVector<TaskCompletionDump> &result);
	static void dumpTaskWaiting(uint64_t nowTimestamp, PodVector<TaskWaitingDump> &result);

	static void setTaskProfilerEnabled(bool enabled);
	static bool isTaskProfilerEnabled();
};

FB_END_PACKAGE1();
