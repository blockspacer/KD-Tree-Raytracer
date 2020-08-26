#pragma once

#include "fb/container/PodVector.h"
#include "fb/profiling/ZoneProfilerCommon.h"

FB_PACKAGE1(profiling)

struct ZoneDumpData
{
	double startTime; // Seconds
	double endTime;
	DumpZoneType zoneType;
	const char* name;
	uint32_t level;
};

struct ZoneThreadData
{
	const char *name;
	uint32_t startIndex;
	uint32_t endIndex;
	uint32_t maxHeight;
};

extern void dumpProfileDataMemory(double maxTime, uint64_t nowTimestamp, PodVector<ZoneDumpData>& outZones, PodVector<double>& outHeartbeats, PodVector<ZoneThreadData>& outThreads);

FB_END_PACKAGE1()
