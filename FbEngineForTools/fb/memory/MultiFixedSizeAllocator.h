#ifndef FB_MEMORY_MULTIFIXEDSIZEALLOCATOR_H
#define FB_MEMORY_MULTIFIXEDSIZEALLOCATOR_H

#include "FixedSizeDynamicAllocator.h"
#include "fb/memory/detail/SimpleStack.h"

FB_PACKAGE1(memory)

	/**
	 * Fixed size allocator for arbitrary amount of instances, which are stored inside given sized allocators.
	 *  - O(n) allocation if deallocating resources, O(1) otherwise.
	 *  - O(1) deallocate.
	 * No additional memory overhead.
	 */
	class MultiFixedSizeAllocator
	{
	public:
		/// Allocates enough memory using several allocators containing @allocationAmount allocations.
		MultiFixedSizeAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType initialAllocatorAmount, SizeType alignment = 0);
		~MultiFixedSizeAllocator();

		bool isEmpty() const;
		bool isFull() const;
		SizeType getAllocationSize() const;

		char *allocate();
		void deallocate(char *ptr);

	private:
		void allocatePools(SizeType amount);
		char *getExternalPointer(char * ptr, SizeType poolIndex) const;
		char *getInternalPointer(char * ptr) const;

		typedef detail::SimpleStack<FixedSizeDynamicAllocator *> AllocatorListType;
		AllocatorListType allocators;

		SizeType allocationAmount;
		SizeType allocationSize;
		SizeType alignment;
		SizeType poolAmount;
		SizeType activePool;
	};

FB_END_PACKAGE1()

#endif
