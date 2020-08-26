#include "Precompiled.h"
#include "FixedSizeDynamicAllocator.h"

FB_PACKAGE1(memory)

	FB_STACK_SET_CLASS(FixedSizeDynamicAllocator);

	// 2 bytes per allocation overhead

	FixedSizeDynamicAllocator::FixedSizeDynamicAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType alignment)
		: allocator(allocationAmount, allocationSize, alignment, allocationAmount * sizeof(uint16_t))
		, allocationAmount(0)
		, freeList(0)
		, freeListAmount(0)
	{
	}

	FixedSizeDynamicAllocator::FixedSizeDynamicAllocator()
		: allocationAmount(0)
		, freeList(0)
		, freeListAmount(0)
	{
	}

	FixedSizeDynamicAllocator::FixedSizeDynamicAllocator(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize, SizeType alignment)
		: allocator(memoryBuffer, memoryBufferSize, allocationSize, alignment)
		, allocationAmount(0)
		, freeList(0)
		, freeListAmount(0)
	{
	}

	FixedSizeDynamicAllocator::~FixedSizeDynamicAllocator()
	{
	}

	void FixedSizeDynamicAllocator::init(SizeType allocationAmountParam, SizeType allocationSize, SizeType alignment)
	{
		allocator.init(allocationAmountParam, allocationSize, alignment, allocationAmountParam * sizeof(uint16_t));

		freeList = (uint16_t *) allocator.getExtraBuffer();
		fb_assert(allocationAmountParam <= 65535);
	}

	bool FixedSizeDynamicAllocator::isEmpty() const
	{
		return allocationAmount == 0;
	}

	bool FixedSizeDynamicAllocator::isFull() const
	{
		if (allocationAmount == allocator.getMaxAllocationAmount() && freeListAmount == 0)
			return true;

		return false;
	}

	SizeType FixedSizeDynamicAllocator::getMaxAllocationAmount() const
	{
		return allocator.getMaxAllocationAmount();
	}

	bool FixedSizeDynamicAllocator::doesOwnPointer(char *ptr) const
	{
		return allocator.doesOwnPointer(ptr);
	}

	char *FixedSizeDynamicAllocator::allocate()
	{
		if (freeListAmount)
		{
			++allocationAmount;
			--freeListAmount;

			uint16_t index = freeList[freeListAmount ];
			return allocator.getAllocation(index);
		}

		if (isFull())
			return 0;

		return allocator.getAllocation(allocationAmount++);
	}

	void FixedSizeDynamicAllocator::deallocate(char *ptr)
	{
		FB_STACK_METHOD();

		--allocationAmount;
		fb_assert(allocationAmount < allocationAmount + 1);

		freeList[freeListAmount] = uint16_t(allocator.getIndex(ptr));
		++freeListAmount;
	}

FB_END_PACKAGE1()


