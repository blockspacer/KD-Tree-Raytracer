#pragma once

#include "IBlockAllocator.h"
#include "IAllocator.h"
#include "AtomicPointerList.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/GlobalMemoryOperators.h"
#include "fb/lang/thread/Mutex.h"

FB_PACKAGE1(memory)

// Provide 'fast' allocation/deallocation performance within fixed size chunks.
// When bigAllocator runs out of space, allocations will be directed to backupAllocator, if present. Otherwise, NULL is returned.
// Allocations outside the initialised maxAllocationSize will be directed to oversizeAllocator, if present. Otherwise NULL is returned.

class MultiSizePool: public IBlockAllocator
{
	IBlockAllocator *allocator = nullptr;
	IAllocator *backupAllocator = nullptr;
	IAllocator *oversizeAllocator = nullptr;

	lang::AtomicUInt64 backupAllocationBytes;
	lang::AtomicUInt64 oversizeAllocationBytes;

	uint16_t *allocationLevelSizeInBytes = nullptr;
	//void **freePointerList = NULL;
	AtomicPointerList *freePointerLists = nullptr;

	uint32_t maxAllocationSizeInBytes = 0;
	uint32_t chunkAmount = 0;
	uint32_t growMode = 0;
	uint32_t alignmentInBytes = 0;
	uint32_t flags = 0;

	// Internal cache. Wraps large allocator block to multiple internal allocations
	Mutex mutex;

	struct AllocatorBlock
	{
		char *data = nullptr;
		lang::AtomicInt32 currentOffsetInBytes;
	};
	//AllocatorBlock *allocatorBlock;
	lang::AtomicPointer allocatorBlock;
	uint32_t allocatorBlockSizeInBytes = 0;

	//uint8_t *allocatorBlock = NULL;
	//uint32_t allocatorBlockUsedInBytes = 0;

	// Imp functions don't protect themselves, that is left for the public interface
	//void addFreePointersImp(uint32_t chunkIndex, uint32_t pointerSizeInBytes, void *pointers, uint32_t amount);
	void allocateInternalSpace(uint32_t chunkIndex, uint32_t pointerSizeInBytes);
	void *allocateImp(uint64_t sizeInBytes);
	void deallocateImp(void *pointer, uint64_t sizeInBytes);
	uint32_t getChunkIndex(uint32_t sizeInBytes) const;

	// Not defined
	MultiSizePool(const MultiSizePool &);
	void operator = (const MultiSizePool &);

	void printAllocationDebugData(SizeType id);

public:
	enum Flag
	{
		// Thread safety applies to MultiSizePool itself and it's own use of given allocators.
		// Use thread safe allocators if allocator instances are shared and possibly called from multiple threads.
		FlagThreadSafe = 1,

		// Should memory be tracked and free'd back to given pools.
		FlagFreeMememory = FlagThreadSafe << 1,
	};

	enum GrowMode
	{
		GrowModeInvalid,

		// Each level is grown by alignment
		GrowModeLinear,
		// Generate quarter increments between pow2 values (while meeting alignment)
		GrowModeQuarter,
		// Generate half increments between pow2 values (while meeting alignment)
		GrowModeHalf,
		// Each level is twice the size of the previous one (while meeting alignment)
		GrowModeDouble,
	};

	struct InitParams
	{
		// Allocators
		IBlockAllocator *allocator = nullptr;
		IAllocator *backupAllocator = nullptr;
		IAllocator *oversizeAllocator = nullptr;
		uint32_t maxAllocationSizeInBytes = 0;
		uint32_t alignmentInBytes = 0;
		GrowMode growMode = GrowModeInvalid;

		bool validate() const
		{
			if (!allocator || !maxAllocationSizeInBytes || !alignmentInBytes || growMode == GrowModeInvalid)
				return false;
			return true;
		}
	};

	// Does NOT take ownership of these allocators instances
	MultiSizePool(const InitParams &params, uint32_t flags);
	~MultiSizePool();

	SizeType getBlockSize() const final;
	void *allocate(uint64_t sizeInBytes) final;
	void deallocate(void *pointer, uint64_t sizeInBytes) final;

	virtual uint64_t getCurrentAllocationBytes() const final;

	FB_ADD_GLOBAL_CLASS_MEMORY_OVERLOADS(MultiSizePool)
};

FB_END_PACKAGE1()
