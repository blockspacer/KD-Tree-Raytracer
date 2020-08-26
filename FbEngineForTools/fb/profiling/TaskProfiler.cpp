#include "Precompiled.h"
#include "TaskProfiler.h"

#include "fb/container/PodVector.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/thread/DataGuard.h"
#include "fb/profiling/StupidRingBuffer.h"
#include "fb/profiling/ZoneProfiler.h" // For profiling :D
#include "fb/profiling/ZoneProfilerCommon.h"

#if FB_BUILD != FB_FINAL_RELEASE
#define FB_ENABLE_TASK_PROFILER_BY_DEFAULT true
#else
#define FB_ENABLE_TASK_PROFILER_BY_DEFAULT false
#endif

FB_PACKAGE1(profiling)

namespace
{
struct TaskCreation
{
	uint64_t createdTime;
	uint64_t startedTime;
	uint32_t creationThreadId;
	uint32_t startThreadId;
	uint32_t dependencyId;
};
struct TaskCompletion
{
	uint64_t startTime;
	uint64_t endTime;
	uint32_t threadId;
	uint32_t dependencyId;
};
struct TaskDependencyWait
{
	uint64_t startTime;
	uint64_t endTime;
	uint32_t threadId;
	uint32_t dependencyId;
};

struct TaskProfilerState
{
	enum : SizeType
	{
		BufferSize = 64 * 1024
	};

	struct BufferHolder
	{
		TaskCreation a[BufferSize];
		TaskCompletion b[BufferSize];
		TaskDependencyWait c[BufferSize];
	};

	BufferHolder *bufferHolder = nullptr;
	StupidRingBuffer<TaskCreation, BufferSize> taskCreations;
	StupidRingBuffer<TaskCompletion, BufferSize> taskCompletions;
	StupidRingBuffer<TaskDependencyWait, BufferSize> taskDependencies;


	TaskCreation bufferA[BufferSize];
	TaskCompletion bufferB[BufferSize];
	TaskDependencyWait bufferC[BufferSize];

	TaskProfilerState()
		: bufferHolder(new BufferHolder)
		, taskCreations(bufferHolder->a)
		, taskCompletions(bufferHolder->b)
		, taskDependencies(bufferHolder->c)
	{
	}
};

static ScopedRef<TaskProfilerState> getState()
{
	static DataGuard<TaskProfilerState> state("TaskProfilerState DataGuard");
	return state;
}

static bool &enabledBool()
{
	struct PaddedBool
	{
		char padding1[lang::CacheLineAlignment];
		bool enabled = FB_ENABLE_TASK_PROFILER_BY_DEFAULT;
		char padding2[lang::CacheLineAlignment];
	};
	static PaddedBool state;
	return state.enabled;
}
}


void TaskProfiler::taskStarted(uint64_t creationTimeStamp, uint32_t creationThread, uint32_t dependencyId)
{
	if (!enabledBool())
		return;

	FB_ZONE("TaskProfiler::taskStarted");
	TaskCreation t; 
	t.createdTime = creationTimeStamp;
	t.creationThreadId = creationThread;
	t.startedTime = TaskProfiler::getTimeStamp();
	t.startThreadId = TaskProfiler::getThreadId();
	t.dependencyId = dependencyId;
	getState()->taskCreations.pushBack(t);
}
void TaskProfiler::taskDone(uint64_t startTimeStamp, uint32_t dependencyId)
{
	if (!enabledBool())
		return;

	FB_ZONE("TaskProfiler::taskDone");
	TaskCompletion t;
	t.startTime = startTimeStamp;
	t.endTime = TaskProfiler::getTimeStamp();
	t.threadId = TaskProfiler::getThreadId();
	t.dependencyId = dependencyId;
	getState()->taskCompletions.pushBack(t);
}
void TaskProfiler::finishedWaitingForTaskDependency(uint64_t startTimeStamp, uint32_t dependencyId)
{
	if (!enabledBool())
		return;

	FB_ZONE("TaskProfiler::finishedWaitingForTaskDependency");
	TaskDependencyWait t;
	t.startTime = startTimeStamp;
	t.endTime = TaskProfiler::getTimeStamp();
	t.threadId = TaskProfiler::getThreadId();
	t.dependencyId = dependencyId;
	getState()->taskDependencies.pushBack(t);
}

