#pragma once

#include "fb/lang/platform/GlobalConfig.h"

#ifndef FB_ALLOCATION_PROFILER_ENABLED
#define FB_ALLOCATION_PROFILER_ENABLED FB_FALSE
#endif

FB_DECLARE0(HeapString)
FB_DECLARE_TEMPLATED_STRUCT0(PodVector)

FB_PACKAGE1(profiling)

struct DumpAllocationEvent
{
	const char *name;
	double time;
	uint64_t currentAllocationAmount;
	int32_t allocationDelta;
	uint32_t thread;
	uintptr_t stackTraceHash;
};

struct AllocationProfiler
{
	static uint64_t getCpuTimeStamp();

#if FB_ALLOCATION_PROFILER_ENABLED == FB_TRUE
	static void pushAllocationEvent(uint64_t allocationAmount, const char *staticNamePtr);
	static void pushFreeEvent(uint64_t allocationAmount, const char *staticNamePtr);
	static bool isAllocationProfilerEnabled();
#else
	static FB_FORCEINLINE void pushAllocationEvent(uint64_t, const char *) { }
	static FB_FORCEINLINE void pushFreeEvent(uint64_t, const char *staticNamePtr) { }
	static FB_FORCEINLINE constexpr bool isAllocationProfilerEnabled() { return false; }
#endif

	static void dumpAllocationEvents(uint64_t nowTimestamp, PodVector<DumpAllocationEvent> &dumpResult);
	static void setAllocationProfilerEnabled(bool enabled);
	static void setAllocationProfilerCallstacksEnabled(bool enabled);
	static void clearAllocationProfilerCallstacks();

	static void appendCallstack(HeapString &str, uintptr_t stackTraceHash);
};

FB_END_PACKAGE1();
