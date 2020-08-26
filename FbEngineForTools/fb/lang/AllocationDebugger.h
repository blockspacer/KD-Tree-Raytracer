#pragma once

#include "fb/lang/Types.h"
#include "fb/lang/MemoryOperatorsConfig.h"

FB_DECLARE(lang, IAllocationDebuggerDump)

FB_PACKAGE1(lang)

class AllocationDebugger
{
	AllocationDebugger();
	~AllocationDebugger();

public:
	/* Pointer is expected to be the raw pointer with totalNumExtraBytes allocated. Size is expected to be the
	 * requested allocation size */
	void addAllocation(void *pointer, size_t size);
	void removeAllocation(void *pointer);
	void modifyAllocation(void *oldPointer, void *newPointer, size_t newSize);
	IAllocationDebuggerDump &getDump();
	uint64_t getTimeStamp() const;

	/* Amount of bytes dedicated to things. These should be easy to change, if we need to due to alignment requirements
	 * etc.. Current values add 16 or 32 bytes in front of returned pointer, so probably OK for most platforms */
	enum DebuggerConstants : unsigned
	{
		/* Allocation debug info */
#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
		numAllocationTrackerBytes = 16,
	#if FB_ENABLE_NAZI_ALLOCATOR == FB_TRUE
		numForwardSafeZoneBlocks = 2,
		numAfterSafeZoneBlocks = 1,
	#else
		numForwardSafeZoneBlocks = 0,
		numAfterSafeZoneBlocks = 0,
	#endif
#elif FB_ENABLE_ALLOCATION_DEBUGGER == FB_FALSE
		numForwardSafeZoneBlocks = 0,
		numAfterSafeZoneBlocks = 0,
		numAllocationTrackerBytes = 0,
#else
#error FB_ENABLE_ALLOCATION_DEBUGGER undefined or invalid
#endif
		numForwardSafeZoneBytes = sizeof(uint64_t) * numForwardSafeZoneBlocks,
		numAfterSafeZoneBytes = sizeof(uint64_t) * numAfterSafeZoneBlocks,
		totalNumExtraBytes = numAllocationTrackerBytes + numForwardSafeZoneBytes + numAfterSafeZoneBytes,
		totalNumPrefixBytes = numAllocationTrackerBytes + numForwardSafeZoneBytes
	};

	static AllocationDebugger &getInstance();

	class Impl;

private:
	Impl &impl;
};


FB_END_PACKAGE1()