uint64_t TaskProfiler::getTimeStamp()
{
	if (!enabledBool())
		return 0;

	return ZoneTimeStamp::getCpuTimestamp();
}
uint32_t TaskProfiler::getThreadId()
{
	if (!enabledBool())
		return 1;

	return ZoneThreadId::getZoneThreadId();
}

void TaskProfiler::dumpTaskCreations(uint64_t nowTimestamp, PodVector<TaskCreationDump> &result)
{
	FB_ZONE("TaskProfiler::dumpTaskCreations");
	ScopedRef<TaskProfilerState> stateRef = getState();
	TaskProfilerState &state = stateRef.getRawRef();
	result.reserve(result.getSize() + state.taskCreations.getSize());

	double conversionMult = ZoneTimeStamp::convertCpuTimestampToSeconds(1);
	double currentTime = nowTimestamp * conversionMult;
	for (TaskCreation tc : state.taskCreations)
	{
		if (tc.startedTime > nowTimestamp)
			break;

		TaskCreationDump &t = result.pushBack();
		t.createdTime = tc.createdTime * conversionMult - currentTime;
		t.startedTime = tc.startedTime * conversionMult - currentTime;
		t.creationThread = tc.creationThreadId - 1U; // 0th thread id is not used
		t.runningThread = tc.startThreadId - 1U; // 0th thread id is not used
		t.dependencyId = tc.dependencyId;
	}
}
void TaskProfiler::dumpTaskCompletions(uint64_t nowTimestamp, PodVector<TaskCompletionDump> &result)
{
	FB_ZONE("TaskProfiler::dumpTaskCompletions");
	ScopedRef<TaskProfilerState> stateRef = getState();
	TaskProfilerState &state = stateRef.getRawRef();
	result.reserve(result.getSize() + state.taskCompletions.getSize());

	double conversionMult = ZoneTimeStamp::convertCpuTimestampToSeconds(1);
	double currentTime = nowTimestamp * conversionMult;
	for (TaskCompletion tc : state.taskCompletions)
	{
		if (tc.endTime > nowTimestamp)
			break;

		TaskCompletionDump &t = result.pushBack();
		t.startedTime = tc.startTime * conversionMult - currentTime;
		t.finishedTime = tc.endTime * conversionMult - currentTime;
		t.runningThread = tc.threadId - 1U; // 0th thread id is not used
		t.dependencyId = tc.dependencyId;
	}
}
void TaskProfiler::dumpTaskWaiting(uint64_t nowTimestamp, PodVector<TaskWaitingDump> &result)
{
	FB_ZONE("TaskProfiler::dumpTaskCompletions");
	ScopedRef<TaskProfilerState> stateRef = getState();
	TaskProfilerState &state = stateRef.getRawRef();
	result.reserve(result.getSize() + state.taskDependencies.getSize());

	double conversionMult = ZoneTimeStamp::convertCpuTimestampToSeconds(1);
	double currentTime = nowTimestamp * conversionMult;
	for (TaskDependencyWait tc : state.taskDependencies)
	{
		if (tc.endTime > nowTimestamp)
			break;

		TaskWaitingDump &t = result.pushBack();
		t.startedWaitingTime = tc.startTime * conversionMult - currentTime;
		t.finishedWaitingTime = tc.endTime * conversionMult - currentTime;
		t.waitingThread = tc.threadId - 1U; // 0th thread id is not used
		t.dependencyId = tc.dependencyId;
	}
}

void TaskProfiler::setTaskProfilerEnabled(bool enabled)
{
	if (enabledBool() != enabled)
	{
		ScopedRef<TaskProfilerState> stateRef = getState();
		if (!enabled)
		{
			if (enabledBool() != enabled)
			{
				// TODO: Free memory
				stateRef->taskCreations.clear();
				stateRef->taskCompletions.clear();
				stateRef->taskDependencies.clear();
				enabledBool() = false;
			}
		}
		else
		{
			enabledBool() = true;
		}
	}
}
bool TaskProfiler::isTaskProfilerEnabled()
{
	// Not thread-safe but it's just a bool. Gotta go fast.
	return enabledBool();
}

FB_END_PACKAGE1();
