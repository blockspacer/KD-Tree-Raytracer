#pragma once

#include "fb/lang/platform/Compiler.h"

#if (FB_COMPILER == FB_MSC)
	#define FB_PREFETCH(POINTER) 
#elif (FB_COMPILER == FB_ICC)
	#define FB_PREFETCH(POINTER)
#elif (FB_COMPILER == FB_CLANG)
	#define FB_PREFETCH(POINTER)
#elif (FB_COMPILER == FB_GNUC)
	#define FB_PREFETCH(POINTER)
#else
	#error "Unsupported compiler."
#endif
