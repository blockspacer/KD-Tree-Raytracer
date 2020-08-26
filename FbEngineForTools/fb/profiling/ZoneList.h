#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/GlobalMemoryOperators.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/profiling/ZoneRing.h"
#include "fb/string/StaticString.h"

FB_PACKAGE1(profiling)

struct ZoneEnter
{
	ZoneType zoneType = ZoneWork;
	const char *id = nullptr;
	uint64_t time = 0;
};

struct ZoneSync
{
	uint64_t coTime;
	uint64_t cpuTime;
	uint32_t numEvents;
};

struct ZoneList
{
	ZoneList()
		: isCoZone(false)
		, begin(0)
		, end(0)
	{
		fb_assert(finishedEvents.vec.getSize() > 0);
	}

	uint8_t cachelinePad0[lang::CacheLineAlignment];

	bool isCoZone;
	uint64_t begin, end;
	StaticString threadName;
	PodVector<ZoneSync> syncs;
	PodVector<ZoneEnter> enterEvents;
	ZoneRing finishedEvents;
	int64_t coTimeOffset;
	int64_t cpuTimeOffset;
	double coTimeToCpuTime = 0.0;

	uint8_t cachelinePad1[lang::CacheLineAlignment];

	lang::AtomicUInt32 isReferenced;

	uint8_t cachelinePad2[lang::CacheLineAlignment];

	lang::AtomicUInt32 listFreezeFlag;

	uint8_t cachelinePad3[lang::CacheLineAlignment];

	Mutex freezeEndMutex;

	uint8_t cachelinePad4[lang::CacheLineAlignment];

	struct StRingBuffer
	{
		char *buffer = NULL;
		SizeType head = 0;
	} stringRingBuffer;


	uint8_t cachelinePad5[lang::CacheLineAlignment];

	// Use lang::osAllocate to avoid recursion at startup
	FB_ADD_GLOBAL_CLASS_MEMORY_OVERLOADS(HeapAllocator)
};

FB_END_PACKAGE1();
