#include "Precompiled.h"
#include "MultiFixedSizeDynamicAllocator.h"

// With the define below, all allocations will be directed to general allocator instead
//#define DISABLE_POOLING

FB_PACKAGE1(memory)

	FB_STACK_SET_CLASS(MultiFixedSizeDynamicAllocator);

	// Default space for allocator pointers (and step size for resizing up)
	static const int MultiFixedSizeDynamicAllocator_DefaultAllocatorReserve = 8;

	MultiFixedSizeDynamicAllocator::MultiFixedSizeDynamicAllocator(SizeType allocationAmount, SizeType allocationSize_, SizeType initialAllocators, SizeType alignment_)
	:	allocationSize(allocationSize_)
	,	alignment(alignment_)
	,	reserveAmount(allocationAmount)
	,	currentAllocations(0)
	,	allocatorList(0)
	,	freeList(0)
	{
		/*
		if (MultiFixedSizeDynamicAllocator_DefaultAllocatorReserve > initialAllocators)
			allocatorList.reserve(MultiFixedSizeDynamicAllocator_DefaultAllocatorReserve);

		allocatePools(initialAllocators);
		*/
		init(allocationAmount, allocationSize_, initialAllocators, 0, alignment_);
	}

	MultiFixedSizeDynamicAllocator::MultiFixedSizeDynamicAllocator()
	:	allocationSize(0)
	,	alignment(0)
	,	reserveAmount(0)
	,	currentAllocations(0)
	,	allocatorList(0)
	,	freeList(0)
	{
	}

	MultiFixedSizeDynamicAllocator::~MultiFixedSizeDynamicAllocator()
	{
		for (SizeType i = 0; i < allocatorList.getSize(); ++i)
		{
			FixedSizeAllocatorBase *a = allocatorList.get(i);
			delete a;
		}
	}

	void MultiFixedSizeDynamicAllocator::clear()
	{
		currentAllocations = 0;
		for (SizeType i = 0; i < allocatorList.getSize(); ++i)
		{
			FixedSizeAllocatorBase *a = allocatorList.get(i);
			delete a;
		}
		allocatorList.clear();
		freeList.clear();
	}

	void MultiFixedSizeDynamicAllocator::init(SizeType allocationAmount, SizeType allocationSize_, SizeType initialAllocators, SizeType freeListReserve, SizeType alignment_)
	{
		allocationSize = allocationSize_;
		alignment = alignment_;
		reserveAmount = allocationAmount;

		if (MultiFixedSizeDynamicAllocator_DefaultAllocatorReserve > initialAllocators)
			allocatorList.reserve(MultiFixedSizeDynamicAllocator_DefaultAllocatorReserve);

		freeList.reserve(freeListReserve);
		allocatePools(initialAllocators);
	}

	bool MultiFixedSizeDynamicAllocator::isEmpty() const
	{ 
		return currentAllocations == 0; 
	}

	bool MultiFixedSizeDynamicAllocator::isFull() const
	{ 
		return false; 
	}

	SizeType MultiFixedSizeDynamicAllocator::getAllocationSize() const
	{ 
		return allocationSize; 
	}

	SizeType MultiFixedSizeDynamicAllocator::getAllocationAmount() const
	{ 
		return currentAllocations; 
	}

	SizeType MultiFixedSizeDynamicAllocator::getFreelistSize() const
	{
		return freeList.getCapacity();
	}

	char *MultiFixedSizeDynamicAllocator::allocate()
	{
		#ifdef DISABLE_POOLING
			++currentAllocations;
			return new char[allocationSize];
		#endif

		if (allocatorList.isEmpty())
			allocatePools(1);

		SizeType pool = currentAllocations / reserveAmount;
		SizeType index = currentAllocations % reserveAmount;
		++currentAllocations;

		if (!freeList.isEmpty())
			return freeList.pop();

		if (pool >= allocatorList.getSize())
			allocatePools(1);

		return allocatorList.get(pool)->getAllocation(index);
	}

	void MultiFixedSizeDynamicAllocator::deallocate(char *ptr)
	{
		--currentAllocations;
		fb_assert(currentAllocations < currentAllocations + 1);

		#ifdef DISABLE_POOLING
			delete[] ptr;
			return;
		#endif

		SizeType s = freeList.getSize();
		SizeType c = freeList.getCapacity();
		if (s == c)
		{
			c = (3 * c) / 2;
			c += 8;
			freeList.reserve(c);
		}

		freeList.push(ptr);
	}

	void MultiFixedSizeDynamicAllocator::allocatePools(SizeType amount)
	{
		#ifdef DISABLE_POOLING
			return;
		#endif

		FB_STACK_METHOD();

		// AllocatorList doesn't know to resize properly without a manual intervention
		SizeType listCapacity = allocatorList.getCapacity();
		SizeType listSize = allocatorList.getSize();
		if (listSize + amount >= listCapacity)
			allocatorList.reserve(listCapacity + MultiFixedSizeDynamicAllocator_DefaultAllocatorReserve);

		for (SizeType i = 0; i < amount; ++i)
		{
			FixedSizeAllocatorBase *a = new FixedSizeAllocatorBase(reserveAmount, allocationSize, alignment);
			allocatorList.push(a);
		}
	}

FB_END_PACKAGE1()
