#ifndef FB_MEMORY_FIXEDSIZELARGEDYNAMICALLOCATOR_H
#define FB_MEMORY_FIXEDSIZELARGEDYNAMICALLOCATOR_H

#include "FixedSizeAllocatorBase.h"
#include "fb/memory/detail/SimpleStack.h"
#include "fb/lang/IntTypes.h"

FB_PACKAGE1(memory)

	/**
	 * Fixed size allocator inside static memory buffer.
	 *  - O(1) allocate / deallocate.
	 * Additional memory overhead for deallocation management.
	 */
	class FixedSizeLargeDynamicAllocator
	{
	public:
		FixedSizeLargeDynamicAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType alignment = 0);
		FixedSizeLargeDynamicAllocator();
		FixedSizeLargeDynamicAllocator(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize, SizeType alignment = 0);
		~FixedSizeLargeDynamicAllocator();

		void init(SizeType allocationAmount, SizeType allocationSize, SizeType alignment = 0);

		bool isEmpty() const;
		bool isFull() const;
		SizeType getMaxAllocationAmount() const;
		bool doesOwnPointer(char *ptr) const;
		bool isReady() const { return allocator.isReady(); }

		char *allocate();
		void deallocate(char *ptr);

	private:
		FixedSizeAllocatorBase allocator;
		uint32_t allocationAmount;
		uint32_t *freeList;
		uint32_t freeListAmount;
	};

FB_END_PACKAGE1()

#endif
