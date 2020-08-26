#ifndef FB_MEMORY_FIXEDSIZEALLOCATOR_H
#define FB_MEMORY_FIXEDSIZEALLOCATOR_H

#include "FixedSizeAllocatorBase.h"

FB_PACKAGE1(memory)

	typedef char * AllocationPtrType;
	typedef const char * ConstAllocationPtrType;

	/**
	 * Fixed size allocator inside static memory buffer.
	 *  - O(n) allocation if deallocating resources, O(1) otherwise.
	 *  - O(1) deallocate.
	 * No additional memory overhead.
	 */
	class FixedSizeAllocator
	{
		// Not defined
		FixedSizeAllocator(const FixedSizeAllocator &);
		void operator= (const FixedSizeAllocator &);

	public:
		/// Allocates enough memory for @allocationAmount allocations. 
		/// Exact used memory amount depends on internal overhead and alignment requirements.
		FixedSizeAllocator(SizeType allocationAmount, SizeType allocationSize, SizeType alignment = 0);
		/// Allocates instances within given buffer. Possible instance amount is not defined as it
		/// depends on internal overhead and alignment.
		FixedSizeAllocator(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize, SizeType alignment = 0);
		~FixedSizeAllocator();

		bool isEmpty() const;
		bool isFull() const;

		SizeType getMaxAllocationAmount() const;
		SizeType getCurrentAllocationAmount() const;
		/// Returns any (used or unused) allocation [0, maxAllocations[
		AllocationPtrType getAllocation(SizeType index) const;

		char *allocate();
		void deallocate(char *ptr);

	private:
		bool isFree(SizeType index) const;
		bool isFree(const char *ptr) const;
		void init();

		char *getAllocationImp(int index);
		void reserve(char *ptr);
		void free(AllocationPtrType ptr);

		FixedSizeAllocatorBase allocator;
		SizeType allocationSize;
		SizeType currentAllocations;
	};

FB_END_PACKAGE1()

#endif
