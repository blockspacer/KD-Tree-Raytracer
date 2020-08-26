#include "Precompiled.h"
#include "ZoneProfiler.h"

#include "fb/algorithm/BinarySearch.h"
#include "fb/container/PodVector.h"
#include "fb/container/Vector.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/Defer.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/Likely.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/thread/Semaphore.h"
#include "fb/lang/time/HighResolutionTime.h"
#include "fb/profiling/ZoneList.h"
#include "fb/profiling/ZoneProfilerCommon.h"
#include "fb/profiling/ZoneProfilerMemoryDump.h"
#include "fb/string/HeapString.h"
#include "fb/string/StringRef.h"

#include <cstdlib>
#include <cstdio>

FB_PACKAGE1(profiling)

#ifndef FB_USE_ZONEPROFILER_TO_PROFILE_STARTUP
	// ZoneProfiler is disabled at startup by default to avoid making large allocations
	// and wasting time freeing those allocations when the options.txt gets loaded.
	// (On editor builds freed buffers are memset to some magic value which can take a while.)
	//
	// To enable profiling early startup times edit this line (be careful not to commit)
	// or add this to your LocalConfig.h
	#define FB_USE_ZONEPROFILER_TO_PROFILE_STARTUP FB_FALSE
#endif

#if FB_USE_ZONEPROFILER_TO_PROFILE_STARTUP == FB_TRUE
static const bool defaultEnableZoneProfiler = true;
#else
static const bool defaultEnableZoneProfiler = false;
#endif

#if (FB_BUILD != FB_FINAL_RELEASE)
static const bool defaultEnableLongHistory = true;
#else
static const bool defaultEnableLongHistory = false;
#endif

bool g_enableZoneProfiler = defaultEnableZoneProfiler;
bool g_enableLongZoneProfilerHistory = defaultEnableLongHistory;

struct ZoneSemaphores
{
	ZoneSemaphores()
		: frozenThreadSemaphore(0)
		, freezeRequesterThreadSemaphore(0)
	{
	}

	// --------------------
	uint8_t pad1[lang::CacheLineAlignment];
	// --------------------
	// -- These are written and read rarely, but still shared between threads.

	lang::AtomicUInt32 frozenThreadCount;
	Semaphore frozenThreadSemaphore;
	Semaphore freezeRequesterThreadSemaphore;

	// --------------------
	uint8_t pad2[lang::CacheLineAlignment];
};
static ZoneSemaphores &getZoneSemaphores()
{
	static ZoneSemaphores zoneSemaphores;
	return zoneSemaphores;
}

struct ZoneCtx;
const ZoneCtx *g_zoneCtxDebugSingleton = nullptr;

struct ZoneCtx
{
	// Maximum _concurrent_ ZoneLists
	static const uint32_t MaxZoneLists = 128;

	ZoneCtx()
	{
		g_zoneCtxDebugSingleton = this;
	}

	// -- These are read on _every_ ZoneProfiler event operation! They need to be read-only
	//    99% of the time to be shared between all cores!

	// Written only when freezing the profiler for dumping data
	// Freezes all ZoneLists where as zl->listFreezeFlag only freezes that list
	lang::AtomicUInt32 freezeFlag;

	// Written only when one of these changes, the value is mirrored below, so that the
	// compiler doesn't even accidentally write these
	// Marked volatile to coerce compiler to check the value before setting it -- avoids having to sync memory between cores
	volatile bool enableZoneProfiler = defaultEnableZoneProfiler; // This setting stored here for more optimal accessing between CPU cores
	volatile bool enableLongHistory = defaultEnableLongHistory; // This setting stored here for more optimal accessing between CPU cores

	// Written only when a new thread is created
	ZoneList *zoneLists[MaxZoneLists] = { nullptr };

	// --------------------
	uint8_t pad1[lang::CacheLineAlignment];
	// --------------------
	// -- Guarded by `listAllocMutex` modified when allocating zones

	Mutex listAllocMutex;
	PodVector<uint32_t> freeZoneListIndices;
	uint32_t numZoneLists = 1;

	// --------------------
	uint8_t pad2[lang::CacheLineAlignment];
	// --------------------
	// -- Only written and read to by the main thread, no limitations

	// Mirrors the state of the above boolean flags.
	// Marked volatile to coerce compiler to check the value before setting it -- avoids having to sync memory between cores
	volatile bool enableZoneProfilerPrevious = defaultEnableZoneProfiler;
	volatile bool enableLongHistoryPrevious = defaultEnableLongHistory;
	PodVector<uint64_t> heartBeats;
	Mutex heartBeatMutex;
};

static ZoneCtx &getZoneCtxImpl()
{
	alignas(64) static ZoneCtx ctx;
	return ctx;
}
FB_FORCEINLINE static ZoneCtx &getZoneCtx()
{
	thread_local ZoneCtx &ctx = getZoneCtxImpl();
	return ctx;
}

bool threadLeakAssert = false;

void disableThreadLeakAssert()
{
	threadLeakAssert = true;
}

static FB_FORCEINLINE uint32_t getZoneThreadId()
{
	return ZoneThreadId::getZoneThreadId();
}

SizeType allocateZoneListIndex()
{
	ZoneCtx &zoneCtx = getZoneCtx();
	MutexGuard mg(zoneCtx.listAllocMutex);

	// First allocate all thread slots consecutively
	uint32_t newIndex = zoneCtx.numZoneLists;
	if (newIndex < ZoneCtx::MaxZoneLists)
	{
		zoneCtx.numZoneLists = newIndex + 1;
		return newIndex;
	}

	// If all are allocated start reusing them in _FIFO_ order,
	// so it's possible to inspect threads that have ended recently.
	if (!zoneCtx.freeZoneListIndices.isEmpty())
	{
		uint32_t index = zoneCtx.freeZoneListIndices[0];
		zoneCtx.freeZoneListIndices.eraseIndex(0);

		// Delete original ZoneList
		delete zoneCtx.zoneLists[index];
		zoneCtx.zoneLists[index] = nullptr;

		return index;
	}

	fb_assertf(threadLeakAssert, "Error: There are %u consecutive threads running in ZoneProfiler, probably leaking threads!", ZoneCtx::MaxZoneLists);
	return 0;
}

