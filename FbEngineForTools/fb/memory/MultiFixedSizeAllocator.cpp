#include "Precompiled.h"
#include "MultiFixedSizeAllocator.h"
#include "fb/lang/Types.h"
#include "fb/lang/AlignmentFunctions.h"
#include <string.h>

FB_PACKAGE1(memory)

	FB_STACK_SET_CLASS(MultiFixedSizeAllocator);

	// We add an pool index to each allocation
	static const int ExtraSpaceBytesNeeded = 2;
	// Default space for allocator pointers (and step size for resizing up)
	static const int DefaultAllocatorReserve = 8;

	MultiFixedSizeAllocator::MultiFixedSizeAllocator(SizeType allocations_, SizeType allocationSize_, SizeType initialAllocators, SizeType alignment_)
		: allocators(0)
		, allocationAmount(allocations_)
		, allocationSize(allocationSize_)
		, alignment(alignment_)
		, poolAmount(0)
		, activePool(0)
	{
		FB_STACK_METHOD();

		if (DefaultAllocatorReserve > initialAllocators)
			allocators.reserve(DefaultAllocatorReserve);

		if (initialAllocators)
			allocatePools(initialAllocators);
	}

	MultiFixedSizeAllocator::~MultiFixedSizeAllocator()
	{
		FB_STACK_METHOD();
		for (SizeType i = 0; i < poolAmount; ++i)
		{
			FixedSizeDynamicAllocator *a = allocators.get(i);
			delete a;
		}
	}

	bool MultiFixedSizeAllocator::isEmpty() const
	{
		return false;
	}

	bool MultiFixedSizeAllocator::isFull() const
	{
		return false;
	}

	SizeType MultiFixedSizeAllocator::getAllocationSize() const
	{
		return allocationSize;
	}

	char *MultiFixedSizeAllocator::allocate()
	{
		//FB_STACK_METHOD();

		// Can we find a pointer from current pool?
		FixedSizeDynamicAllocator *a = 0;
		if (!allocators.isEmpty())
		{
			a = allocators.get(activePool);
			if (!a->isFull())
				return getExternalPointer(a->allocate(), activePool);

			// From next pool?
			++activePool;
			if (activePool < poolAmount)
			{
				a = allocators.get(activePool);
				if (!a->isFull())
					return getExternalPointer(a->allocate(), activePool);
			}

			// Sigh, just try every one. Unnecessary looping when full, though.
			activePool = 0;
			for (SizeType i = 0; i < poolAmount; ++i)
			{
				FixedSizeDynamicAllocator *b = allocators.get(i);
				if (!b->isFull())
				{
					activePool = i;
					return getExternalPointer(b->allocate(), activePool);
				}
			}
		}

		// Make space
		allocatePools(1);
		activePool = poolAmount - 1;

		a = allocators.get(activePool);
		return getExternalPointer(a->allocate(), activePool);
	}

	void MultiFixedSizeAllocator::deallocate(char *ptr)
	{
		FB_STACK_METHOD();

		ptr = getInternalPointer(ptr);
		uint16_t poolIndex = 0;
		memcpy(&poolIndex, ptr + allocationSize, sizeof(uint16_t));

		FixedSizeDynamicAllocator *a = allocators.get(poolIndex);
		a->deallocate(ptr);
	}

	void MultiFixedSizeAllocator::allocatePools(SizeType amount)
	{
		FB_STACK_METHOD();
		SizeType newPoolAmount = poolAmount + amount;

		// AllocatorList doesn't know to resize properly without a manual intervention
		SizeType listCapacity = allocators.getCapacity();
		if (newPoolAmount >= listCapacity)
			allocators.reserve(newPoolAmount + DefaultAllocatorReserve);

		for (SizeType i = poolAmount; i < newPoolAmount; ++i)
		{
			FixedSizeDynamicAllocator *a = new FixedSizeDynamicAllocator(allocationAmount, allocationSize + ExtraSpaceBytesNeeded, alignment);
			allocators.push(a);
		}

		poolAmount = newPoolAmount;
	}

	char *MultiFixedSizeAllocator::getExternalPointer(char *ptr, SizeType poolIndex) const
	{
		// Save the pool index
		uint16_t tmpIndex = uint16_t(poolIndex);
		memcpy(ptr + allocationSize, &tmpIndex, sizeof(uint16_t));

		return ptr;
	}

	char *MultiFixedSizeAllocator::getInternalPointer(char *ptr) const
	{
		return ptr + allocationSize;
	}

FB_END_PACKAGE1()
