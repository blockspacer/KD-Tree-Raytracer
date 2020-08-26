#include "Precompiled.h"
#if 0

#include "BlockAllocator.h"

#include "ImpByteElementArray.h"
#include "BinarySearch.h"
#include "Alignment.h"

namespace filecache {

// If not in FlagFixedMode, pool level is stored before actual user pointer
typedef uint32_t ExtraSizeType;
static const ExtraSizeType DefaulHeapValue = 0xffffffff;

BlockAllocator::BlockAllocator(uint32_t minAllocationSizeInBytes, uint32_t alignmentInBytes_, uint32_t sizeLevels_, uint32_t allocationChunkSizeInBytes_, GrowMode growMode, uint32_t flags_)
:	flags(flags_)
,	allocationLevelSizeInBytes(NULL)
,	freePointerLists(NULL)
,	sizeLevels(sizeLevels_)
,	allocationChunkSizeInBytes(allocationChunkSizeInBytes_)
,	alignmentInBytes((uint16_t) alignmentInBytes_)
,	extraSizeForAllocationInBytes(0)
,	userAllocationOffsetInBytes(0)
{
	flags |= getAllocatorForcedFlags();

	fileCacheAssert(alignmentInBytes >= 1);
	fileCacheAssert((alignmentInBytes & (alignmentInBytes - 1)) == 0);

	atomicStoreRelaxed(&freeBlockList, createFreeBlockImp());

	// Combine our level data to a single pointer
	uint32_t allocSize = (sizeLevels * sizeof(uint32_t)) + (sizeLevels * sizeof(PointerList));
	char *pointer = (char*) malloc(allocSize);
	allocationLevelSizeInBytes = (uint32_t *) pointer;
	freePointerLists = (PointerList*) (pointer + (sizeLevels * sizeof(uint32_t)));

	if ((flags & (FlagFixedMode | FlagDebugForceHeap)) == 0)
	{
		userAllocationOffsetInBytes += sizeof(ExtraSizeType);
	}

	if (flags & FlagDebugMode)
	{
		uint16_t debugBytes = (uint16_t) getAllocatorDebugAreaInBytesImp(flags);
		userAllocationOffsetInBytes += debugBytes;
		extraSizeForAllocationInBytes += debugBytes;
	}

	// Align extra space/offsets for extra size
	userAllocationOffsetInBytes = (uint16_t) getAligned(userAllocationOffsetInBytes, alignmentInBytes);
	extraSizeForAllocationInBytes += userAllocationOffsetInBytes;
	extraSizeForAllocationInBytes = (uint16_t) getAligned(extraSizeForAllocationInBytes, alignmentInBytes);	

	uint32_t currentSize = minAllocationSizeInBytes;
	for (uint32_t i = 0; i < sizeLevels; ++i)
	{
		// Work out actual size per level as we don't want to keep on doubling possible rounding to higher allocation sizes
		uint32_t actualSize = currentSize;

		// Space for extra info
		actualSize += extraSizeForAllocationInBytes;

		// At least size of our PointerList
		actualSize = (actualSize < sizeof(PointerList)) ? sizeof(PointerList) : actualSize;

		// And finally align to wanted bytes
		actualSize = getAligned(actualSize, alignmentInBytes);

		allocationLevelSizeInBytes[i] = actualSize;

		fileCacheAssert(actualSize <= allocationChunkSizeInBytes);
		atomicStoreRelaxed(&freePointerLists[i].next, NULL);

		if (growMode == GrowModeExponential)
			currentSize <<= 1;
		else
			currentSize += minAllocationSizeInBytes;
	}
}

BlockAllocator::~BlockAllocator()
{
	if (flags & FlagAllocationHandlingLeak)
		return;

	FreeBlock *block = (FreeBlock *) atomicLoadRelaxed(&freeBlockList);
	while (block)
	{
		int32_t freeAmount = atomicLoadRelaxed(&block->currentIndex);
		freeAmount = freeAmount > FreeBlockPointerAmount ?  FreeBlockPointerAmount : freeAmount;

		for (int32_t i = 0; i < freeAmount; ++i)
		{
			void *ptr = atomicLoadRelaxed(&block->blocks[i]);
			deallocateAlignedImp(ptr);
		}

		FreeBlock *nextBlock = (FreeBlock *) atomicLoadRelaxed(&block->next);
		free(block);
		block = nextBlock;
	}

	free(allocationLevelSizeInBytes);
}

void *BlockAllocator::allocatePointer(uint32_t userSizeInBytes)
{
	void *internalPointer = NULL;
	if ((flags & FlagDebugForceHeap) == 0)
	{
		uint32_t sizeInBytes = userSizeInBytes + extraSizeForAllocationInBytes;

		uint32_t levelIndex = binaryFind(allocationLevelSizeInBytes, sizeLevels, sizeInBytes);
		if (levelIndex & (1U << 31U))
			levelIndex &= ~(1U << 31U);

		if (levelIndex < sizeLevels)
		{
			fileCacheAssert(allocationLevelSizeInBytes[levelIndex] >= sizeInBytes);
			internalPointer = allocateFromLevelImp(levelIndex);
		}
		else
		{
			internalPointer = allocateAlignedImp(sizeInBytes, alignmentInBytes);
		}

		fileCacheAssert(internalPointer != NULL);
		if ((flags & FlagFixedMode) == 0)
		{
			fileCacheAssert(extraSizeForAllocationInBytes >= sizeof(ExtraSizeType));

			ExtraSizeType levelValue = (levelIndex < sizeLevels) ? levelIndex : DefaulHeapValue;
			memcpy(internalPointer, &levelValue, sizeof(ExtraSizeType));
		}
	}
	else
	{
		internalPointer = allocateAlignedImp(userSizeInBytes + extraSizeForAllocationInBytes, alignmentInBytes);
	}

	fileCacheAssert(internalPointer != NULL);
	void *userPointer = getUserPointerImp(internalPointer);

	createAllocatorUserPointerImp(flags, userPointer, userSizeInBytes);
	return userPointer;
}

void BlockAllocator::freePointer(void *userPointer, uint32_t userSizeInBytes)
{
	if (!userPointer)
		return;

	validateAllocatorUserPointerImp(flags, userPointer, userSizeInBytes);
	void *internalPointer = getInternalPointerImp(userPointer);

	if ((flags & FlagDebugForceHeap) == 0)
	{
		ExtraSizeType level = 0;

		if (flags & FlagFixedMode)
		{
			uint32_t sizeInBytes = userSizeInBytes + extraSizeForAllocationInBytes;
			uint32_t levelIndex = binaryFind(allocationLevelSizeInBytes, sizeLevels, sizeInBytes);
			if (levelIndex & (1U << 31U))
				levelIndex &= ~(1U << 31U);

			if (levelIndex >= sizeLevels)
			{
				deallocateAlignedImp(internalPointer);
				return;
			}

			level = levelIndex;
		}
		else
		{
			memcpy(&level, internalPointer, sizeof(ExtraSizeType));
			fileCacheAssert( !userSizeInBytes || level == DefaulHeapValue || ((binaryFind(allocationLevelSizeInBytes, sizeLevels, userSizeInBytes + extraSizeForAllocationInBytes) & ~(1<<31)) == level) );

			if (level == DefaulHeapValue)
			{
				deallocateAlignedImp(internalPointer);
				return;
			}
		}

		// Push to front

		PointerList *listPointer = (PointerList *) internalPointer;
		if ((flags & FlagThreadSafe) == 0)
		{
			void *currentHead = atomicLoadRelaxed(&freePointerLists[level].next);
			atomicStoreRelaxed(&listPointer->next, currentHead);
			atomicStoreRelaxed(&freePointerLists[level].next, listPointer);
		}
		else
		{
			for (;;)
			{
				void *currentHead = atomicLoadAcquire(&freePointerLists[level].next);
				atomicStoreRelaxed(&listPointer->next, currentHead);

				if (atomicCompareExchangeWeakAcquireRelease(&freePointerLists[level].next, currentHead, listPointer))
					return;
			}
		}
	}
	else
		deallocateAlignedImp(internalPointer);
}

BlockAllocator::FreeBlock *BlockAllocator::createFreeBlockImp()
{
	if (flags & FlagAllocationHandlingLeak)
		return NULL;

	FreeBlock *result = (FreeBlock *) malloc(sizeof(FreeBlock));
	atomicStoreRelaxed(&result->next, NULL);
	atomicStoreRelaxed(&result->currentIndex, 0);

	return result;
}

void BlockAllocator::pushBlockImp(void *pointer)
{
	if (flags & FlagAllocationHandlingLeak)
		return;

	if ((flags & FlagThreadSafe) == 0)
	{
		FreeBlock *block = (FreeBlock *) atomicLoadRelaxed(&freeBlockList);
		fileCacheAssert(block);

		// Reserve an index
		int32_t index = atomicLoadRelaxed(&block->currentIndex);
		atomicStoreRelaxed(&block->currentIndex, index + 1);
		if (index < FreeBlockPointerAmount)
		{
			// The usual easy case, we have space to add into current block
			atomicStoreRelaxed(&block->blocks[index], pointer);
			return;
		}

		FreeBlock *newBlock = createFreeBlockImp();
		atomicStoreRelaxed(&newBlock->currentIndex, 1);
		atomicStoreRelaxed(&newBlock->blocks[0], pointer);
		atomicStoreRelaxed(&newBlock->next, block);
		atomicStoreRelease(&freeBlockList, newBlock);
		return;
	}

	for (;;)
	{
		FreeBlock *block = (FreeBlock *) atomicLoadAcquire(&freeBlockList);
		fileCacheAssert(block);

		// Reserve an index
		int32_t index = atomicInc(&block->currentIndex);
		if (index < FreeBlockPointerAmount)
		{
			// The usual easy case, we have space to add into current block
			atomicStoreRelaxed(&block->blocks[index], pointer);
			return;
		}

		// Current block is full. Protect creating a new one with mutex.
		// Otherwise we might create several (mostly empty) blocks when multiple threads manage to hit this at the same time.
		// ... Or just keep spinning while waiting for first one to finish adding. Might as well block properly.

		freeBlockGrowMutex.enter();
		{
			// Double check it hasn't been added already
			FreeBlock *currentBlock = (FreeBlock *) atomicLoadRelaxed(&freeBlockList);
			if (currentBlock == block)
			{
				FreeBlock *newBlock = createFreeBlockImp();
				atomicInc(&newBlock->currentIndex);
				atomicStoreRelaxed(&newBlock->blocks[0], pointer);
				atomicStoreRelaxed(&newBlock->next, block);
				atomicStoreRelease(&freeBlockList, newBlock);

				freeBlockGrowMutex.leave();
				return;
			}
		}
		freeBlockGrowMutex.leave();

		// Re-try as there should be space now
	}
}

void *BlockAllocator::getInternalPointerImp(void *ptr) 
{ 
	return ((char*)ptr) - userAllocationOffsetInBytes; 
}

void *BlockAllocator::getUserPointerImp(void *ptr) 
{ 
	return ((char*)ptr) + userAllocationOffsetInBytes; 
}

void *BlockAllocator::allocateFromLevelImp(uint32_t level)
{
	fileCacheAssert(level < sizeLevels);

	for (;;)
	{
		PointerList *currentHead = (PointerList *) atomicLoadAcquire(&freePointerLists[level].next);
		if (!currentHead)
		{
			// We can come here from multiple threads at the same time

			char *chunk = (char*) allocateAlignedImp(allocationChunkSizeInBytes, alignmentInBytes);
			pushBlockImp(chunk);

			uint32_t ptrSizeInBytes = allocationLevelSizeInBytes[level];
			uint32_t pointersInBlock = allocationChunkSizeInBytes / ptrSizeInBytes;
			fileCacheAssert(pointersInBlock > 0);

			// Create linked list out of the chunk.

			char *currentPointer = chunk;
			uint32_t offset = ptrSizeInBytes;
			for (uint32_t i = 0; i < pointersInBlock - 1; ++i)
			{
				char *nextPointer = (char *) (currentPointer + offset);
				atomicStoreRelaxed(&((PointerList *) currentPointer)->next, nextPointer);
				currentPointer = nextPointer;
			}

			// Connect list to last pointer.

			PointerList *lastPointer = (PointerList *) currentPointer;
			for (;;)
			{
				currentHead = (PointerList *) atomicLoadAcquire(&freePointerLists[level].next);
				atomicStoreRelaxed(&lastPointer->next, currentHead);

				if (atomicCompareExchangeWeakAcquireRelease(&freePointerLists[level].next, currentHead, chunk))
					break;
			}
		}

		// Only duplicate the normal case which is popping from the list.
		if ((flags & FlagThreadSafe) == 0)
		{
			currentHead = (PointerList *) atomicLoadRelaxed(&freePointerLists[level].next);
			fileCacheAssert(currentHead);
			void *nextHead = atomicLoadRelaxed(&currentHead->next);
			atomicStoreRelaxed(&freePointerLists[level].next, nextHead);
			return currentHead;
		}
		else
		{
			currentHead = (PointerList *) atomicLoadAcquire(&freePointerLists[level].next);
			if (currentHead)
			{
				void *nextHead = atomicLoadRelaxed(&currentHead->next);
				if (atomicCompareExchangeWeakAcquireRelease(&freePointerLists[level].next, currentHead, nextHead))
					return currentHead;
			}
		}
	}
}

} // filecache

#endif