void freeZoneListIndex(SizeType index)
{
	if (index == 0)
		return;

	ZoneCtx &zoneCtx = getZoneCtx();
	MutexGuard mg(zoneCtx.listAllocMutex);

	// Don't add thread twice to removed list indices
	for (SizeType i : zoneCtx.freeZoneListIndices)
	{
		if (i == index)
			return;
	}
	zoneCtx.freeZoneListIndices.pushBack(index);
}

static uint64_t &getStartTimeRef()
{
	thread_local uint64_t startTime = 0;
	return startTime;
}

void pushZoneStart()
{
	// Can't do much here as this is called very early in program execution
	// Mutices and allocations would cause an infinite recursion
	static lang::AtomicUInt32 startedFlag;
	uint32_t notStartedVal = 0;
	if (!lang::atomicCompareExchangeWeakAcquireRelease(startedFlag, notStartedVal, 1U))
		return;

	getStartTimeRef() = ZoneTimeStamp::getCpuTimestamp();
}
static ZoneList *&getTemporaryZoneDumpPointer()
{
	thread_local ZoneList *zl = nullptr;
	return zl;
}

static ZoneList *getZoneDump()
{
	ZoneList *&zl = getTemporaryZoneDumpPointer();
	if (!zl)
	{
		thread_local bool recursion = false;
		fb_assert(!recursion);
		recursion = true;

		zl = new ZoneList;
	}

	if (getStartTimeRef() != 0)
	{
		ZoneEnter &enter = zl->enterEvents.pushBack();
		enter.zoneType = ZoneWork;
		enter.id = "Start stamp";
		enter.time = getStartTimeRef();
		getStartTimeRef() = 0;
	}
	return zl;
}

FB_NOINLINE ZoneList *getTemporaryZoneStorage()
{
	// Used to get a zoneList before a thread has properly been initialized

	fb_assert(getZoneThreadId() == 0);

	static lang::AtomicUInt32 isNotFirstEntry;
	thread_local bool beginningThread = false;

	uint32_t zero = 0;
	if (FB_LIKELY(1 == atomicLoadRelaxed(isNotFirstEntry)))
	{
		// nop
	}
	else if (atomicCompareExchangeStrongAcquireRelease(isNotFirstEntry, zero, 1) && !beginningThread)
	{
		// Static initializations in main thread cause us to end up here.
		// Start using the actual zoneList immediately.
		// The zones created by the following allocateZoneListIndex will end up in the zoneDump.
		beginningThread = true;
		beginThread("Static initialization thread");
		uint32_t index = getZoneThreadId();
		if (index != 0)
			return getZoneCtx().zoneLists[index];
		else
			return getZoneDump();
	}

	// Avoids taking a mutex when the thread is freshly created
	// Avoids infine recursion and deadlocking

	ZoneList *zoneDump = getZoneDump();
	{
		// If thread already has many zones, assume the beginThread won't be called manually and do it automatically

		if (zoneDump->finishedEvents.getSize() >= 60 && !beginningThread)
		{
			// If a thread neglects to call beginThread, do it for them
			FB_PRINTF("Thread didn't call fb::profiling::beginThread(...) during the first 100 zones.\n");
			beginningThread = true;
			beginThread("Unnamed thread");
			uint32_t index = getZoneThreadId();
			if (index != 0)
				return getZoneCtx().zoneLists[index];
			fb_assert(false && "Failed to allocate thread. Are there too many threads running?");
		}
	}
	return zoneDump;
}

ZoneList *getZoneList()
{
	uint32_t index = getZoneThreadId();

	if (FB_LIKELY(index != 0))
	{
		return getZoneCtx().zoneLists[index];
	}
	else
	{
		// VERY unlikely path, only on first "get" per thread, preferrably via "beginThread()"

		return getTemporaryZoneStorage();
	}
}

SizeType platformGetMaxEvents()
{
#if FB_USE_ZONE_PROFILER != FB_TRUE
	return 0;
#else
	return getZoneCtx().enableLongHistory ? 3000000U : 500000U;
#endif
}

SizeType platformGetMaxEventsForZoneList(SizeType zoneIndex)
{
	const SizeType workerThreadMaxEventCountDivider = 2;
	return zoneIndex <= 1 ? platformGetMaxEvents() : (platformGetMaxEvents() / workerThreadMaxEventCountDivider);
}

uint32_t createCoZone()
{
	uint32_t index = allocateZoneListIndex();

	ZoneCtx &zoneCtx = getZoneCtx();
	getZoneCtx().zoneLists[index] = new ZoneList();
	if (zoneCtx.enableZoneProfiler)
	{
		// Only allocate the ring buffer if zones are in use
		zoneCtx.zoneLists[index]->finishedEvents.resize(platformGetMaxEventsForZoneList(index));
	}

	return index;
}

ZoneList *getCoZone(uint32_t id)
{
	return getZoneCtx().zoneLists[id];
}

void beginThread(const char *name)
{
#if FB_USE_ZONE_PROFILER == FB_TRUE
	uint32_t index = ZoneThreadId::t_zoneListIndex;
	if (index == 0)
	{
		index = allocateZoneListIndex();
		ZoneThreadId::t_zoneListIndex = index;

		ZoneCtx &zoneCtx = getZoneCtx();
		if (ZoneList *&zoneDumpList = getTemporaryZoneDumpPointer())
		{
			zoneCtx.zoneLists[index] = zoneDumpList; // NOTE: Only the pointer is thread local
			zoneDumpList = nullptr;
		}
		else
		{
			zoneCtx.zoneLists[index] = new ZoneList;
		}

		if (zoneCtx.enableZoneProfiler)
		{
			// Only allocate the ring buffer if zones are in use
			zoneCtx.zoneLists[index]->finishedEvents.resize(platformGetMaxEventsForZoneList(index));
		}

		zoneCtx.zoneLists[index]->begin = ZoneTimeStamp::getCpuTimestamp();
	}
	else
	{
		if (index != 1 || name[0] == 'M')
		{
			FB_PRINTF("beginThread being called more than once for thread %d '%s'\n", index, name);
		}
	}

	ZoneList *zl = getZoneList();
	zl->threadName = StaticString::createFromConstChar(name);
	zl->isCoZone = false;
#endif
}

