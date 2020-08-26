#pragma once

#include "fb/lang/platform/FBConstants.h"

#if defined(__clang__)
	#undef FB_COMPILER
	#define FB_COMPILER FB_CLANG
	#define FB_COMPILER_SUPPORTS_CPP11 FB_TRUE
#endif

#if defined(__GNUC__) && !defined(__clang__)
	#define FB_COMPILER FB_GNUC
	#define FB_COMPILER_SUPPORTS_CPP11 FB_FALSE
#endif

#if defined(_MSC_VER) && !defined(__clang__)
	#define FB_COMPILER FB_MSC
	#if (_MSC_VER >= 1700) 
		#define FB_COMPILER_SUPPORTS_CPP11 FB_TRUE
	#else
		#define FB_COMPILER_SUPPORTS_CPP11 FB_FALSE
	#endif
	#if _MSC_VER >= 1910
		/* Please remove this once we move to 2017 (or newer) */
		#define FB_VS2017_IN_USE FB_TRUE
	#else
		#define FB_VS2017_IN_USE FB_FALSE
	#endif
#endif

#ifdef __INTEL_COMPILER
	#undef FB_COMPILER
	#define FB_COMPILER FB_ICC
	// TODO: version detection, newer version support VC11 at least partly
	#ifndef FB_COMPILER_SUPPORTS_CPP11 // MSVC sets this already?
		#define FB_COMPILER_SUPPORTS_CPP11 FB_TRUE
	#endif
#endif

#ifdef __APPLE__
	#undef FB_COMPILER
	#define FB_COMPILER FB_CLANG
#endif
