#include "Precompiled.h"
#include "MemoryFunctions.h"

#include "MemoryOperatorsConfig.h"
#include "AlignmentFunctions.h"
#include "AllocationDebugger.h"
#include "Atomics.h"
#include "FiendishAllocator.h"
#include "GlobalFixedAllocateFunctions.h"
#include "MemoryStats.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/IsSame.h"

#include <cstring>
#include <stdlib.h>

// These fb_static_asserts are here to avoid having to include std::size_t separately
fb_static_assertf((sizeof(fb::size_t) == sizeof(std::size_t)), "fb/lang/Types.h's fb::size_t and std::size_t do not match");
fb_static_assertf((fb::lang::IsSame<fb::size_t, std::size_t>::value), "fb/lang/Types.h's fb::size_t and std::size_t do not match");

FB_PACKAGE1(lang)

/* Wrapper has 16 bytes both before and after allocation. Tested for out of bounds writes on deallocation.
 * Deallocation also overwrites free'd memory to help detecting pointers still being used.
 * Nazi Allocator does the same (and some more), so no need to run both */
#if FB_EDITOR_ENABLED == FB_TRUE && FB_ENABLE_NAZI_ALLOCATOR == FB_FALSE
	#define FB_MEM_FIXED_DEBUG_WRAPPER FB_TRUE
	static const uint32_t debugWrapperUintMarker = 0xc0d3f3f3;
#else
	#define FB_MEM_FIXED_DEBUG_WRAPPER FB_FALSE
#endif

void *globalFixedAllocate(size_t size);
void globalFixedDeallocate(size_t size, void *ptr);

/* Debugging stuff */

#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE

static AllocationDebugger &getAllocationDebugger()
{
	static AllocationDebugger &allocationDebugger = AllocationDebugger::getInstance();
	return allocationDebugger;
}

#endif


static uint32_t getNumExtraBytes()
{
#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
	return AllocationDebugger::totalNumExtraBytes;
#else
	return 0;
#endif
}


#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_TRUE
	static FiendishAllocator &getFiendishAllocator()
	{
		static FiendishAllocator &fiendishAllocator = FiendishAllocator::getInstance();
		return fiendishAllocator;
	}
#endif


struct MemoryStatsImp
{
	AtomicInt64 totalAllocationAmount;
	AtomicInt64 totalAllocationAmountInBytes;
	AtomicInt64 currentAllocationAmount;
	AtomicInt64 currentAllocationAmountInBytes;
	AtomicInt64 peakAllocationAmountInBytes;
};

static void updatePeak(MemoryStatsImp &stats)
{
	int64_t newValue = lang::atomicLoadRelaxed(stats.currentAllocationAmountInBytes);
	while (true)
	{
		int64_t oldValue = lang::atomicLoadRelaxed(stats.peakAllocationAmountInBytes);
		if (oldValue >= newValue)
			break;
		if (lang::atomicCompareExchangeWeakAcquireRelease(stats.peakAllocationAmountInBytes, oldValue, newValue))
			break;
		lang::atomicThreadPause();
	}
}

static MemoryStatsImp &getMemoryStatsImp(MemoryPool pool)
{
	static MemoryStatsImp memoryStats[uint32_t(MemoryPool::Amount)];
	return memoryStats[(uint32_t)pool];
}

MemoryStats getMemoryStats(MemoryPool pool)
{
	MemoryStatsImp &memoryStats = getMemoryStatsImp(pool);
	MemoryStats stats;
	stats.totalAllocationAmount = (uint64_t)lang::atomicLoadRelaxed(memoryStats.totalAllocationAmount);
	stats.totalAllocationAmountInBytes = (uint64_t)lang::atomicLoadRelaxed(memoryStats.totalAllocationAmountInBytes);
	stats.currentAllocationAmount = (uint64_t)lang::atomicLoadRelaxed(memoryStats.currentAllocationAmount);
	stats.currentAllocationAmountInBytes = (uint64_t)lang::atomicLoadRelaxed(memoryStats.currentAllocationAmountInBytes);
	stats.peakAllocationAmountInBytes = (uint64_t)lang::atomicLoadRelaxed(memoryStats.peakAllocationAmountInBytes);
	return stats;
}

#ifndef FB_MEM_ENABLE_STATS
#define FB_MEM_ENABLE_STATS FB_FALSE
#endif