void endThread()
{
#if FB_USE_ZONE_PROFILER == FB_TRUE
	ZoneList *zl = getZoneList();
	zl->end = ZoneTimeStamp::getCpuTimestamp();

	freeZoneListIndex(ZoneThreadId::t_zoneListIndex);
	ZoneThreadId::t_zoneListIndex = 0;
#endif
}

const char *platformGetOutputFile()
{
	return "profile.json";
}
const char *platformGetOutputFileChromeTracing()
{
	return "profile_chrome_tracing.json";
}

ZoneList *getZoneListExternal(SizeType threadIndex)
{
	ZoneCtx &zoneCtx = getZoneCtx();
	if (threadIndex < ZoneCtx::MaxZoneLists)
		return zoneCtx.zoneLists[threadIndex];

	return nullptr;
}

void freezeZoneList(ZoneList *zl)
{
	zl->freezeEndMutex.enter();
	atomicStoreRelaxed(zl->listFreezeFlag, 1);
	while (atomicLoadRelaxed(zl->isReferenced))
	{
		// Wait furiously
	}
}
void unfreezeZoneList(ZoneList *zl)
{
	atomicStoreRelaxed(zl->listFreezeFlag, 0);
	zl->freezeEndMutex.leave();
}

#if FB_USE_ZONE_PROFILER == FB_TRUE

void acquireGlobalZoneListLock()
{
	// Freeze creating new threads and thus freeze the number of zone lists
	ZoneCtx &zoneCtx = getZoneCtx();
	zoneCtx.listAllocMutex.enter();

	// Start freezing all the threads
	atomicStoreRelease(zoneCtx.freezeFlag, 1);

	ZoneSemaphores &zoneSemaphores = getZoneSemaphores();

	// Spin here until none of the zone lists are referenced anymore
	SizeType numZonesReferenced;
	do
	{
		numZonesReferenced = 0;

		for (SizeType i = 1; i < zoneCtx.numZoneLists; i++)
		{
			if (atomicLoadRelaxed(zoneCtx.zoneLists[i]->isReferenced))
				numZonesReferenced++;
		}

		// Wait for reference updates
		for (SizeType i = 0; i < numZonesReferenced; i++)
			zoneSemaphores.freezeRequesterThreadSemaphore.wait();

	} while (numZonesReferenced > 0);
}

void releaseGlobalZoneListLock()
{
	// Stop freezing all the threads
	ZoneCtx &zoneCtx = getZoneCtx();
	atomicStoreRelease(zoneCtx.freezeFlag, 0);

	ZoneSemaphores &zoneSemaphores = getZoneSemaphores();

	// Release waiting frozen threads
	while (atomicLoadAcquire(zoneSemaphores.frozenThreadCount) > 0)
	{
		zoneSemaphores.frozenThreadSemaphore.post(1);
		atomicDecRelaxed(zoneSemaphores.frozenThreadCount);
	}

	// Finally, allow modifying the list of zone lists itself
	zoneCtx.listAllocMutex.leave();
}

FB_NOINLINE static void pushDebugZones(ZoneList *zl, SizeType zoneListIndex, const char *id, ZoneType zoneType, uint64_t startTime, uint64_t endTime)
{
	fb_assert(zoneListIndex == getZoneThreadId() || atomicLoadRelaxed(zl->listFreezeFlag));
	fb_assert(atomicLoadRelaxed(zl->isReferenced));
	fb_assert(id);

	ZoneEvent &e = zl->finishedEvents.pushBack();
	e.zoneType = zoneType;
	e.level = zl->enterEvents.getSize() < 256 ? (uint8_t)zl->enterEvents.getSize() : (uint8_t)255U;
	e.id = id;
	e.timeStart = startTime;
	e.timeEnd = endTime;
}

FB_NOINLINE static void waitForFrozenThreadSemaphoreToEnd(ZoneCtx &zoneCtx, ZoneList *zl)
{
	atomicStoreRelease(zl->isReferenced, 0);

	ZoneSemaphores &zoneSemaphores = getZoneSemaphores();
	atomicIncRelease(zoneSemaphores.frozenThreadCount);

	// The freezing thread may be waiting for this ZoneList to be unreferenced
	zoneSemaphores.freezeRequesterThreadSemaphore.post(1);

	// Wait that the freeze is over
	zoneSemaphores.frozenThreadSemaphore.wait();
}

void lockZoneList(ZoneList *zl);
FB_NOINLINE static void waitForFreezeToEnd(ZoneList *zl)
{
	atomicStoreRelease(zl->isReferenced, 0);

	uint64_t waitingStarted = ZoneTimeStamp::getCpuTimestamp();
	zl->freezeEndMutex.enter();
	while (atomicLoadAcquire(zl->listFreezeFlag))
	{
		// Shouldn't really ever end up here, freezeEndMutex can only be entered after listFreezeFlag has been unset
		// Wait furiously
	}

	zl->freezeEndMutex.leave();

	uint64_t waitingEnded = ZoneTimeStamp::getCpuTimestamp();

	{
		thread_local SizeType recursionCount = 0;
		if (recursionCount > 16)
		{
			// Zone list is being frozen too agressively or something else has gone horribly wrong
			fb_assertf(false, "ZoneList %d is being frozen too agressively or something else has gone horribly wrong.", getZoneThreadId());
			return;
		}

		++recursionCount;
		lockZoneList(zl); // Let's hope we wont recurse too much... I'm pretty sure it'll be fine!
		--recursionCount;

		if (recursionCount == 0)
		{
			pushDebugZones(zl, getZoneThreadId(), "Zone list frozen", ZoneBlock, waitingStarted, waitingEnded);
		}
	}
}

