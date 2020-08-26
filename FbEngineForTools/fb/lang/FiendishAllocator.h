#pragma once

FB_PACKAGE1(lang)

/**
 * Use FB_ENABLE_FIENDISH_NAZI_ALLOCATOR in MemoryOperatorsConfig.h to enable this for all allocations.

 * FiendishAllocator does it's best to mess with code that tries to use already freed memory. Freed memory is held on
 * to and protected for a while longer (depends frequency and size of other allocations), so any access will instantly
 * fault.
 *
 * For full debugging experience, also enable FB_ENABLE_NAZI_ALLOCATOR, as it will better detect small under and
 * overflows and assumptions about zeroed memory.
 */

class FiendishAllocator
{
	FiendishAllocator();
	~FiendishAllocator();

public:
	static FiendishAllocator &getInstance();

	void *allocate(uint64_t sizeInBytes);
	void *reallocate(void *oldPointer, uint64_t newSizeInBytes);
	void free(void *pointer);

	uint64_t getCurrentAllocationBytes();

private:
	class Impl;
	Impl &impl;

	static Impl &getImplInstance();
};

FB_END_PACKAGE1()
