#pragma once

#include "fb/lang/platform/Compiler.h"

#if (FB_COMPILER == FB_MSC)

	#define FB_LIKELY(COND) (COND)
	#define FB_UNLIKELY(COND) (COND)

#elif (FB_COMPILER == FB_ICC)

	#define FB_LIKELY(COND) (COND)
	#define FB_UNLIKELY(COND) (COND)

#elif (FB_COMPILER == FB_CLANG)

	#define FB_LIKELY(COND) (__builtin_expect((COND),1))
	#define FB_UNLIKELY(COND) (__builtin_expect((COND),0))

#elif (FB_COMPILER == FB_GNUC)

	#define FB_LIKELY(COND) (__builtin_expect((COND),1))
	#define FB_UNLIKELY(COND) (__builtin_expect((COND),0))

#else
	#error "Unsupported compiler."
#endif
