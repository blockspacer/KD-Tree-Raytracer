#pragma once

#pragma once
#include "IAllocator.h"
#include "fb/lang/GlobalMemoryOperators.h"

FB_PACKAGE1(memory)

// Simple wrapper for normal (thread safe) heap.
class HeapAllocator: public IAllocator
{
public:
	void *allocate(uint64_t size) final;
	void deallocate(void *pointer, uint64_t size = 0) final;

	FB_ADD_GLOBAL_CLASS_MEMORY_OVERLOADS(HeapAllocator)
};

FB_END_PACKAGE1()
