#pragma once

#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/platform/Compiler.h"
#include "fb/lang/platform/FBConstants.h"

#if (FB_COMPILER == FB_MSC || FB_COMPILER == FB_CLANG)
#define FB_LANG_MEMORYOPERATOR_THROW(...) throw(__VA_ARGS__)
#else
#define FB_LANG_MEMORYOPERATOR_THROW(...)
#endif

// Macros for adding inlined class memory overloads. Somewhat more efficient than using the global versions.
// Single and multithreaded versions have no difference, separated for legacy reasons.

#define FB_ADD_CLASS_MEMORY_OVERLOADS(className) \
	static void *operator new(size_t size, void* ptr) FB_LANG_MEMORYOPERATOR_THROW(...) \
	{ \
		return ptr; \
	} \
	static void *operator new(size_t size) FB_LANG_MEMORYOPERATOR_THROW(...) \
	{ \
		return fb::lang::allocateFixed(sizeof(className)); \
	} \
	static void operator delete(void *pointer) \
	{ \
		fb::lang::freeFixed(pointer, sizeof(className)); \
	} \
	static void operator delete(void *pointer, void *pointer2) \
	{ \
	}

#define FB_ADD_CLASS_MEMORY_OVERLOADS_SINGLETHREAD(className) \
	static void *operator new(size_t size, void* ptr) \
	{ \
		return ptr; \
	} \
	static void *operator new(size_t size) FB_LANG_MEMORYOPERATOR_THROW(...) \
	{ \
		return fb::lang::allocateFixed(sizeof(className)); \
	} \
	static void operator delete(void *pointer) FB_LANG_MEMORYOPERATOR_THROW(...) \
	{ \
		fb::lang::freeFixed(pointer, sizeof(className)); \
	} \
	static void operator delete(void *pointer, void *pointer2) \
	{ \
	}
