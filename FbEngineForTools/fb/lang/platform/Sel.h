#pragma once

#include "fb/lang/platform/Compiler.h"

#if (FB_COMPILER == FB_MSC)

	#define FB_FSEL(a, b, c) (a >= 0.f ? b : c)
	#define FB_DSEL(a, b, c) (a >= 0.0 ? b : c)

#elif (FB_COMPILER == FB_ICC)

	#define FB_FSEL(a, b, c) (a >= 0.f ? b : c)
	#define FB_DSEL(a, b, c) (a >= 0.0 ? b : c)

#elif (FB_COMPILER == FB_CLANG)

	#define FB_FSEL(a, b, c) (a >= 0.f ? b : c)
	#define FB_DSEL(a, b, c) (a >= 0.0 ? b : c)

#elif (FB_COMPILER == FB_GNUC)

	#define FB_FSEL(a, b, c) (a >= 0.f ? b : c)
	#define FB_DSEL(a, b, c) (a >= 0.0 ? b : c)

#else
	#error "Unsupported compiler."
#endif
