#pragma once

/* AllocationDebugger inspects every allocation and does some checks based on other defines */

#define FB_ENABLE_ALLOCATION_DEBUGGER FB_FALSE


/* ALLOCATION_TRACKER uses call stacks to identify points of allocation. Somewhat expensive */
#define FB_ENABLE_ALLOCATION_TRACKER FB_FALSE

/* NAZI_ALLOCATOR returns non-zeroed memory, overwrites freed memory and checks for over and underflows. FIENDISH
 * variation uses VirtualAlloc to protect any freed allocations. Only turn these on locally when needed. 
 *
 * Note: FIENDISH_NAZI_ALLOCATOR uses large amounts of memory.
 */

#define FB_ENABLE_NAZI_ALLOCATOR FB_FALSE
#define FB_ENABLE_FIENDISH_NAZI_ALLOCATOR FB_FALSE

#if FB_ENABLE_NAZI_ALLOCATOR == FB_TRUE || FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
	#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
		/* Ok */
	#elif FB_ENABLE_ALLOCATION_DEBUGGER == FB_FALSE
		#error Must enable FB_ENABLE_ALLOCATION_DEBUGGER for specific allocator debuggers to make sense
	#else 
		#error FB_ENABLE_ALLOCATION_DEBUGGER define is invalid or missing
	#endif
#endif

#if FB_ENABLE_ALLOCATION_DEBUGGER == FB_TRUE
	#if FB_ENABLE_NAZI_ALLOCATOR == FB_FALSE && FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_FALSE && FB_ENABLE_ALLOCATION_TRACKER == FB_FALSE
		#error FB_ENABLE_ALLOCATION_DEBUGGER is FB_TRUE, but no specific allocator debuggers are enabled
	#endif
#endif

/* Use this to disable memory pools. Instead pool allocators will return individual allocations (idea is to enable NAZI 
 * allocator in addition to this). */
/* TODO: Could add separate defines for different types of pools. */
#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_TRUE
	/* Automatically disabled, as they won't be used anyway */
	#define FB_POOL_ALLOCATORS_DISABLED FB_TRUE
#else
	#define FB_POOL_ALLOCATORS_DISABLED FB_FALSE
#endif

/* If set to true, will attempt to use large pages instead of standard 4 kB when it makes sense */
#if FB_EDITOR_ENABLED == FB_TRUE && FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_FALSE && FB_ENGINE_FOR_TOOLS == FB_FALSE
	#define FB_USE_LARGE_PAGES FB_TRUE
#else
	#define FB_USE_LARGE_PAGES FB_FALSE
#endif