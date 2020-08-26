#pragma once

#include "fb/lang/platform/ForceInline.h"
#include "fb/lang/time/HighResolutionTime.h"

FB_DECLARE_STRUCT(profiling, ZoneRing)

FB_PACKAGE1(profiling)

enum DumpZoneType
{
	DumpZoneTypeWork,
	DumpZoneTypeBlock
};

struct ZoneTimeStamp
{
	static uint64_t getCpuTimestamp()
	{
		return getHighResolutionTimeValue();
	}

	static double convertCpuTimestampToSeconds(uint64_t timestamp)
	{
		static const double inverseFrequence = 1.0 / getHighResolutionTimeFrequency();
		return timestamp * inverseFrequence;
	}
	static uint64_t convertSecondsToCpuTimestamp(double seconds)
	{
		static const double freq = double(getHighResolutionTimeFrequency());
		return uint64_t(seconds * freq);
	}
};

struct ZoneRingBound
{
	static SizeType lower(const ZoneRing &ring, uint64_t nowTime);
	static SizeType upper(const ZoneRing &ring, SizeType startIndex, uint64_t nowTime);
};

struct ZoneThreadId
{
	static thread_local uint32_t t_zoneListIndex;
	static FB_FORCEINLINE uint32_t getZoneThreadId()
	{
		return t_zoneListIndex;
	}
};

FB_END_PACKAGE1()
