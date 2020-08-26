#pragma once

#include "fb/lang/FBAssert.h"
#include "Config.h"
#include <stdlib.h>

#if (FB_COMPILER == FB_MSC)
#include <math.h>
#pragma warning(push)
/* 4548: Expression before comma has no effect; expected expression with side-effect */
#pragma warning(disable: 4548)
#include <intrin.h>
#pragma warning(pop)
#endif

// NOTE: use these "fb_heap_assert()" macros instead of using the functions directly.

#if (FB_MEMORY_HEAP_ASSERT_ENABLED == FB_TRUE)
	#define fb_heap_assert(p_ptr) fb::memory::checkHeapAddress(p_ptr)
	#define fb_heap_composition_assert(p_owner_ptr, p_component_ptr) fb::memory::checkHeapComposition(p_owner_ptr, p_component_ptr)

	#if (FB_COMPILER == FB_MSC)
		// ok
	#else
		#error "FB_MEMORY_HEAP_ASSERT_ENABLED not supported by the build platform."
	#endif

	#if (FB_BUILD == FB_FINAL_RELEASE)
		// the heap checks are not quite 100% sure, you really should not enable them in the final release!
		#error "Heap asserts enabled for final release. This is probably a mistake."
	#endif

#elif (FB_MEMORY_HEAP_ASSERT_ENABLED == FB_FALSE)
	#define fb_heap_assert(p_ptr) 
	#define fb_heap_composition_assert(p_owner_ptr, p_component_ptr) 
#else
	#error "FB_MEMORY_HEAP_ASSERT_ENABLED define is missing or invalid."
#endif

FB_PACKAGE1(memory)

#if (FB_COMPILER == FB_MSC)
	extern const void *stackMax;
	extern const void *stackMin;
	extern uint32_t mainStackThreadId;

	void initCheckHeapAddress(const void *ptrToStack);
	bool hasMainThreadHeap();

	inline bool isAddressInStack(const void *ptr)
	{
		return (ptr >= stackMin && ptr <= stackMax);
	}

	// Checking that a pointer address is in heap.
	// (Note, this is in no way a 100% safe check, just to catch some obvious cases)
	inline void checkHeapAddress(const void *ptr)
	{
		if (!hasMainThreadHeap()) return;
		if (isAddressInStack(ptr))
		{
			fb_assert(0 && "checkHeapAddress - Pointer appears to point to stack data, rather than pointing to heap data.");
		}
	}

	// Checks that the if the given owner pointer is in heap, then the given component pointer must be in heap too.
	// if the owner pointer is in stack, then the component pointer may be in stack or heap
	// (null is never acceptable.)
	inline void checkHeapComposition(const void *ownerPtr, const void *compositeComponentPtr)
	{
		if (!hasMainThreadHeap()) return;
		if (!isAddressInStack(ownerPtr))
		{
			if (compositeComponentPtr != NULL)
			{
				checkHeapAddress(compositeComponentPtr);
			}
		}
	}

#else
	// Disabled for all other builds than debug on windows platform.
	inline void initCheckHeapAddress(const void *ptrToStack) { /* nop */ }
	// Disabled for all other builds than debug on windows platform.
	inline void checkHeapAddress(const void *ptr) { /* nop */ }
	inline void checkHeapComposition(const void *ownerPtr, const void *compositeComponentPtr) { /* nop */ }
#endif


FB_END_PACKAGE1()
