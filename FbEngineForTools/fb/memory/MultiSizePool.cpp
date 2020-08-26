#include "Precompiled.h"
#include "MultiSizePool.h"

#include "fb/algorithm/BinarySearch.h"
#include "fb/lang/AlignmentFunctions.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/MemoryOperatorsConfig.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/PlacementNew.h"

// Make sure we don't have bloated atomics
fb_static_assert(sizeof(void*) == sizeof(fb::lang::AtomicPointer));
//#define FB_SKIP_POOL

FB_PACKAGE1(memory)

uint32_t getChunkAmountEstimate(uint32_t maxAllocationSizeInBytes, uint32_t alignmentInBytes, MultiSizePool::GrowMode growMode)
{
	// Must be pow2
	fb_assert((alignmentInBytes & (alignmentInBytes - 1)) == 0);
	fb_assert(maxAllocationSizeInBytes > alignmentInBytes);

	if (growMode == MultiSizePool::GrowModeLinear)
		return maxAllocationSizeInBytes / alignmentInBytes;

	// Pow2 variants

	// Must be pow2
	fb_assert((maxAllocationSizeInBytes & (maxAllocationSizeInBytes - 1)) == 0);
	uint32_t result = 1;
	while (maxAllocationSizeInBytes > alignmentInBytes)
	{
		++result;
		maxAllocationSizeInBytes /= 2;
	}

	if (growMode == MultiSizePool::GrowModeQuarter)
		result *= 4;
	else if (growMode == MultiSizePool::GrowModeHalf)
		result *= 2;

	return result;
}

#define FB_MULTISIZE_POOL_PROFILING FB_FALSE

void MultiSizePool::allocateInternalSpace(uint32_t chunkIndex, uint32_t pointerSizeInBytes)
{
	fb_assert(chunkIndex < chunkAmount);

	// Multiple threads can come here at the same time, and they all end up allocating bunch of pointers to their own chunk.
	// However, we have to make sure allocator is protected as otherwise we'd waste a lot of memory

	uint32_t pointerAllocationAmount = 16;
	uint32_t chunkAllocationSize = pointerSizeInBytes * pointerAllocationAmount;

	for (;;)
	{
		AllocatorBlock *currentBlock = (AllocatorBlock *) atomicLoadAcquire(allocatorBlock);
		if (currentBlock)
		{
			int32_t currentOffset = atomicAddRelaxed(currentBlock->currentOffsetInBytes, int32_t(chunkAllocationSize));
			if (currentOffset + chunkAllocationSize <= allocatorBlockSizeInBytes)
			{
				// Success.
				char *newPointer = currentBlock->data + currentOffset;
				freePointerLists[chunkIndex].push(newPointer, pointerSizeInBytes, pointerAllocationAmount);
				return;
			}
		}

		// Rare case, not enough space. 
		// Allocate a new buffer, protect with mutex as we don't want to do this from multiple threads.

		if (flags & FlagThreadSafe)
			mutex.enter();

		// Another thread might have already allocated more space
		if (currentBlock == atomicLoadAcquire(allocatorBlock))
		{
			char *newPointer = (char*) allocator->allocate(allocatorBlockSizeInBytes);
			if (!newPointer)
			{
				newPointer = (char*)backupAllocator->allocate(allocatorBlockSizeInBytes);
				atomicAddRelaxed(backupAllocationBytes, allocatorBlockSizeInBytes);
			}

			AllocatorBlock *newBlock = (AllocatorBlock *) newPointer;
			new (&newBlock->currentOffsetInBytes) lang::AtomicInt32();
			newBlock->data = newPointer + sizeof(AllocatorBlock);

			lang::atomicStoreRelease(allocatorBlock, newBlock);
		}

		if (flags & FlagThreadSafe)
			mutex.leave();
	}
}

void *MultiSizePool::allocateImp(uint64_t sizeInBytes64)
{
	if (sizeInBytes64 <= maxAllocationSizeInBytes)
	{
		fb_static_assert(sizeof(maxAllocationSizeInBytes) == 4 && "maxAllocationSizeInBytes may be more than 32 bits can handle");
		uint32_t sizeInBytes = uint32_t(sizeInBytes64);
		uint32_t chunkIndex = getChunkIndex(sizeInBytes);
		for (;;)
		{
			void *ptr = freePointerLists[chunkIndex].pop();
			if (ptr)
				return ptr;

			allocateInternalSpace(chunkIndex, (chunkIndex + 1) * 8);
		}
	}

	if (oversizeAllocator)
	{
		atomicAddRelaxed(oversizeAllocationBytes, sizeInBytes64);
		return oversizeAllocator->allocate(sizeInBytes64);
	}

	return nullptr;
}