void lockZoneList(ZoneList *zl)
{
again:
	atomicStoreRelease(zl->isReferenced, 1);

	// VERY unlikely to go in this slow path
	ZoneCtx &zoneCtx = getZoneCtx();
	if (FB_LIKELY(false == atomicLoadAcquire(zoneCtx.freezeFlag) && false == atomicLoadAcquire(zl->listFreezeFlag)))
	{
		// Likely, fast path
		return;
	}
	else if (atomicLoadAcquire(zoneCtx.freezeFlag))
	{
		// Every thread waits for freeze to end
		waitForFrozenThreadSemaphoreToEnd(zoneCtx, zl);
	}
	else
	{
		// Individual threads wait for their corresponding freeze to end
		waitForFreezeToEnd(zl);
	}

	// Technically a tail-call but don't trust the optimizer
	goto again;
}

static void lockZoneListForAppend(ZoneList *zl)
{
	lockZoneList(zl);
}

void unlockZoneList(ZoneList *zl)
{
	atomicStoreRelease(zl->isReferenced, 0);

	// VERY unlikely to go in this slow path
	// The freezing thread may be waiting for this ZoneList to be unreferenced
	ZoneCtx &zoneCtx = getZoneCtx();
	if (FB_LIKELY(false == atomicLoadAcquire(zoneCtx.freezeFlag)))
	{
		// Likely
	}
	else
	{
		ZoneSemaphores &zoneSemaphores = getZoneSemaphores();
		zoneSemaphores.freezeRequesterThreadSemaphore.post(1);
	}
}

FB_FORCEINLINE static void pushZoneImpl(const char *id, ZoneType type)
{
	if (!getZoneCtx().enableZoneProfiler)
		return;

	fb_assert(id);

	ZoneList *zl = getZoneList();
	lockZoneListForAppend(zl);

	ZoneEnter &e = zl->enterEvents.pushBack();
	e.zoneType = type;
	e.id = id;
	e.time = ZoneTimeStamp::getCpuTimestamp();

	unlockZoneList(zl);
}

void pushZone(const string::StringLiteral &name, ZoneType type)
{
	pushZoneImpl(name.getPointer(), type);
}

void pushZone(const StaticString &name, ZoneType type)
{
	pushZoneImpl(name.getPointer(), type);
}

void pushZone(const DynamicString &name, ZoneType type)
{
	if (!getZoneCtx().enableZoneProfiler)
		return;

	name.convertToStatic();
	pushZoneImpl(name.getPointer(), type);
}

void pushZoneTrustMe(const char *name, ZoneType type)
{
	pushZoneImpl(name, type);
}

void pushZone(const char *name, ZoneType type)
{
	pushZoneImpl(name, type);
}

void popZone()
{
	if (!getZoneCtx().enableZoneProfiler)
		return;

	ZoneList *zl = getZoneList();
	lockZoneListForAppend(zl);

	ZoneEnter enter;
	if (FB_LIKELY(!zl->enterEvents.isEmpty()))
	{
		enter = zl->enterEvents.getBack();
		zl->enterEvents.popBack();

		fb_assert(enter.id);
	}
	SizeType level = zl->enterEvents.getSize();

	ZoneEvent &e = zl->finishedEvents.pushBack();
	e.zoneType = enter.id ? enter.zoneType : ZoneWork;
	e.level = level < 256U ? (uint8_t)level : (uint8_t)255U;
	e.id = enter.id ? enter.id : "???";
	e.timeStart = enter.time;
	e.timeEnd = ZoneTimeStamp::getCpuTimestamp();

	fb_assert(e.id);

	unlockZoneList(zl);
}
static SizeType getStringBufferCapacityForThread(SizeType threadIndex)
{
	enum
	{
		MainThreadBufferCapacity = FB_EDITOR_ENABLED == FB_TRUE ? 64 * 1024 * 1024 : 16 * 1024 * 1024,
		WorkerThreadBufferCapacity = FB_EDITOR_ENABLED == FB_TRUE ? 8 * 1024 * 1024 : 1 * 1024 * 1024
	};
	return threadIndex == 1 ? MainThreadBufferCapacity : WorkerThreadBufferCapacity;
}

