#pragma once

#if (FB_BUILD == FB_DEBUG)

	#define FB_MEMORY_DEBUG_STATS_ENABLED FB_TRUE
	// Adds breakpoint support for debug stats. This will affect performance.
	#define FB_MEMORY_DEBUG_STATS_SUPPORT_BREAKPOINTS FB_TRUE

#elif (FB_BUILD == FB_RELEASE)
	#define FB_MEMORY_DEBUG_STATS_ENABLED FB_TRUE
	#define FB_MEMORY_DEBUG_STATS_SUPPORT_BREAKPOINTS FB_FALSE
#elif (FB_BUILD == FB_FINAL_RELEASE)
	#define FB_MEMORY_DEBUG_STATS_ENABLED FB_TRUE
	#define FB_MEMORY_DEBUG_STATS_SUPPORT_BREAKPOINTS FB_FALSE
#else
	#error "Unknown build."
#endif
