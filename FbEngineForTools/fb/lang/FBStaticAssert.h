#pragma once

#define fb_static_assertf(x, m) static_assert(x, m);
#define fb_static_assert(x) static_assert(x, "static_assert: " #x)

// apparently, there are some issues with static asserts getting triggered on gcc even without template specialization.
// thus a separate solution for those asserts.

#if (FB_COMPILER == FB_MSC)
	#define fb_template_specialization_assert(x) fb_static_assert(x)
#elif (FB_COMPILER == FB_ICC)
	#define fb_template_specialization_assert(x) fb_static_assert(x)
#elif FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC
	#include "fb/lang/FBAssert.h"
	#define fb_template_specialization_assert(x) fb_expensive_assert(x)
#else
	#error "Unsupported compiler."
#endif
