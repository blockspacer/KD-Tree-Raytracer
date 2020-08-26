#include "Precompiled.h"
#include "AllocationProfiler.h"

#include "fb/container/PodVector.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/Defer.h"
#include "fb/lang/hash/Hash.h"
#include "fb/lang/platform/Likely.h"
#include "fb/lang/ProgrammerAssertPrinting.h"
#include "fb/lang/stacktrace/StackTrace.h"
#include "fb/lang/thread/DataGuard.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/profiling/StupidHashMap.h"
#include "fb/profiling/StupidRingBuffer.h"
#include "fb/profiling/ZoneProfilerCommon.h"

#define FB_ENABLE_ALLOCATION_PROFILER_AT_STARTUP false

#if FB_BUILD == FB_FINAL_RELEASE
#define FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS FB_FALSE
#else
#define FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS FB_TRUE
#endif

FB_PACKAGE1(profiling)

namespace
{
struct PaddedFlag
{
	char padding1[lang::CacheLineAlignment];
	lang::AtomicUInt32 flag;
	char padding2[lang::CacheLineAlignment];

	PaddedFlag()
	{
		if (FB_ENABLE_ALLOCATION_PROFILER_AT_STARTUP)
			atomicStoreRelaxed(flag, 1);
		else
			atomicStoreRelaxed(flag, 0);
	}
};
static lang::AtomicUInt32 &getEnabledFlag()
{
	static PaddedFlag flag;
	return flag.flag;
}
static lang::AtomicUInt32 &getCallstacksEnabledFlag()
{
	static PaddedFlag flag;
	return flag.flag;
}
static void setEnabled(bool enabled)
{
	lang::atomicStoreRelaxed(getEnabledFlag(), enabled ? 1U : 0U);
}
static bool getEnabled()
{
	if (FB_LIKELY(lang::atomicLoadRelaxed(getEnabledFlag()) == 0))
		return false;
	else
		return true;
}
static void setCallstacksEnabled(bool enabled)
{
	lang::atomicStoreRelaxed(getCallstacksEnabledFlag(), enabled ? 1U : 0U);
}
static bool getCallstacksEnabled()
{
	if (FB_LIKELY(lang::atomicLoadRelaxed(getCallstacksEnabledFlag()) == 0))
		return false;
	else
		return true;
}

struct AllocationEvent
{
	const char *name;
	uint64_t time;
	uint64_t allocationAmount;
	int32_t allocationDelta;
	uint32_t threadId;
	uintptr_t stackTraceHash;
};

typedef lang::CallStackCapture<16> StackTrace;
struct AllocationProfilerState
{
	enum : SizeType
	{
		RingBufferSize = 32 * 1024,
		HashMapPowerOfTwo = 16
	};

	typedef StupidRingBuffer<AllocationEvent, RingBufferSize> RingBufferType;
	RingBufferType allocationEvents;
	AllocationEvent allocationEventsValueBuffer[RingBufferType::BufferSize];

#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	typedef StupidHashMap<StackTrace, HashMapPowerOfTwo> MapType;
	MapType stackMap;
	StackTrace stackMapValueBuffer[MapType::BufferSize];
	MapType::MiniHashType stackMapOccupancyBuffer[MapType::HashBufferSize];
#endif

	Mutex mutex;

	AllocationProfilerState()
		: allocationEvents(allocationEventsValueBuffer)
#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
		, stackMap(stackMapValueBuffer, stackMapOccupancyBuffer)
#endif
	{
	}
	~AllocationProfilerState()
	{
		setEnabled(false);
	}
};

static AllocationProfilerState *getState()
{
	static AllocationProfilerState state;
	return &state;
}

static Mutex &getMutex()
{
	return getState()->mutex;
}

static lang::AtomicUInt64 g_currentAllocationAmount;

static FB_FORCEINLINE uint64_t addAllocationAmount(uint64_t val)
{
	return lang::atomicAddRelaxed(g_currentAllocationAmount, val) + val;
}

static FB_FORCEINLINE uint64_t subAllocationAmount(uint64_t val)
{
	return lang::atomicSubRelaxed(g_currentAllocationAmount, val) - val;
}

static FB_FORCEINLINE uint32_t getThreadId()
{
	return ZoneThreadId::getZoneThreadId();
}

static uint64_t getTimeStamp()
{
	return ZoneTimeStamp::getCpuTimestamp();
}

static uintptr_t captureStackTrace(StackTrace &trace)
{
	if (getCallstacksEnabled() && DebugHelp::getStackTrace(trace, 2))
		return getHashValue(trace.getCapturedFrames(), sizeof(trace.getCapturedFrames()[0]) * trace.numCapturedFrames);

	return 0;
}
}

