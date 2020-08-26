#ifndef FB_MEMORY_FIXEDSIZEDYNAMICALLOCATOR_H
#define FB_MEMORY_FIXEDSIZEDYNAMICALLOCATOR_H

#include "FixedSizeAllocatorBase.h"
#include "fb/memory/detail/SimpleStack.h"
#include "fb/lang/IntTypes.h"

FB_PACKAGE1(memory)

	/**
	 * Fixed size allocator inside static memory buffer.
	 *  - O(1) allocate / deallocate.
	 * Additional memory overhead for deallocation management.
	 */
	class FixedSizeDynamicAllocator
	{
	public:
		FixedSizeDynamicAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType alignment = 0);
		FixedSizeDynamicAllocator();
		FixedSizeDynamicAllocator(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize, SizeType alignment = 0);
		~FixedSizeDynamicAllocator();

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
		SizeType allocationAmount;

		uint16_t *freeList;
		uint16_t freeListAmount;
		//typedef detail::SimpleStack<char *> FreeListType;
		//FreeListType freeList;
	};

FB_END_PACKAGE1()

#endif