void heartBeat()
{
	ZoneCtx &zoneCtx = getZoneCtx();
	MutexGuard mg(zoneCtx.heartBeatMutex);
	applySettings();

	static const SizeType maxHeartBeats = 1024 * 16;
	zoneCtx.heartBeats.reserve(maxHeartBeats);
	zoneCtx.heartBeats.pushBack(ZoneTimeStamp::getCpuTimestamp());

	if (zoneCtx.heartBeats.getSize() >= maxHeartBeats)
		zoneCtx.heartBeats.erase(zoneCtx.heartBeats.getBegin(), zoneCtx.heartBeats.getBegin() + maxHeartBeats / 2);

	static const bool zoneProfilerMemoryUsageInspection = false;
	if (zoneProfilerMemoryUsageInspection)
	{
		static uint64_t lastTimeStamp = 0;
		if (ZoneTimeStamp::getCpuTimestamp() > lastTimeStamp)
		{
			lastTimeStamp = ZoneTimeStamp::getCpuTimestamp() + 10000000;
			uint64_t totalMemoryUsed = 0;
			uint64_t stringBufferUsage = 0;
			for (SizeType i = 0; i < zoneCtx.numZoneLists; ++i)
			{
				if (!zoneCtx.zoneLists[i])
					continue;
				totalMemoryUsed += zoneCtx.zoneLists[i]->finishedEvents.getCapacity() * sizeof(ZoneEvent);
				totalMemoryUsed += zoneCtx.zoneLists[i]->syncs.getCapacity() * sizeof(ZoneSync);
				if (zoneCtx.zoneLists[i]->stringRingBuffer.buffer)
				{
					stringBufferUsage += getStringBufferCapacityForThread(i);
				}
			}
			FB_UNUSED_NAMED_VAR(uint64_t, million) = 1000000;
			FB_PRINTF("ZoneProfiler uses %d.%d million bytes of memory, plus the ring buffers use %d.%d million bytes. Note that on non-editor builds the string buffers are a lot smaller.\n",
				totalMemoryUsed / million, totalMemoryUsed % million, stringBufferUsage / million, stringBufferUsage % million);
		}
	}
}
float getLastHeartBeatInterval()
{
	ZoneCtx &zoneCtx = getZoneCtx();
	MutexGuard mg(zoneCtx.heartBeatMutex);
	SizeType count = zoneCtx.heartBeats.getSize();
	if (count >= 2)
	{
		uint64_t diff = zoneCtx.heartBeats[count - 1] - zoneCtx.heartBeats[count - 2];
		return (float)ZoneTimeStamp::convertCpuTimestampToSeconds(diff);
	}
	return 0.0f;
}
void applySettings()
{
	// Apply settings

	bool refreshRingBufferSizes = false;
	ZoneCtx &zoneCtx = getZoneCtx();
	if (zoneCtx.enableZoneProfilerPrevious != g_enableZoneProfiler)
	{
		zoneCtx.enableZoneProfilerPrevious = g_enableZoneProfiler;
		zoneCtx.enableZoneProfiler = g_enableZoneProfiler;
		if (zoneCtx.enableZoneProfiler)
		{
			refreshRingBufferSizes = true;
		}

		if (!zoneCtx.enableZoneProfiler)
		{
			// Release already allocated memory

			acquireGlobalZoneListLock();

			for (SizeType i = 0; i < zoneCtx.numZoneLists; ++i)
			{
				ZoneList *zl = zoneCtx.zoneLists[i];
				if (!zl)
					continue;

				zl->enterEvents.clear();

				// Releases memory and resets read and write heads
				zl->finishedEvents.reset();

				if (zl->stringRingBuffer.buffer)
				{
					lang::freeMemory(zl->stringRingBuffer.buffer);
					zl->stringRingBuffer.buffer = NULL;
					zl->stringRingBuffer.head = 0;
				}
			}

			releaseGlobalZoneListLock();
		}
	}

	if (!zoneCtx.enableZoneProfiler)
		return; // Don't apply long history changes when zone profiler is disabled.

	if (zoneCtx.enableLongHistoryPrevious != g_enableLongZoneProfilerHistory || refreshRingBufferSizes)
	{
		zoneCtx.enableLongHistoryPrevious = g_enableLongZoneProfilerHistory;
		zoneCtx.enableLongHistory = g_enableLongZoneProfilerHistory;

		// Reserve or release memory

		for (SizeType i = 0; i < zoneCtx.numZoneLists; ++i)
		{
			ZoneList *zl = zoneCtx.zoneLists[i];
			if (!zl)
				continue;

			freezeZoneList(zl);
			FB_DEFER(ZoneList *, zl, {
				unfreezeZoneList(zl);
			});


			SizeType newSize = platformGetMaxEventsForZoneList(i);
			if (zl->finishedEvents.vec.getSize() != newSize)
			{
				// Releases previously allocated memory
				zl->finishedEvents.resize(newSize);
			}
		}
	}
}

CoThread createCoThread(const char *name)
{
	uint32_t id = createCoZone();
	ZoneList *zl = getCoZone(id);
	zl->threadName = StaticString::createFromConstChar(name);
	zl->isCoZone = true;

	return id;
}

void destroyCoThread(CoThread thread)
{
	freeZoneListIndex((uint32_t)thread);
}

void syncCoThread(CoThread thread, uint64_t coTime, double coTimeToSeconds)
{
	ZoneList *zl = getCoZone(thread);
	lockZoneList(zl);

	uint64_t cpuTime = getHighResolutionTimeValue();

	double cpuTimeToSeconds = 1.0 / getHighResolutionTimeFrequency();
	zl->coTimeToCpuTime = coTimeToSeconds / cpuTimeToSeconds;

	zl->coTimeOffset = (int64_t)coTime;
	zl->cpuTimeOffset = (int64_t)cpuTime;

	unlockZoneList(zl);
}

uint64_t convertCoTime(CoThread thread, uint64_t coTime)
{
	ZoneList *zl = getCoZone(thread);
	int64_t coTicks = (int64_t)coTime - zl->coTimeOffset;
	int64_t cpuTicks = (int64_t)((double)coTicks * zl->coTimeToCpuTime);
	uint64_t cpuTime = (uint64_t)(cpuTicks + zl->cpuTimeOffset);
	return cpuTime;
}

void pushCoZone(CoThread thread, uint64_t coTime, const char *name, ZoneType type)
{
	ZoneList *zl = getCoZone(thread);
	lockZoneListForAppend(zl);

	int64_t coTicks = (int64_t)coTime - zl->coTimeOffset;
	int64_t cpuTicks = (int64_t)((double)coTicks * zl->coTimeToCpuTime);
	uint64_t cpuTime = (uint64_t)(cpuTicks + zl->cpuTimeOffset);

	ZoneEnter &e = zl->enterEvents.pushBack();
	e.time = cpuTime;
	e.id = name;
	e.zoneType = type;

	unlockZoneList(zl);
}

void popCoZone(CoThread thread, uint64_t coTime)
{
	ZoneList *zl = getCoZone(thread);
	lockZoneListForAppend(zl);

	ZoneEnter enter;
	if (zl->enterEvents.getSize() > 0)
	{
		enter = zl->enterEvents.getBack();
		zl->enterEvents.popBack();
	}

	SizeType level = zl->enterEvents.getSize();
	int64_t coTicks = (int64_t)coTime - zl->coTimeOffset;
	int64_t cpuTicks = (int64_t)((double)coTicks * zl->coTimeToCpuTime);
	uint64_t cpuTime = (uint64_t)(cpuTicks + zl->cpuTimeOffset);

	ZoneEvent &e = zl->finishedEvents.pushBack();
	e.zoneType = enter.zoneType;
	e.id = enter.id;
	e.timeStart = enter.time;
	e.level = level < 256U ? (uint8_t)level : (uint8_t)255U;
	e.timeEnd = cpuTime;

	unlockZoneList(zl);
}

