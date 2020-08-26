#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE1(memory)

// Generic allocation interface
class IAllocator
{
public:
	virtual ~IAllocator() {}

	virtual void *allocate(uint64_t size) = 0;
	// Extra free parameter is not used, present to be more easily swappable with block allocator
	virtual void deallocate(void *pointer, uint64_t size = 0) = 0;

	// ToDo: If properly going down this route ...
	//virtual void *allocateAligned(SizeType size, SizeType alignment) = 0;
	//virtual void deallocateAligned(void *pointer, SizeType size = 0) = 0;
	// Return total size of the pointer, >= to allocation size.
	//virtual SizeType getUsablePointerSize(void *pointer) const = 0;
};

FB_END_PACKAGE1()
