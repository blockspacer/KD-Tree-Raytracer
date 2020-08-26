#ifndef INCLUDED_FB_MEMORY_BLOCKALLOCATOR_H
#define INCLUDED_FB_MEMORY_BLOCKALLOCATOR_H

#if 0

#pragma once
#include "fb/lang/Atomics.h"
#include "Mutex.h"

namespace filecache {

struct BlockAllocator: public IAllocator
{
	struct PointerList
	{
		AtomicPointer next;
	};

	// Allocation chunks are tracked with this linked structure to allow freeing them back to heap once allocator dies.
	// Always uses atomics to prevent copy-pasting similar code pointlessly to all derived classes. This shouldn't really slow down non-atomic allocator.
	static const uint32_t FreeBlockPointerAmount = 1024 - 2;
	struct FreeBlock
	{
		AtomicPointer blocks[FreeBlockPointerAmount];
		AtomicPointer next;
		AtomicInt32 currentIndex;
	};

	enum GrowMode
	{
		// Each level is grown by minAllocationSize
		GrowModeLinear,
		// Each level is twice the size of the previous one
		GrowModeExponential,
	};

	BlockAllocator(uint32_t minAllocationSizeInBytes, uint32_t alignmentInBytes, uint32_t sizeLevels, uint32_t allocationChunkSizeInBytes, GrowMode growMode, uint32_t flags);
	~BlockAllocator();

	void *allocatePointer(uint32_t sizeInBytes) final;
	void freePointer(void *pointer, uint32_t sizeInBytes) final;

private:
	FreeBlock *createFreeBlockImp();
	void pushBlockImp(void *pointer);

	//void createUserPointerImp(void *userPointer, uint32_t userSizeInBytes) const;
	//void validateUserPointerImp(void *userPointer, uint32_t userSizeInBytes) const;

	// From user visible pointer to our internal pointer
	void *getInternalPointerImp(void *ptr);
	// From internal pointer to user visible pointer
	void *getUserPointerImp(void *ptr);
	void *allocateFromLevelImp(uint32_t level);

	AtomicPointer freeBlockList;
	uint32_t flags;
	Mutex freeBlockGrowMutex;

	uint32_t *allocationLevelSizeInBytes;
	PointerList *freePointerLists;
	uint32_t sizeLevels;
	uint32_t allocationChunkSizeInBytes;
	uint16_t alignmentInBytes;
	// Byte amount to pad allocation with (space for ExtraSizeType, possible debug padding etc) 
	uint16_t extraSizeForAllocationInBytes;
	// Offset to user pointer from internal pointer 
	uint16_t userAllocationOffsetInBytes;

	// Not defined, don't copy by value
	BlockAllocator(const BlockAllocator &);
	void operator = (const BlockAllocator &);
};

} // filecache

#endif

#endif