#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE1(memory)

// Interface for allocating memory in fixed-size blocks.
// Allocations of 'size' bytes will be aligned to internal block size.
// Size of the original allocation has to be given as a parameter to free.
class IBlockAllocator
{
public:
	virtual ~IBlockAllocator() {}

	virtual SizeType getBlockSize() const = 0;
	virtual void *allocate(uint64_t size) = 0;
	virtual void deallocate(void *pointer, uint64_t size) = 0;

	/* Returns bytes currently allocated from whatever backing mechanism (e.g. malloc) is used */
	virtual uint64_t getCurrentAllocationBytes() const = 0;
};

FB_END_PACKAGE1()
