#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE1(memory)

	/**
	 * Fixed size allocator inside static memory buffer.
	 *  - O(n) allocation if deallocating resources, O(1) otherwise.
	 *  - O(1) deallocate.
	 * No additional memory overhead.
	 */
	class FixedSizeAllocatorBase
	{
		// Not defined
		FixedSizeAllocatorBase(const FixedSizeAllocatorBase &);
		void operator= (const FixedSizeAllocatorBase &);

	public:
		/// Allocates enough memory for @allocationAmount allocations. 
		/// Exact used memory amount depends on internal overhead and alignment requirements.
		FixedSizeAllocatorBase(SizeType allocationAmount, SizeType allocationSize, SizeType alignment = 0, SizeType extraBufferSize = 0);
		/// Allocates instances within given buffer. Possible instance amount is not defined as it
		/// depends on internal overhead and alignment.
		FixedSizeAllocatorBase(char *memoryBuffer, SizeType memoryBufferSize, SizeType allocationSize, SizeType alignment = 0);
		FixedSizeAllocatorBase();
		~FixedSizeAllocatorBase();

		void init(SizeType allocationAmount, SizeType allocationSize, SizeType alignment = 0, SizeType extraBufferSize = 0);

		SizeType getMaxAllocationAmount() const;
		char *getAllocation(SizeType index) const;

		char *getExtraBuffer() const;
		SizeType getIndex(char *ptr) const;
		bool doesOwnPointer(char *ptr) const;

		bool isReady() const { return memoryBuffer && allocationSize; }

	private:
		char *memoryBuffer;
		SizeType memoryBufferSize;
		SizeType allocationSize;
		SizeType extraBufferSize;
		char freeBuffer;
	};

FB_END_PACKAGE1()