static void copyZoneEvents(uint32_t zi, uint64_t startTime, uint64_t nowTime, PodVector<ZoneEvent> &copiedEvents, PodVector<ZoneEnter> &copiedUnfinishedEvents)
{
	ZoneCtx &zoneCtx = getZoneCtx();
	ZoneList *zlPtr = zoneCtx.zoneLists[zi];

	bool frozen = getZoneThreadId() != zi; // Don't freeze current thread's zone list, this could cause a dead lock if a zone is pushed during the dumping

	uint64_t freezingStart = ZoneTimeStamp::getCpuTimestamp();
	if (frozen)
		freezeZoneList(zlPtr);
	uint64_t freezingEnd = ZoneTimeStamp::getCpuTimestamp();

	FB_DEFER(ZoneList *, zlPtr, bool, frozen, {
		if (frozen)
			unfreezeZoneList(zlPtr);
	});

	const ZoneList &zl = *zlPtr;

	uint64_t lowerBoundStart = ZoneTimeStamp::getCpuTimestamp();
	SizeType startIndex = ZoneRingBound::lower(zl.finishedEvents, startTime);

	uint64_t lowerBoundEnd = ZoneTimeStamp::getCpuTimestamp();
	SizeType endIndex = ZoneRingBound::upper(zl.finishedEvents, startIndex, nowTime);

	uint64_t upperBoundEnd = ZoneTimeStamp::getCpuTimestamp();

	SizeType size = zl.finishedEvents.vec.getSize();

	copiedEvents.clear();
	fb_assertf(copiedEvents.getCapacity() >= copiedEvents.getSize() + zl.finishedEvents.vec.getSize(), "%d >= %d + %d", copiedEvents.getCapacity(), copiedEvents.getSize(), zl.finishedEvents.vec.getSize());

	if (startIndex == endIndex)
	{
		// no zones
	}
	else if (startIndex < endIndex)
	{
		copiedEvents.insertIndex(copiedEvents.getSize(), zl.finishedEvents.vec.getBegin() + startIndex, endIndex - startIndex);
	}
	else
	{
		copiedEvents.insertIndex(copiedEvents.getSize(), zl.finishedEvents.vec.getBegin() + startIndex, size - startIndex);
		copiedEvents.insertIndex(copiedEvents.getSize(), zl.finishedEvents.vec.getBegin(), endIndex);
	}
	copiedUnfinishedEvents = zl.enterEvents;

	uint64_t copyingEnd = ZoneTimeStamp::getCpuTimestamp();

	{
		// HACK: Hackish way to push debug zones without actually calling "lockZoneListForAppend()"
		// Calling "lockZoneList()" for a frozen ZoneList would cause a dead lock
		ZoneList *dumpThreadZoneList = getZoneList();
		fb_assert(!atomicLoadRelaxed(dumpThreadZoneList->isReferenced));
		atomicStoreRelaxed(dumpThreadZoneList->isReferenced, 1);
		pushDebugZones(dumpThreadZoneList, getZoneThreadId(), "freezing ZoneList", ZoneBlock, freezingStart, freezingEnd);
		pushDebugZones(dumpThreadZoneList, getZoneThreadId(), "lower bound", ZoneWork, lowerBoundStart, lowerBoundEnd);
		pushDebugZones(dumpThreadZoneList, getZoneThreadId(), "upper bound", ZoneWork, lowerBoundEnd, upperBoundEnd);
		pushDebugZones(dumpThreadZoneList, getZoneThreadId(), "copying", ZoneWork, upperBoundEnd, copyingEnd);
		atomicStoreRelaxed(dumpThreadZoneList->isReferenced, 0);
	}
}

static bool needToEscapeString(const char *str)
{
	for (SizeType i = 0; i < 512; ++i)
	{
		char c = str[i];
		if (FB_LIKELY(c != '\0' && c != '"' && c != '\\'))
			continue;

		if (FB_LIKELY(c == '\0'))
			return false;

		return true;
	}

	return false;
}

static void escapeString(HeapString &str)
{
	str.replace("\\", "\\\\");
	str.replace("\"", "\\\"");
}

