#pragma once

#include "MemoryFunctions.h"

#define FB_ADD_GLOBAL_CLASS_MEMORY_OVERLOADS(className) \
	static void *operator new(size_t size) \
	{ \
		return fb::lang::osAllocate(size); \
	} \
	static void operator delete(void *pointer) \
	{ \
		fb::lang::osFree(pointer); \
	} \
	static void operator delete(void *pointer, size_t) \
	{ \
		fb::lang::osFree(pointer); \
	}
