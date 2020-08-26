#pragma once

#include "fb/container/PodVector.h"
#include "fb/profiling/ZoneProfilerCommon.h"

FB_PACKAGE1(profiling)

struct ZoneList;

struct ZoneProfilerCallstackDump
{
	struct ZoneCallstackFrame
	{
		double startTime; // Seconds
		DumpZoneType zoneType;
		const char* name;
	};

	typedef PodVector<ZoneCallstackFrame> ResultVector;

	struct Args
	{
		double startTime = 0.0; // How many seconds in the past should callstack be captured from
		double maxDuration = 0.05; // How many seconds long should the callstack at most be
		SizeType maxDepth = 16U;

		uint64_t optionalTimeStamp = 0; // (Optional) Use this time stamp instead of current time, leave as zero to use current time
	};

	static void dumpProfileCallstack(const Args &args, ZoneProfilerCallstackDump::ResultVector& outZones);

	static void dumpProfileCallstackOnThread(const Args &args, SizeType threadIndex, ZoneProfilerCallstackDump::ResultVector& outZones);

	static void dumpProfileCallstackZoneList(const Args &args, const ZoneList &zoneList, PodVector<ZoneCallstackFrame>& outZones);
};

FB_END_PACKAGE1()
