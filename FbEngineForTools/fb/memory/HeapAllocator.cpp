#include "Precompiled.h"
#include "HeapAllocator.h"

#include "fb/lang/MemoryFunctions.h"

FB_PACKAGE1(memory)

void *HeapAllocator::allocate(uint64_t size)
{
	return lang::osAllocate(size);
}

void HeapAllocator::deallocate(void *pointer, uint64_t)
{
	lang::osFree(pointer);
}

FB_END_PACKAGE1()