#define fbMemAssert(pred) \
	do \
	{ \
		if (!(pred)) \
		{ \
			__debugbreak(); \
		} \
	} while (false)


// Runtime heap

#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_TRUE
	#define osImpAlloc(size) getFiendishAllocator().allocate(size)
	#define osImpRealloc(buffer, size) getFiendishAllocator().reallocate(buffer, size)
	#define osImpFree(buffer) getFiendishAllocator().free(buffer)
#else
	#if FB_MEM_ENABLE_STATS == FB_TRUE
		#define osImpAlloc(size) ::malloc(size)
		#define osImpFree(buffer) ::free(buffer)
	#elif FB_MEM_ENABLE_STATS == FB_FALSE
		#define osImpAlloc(size) ::malloc(size)
		#define osImpRealloc(buffer, size) ::realloc(buffer, size)
		#define osImpFree(buffer) ::free(buffer)
	#endif
#endif

void *osAllocate(size_t sizeInBytes)
{
	sizeInBytes = (sizeInBytes) ? sizeInBytes : 1;

	#if FB_MEM_ENABLE_STATS == FB_TRUE
		MemoryStatsImp &stats = getMemoryStatsImp(MemoryPool::RuntimeHeap);
		atomicIncRelaxed(stats.totalAllocationAmount);
		atomicAddRelaxed(stats.totalAllocationAmountInBytes, int64_t(sizeInBytes));
		atomicIncRelaxed(stats.currentAllocationAmount);
		atomicAddRelaxed(stats.currentAllocationAmountInBytes, int64_t(sizeInBytes));
		updatePeak(stats);

		uint64_t *ptr = (uint64_t *) osImpAlloc(sizeInBytes + 16);
		ptr[0] = sizeInBytes;
		return ptr + 2;
	#else
		return osImpAlloc(sizeInBytes);
	#endif
}

void *osReallocate(void *buffer, size_t sizeInBytes)
{
	#if FB_MEM_ENABLE_STATS == FB_TRUE
		void *newBuffer = osAllocate(sizeInBytes);
		if (buffer)
		{
			uint64_t *oldBuffer = (uint64_t *) buffer;
			size_t oldSizeInBytes = *(oldBuffer - 2);
			size_t minSizeInBytes = sizeInBytes < oldSizeInBytes ? sizeInBytes : oldSizeInBytes;
			memcpy(newBuffer, buffer, minSizeInBytes);
			osFree(buffer);
		}

		return newBuffer;
	#else
		return osImpRealloc(buffer, sizeInBytes);
	#endif
}

void osFree(void *buffer)
{
	if (!buffer)
		return;

	#if FB_MEM_ENABLE_STATS == FB_TRUE
		uint64_t *ptr = (uint64_t *) buffer;
		ptr -= 2;
		size_t sizeInBytes = ptr[0];

		MemoryStatsImp &stats = getMemoryStatsImp(MemoryPool::RuntimeHeap);
		atomicDecRelaxed(stats.currentAllocationAmount);
		atomicSubRelaxed(stats.currentAllocationAmountInBytes, int64_t(sizeInBytes));
		osImpFree(ptr);
	#else
		osImpFree(buffer);
	#endif
}


// Fixed versions

#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE || FB_ENGINE_FOR_TOOLS == FB_TRUE
	#define FB_MEM_FIXED_USE_POOL FB_FALSE
	#define FB_MEM_GENERIC_USE_POOL FB_FALSE
#else
	#define FB_MEM_FIXED_USE_POOL FB_TRUE
	#define FB_MEM_GENERIC_USE_POOL FB_TRUE
#endif

#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_TRUE
	#define fixedPoolAlloc(size) getFiendishAllocator().allocate(size)
	#define fixedPoolFree(size, buffer) getFiendishAllocator().free(buffer)
#else
	#if FB_MEM_FIXED_USE_POOL == FB_TRUE
		#define fixedPoolAlloc(size) globalFixedAllocate(size)
		#define fixedPoolFree(size, buffer) globalFixedDeallocate(size, buffer)
	#elif FB_MEM_FIXED_USE_POOL == FB_FALSE
		#define fixedPoolAlloc(size) osAllocate(size)
		#define fixedPoolFree(size, buffer) osFree(buffer)
	#endif
#endif

