#pragma once

#include "IBlockAllocator.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/ScopedPointer.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/GlobalMemoryOperators.h"

FB_PACKAGE1(memory)

// Allocator to provide big (blockSize, think 'megabytes') chunks of memory to sub-allocators.
// Should be initialised close'ish to estimated memory usage.
// Does NOT release memory.
class SystemAllocator: public IBlockAllocator
{
	Mutex mutex;
	uint8_t *cachedPointer = nullptr;
	uint64_t cachedTotalMemoryAmount = 0;
	uint64_t cachedUsedMemoryAmount = 0;
	lang::AtomicUInt64 totalSizeAllocated;
	uint32_t flags = 0;

	void *allocateImp(uint64_t size);

	// Not defined
	SystemAllocator(const SystemAllocator &);
	void operator = (const SystemAllocator &);

public:
	enum Flag
	{
		FlagAllocateOverInitialLimit = 1,
		FlagThreadSafe = FlagAllocateOverInitialLimit << 1,

		FlagDefault = FlagAllocateOverInitialLimit | FlagThreadSafe,
	};

	SystemAllocator(uint64_t initialMemoryAmount, uint32_t flags);
	~SystemAllocator();

	virtual SizeType getBlockSize() const final;
	virtual void *allocate(uint64_t size) final;
	virtual void deallocate(void *pointer, uint64_t size) final;

	virtual uint64_t getCurrentAllocationBytes() const final;

	FB_ADD_GLOBAL_CLASS_MEMORY_OVERLOADS(SystemAllocator)
};

FB_END_PACKAGE1()