static void dumpProfileDataProfileView(double maxTimeSeconds, const StringRef &filepath)
{
	ZoneCtx &zoneCtx = getZoneCtx();

	uint64_t nowTime = ZoneTimeStamp::getCpuTimestamp();
	uint64_t maxTime = ZoneTimeStamp::convertSecondsToCpuTimestamp(maxTimeSeconds);
	uint64_t startTime = nowTime >= maxTime ? nowTime - maxTime : 0;

	static PodVector<ZoneEvent> copiedEvents;
	copiedEvents.clear();
	copiedEvents.reserve(platformGetMaxEventsForZoneList(1));
	CachePodVector<ZoneEnter, 64> copiedUnfinishedEvents;

	const char *outputFile = filepath.getPointer();

	FILE *file = fopen(outputFile, "wb");
	if (file == NULL)
	{
		return;
	}
	fprintf(file, "{ \"threads\": [");
	bool firstThread = true;
	for (uint32_t zi = 1; zi < zoneCtx.numZoneLists; zi++)
	{
		if (!zoneCtx.zoneLists[zi])
			continue;

		const ZoneList &zl = *zoneCtx.zoneLists[zi];
		copyZoneEvents(zi, startTime, nowTime, copiedEvents, copiedUnfinishedEvents);

		fprintf(file, "%s", firstThread ? "\n" : ",\n");
		firstThread = false;

		fprintf(file, "{\n\"name\": \"%s\",\"events\":[\n", zl.threadName.getPointer());
		bool first = true;

		SizeType startIndex = ZoneRingBound::lower(zl.finishedEvents, startTime);
		FB_UNUSED_NAMED_VAR(SizeType, endIndex) = ZoneRingBound::upper(zl.finishedEvents, startIndex, nowTime);

		fb_assert(startIndex < zl.finishedEvents.vec.getSize());
		fb_assert(endIndex < zl.finishedEvents.vec.getSize());

		for (const ZoneEvent e : copiedEvents)
		{
			uint64_t ts = e.timeStart;

			if (ts < startTime)
				continue;
			if (ts >= nowTime)
				break;

			double startSeconds = ZoneTimeStamp::convertCpuTimestampToSeconds(ts - startTime);
			double endSeconds = ZoneTimeStamp::convertCpuTimestampToSeconds(e.timeEnd - startTime);

			if (!first)
				fprintf(file, ",");

			if (FB_LIKELY(!needToEscapeString(e.id)))
			{
				fprintf(file, "{ \"zone\": %d, \"id\": \"%s\", \"timeStart\": %f, \"timeEnd\": %f, \"level\": %d  }\n", e.zoneType, e.id, startSeconds, endSeconds, e.level);
			}
			else
			{
				TempString str(e.id);
				escapeString(str);
				fprintf(file, "{ \"zone\": %d, \"id\": \"%s\", \"timeStart\": %f, \"timeEnd\": %f, \"level\": %d  }\n", e.zoneType, str.getPointer(), startSeconds, endSeconds, e.level);
			}

			first = false;
		}

		for (SizeType ei = copiedUnfinishedEvents.getSize(); ei-- > 0;)
		{
			const ZoneEnter e = copiedUnfinishedEvents[ei];
			double startSeconds = ZoneTimeStamp::convertCpuTimestampToSeconds(e.time - startTime);
			startSeconds = FB_DMAX(startSeconds, 0);

			if (!first)
				fprintf(file, ",");

			if (FB_LIKELY(!needToEscapeString(e.id)))
			{
				fprintf(file, "{ \"zone\": %d, \"id\": \"%s\", \"timeStart\": %f, \"timeEnd\": %f, \"level\": %d  }\n", e.zoneType, e.id, startSeconds, maxTimeSeconds, ei);
			}
			else
			{
				TempString str(e.id);
				escapeString(str);
				fprintf(file, "{ \"zone\": %d, \"id\": \"%s\", \"timeStart\": %f, \"timeEnd\": %f, \"level\": %d  }\n", e.zoneType, str.getPointer(), startSeconds, maxTimeSeconds, ei);
			}

			first = false;
		}

		fprintf(file, "]}");
	}
	fprintf(file, "] }\n");
	fclose(file);
}

void dumpProfileData(double maxTime, const StringRef &filepath)
{
	acquireGlobalZoneListLock();

	dumpProfileDataProfileView(maxTime, filepath);

	releaseGlobalZoneListLock();
}

void dumpProfileDataMemory(double maxTimeSeconds, uint64_t nowTimestamp, PodVector<ZoneDumpData> &outZones, PodVector<double> &outHeartBeats, PodVector<ZoneThreadData> &outThreads)
{
	uint64_t maxTime = ZoneTimeStamp::convertSecondsToCpuTimestamp(maxTimeSeconds);
	uint64_t startTime = nowTimestamp >= maxTime ? nowTimestamp - maxTime : 0;

	ZoneCtx &zoneCtx = getZoneCtx();

	{
		MutexGuard mg(zoneCtx.heartBeatMutex);
		outHeartBeats.clear();
		PodVector<uint64_t>::ConstIterator it = algorithm::lowerBound(zoneCtx.heartBeats.getBegin(), zoneCtx.heartBeats.getEnd(), startTime);
		SizeType newHeartBeatsCapacity = (SizeType)(zoneCtx.heartBeats.getEnd() - it);
		outHeartBeats.reserve(newHeartBeatsCapacity);

		for (; it != zoneCtx.heartBeats.getEnd(); it++)
		{
			outHeartBeats.pushBack(ZoneTimeStamp::convertCpuTimestampToSeconds(*it));
		}
	}

	MutexGuard ga(zoneCtx.listAllocMutex);

	outZones.clear();
	outThreads.clear();

	static PodVector<ZoneEvent> copiedEvents;
	copiedEvents.clear();
	copiedEvents.reserve(platformGetMaxEventsForZoneList(1));
	CachePodVector<ZoneEnter, 64> copiedUnfinishedEvents;

	uint32_t startZoneIndex = 1;
	outThreads.resize(zoneCtx.numZoneLists - startZoneIndex);
	for (uint32_t zi = startZoneIndex; zi < zoneCtx.numZoneLists; ++zi)
	{
		if (!zoneCtx.zoneLists[zi])
			continue;

		const ZoneList &zl = *zoneCtx.zoneLists[zi];
		ZoneThreadData &threadData = outThreads[zi - 1U]; // ZoneList 0 doesn't exist
		threadData.name = zl.threadName.getPointer();
		threadData.startIndex = outZones.getSize();
		threadData.maxHeight = 0;

		fb_assert(threadData.name != NULL);

		FB_ZONE_ENTER("copyZoneEvents");
		copyZoneEvents(zi, startTime, nowTimestamp, copiedEvents, copiedUnfinishedEvents);
		FB_ZONE_EXIT();

		static const SizeType maxAllocationCount = 1024 * 1024 * 1024 / sizeof(ZoneDumpData);
		SizeType resultCount = outZones.getSize();

		uint64_t dumpLoopStart = ZoneTimeStamp::getCpuTimestamp();
		FB_ZONE_ENTER(zl.threadName);

		for (const ZoneEvent e : copiedEvents)
		{
			// NOTE: convertCpuTimestampToSeconds() works for unsigned values so do the difference in
			// reverse and negate the result.
			double startSeconds = -ZoneTimeStamp::convertCpuTimestampToSeconds(nowTimestamp >= e.timeStart ? nowTimestamp - e.timeStart : 0);
			double endSeconds = -ZoneTimeStamp::convertCpuTimestampToSeconds(nowTimestamp >= e.timeEnd ? nowTimestamp - e.timeEnd : 0);

			fb_assert(e.id);

			if (startSeconds >= 0)
				continue;

			if (threadData.maxHeight <= e.level)
				threadData.maxHeight = e.level + 1U;

			ZoneDumpData &zoneDumpData = outZones.pushBack();
			zoneDumpData.startTime = startSeconds;
			zoneDumpData.endTime = FB_DMIN(endSeconds, 0);
			zoneDumpData.zoneType = e.zoneType == ZoneBlock ? DumpZoneTypeBlock : DumpZoneTypeWork;
			zoneDumpData.name = e.id;
			zoneDumpData.level = e.level;

			if (FB_LIKELY(++resultCount < maxAllocationCount))
			{
				// likely
			}
			else
			{
				break;
			}
		}

		FB_ZONE_EXIT();
		uint64_t dumpLoopEnd = ZoneTimeStamp::getCpuTimestamp();

		FB_ZONE_ENTER("Unfinished events");

		for (SizeType ei = copiedUnfinishedEvents.getSize(); ei-- > 0 && resultCount < maxAllocationCount;)
		{
			const ZoneEnter enter = copiedUnfinishedEvents[ei];
			fb_assert(enter.id);

			if (enter.time > nowTimestamp)
				continue;

			// NOTE: convertCpuTimestampToSeconds() works for unsigned values so do the difference in
			// reverse and negate the result.
			double startSeconds = -ZoneTimeStamp::convertCpuTimestampToSeconds(nowTimestamp >= enter.time ? nowTimestamp - enter.time : 0);

			if (threadData.maxHeight <= ei)
				threadData.maxHeight = ei + 1U;

			ZoneDumpData &zoneDumpData = outZones.pushBack();
			zoneDumpData.level = ei;
			zoneDumpData.name = enter.id;
			zoneDumpData.startTime = FB_DMAX(startSeconds, -maxTimeSeconds);
			zoneDumpData.zoneType = enter.zoneType == ZoneBlock ? DumpZoneTypeBlock : DumpZoneTypeWork;
			zoneDumpData.endTime = 0;
			++resultCount;

			fb_assertf(zoneDumpData.endTime - zoneDumpData.startTime < 1000, "%f - %f == %f", zoneDumpData.startTime, zoneDumpData.endTime, zoneDumpData.startTime - zoneDumpData.endTime);
		}

		FB_ZONE_EXIT();

		if (zi <= 1)
		{
			FB_UNUSED_NAMED_VAR(double, t) = ZoneTimeStamp::convertCpuTimestampToSeconds(dumpLoopEnd - dumpLoopStart);
			FB_PRINTF("Dump took %f ns per zone, or %f ms in total for %dk zones\n", t * 1000 * 1000 * 1000 / copiedEvents.getSize(), t * 1000, copiedEvents.getSize() / 1000);
		}

		threadData.endIndex = outZones.getSize();
		fb_assert(resultCount == outZones.getSize());

		if (resultCount >= maxAllocationCount)
		{
			FB_PRINTF("ZoneDump size exceeds maximum allocation limit. Truncating to %d MB.\n", maxAllocationCount * sizeof(ZoneDumpData) / 1024 / 1024);
			break;
		}
	}
}

