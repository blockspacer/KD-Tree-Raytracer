#include "Precompiled.h"
#include "FixedSizeLargeDynamicAllocator.h"

FB_PACKAGE1(memory)

	FB_STACK_SET_CLASS(FixedSizeLargeDynamicAllocator);

	// 4 bytes per allocation overhead

	FixedSizeLargeDynamicAllocator::FixedSizeLargeDynamicAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType alignment)
		: allocator(allocationAmount, allocationSize, alignment, allocationAmount * sizeof(uint16_t))
		, allocationAmount(0)
		, freeList(0)
		, freeListAmount(0)
	{
	}

	FixedSizeLargeDynamicAllocator::FixedSizeLargeDynamicAllocator()
		: allocationAmount(0)
		, freeList(0)
		, freeListAmount(0)
	{
	}

	FixedSizeLargeDynamicAllocator::FixedSizeLargeDynamicAllocator(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize, SizeType alignment)
		: allocator(memoryBuffer, memoryBufferSize, allocationSize, alignment)
		, allocationAmount(0)
		, freeList(0)
		, freeListAmount(0)
	{
	}

	FixedSizeLargeDynamicAllocator::~FixedSizeLargeDynamicAllocator()
	{
	}

	void FixedSizeLargeDynamicAllocator::init(SizeType allocationAmountParam, SizeType allocationSize, SizeType alignment)
	{
		allocator.init(allocationAmountParam, allocationSize, alignment, allocationAmountParam * sizeof(uint32_t));
		freeList = (uint32_t *) allocator.getExtraBuffer();
	}

	bool FixedSizeLargeDynamicAllocator::isEmpty() const
	{
		return allocationAmount == 0;
	}

	bool FixedSizeLargeDynamicAllocator::isFull() const
	{
		if (allocationAmount == allocator.getMaxAllocationAmount() && freeListAmount == 0)
			return true;

		return false;
	}

	SizeType FixedSizeLargeDynamicAllocator::getMaxAllocationAmount() const
	{
		return allocator.getMaxAllocationAmount();
	}

	bool FixedSizeLargeDynamicAllocator::doesOwnPointer(char *ptr) const
	{
		return allocator.doesOwnPointer(ptr);
	}

	char *FixedSizeLargeDynamicAllocator::allocate()
	{
		if (freeListAmount)
		{
			++allocationAmount;
			--freeListAmount;

			uint32_t index = freeList[freeListAmount];
			return allocator.getAllocation(index);
		}

		if (isFull())
			return 0;

		return allocator.getAllocation(allocationAmount++);
	}

	void FixedSizeLargeDynamicAllocator::deallocate(char *ptr)
	{
		//FB_STACK_METHOD();

		--allocationAmount;
		fb_assert((int32_t)allocationAmount >= 0);

		freeList[freeListAmount] = allocator.getIndex(ptr);
		++freeListAmount;
	}

FB_END_PACKAGE1()
