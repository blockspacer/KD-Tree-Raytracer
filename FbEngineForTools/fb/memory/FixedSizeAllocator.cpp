#include "Precompiled.h"
#include "Config.h"
#include "FixedSizeAllocator.h"
#include "fb/memory/stats/DebugStats.h"
#include "fb/lang/AlignmentFunctions.h"

FB_PACKAGE1(memory)

	FB_STACK_SET_CLASS(FixedSizeAllocator);

	// We use 1 byte before buffer as deletion flag.
	// We could make this 4 bytes to help with alignment but that's users responsibility with separate parameter
	static const int ExtraSpaceNeeded = 1;

	FixedSizeAllocator::FixedSizeAllocator(SizeType allocationAmount, SizeType allocationSize_, SizeType alignment_)
		: allocator(allocationAmount, allocationSize_ + ExtraSpaceNeeded, alignment_)
		, allocationSize(allocationSize_)
		, currentAllocations(0)
	{
		init();
	}

	FixedSizeAllocator::FixedSizeAllocator(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize_, SizeType alignment_)
	:	allocator(memoryBuffer, memoryBufferSize, allocationSize_ + ExtraSpaceNeeded, alignment_)
	,	allocationSize(allocationSize_)
	,	currentAllocations(0)
	{
		init();
	}

	FixedSizeAllocator::~FixedSizeAllocator()
	{
	}

	bool FixedSizeAllocator::isEmpty() const
	{
		if (!currentAllocations)
			return true;

		return false;
	}

	bool FixedSizeAllocator::isFull() const
	{
		if (currentAllocations >= allocator.getMaxAllocationAmount())
			return true;

		return false;
	}

	SizeType FixedSizeAllocator::getMaxAllocationAmount() const
	{
		return allocator.getMaxAllocationAmount();
	}

	SizeType FixedSizeAllocator::getCurrentAllocationAmount() const
	{
		return currentAllocations;
	}

	AllocationPtrType FixedSizeAllocator::getAllocation(SizeType index) const
	{
		return allocator.getAllocation(index);
	}

	AllocationPtrType FixedSizeAllocator::allocate()
	{
		//FB_STACK_METHOD();

		if (isFull())
			return 0;

		// Try to find one from current amount index to optimize "no deletion" case
		char *ptr = getAllocation(currentAllocations);
		if (isFree(ptr))
		{
			reserve(ptr);
			return ptr;
		}

		// If that didn't work out, just do O(n)
		SizeType maxAllocations = getMaxAllocationAmount();
		for (SizeType i = 0; i < maxAllocations; ++i)
		{
			ptr = getAllocation(i);
			if (isFree(ptr))
			{
				reserve(ptr);
				return ptr;
			}
		}

		return 0;
	}

	void FixedSizeAllocator::deallocate(AllocationPtrType ptr)
	{
		if (!ptr)
			return;

		fb_assert(!isFree(ptr));

		--currentAllocations;
		fb_assert(currentAllocations < currentAllocations + 1);
		this->free(ptr);
	}

	bool FixedSizeAllocator::isFree(ConstAllocationPtrType ptr) const
	{
		return *(ptr + allocationSize) != 1;
	}

	bool FixedSizeAllocator::isFree(SizeType index) const
	{
		return isFree(allocator.getAllocation(index));
	}

	void FixedSizeAllocator::init()
	{
		FB_STACK_METHOD();

		// Flag all allocations as free
		SizeType maxAllocations = getMaxAllocationAmount();
		for (SizeType i = 0; i < maxAllocations; ++i)
		{
			char *ptr = getAllocation(i);
			this->free(ptr);
		}
	}

	void FixedSizeAllocator::reserve(char *ptr)
	{
		++currentAllocations;
		fb_assert(currentAllocations <= getMaxAllocationAmount());

		*(ptr + allocationSize) = 1;
	}

	void FixedSizeAllocator::free(AllocationPtrType ptr)
	{
		*(ptr + allocationSize) = 0;
	}

FB_END_PACKAGE1()