const char *pushTempZoneString(const StringRef &str)
{
	return pushTempZoneString(str.getPointer(), str.getLength());
}
const char *pushTempZoneString(const char *str, const SizeType strlen)
{
	if (!getZoneCtx().enableZoneProfiler)
		return "DISABLED";

	const SizeType bufferCapacity = getStringBufferCapacityForThread(getZoneThreadId());

	fb_assert(strlen < bufferCapacity);
	fb_assert(str[strlen] == '\0');

	const SizeType len = strlen + 1;

	ZoneList *zl = getZoneList(); // Thread local

	zl->stringRingBuffer.head += len;
	if (zl->stringRingBuffer.head < bufferCapacity)
	{
		// likely
	}
	else
	{
		zl->stringRingBuffer.head = len; // Just loop over old strings, it's probably fine!
	}

	SizeType start = zl->stringRingBuffer.head - len;

	fb_assert(start <= bufferCapacity - len);

	if (zl->stringRingBuffer.buffer)
	{
		// likely
	}
	else
	{
		zl->stringRingBuffer.buffer = (char *)lang::allocateMemory(bufferCapacity);
	}
	lang::MemCopy::copy(zl->stringRingBuffer.buffer + start, str, len);
	return zl->stringRingBuffer.buffer + start;
}

#else

void pushZone(const char *id, ZoneType type)
{
}
void popZone(const char *id, ZoneType type)
{
}
void heartBeat()
{
}
void dumpProfileData(double maxTime, const StringRef &filepath)
{
}
CoThread createCoThread(const char *name)
{
	return 0;
}
void syncCoThread(CoThread thread, uint64_t coTime)
{
}
void syncCoThread(CoThread thread, uint64_t coTime, uint64_t cpuTime)
{
}
uint64_t convertCoTime(CoThread thread, uint64_t coTime)
{
	return 0;
}
void pushCoZone(CoThread thread, uint64_t coTime, const char *name, ZoneType type)
{
}
void popCoZone(CoThread thread, uint64_t coTime)
{
}
void dumpProfileDataMemory(double maxTime, uint64_t nowTimestamp, PodVector<ZoneDumpData>& outZones, PodVector<double>& outHeartbeats, PodVector<ZoneThreadData>& outThreads)
{
	// Add empty thread data for all of the threads. Used by other profilers.
	// Bit ugly to have this here but it's the easiest way to get a hold of zoneCtx.numZoneLists
	ZoneCtx &zoneCtx = getZoneCtx();
	outThreads.resize(zoneCtx.numZoneLists - 1U);
	for (SizeType i = 0; i < outThreads.getSize(); ++i)
	{
		const ZoneList &zl = *zoneCtx.zoneLists[i + 1U]; // ZoneList 0 doesn't exist
		ZoneThreadData &threadData = outThreads[i];
		threadData.name = zl.threadName.getPointer();
		threadData.startIndex = 0;
		threadData.endIndex = 0;
		threadData.maxHeight = 1;
	}
}
const char *pushTempZoneString(const char *str, const SizeType strlen)
{
	return "";
}
const char *pushTempZoneString(const StringRef &str)
{
	return "";
}
float getLastHeartBeatInterval()
{
	return 0.0f;
}
void applySettings()
{
}

#endif

FB_END_PACKAGE1()
