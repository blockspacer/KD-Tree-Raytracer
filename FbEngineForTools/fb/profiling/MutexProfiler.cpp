#include "Precompiled.h"
#include "MutexProfiler.h"

#if FB_MUTEX_PROFILER_ENABLED == FB_TRUE

#include "fb/container/PodVector.h"
#include "fb/container/LinearMap.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/hash/Hash.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/PlacementNew.h"
#include "fb/lang/ProgrammerAssertPrinting.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/profiling/StupidHashMap.h"
#include "fb/profiling/StupidRingBuffer.h"
#include "fb/profiling/ZoneProfilerCommon.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"

#define FB_ENABLE_MUTEX_PROFILER_BY_DEFAULT false

FB_PACKAGE1(profiling)

namespace
{
typedef uint8_t ThreadIdType;
struct MutexEvent
{
	const Mutex *mutexPtr = nullptr;
	uint64_t startEntering = 0;
	uint64_t finishEntering = 0;
	uint64_t entered = 0;
	uint64_t left = 0;
	uint64_t overheadEnd = 0;
	ThreadIdType threadId = 0;
};

thread_local MutexEvent openingMutexEvent;
enum
{
	MaxOpenMutices = 64
};
static PodVector<MutexEvent> &getOpenMutexEventStack()
{
	thread_local StaticPodVector<MutexEvent, MaxOpenMutices> openMutexEventStack;
	return openMutexEventStack;
}
thread_local bool ignoreAllMuticesOnThisThread = false;

#if FB_ASSERT_ENABLED == FB_TRUE
#define FB_MUTEX_PROFILER_ASSERT(p_pred)                             \
	do                                                               \
	{                                                                \
		if (FB_LIKELY(!!(p_pred)))                                   \
		{                                                            \
		}                                                            \
		else                                                         \
		{                                                            \
			bool ignoreOriginalValue = ignoreAllMuticesOnThisThread; \
			ignoreAllMuticesOnThisThread = true;                     \
			FB_ASSERT_DEBUG_BREAK();                                 \
			ignoreAllMuticesOnThisThread = ignoreOriginalValue;      \
		}                                                            \
	} while (false)
#else
#define FB_MUTEX_PROFILER_ASSERT(p_pred) \
	do                                   \
	{                                    \
	} while (false)
#endif

static lang::AtomicUInt32 &getEnabledFlag()
{
	struct PaddedFlag
	{
		char padding1[lang::CacheLineAlignment];
		lang::AtomicUInt32 flag;
		char padding2[lang::CacheLineAlignment];

		PaddedFlag() { atomicStoreRelaxed(flag, !!(FB_ENABLE_MUTEX_PROFILER_BY_DEFAULT) ? 1U : 0U); }
	};
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

struct MutexPair
{
	MutexPair()
	{
	}

	MutexPair(const MutexPair &o)
		: firstMutex(o.firstMutex)
		, secondMutex(o.secondMutex)
	{
	}

	MutexPair(const Mutex *firstMutex, const Mutex *secondMutex)
		: firstMutex(firstMutex)
		, secondMutex(secondMutex)
	{
	}

	const Mutex *firstMutex = nullptr;
	const Mutex *secondMutex = nullptr;
};

typedef lang::CallStackCapture<16> StackTrace;
struct MutexProfilerState
{
	enum : SizeType
	{
		RingBufferSize = 256 * 1024,
		HashMapPowerOfTwo = 14,
		MutexPairHashMapPowerOfTwo = 14
	};

	Mutex mutex;
	typedef StupidRingBuffer<MutexEvent, RingBufferSize> RingBufferType;
	RingBufferType mutexEvents;
	MutexEvent mutexEventsValueBuffer[RingBufferType::BufferSize];

	typedef StupidHashMap<StackTrace, HashMapPowerOfTwo> MapType;
	MapType stackMap;
	StackTrace stackMapValueBuffer[MapType::BufferSize];
	MapType::MiniHashType stackMapOccupancyBuffer[MapType::HashBufferSize];

	typedef StupidHashMap<MutexPair, MutexPairHashMapPowerOfTwo> MutexPairMapType;
	MutexPairMapType mutexPairMap;
	MutexPair mutexPairMapValueBuffer[MutexPairMapType::BufferSize];
	MutexPairMapType::MiniHashType mutexPairMapOccupancyBuffer[MutexPairMapType::HashBufferSize];

	LinearMap<const Mutex *, StaticString> mutexNames; // Can't be accessed during normal mutex handling due to allocation

	MutexProfilerState()
		: mutexEvents(mutexEventsValueBuffer)
		, stackMap(stackMapValueBuffer, stackMapOccupancyBuffer)
		, mutexPairMap(mutexPairMapValueBuffer, mutexPairMapOccupancyBuffer)
	{
	}
	~MutexProfilerState()
	{
		// Disable mutex profiler before application exit to avoid profiling when stuff is being removed
		setEnabled(false);
	}
};

static MutexProfilerState &getState()
{
	struct InitState
	{
		MutexProfilerState *state = nullptr;
		InitState()
		{
			void *buffer = lang::osAllocate(sizeof(MutexProfilerState));
			state = new (buffer) MutexProfilerState;
		}

		~InitState()
		{
			delete state;
			state = nullptr;
		}
	};

	static InitState state;
	return *state.state;
}
static uint64_t getTimeStamp()
{
	return ZoneTimeStamp::getCpuTimestamp();
}
static void addPairs(MutexProfilerState::MutexPairMapType &mutexPairMap, const PodVector<MutexEvent> &openEvents, uint32_t threadIndex, const Mutex *mutex)
{
	for (SizeType i = 0; i < openEvents.getSize(); ++i)
	{
		if (openEvents[i].mutexPtr == mutex)
			continue;

		if (openEvents[i].threadId != threadIndex)
			continue;

		MutexPair pair(openEvents[i].mutexPtr, mutex);
		uint64_t hash = getHashValue64(&pair, sizeof(pair), 1337);
		if (mutexPairMap.canSet(hash))
			mutexPairMap.set(hash) = pair;
	}
}
}
static ThreadIdType getThreadId()
{
	const ThreadIdType maxValue = (1llu << (8 * sizeof(ThreadIdType))) - 1;
	uint32_t threadId = ZoneThreadId::getZoneThreadId();
	if (threadId < maxValue)
		return ThreadIdType(threadId);

	return maxValue;
}

void MutexProfiler::startEntering(const Mutex *mutex)
{
	if (!getEnabled())
		return;

	if (ignoreAllMuticesOnThisThread)
		return;

	FB_MUTEX_PROFILER_ASSERT(mutex);

	openingMutexEvent.mutexPtr = mutex;
	openingMutexEvent.startEntering = getTimeStamp();
}
void MutexProfiler::finishEntering(const Mutex *mutex)
{
	if (!getEnabled())
		return;

	if (openingMutexEvent.mutexPtr != mutex)
		return;

	FB_MUTEX_PROFILER_ASSERT(mutex && !ignoreAllMuticesOnThisThread);

	openingMutexEvent.finishEntering = getTimeStamp();
}
void MutexProfiler::entered(const Mutex *mutex)
{
	if (!getEnabled())
		return;

	if (openingMutexEvent.mutexPtr != mutex || ignoreAllMuticesOnThisThread)
		return;

	FB_MUTEX_PROFILER_ASSERT(mutex && !ignoreAllMuticesOnThisThread);

	openingMutexEvent.entered = getTimeStamp();
	openingMutexEvent.threadId = getThreadId();

	PodVector<MutexEvent> &openMutexEventStack = getOpenMutexEventStack();
	FB_MUTEX_PROFILER_ASSERT(openMutexEventStack.getSize() < openMutexEventStack.getCapacity());
	FB_MUTEX_PROFILER_ASSERT(openMutexEventStack.getCapacity() == MaxOpenMutices);

	if (openMutexEventStack.getSize() < openMutexEventStack.getCapacity())
	{
		openMutexEventStack.pushBack(openingMutexEvent);
		FB_MUTEX_PROFILER_ASSERT(openMutexEventStack.getCapacity() == MaxOpenMutices);
	}

	openingMutexEvent = MutexEvent();
}
void MutexProfiler::left(const Mutex *mutex)
{
	if (!getEnabled())
		return;

	if (ignoreAllMuticesOnThisThread)
		return;

	FB_MUTEX_PROFILER_ASSERT(mutex);

	uint64_t timeStamp = getTimeStamp();

	PodVector<MutexEvent> &openMutexEventStack = getOpenMutexEventStack();
	FB_MUTEX_PROFILER_ASSERT(openMutexEventStack.getCapacity() == MaxOpenMutices);

	for (SizeType i = openMutexEventStack.getSize(); i-- > 0;)
	{
		if (openMutexEventStack[i].mutexPtr == mutex)
		{
			MutexEvent mutexEvent = openMutexEventStack[i];
			openMutexEventStack.eraseIndex(i);

			mutexEvent.left = timeStamp;
			FB_MUTEX_PROFILER_ASSERT(mutexEvent.threadId == getThreadId() || mutexEvent.threadId == 0);
			if (FB_LIKELY(mutexEvent.threadId > 0))
			{
				// Likely
			}
			else
			{
				// At the very beginning of thread, ThreadId might not have yet been initialized.
				return;
			}

			ignoreAllMuticesOnThisThread = true;
			MutexProfilerState &state = getState();
			state.mutex.enter();

			if (state.stackMap.canSet(uintptr_t(mutex)))
				DebugHelp::getStackTrace(state.stackMap.set(uintptr_t(mutex)), 2);

			addPairs(state.mutexPairMap, openMutexEventStack, mutexEvent.threadId, mutex);

			mutexEvent.overheadEnd = getTimeStamp();
			state.mutexEvents.pushBack(mutexEvent);
			state.mutex.leave();
			ignoreAllMuticesOnThisThread = false;
			return;
		}
	}
}

void MutexProfiler::dumpMutexEvents(uint64_t nowTimestamp, PodVector<MutexProfilerDumpEvent> &mutexEvents)
{
	{
		// Do reserve outside of the mutex to avoid deadlocking with AllocationProfiler
		SizeType requiredCapacity = mutexEvents.getSize() + MutexProfilerState::RingBufferSize;
		if (mutexEvents.getCapacity() < requiredCapacity)
			mutexEvents.reserve(requiredCapacity);
	}

	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();

	double conversionMult = ZoneTimeStamp::convertCpuTimestampToSeconds(1);
	double currentTime = nowTimestamp * conversionMult;
	for (MutexEvent e : state.mutexEvents)
	{
		if (e.startEntering > nowTimestamp)
			break;

		MutexProfilerDumpEvent &t = mutexEvents.pushBack();
		t.timeStampEnteringStarted = e.startEntering * conversionMult - currentTime;
		t.timeStampEnteringFinished = e.finishEntering * conversionMult - currentTime;
		t.timeStampEntered = e.entered * conversionMult - currentTime;
		t.timeStampLeft = e.left * conversionMult - currentTime;
		t.timeStampOverheadEnd = e.overheadEnd * conversionMult - currentTime;
		t.threadId = e.threadId - 1U; // 0th thread id is not used
		t.mutexPtr = e.mutexPtr;
	}

	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::dumpMutexPairs(PodVector<MutexPairDump> &mutexPairs)
{
	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();

	for (SizeType i = 0; i < MutexProfilerState::MutexPairMapType::BufferSize; ++i)
	{
		if (state.mutexPairMapOccupancyBuffer[i] != 0)
		{
			MutexPairDump &p = mutexPairs.pushBack();
			p.firstMutexPtr = state.mutexPairMapValueBuffer[i].firstMutex;
			p.secondMutexPtr = state.mutexPairMapValueBuffer[i].secondMutex;
		}
	}

	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::clearMutexPairs()
{
	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();
	state.mutexPairMap.clear();
	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::setMutexName(const Mutex *mutexPtr, const char *name)
{
	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();
	state.mutexNames[mutexPtr] = StaticString(name);
	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::appendMutexName(HeapString &result, const Mutex *mutexPtr)
{
	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();

	if (const StaticString *namePtr = state.mutexNames.tryFind(mutexPtr))
		result << *namePtr;
	else
		result << "Unnamed mutex";

	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::appendCallstack(HeapString &result, const Mutex *mutexPtr)
{
	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();

	if (state.stackMap.canGet(uintptr_t(mutexPtr)))
		ProgrammerAssertPrinting::appendStackTraceCleaned(result, state.stackMap.get(uintptr_t(mutexPtr)));

	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::appendSingleRowCallstack(HeapString &result, const Mutex *mutexPtr)
{
	ignoreAllMuticesOnThisThread = true;
	MutexProfilerState &state = getState();
	state.mutex.enter();

	if (state.stackMap.canGet(uintptr_t(mutexPtr)))
		ProgrammerAssertPrinting::appendStackTraceSingleRow(result, state.stackMap.get(uintptr_t(mutexPtr)));

	state.mutex.leave();
	ignoreAllMuticesOnThisThread = false;
}

void MutexProfiler::setMutexProfilerEnabled(bool enabled)
{
	if (getEnabled() != enabled)
	{
		MutexProfilerState &state = getState();
		state.mutex.enter();
		if (!enabled)
		{
			if (getEnabled() != enabled)
			{
				// TODO: Free memory
				state.mutexEvents.clear();
				setEnabled(false);
			}
		}
		else
		{
			setEnabled(true);
		}
		state.mutex.leave();
	}
}
bool MutexProfiler::isMutexProfilerEnabled()
{
	return getEnabled();
}

FB_END_PACKAGE1();

#undef FB_MUTEX_PROFILER_ASSERT

#endif