void *allocateFixed(size_t sizeInBytes)
{
	#if FB_MEM_ENABLE_STATS == FB_TRUE
		MemoryStatsImp &stats = getMemoryStatsImp(MemoryPool::FixedPools);
		atomicIncRelaxed(stats.totalAllocationAmount);
		atomicAddRelaxed(stats.totalAllocationAmountInBytes, int64_t(sizeInBytes));
		atomicIncRelaxed(stats.currentAllocationAmount);
		atomicAddRelaxed(stats.currentAllocationAmountInBytes, int64_t(sizeInBytes));
		updatePeak(stats);
	#endif

	size_t sizeToAllocateInBytes = sizeInBytes + getNumExtraBytes();
	void *ptr = nullptr;

	#if FB_MEM_FIXED_DEBUG_WRAPPER == FB_TRUE
		size_t unalignedSizeToAllocateInBytes = sizeToAllocateInBytes;
		sizeToAllocateInBytes = FB_ALIGN_VALUE(sizeToAllocateInBytes, 16);
		uint32_t *uintPtr = (uint32_t *)fixedPoolAlloc(sizeToAllocateInBytes + 32);
		uintPtr[0] = debugWrapperUintMarker;
		uintPtr[1] = debugWrapperUintMarker;
		uintPtr[2] = debugWrapperUintMarker;
		uintPtr[3] = debugWrapperUintMarker;
		memcpy(((char*)uintPtr) + unalignedSizeToAllocateInBytes + 16, &debugWrapperUintMarker, 4);
		memcpy(((char*)uintPtr) + unalignedSizeToAllocateInBytes + 20, &debugWrapperUintMarker, 4);
		memcpy(((char*)uintPtr) + unalignedSizeToAllocateInBytes + 24, &debugWrapperUintMarker, 4);
		memcpy(((char*)uintPtr) + unalignedSizeToAllocateInBytes + 28, &debugWrapperUintMarker, 4);

		ptr = uintPtr + 4;
	#else
		ptr = fixedPoolAlloc(sizeToAllocateInBytes);
	#endif
	#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
		getAllocationDebugger().addAllocation(ptr, sizeInBytes);
		char *charPtr = reinterpret_cast<char*>(ptr);
		charPtr += AllocationDebugger::totalNumPrefixBytes;
		ptr = charPtr;
	#endif

	return ptr;
}

void *reallocateFixed(void *buffer, size_t oldSizeInBytes, size_t newSizeInBytes)
{
	if (oldSizeInBytes == newSizeInBytes)
		return buffer;

	void *newBuffer = allocateFixed(newSizeInBytes);
	if (buffer)
	{
		size_t minSizeInBytes = newSizeInBytes < oldSizeInBytes ? newSizeInBytes : oldSizeInBytes;
		memcpy(newBuffer, buffer, minSizeInBytes);
	}

	freeFixed(buffer, oldSizeInBytes);
	return newBuffer;
}

void freeFixed(void *buffer, size_t sizeInBytes)
{
	if (!buffer)
		return;

	#if FB_MEM_ENABLE_STATS == FB_TRUE
		MemoryStatsImp &stats = getMemoryStatsImp(MemoryPool::FixedPools);
		atomicDecRelaxed(stats.currentAllocationAmount);
		atomicSubRelaxed(stats.currentAllocationAmountInBytes, int64_t(sizeInBytes));
	#endif

		sizeInBytes += getNumExtraBytes();
	#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
		char *charPtr = reinterpret_cast<char*>(buffer);
		charPtr -= AllocationDebugger::totalNumPrefixBytes;
		buffer = charPtr;
		getAllocationDebugger().removeAllocation(buffer);
	#endif

	#if FB_MEM_FIXED_DEBUG_WRAPPER == FB_TRUE
		size_t unalignedSizeToAllocateInBytes = sizeInBytes;
		sizeInBytes = FB_ALIGN_VALUE(sizeInBytes, 16);
		uint32_t *ptr = (uint32_t *) buffer;
		ptr -= 4;
		fbMemAssert(ptr[0] == debugWrapperUintMarker);
		fbMemAssert(ptr[1] == debugWrapperUintMarker);
		fbMemAssert(ptr[2] == debugWrapperUintMarker);
		fbMemAssert(ptr[3] == debugWrapperUintMarker);
		//fbMemAssert(ptr[(sizeInBytes / 4) + 2] == 0xc0def3f3);
		//fbMemAssert(ptr[(sizeInBytes / 4) + 3] == 0xc0def3f3);
		fbMemAssert(memcmp(((char*)buffer) + unalignedSizeToAllocateInBytes + 0, &debugWrapperUintMarker, 4) == 0);
		fbMemAssert(memcmp(((char*)buffer) + unalignedSizeToAllocateInBytes + 4, &debugWrapperUintMarker, 4) == 0);
		fbMemAssert(memcmp(((char*)buffer) + unalignedSizeToAllocateInBytes + 8, &debugWrapperUintMarker, 4) == 0);
		fbMemAssert(memcmp(((char*)buffer) + unalignedSizeToAllocateInBytes + 12, &debugWrapperUintMarker, 4) == 0);

		// 0x7f7f7f7f
		memset(ptr, 0x7f, sizeInBytes + 32);
		return fixedPoolFree(sizeInBytes + 32, ptr);
	#else
		fixedPoolFree(sizeInBytes, buffer);
	#endif	
}