void MultiSizePool::deallocateImp(void *pointer, uint64_t sizeInBytes64)
{
	if (sizeInBytes64 <= maxAllocationSizeInBytes)
	{
		fb_static_assert(sizeof(maxAllocationSizeInBytes) == 4 && "maxAllocationSizeInBytes may be more than 32 bits can handle");
		uint32_t sizeInBytes = uint32_t(sizeInBytes64);
		uint32_t chunkIndex = getChunkIndex(sizeInBytes);
		freePointerLists[chunkIndex].push(pointer);
		return;
	}

	if (oversizeAllocator)
	{
		oversizeAllocator->deallocate(pointer, sizeInBytes64);
		atomicSubRelaxed(oversizeAllocationBytes, sizeInBytes64);
		return;
	}

	fb_assert(!"Trying to release pointer which was not allocated through MultiSizePool" && 0);
}

uint32_t MultiSizePool::getChunkIndex(uint32_t sizeInBytes) const
{
	return (sizeInBytes / 8) - 1;
}

MultiSizePool::MultiSizePool(const InitParams &params, uint32_t flags_)
	: allocator(params.allocator)
	, backupAllocator(params.backupAllocator)
	, oversizeAllocator(params.oversizeAllocator)
	, maxAllocationSizeInBytes(params.maxAllocationSizeInBytes)
	, alignmentInBytes(params.alignmentInBytes)
	, flags(flags_)
{
	fb_assert(params.validate());
	fb_assert(alignmentInBytes >= sizeof(void*));
	fb_assert(lang::AtomicPointerImp().is_lock_free());

	lang::atomicStoreRelaxed(allocatorBlock, nullptr);
	allocatorBlockSizeInBytes = allocator->getBlockSize() - sizeof(AllocatorBlock);
	growMode = params.growMode;

	uint32_t chunkAmountEstimate = getChunkAmountEstimate(maxAllocationSizeInBytes, alignmentInBytes, params.growMode);
	
	char *pointer = (char*) lang::osAllocate(chunkAmountEstimate * (sizeof(AtomicPointerList) + sizeof(uint16_t)) + 16u);
	allocationLevelSizeInBytes = (uint16_t *) pointer;
	
	uintptr_t freeListPointerInBytes = uintptr_t(pointer + chunkAmountEstimate * sizeof(uint16_t));
	freeListPointerInBytes = FB_ALIGN_VALUE(freeListPointerInBytes, uintptr_t(16u));
	freePointerLists = (AtomicPointerList*) freeListPointerInBytes;
	for (uint32_t i = 0; i < chunkAmountEstimate; ++i)
	{
		new (&freePointerLists[i]) AtomicPointerList();
		freePointerLists[i].useAtomics = (flags & FlagThreadSafe) ? true : false;
	}

	// Calculate actual chunk limits

	uint32_t currentSizeInBytes = alignmentInBytes;
	uint32_t previousPow2 = alignmentInBytes;
	for (uint32_t i = 0; i < chunkAmountEstimate + 1; ++i)
	{
		allocationLevelSizeInBytes[i] = (uint16_t) currentSizeInBytes;

		if (currentSizeInBytes == maxAllocationSizeInBytes)
		{
			chunkAmount = i + 1;
			break;
		}

		if (params.growMode == GrowModeLinear)
			currentSizeInBytes += alignmentInBytes;
		else if (params.growMode == GrowModeDouble)
			currentSizeInBytes *= 2;
		else
		{
			if (params.growMode == GrowModeQuarter)
				currentSizeInBytes += previousPow2 / 4;
			else if (params.growMode == GrowModeHalf)
				currentSizeInBytes += previousPow2 / 2;

			if ((currentSizeInBytes & (currentSizeInBytes - 1)) == 0)
				previousPow2 = currentSizeInBytes;
		}
	}

	fb_assert(chunkAmount && (chunkAmount <= chunkAmountEstimate));
}

MultiSizePool::~MultiSizePool()
{
	lang::osFree(allocationLevelSizeInBytes);
}

SizeType MultiSizePool::getBlockSize() const
{
	return alignmentInBytes;
}

void *MultiSizePool::allocate(uint64_t sizeInBytes)
{
#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE
	return lang::allocateMemory(sizeInBytes);
#elif FB_POOL_ALLOCATORS_DISABLED == FB_FALSE

	sizeInBytes = FB_ALIGN_VALUE(sizeInBytes, uint64_t(alignmentInBytes));
	#ifdef FB_SKIP_POOL
		void *result = lang::osAllocate(sizeInBytes);
	#else
		void *result = allocateImp(sizeInBytes);
	#endif


	return result;
#endif
}

void MultiSizePool::deallocate(void *pointer, uint64_t sizeInBytes)
{
#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE
	lang::freeMemory(pointer);
#elif FB_POOL_ALLOCATORS_DISABLED == FB_FALSE
	if (!pointer)
		return;

	#ifdef FB_SKIP_POOL
		lang::osFree(pointer);
	#else
		sizeInBytes = FB_ALIGN_VALUE(sizeInBytes, uint64_t(alignmentInBytes));
		deallocateImp(pointer, sizeInBytes);
	#endif
#endif
}

uint64_t MultiSizePool::getCurrentAllocationBytes() const
{
	uint64_t total = allocator ? allocator->getCurrentAllocationBytes() : 0;
	total += atomicLoadRelaxed(backupAllocationBytes);
	total += atomicLoadRelaxed(oversizeAllocationBytes);
	return total;
}

FB_END_PACKAGE1()
