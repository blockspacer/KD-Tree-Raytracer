#ifndef FB_MEMORY_MULTIFIXEDSIZEDYNAMICALLOCATOR_H
#define FB_MEMORY_MULTIFIXEDSIZEDYNAMICALLOCATOR_H

#include "fb/memory/detail/SimpleStack.h"
#include "FixedSizeAllocatorBase.h"

FB_PACKAGE1(memory)

	/**
	 * Fixed size allocator for arbitrary amount of instances, which are stored inside given sized allocators.
	 *  - O(n) allocation if deallocating resources, O(1) otherwise.
	 *  - O(1) deallocate.
	 * No additional memory overhead.
	 */
	class MultiFixedSizeDynamicAllocator
	{
	public:
		/// Allocates enough memory for @allocationAmount allocations. 
		/// Exact amount depends on internal overhead and alignment requirements
		MultiFixedSizeDynamicAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType initialAllocators, SizeType alignment = 0);
		MultiFixedSizeDynamicAllocator();
		/// Allocates instances within given buffer. Possible instance amount is not defined as it
		/// depends on internal overhead and alignment.
		~MultiFixedSizeDynamicAllocator();

		void init(SizeType allocationAmount, SizeType allocationSize, SizeType initialAllocators, SizeType freeListReserve, SizeType alignment = 0);

		bool isEmpty() const;
		bool isFull() const;
		SizeType getAllocationSize() const;
		SizeType getAllocationAmount() const;
		SizeType getFreelistSize() const;

		void clear();

		char *allocate();
		void deallocate(char *ptr);

	private:
		void allocatePools(SizeType amount);

		SizeType allocationSize;
		SizeType alignment;
		SizeType reserveAmount;
		SizeType currentAllocations;

		typedef detail::SimpleStack<FixedSizeAllocatorBase *> AllocatorListType;
		AllocatorListType allocatorList;

		typedef detail::SimpleStack<char*> FreeListType;
		FreeListType freeList;
	};

FB_END_PACKAGE1()

#endif