// Generic versions

#if FB_MEM_GENERIC_USE_POOL == FB_TRUE
		static const uint32_t GenericExtraInts = 2;
		static const uint32_t GenericExtraSizeInBytes = GenericExtraInts * sizeof(int);
#endif

void *allocateMemory(size_t sizeInBytes)
{
	#if FB_MEM_GENERIC_USE_POOL == FB_FALSE
		return osAllocate(sizeInBytes);
	#else
		size_t impSizeInBytes = sizeInBytes + GenericExtraSizeInBytes;
		uint32_t *ptr = (uint32_t *) allocateFixed(impSizeInBytes);
		ptr[0] = (uint32_t) sizeInBytes;
		return ptr + GenericExtraInts;
	#endif
}

void *reallocateMemory(void *buffer, size_t sizeInBytes)
{
	#if FB_MEM_GENERIC_USE_POOL == FB_FALSE
		return osReallocate(buffer, sizeInBytes);
	#else
		uint32_t *ptr = (uint32_t *) buffer;
		size_t oldImpSizeInBytes = 0;
		if (ptr)
		{
			ptr -= GenericExtraInts;
			oldImpSizeInBytes = ptr[0] + GenericExtraSizeInBytes;
		}

		size_t newUserSizeInBytes = sizeInBytes + GenericExtraSizeInBytes;
		ptr = (uint32_t *) reallocateFixed(ptr, oldImpSizeInBytes, newUserSizeInBytes);
		ptr[0] = (uint32_t) sizeInBytes;
		return ptr + GenericExtraInts;
	#endif
}

void freeMemory(void *buffer)
{
	if (buffer)
	{
		#if FB_MEM_GENERIC_USE_POOL == FB_FALSE
			return osFree(buffer);
		#else
			uint32_t *ptr = (uint32_t *) buffer;
			ptr -= GenericExtraInts;

			size_t oldImpSizeInBytes = ptr[0] + GenericExtraSizeInBytes;
			freeFixed(ptr, oldImpSizeInBytes);
		#endif
	}
}


#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_TRUE
#define alignedPoolAlloc(size) getFiendishAllocator().allocate(size)
#define alignedPoolFree(size, buffer) getFiendishAllocator().free(buffer)
#else
	#define FB_MEM_ALIGNED_USE_POOL FB_TRUE
	#if FB_MEM_ALIGNED_USE_POOL == FB_TRUE
		#define alignedPoolAlloc(size) globalFixedAllocate(size)
		#define alignedPoolFree(size, buffer) globalFixedDeallocate(size, buffer)
	#elif FB_MEM_ALIGNED_USE_POOL == FB_FALSE
		#define alignedPoolAlloc(size) osAllocate(size)
		#define alignedPoolFree(size, buffer) osFree(buffer)
	#endif
#endif

static inline uint32_t getExtraSizeInBytesForAlignment(uint32_t alignment)
{
	uint32_t extraSizeInBytes = 2 * alignment - 1;
	extraSizeInBytes = extraSizeInBytes < 8 ? 8 : extraSizeInBytes;

	return extraSizeInBytes;
}