uint64_t AllocationProfiler::getCpuTimeStamp()
{
	return getTimeStamp();
}

#if FB_ALLOCATION_PROFILER_ENABLED == FB_TRUE
void AllocationProfiler::pushAllocationEvent(uint64_t allocationAmount, const char *staticNamePtr)
{
	uint64_t currentAllocationAmount = addAllocationAmount(allocationAmount);

	if (!getEnabled())
		return;

	AllocationEvent e;
	e.name = staticNamePtr;
	e.time = getCpuTimeStamp();
	e.allocationAmount = currentAllocationAmount;
	e.allocationDelta = int32_t(allocationAmount);
	e.threadId = getThreadId();

#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	StackTrace trace;
	e.stackTraceHash = captureStackTrace(trace);
#endif
	
	MutexGuard g(getMutex());
	getState()->allocationEvents.pushBack(e);

#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	static uint64_t stacksAdded = 0;
	if (e.stackTraceHash != 0 && getState()->stackMap.canSet(e.stackTraceHash))
	{
		++stacksAdded;
		getState()->stackMap.set(e.stackTraceHash) = trace;
	}
#endif
}

void AllocationProfiler::pushFreeEvent(uint64_t allocationAmount, const char *staticNamePtr)
{
	uint64_t currentAllocationAmount = subAllocationAmount(allocationAmount);

	if (!getEnabled())
		return;

	AllocationEvent e;
	e.name = staticNamePtr;
	e.time = getCpuTimeStamp();
	e.allocationAmount = currentAllocationAmount;
	e.allocationDelta = -int32_t(allocationAmount);
	e.threadId = getThreadId();

#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	StackTrace trace;
	e.stackTraceHash = captureStackTrace(trace);
#endif

	MutexGuard g(getMutex());
	getState()->allocationEvents.pushBack(e);

#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	static uint64_t stacksAdded = 0;
	if (e.stackTraceHash != 0 && getState()->stackMap.canSet(e.stackTraceHash))
	{
		++stacksAdded;
		getState()->stackMap.set(e.stackTraceHash) = trace;
	}
#endif
}

bool AllocationProfiler::isAllocationProfilerEnabled()
{
	return getEnabled();
}
#endif

void AllocationProfiler::dumpAllocationEvents(uint64_t nowTimestamp, PodVector<DumpAllocationEvent> &dumpResult)
{
	MutexGuard g(getMutex());
	AllocationProfilerState *state = getState();
	double conversionMult = ZoneTimeStamp::convertCpuTimestampToSeconds(1);
	double nowTime = nowTimestamp * conversionMult;
	uint64_t start = nowTimestamp - ZoneTimeStamp::convertSecondsToCpuTimestamp(60);
	for (AllocationEvent e : state->allocationEvents)
	{
		if (e.time > nowTimestamp || e.time < start)
			continue;

		DumpAllocationEvent &t = dumpResult.pushBack();
		t.name = e.name;
		t.time = (e.time) * conversionMult - nowTime;
		t.currentAllocationAmount = e.allocationAmount;
		t.allocationDelta = e.allocationDelta;
		t.thread = e.threadId;
		t.stackTraceHash = e.stackTraceHash;
	}
}

void AllocationProfiler::setAllocationProfilerEnabled(bool enabled)
{
	if (enabled)
	{
		// Make sure the state is created before push is called
		MutexGuard g(getMutex());
		getState();
	}

	setEnabled(enabled);
}

void AllocationProfiler::setAllocationProfilerCallstacksEnabled(bool enabled)
{
	setCallstacksEnabled(enabled);
}

void AllocationProfiler::clearAllocationProfilerCallstacks()
{
#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	MutexGuard g(getMutex());
	AllocationProfilerState &state = *getState();
	lang::MemSet::set(state.stackMapOccupancyBuffer, 0, sizeof(state.stackMapOccupancyBuffer));
#endif
}

void AllocationProfiler::appendCallstack(HeapString &result, uintptr_t stackTraceHash)
{
#if FB_ENABLE_ALLOCATION_PROFILER_CALLSTACKS == FB_TRUE
	const StackTrace *trace = nullptr;

	{
		MutexGuard g(getMutex());
		if (!getState()->stackMap.canGet(stackTraceHash))
			return;

		// Once assigned to stackMap a value can never be changed, moved or erased,
		// so taking a pointer here should be safe. Using a pointer avoids any possibility
		// of an allocation occurring here while AllocationProfiler's mutex is being held.
		trace = &getState()->stackMap.get(stackTraceHash);
	}

	ProgrammerAssertPrinting::appendStackTraceCleaned(result, *trace);
#endif
}

FB_END_PACKAGE1();
