#pragma once

#include "fb/profiling/ZoneType.h"

FB_PACKAGE1(profiling)

struct ZoneEvent
{
	ZoneType zoneType;
	uint8_t level;
	const char *id;
	uint64_t timeStart;
	uint64_t timeEnd;
};

FB_END_PACKAGE1();