void *allocateAligned(size_t sizeInBytes, uint32_t alignment)
{
	#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
		fb_static_assert(((AllocationDebugger::totalNumPrefixBytes - 1) & AllocationDebugger::totalNumPrefixBytes) == 0 && 
			"AllocationDebugger::totalNumPrefixBytes is not a power of two");
		fb_static_assert(AllocationDebugger::totalNumPrefixBytes + AllocationDebugger::numAfterSafeZoneBytes == AllocationDebugger::totalNumExtraBytes &&
			"Allocation tracker logic changed. Check the code below");
		uint32_t debugDataFixAmountInBytes = lang::max<uint32_t>(alignment, AllocationDebugger::totalNumPrefixBytes);
		uint32_t debugDataExtraAmountInBytes = alignment <= AllocationDebugger::totalNumPrefixBytes ?
			AllocationDebugger::totalNumExtraBytes :
			alignment + AllocationDebugger::numAfterSafeZoneBytes;
		size_t sizeForAllocationDebugger = sizeInBytes + debugDataExtraAmountInBytes - AllocationDebugger::totalNumExtraBytes;
		sizeInBytes += debugDataExtraAmountInBytes;
	#endif

	uint32_t extraSizeInBytes = getExtraSizeInBytesForAlignment(alignment);
	uintptr_t ptr = (uintptr_t) alignedPoolAlloc(sizeInBytes + extraSizeInBytes);
	uintptr_t alignedPtr = FB_ALIGN_VALUE(ptr, size_t(alignment)) + alignment;
	uint32_t *infoPtr = (uint32_t *) alignedPtr;
	infoPtr -= 2;
	infoPtr[0] = (uint32_t) sizeInBytes;
	infoPtr[1] = (uint32_t) (alignedPtr - ptr);

	#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
		char *charPtr = reinterpret_cast<char*>(alignedPtr);
		getAllocationDebugger().addAllocation(charPtr, sizeForAllocationDebugger);
		return charPtr + debugDataFixAmountInBytes;
	#else
		return (void*) alignedPtr;
	#endif
}

void *reallocateAligned(void *buffer, size_t sizeInBytes, uint32_t alignment)
{
	void *newBuffer = allocateAligned(sizeInBytes, alignment);
	if (buffer)
	{
		uint32_t *infoPtr = (uint32_t *) buffer;
		infoPtr -= 2;
		uint32_t oldSizeInBytes = infoPtr[0];

		size_t minSizeInBytes = sizeInBytes < oldSizeInBytes ? sizeInBytes : oldSizeInBytes;
		memcpy(newBuffer, buffer, minSizeInBytes);
	}

	freeAligned(buffer, alignment);
	return newBuffer;
}

void freeAligned(void *buffer, uint32_t alignment)
{
	if (buffer)
	{
		#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
			uint32_t debugDataFixAmountInBytes = lang::max<uint32_t>(alignment, AllocationDebugger::totalNumPrefixBytes);
			char *charPtr = reinterpret_cast<char*>(buffer);
			charPtr -= debugDataFixAmountInBytes;
			buffer = charPtr;
			getAllocationDebugger().removeAllocation(buffer);
		#endif
		uint32_t extraSizeInBytes = getExtraSizeInBytesForAlignment(alignment);
		uint32_t *infoPtr = (uint32_t *) buffer;
		infoPtr -= 2;

		uint32_t sizeInBytes = infoPtr[0];
		alignedPoolFree(sizeInBytes + extraSizeInBytes, ((char*) buffer) - infoPtr[1]);
	}
}


FB_END_PACKAGE1()

#define FB_MEM_OPERATORS_USE_POOL FB_TRUE

FB_PACKAGE0()
	static inline void *newImp(std::size_t sizeInBytes, bool)
	{
		#if FB_MEM_OPERATORS_USE_POOL == FB_FALSE
			return lang::osAllocate(sizeInBytes);
		#else
			return lang::allocateMemory(sizeInBytes);
		#endif
	}

	static inline void freeImp(void *buffer, bool)
	{
		#if FB_MEM_OPERATORS_USE_POOL == FB_FALSE
			return lang::osFree(buffer);
		#else
			return lang::freeMemory(buffer);
		#endif
	}

FB_END_PACKAGE0()

// Stuff below has to match to what each compiler implements

// New

void *operator new(std::size_t sizeInBytes)
{
	return fb::newImp(sizeInBytes, false);
}

void *operator new[](std::size_t sizeInBytes)
{
	return fb::newImp(sizeInBytes, true);
}


// Delete

void operator delete(void *buffer)
{
	fb::freeImp(buffer, false);
}

void operator delete[](void *buffer)
{
	fb::freeImp(buffer, true);
}

// Delete with size

void operator delete(void *buffer, std::size_t )
{
	fb::freeImp(buffer, false);
}

void operator delete[](void *buffer, std::size_t )
{
	fb::freeImp(buffer, true);
}


