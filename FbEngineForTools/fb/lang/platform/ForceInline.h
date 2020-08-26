#pragma once

#include "fb/lang/platform/Compiler.h"

#if (FB_COMPILER == FB_MSC)
	#define FB_FORCEINLINE __forceinline
	#define FB_NOINLINE __declspec(noinline)

#elif (FB_COMPILER == FB_ICC)

	#define FB_FORCEINLINE __forceinline
	#define FB_NOINLINE 

#elif (FB_COMPILER == FB_CLANG)

	#define FB_FORCEINLINE __inline__
	#define FB_NOINLINE __attribute__ ((noinline))

#elif (FB_COMPILER == FB_GNUC)

	#define FB_FORCEINLINE __inline__
	#define FB_NOINLINE __attribute__ ((noinline))

#else
	#error "Unsupported compiler."
#endif
