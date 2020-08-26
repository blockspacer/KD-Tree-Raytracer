#pragma once

#include "fb/lang/platform/Platform.h"

/* Need to define FB_PRINTF_ENABLED early enough to avoid trouble with include order */
#if (FB_BUILD == FB_FINAL_RELEASE)
	#include "fb/lang/platform/PostGlobalConfigRenderer.h"

	// NOTE: Final release shouldn't print out anything (e.g. console platforms might forbid printf or debug data in their requirements)
	#if (FB_NULL_RENDERER == FB_TRUE)
		// HACK: Assuming we are now running dedicated server so allow printf to get errors printed out
		#define FB_PRINTF_ENABLED FB_TRUE
		#include "fb/lang/PrintfHandler.h"
		#define FB_PRINTF(...) (fb::lang::PrintfHandler::getPrintfHandler().doPrintf(__VA_ARGS__))
	#else
		#define FB_PRINTF_ENABLED FB_FALSE
		#define FB_PRINTF(...) ((void)0)
	#endif
#else
	#define FB_PRINTF_ENABLED FB_TRUE
	#include "fb/lang/PrintfHandler.h"
	#define FB_PRINTF(...) (fb::lang::PrintfHandler::getPrintfHandler().doPrintf(__VA_ARGS__))
#endif

#if (FB_BUILD == FB_DEBUG)
	#define FB_DEBUG_PRINTF(...) FB_PRINTF(__VA_ARGS__)
#else
	#define FB_DEBUG_PRINTF(...) ((void)0)
#endif
