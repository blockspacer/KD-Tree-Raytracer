#pragma once

#include "fb/string/StaticString.h"

FB_PACKAGE1(memory)

	/**
	 * This allocator is ment for dynamically splitting a chunk of memory into variable sized memory allocations. 
	 * Maximum amount of allocations is defined during construction. 
	 * It is assumed, that maximum amount of allocations is not "too big", as there are some overhead on keeping the internal structures in order.
	 * This is a simple allocator, but is should perform reasonably given the constraints.
	 */
	class TempVariableAllocator
	{
		// Not defined
		TempVariableAllocator(const TempVariableAllocator &);
		void operator= (const TempVariableAllocator &);

		struct AllocationInfo
		{
			size_t offset;
			size_t size;

			AllocationInfo()
			:	offset(0)
			,	size(0)
			{
			}
		};

		void init();
		void removeFromArray(AllocationInfo *chunks, SizeType bestIndex, SizeType &numChunks);
		void addToArray(AllocationInfo *chunks, SizeType insertIndex, SizeType &numChunks, AllocationInfo value);
		void mergeAllocation(SizeType index);
		void debug();

		SizeType getMaxFreeChunks() const;
		SizeType getAllocationIndex(char *ptr) const;
		SizeType getIndex(AllocationInfo *chunks, SizeType allocations, SizeType offset) const;

	public:
		TempVariableAllocator(SizeType bufferSize, SizeType maxAllocations);
		TempVariableAllocator(void *buffer, SizeType bufferSize, SizeType maxAllocations);
		~TempVariableAllocator();

		bool doesOwnPointer(char *ptr) const;
		char *allocate(SizeType size);
		void deallocate(char *ptr);
		char *reallocate(char *ptr, SizeType size);

		SizeType getAllocationsImp() const { return numAllocations; }
		SizeType getFreeChunksImp() const { return numFreeChunks; }
		SizeType getMaxAllocations() const;

		void setIdString(const DynamicString &idString);
		void setWarnOnOutOfSpace(bool value);
		void printStats();

	private:
		SizeType maxAllocations = 0;
		char *memoryBuffer = nullptr;
		SizeType memoryBufferSize = 0;
		bool freeMemoryBuffer = false;
		bool warnOnOutOfSpace = true;

		// Storage for actual allocations.
		AllocationInfo *allocations;
		SizeType numAllocations;
		// Storage for free lists
		AllocationInfo *freeChunks;
		SizeType numFreeChunks;

		// Static info
		StaticString idString;
	};

FB_END_PACKAGE1()
