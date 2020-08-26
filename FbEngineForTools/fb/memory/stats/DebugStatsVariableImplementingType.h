#pragma once

#include "fb/container/ArraySlice.h"

FB_PACKAGE2(memory, stats)

// explicit typedef cannot be used if array debug stats are used. (that would cause static array initialization issues)
//FB_EXPLICIT_TYPEDEF(int, DebugStatsVariableImplementingType);

typedef int64_t DebugStatsVariableImplementingType;
typedef fb::ArraySlice <fb::memory::stats::DebugStatsVariableImplementingType> DebugStatsArrayVariableImplementingType;

FB_END_PACKAGE2()
