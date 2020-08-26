#pragma once

FB_PACKAGE1(memory)

	template<typename InstanceType, typename FixedSizeAllocatorType>
	class InstancePool
	{
	public:
		inline InstancePool(SizeType instanceAmount, SizeType alignment = 0);
		inline InstancePool(char *memoryBuffer, SizeType memorySize, SizeType alignment = 0);
		inline ~InstancePool();

		inline bool isEmpty() const;

		inline InstanceType *getPointer();
		inline InstanceType *getPointerWithoutDefaultContructor();

		/// Internally calls the destructor of ptr.
		inline void freePointer(InstanceType *ptr);
		
		inline void clear() { allocator.clear(); }

	private:
		FixedSizeAllocatorType allocator;
	};

	#include "InstancePoolInline.h"

FB_END_PACKAGE1()
