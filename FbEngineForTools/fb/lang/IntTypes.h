#pragma once

#include "fb/lang/platform/Platform.h"

// this files gives the include/typedef of the int64_t
// (should rather include this file than use the platform specific ifdefs to typedef it)

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef __int32 int32_t;
	typedef unsigned __int32 uint32_t;
	typedef __int16 int16_t;
	typedef unsigned __int16 uint16_t;
	typedef signed __int8 int8_t;
	typedef unsigned __int8 uint8_t;

	#ifdef _WIN64
		typedef __int64 intptr_t;
	#else
		typedef int intptr_t;
	#endif
#else
	#define __STDC_FORMAT_MACROS 1
	#include <stddef.h>
	#include <inttypes.h>
	#include <stdint.h>
#endif

/* Define format string for uint64_t */
	#define FB_FSU64 "I64u"
	#define FB_FSI64 "I64d"
	#define FB_FSX64 "I64X"
	#define FB_PRIX8 "X"
	#define FB_PRIX16 "X"
	#define FB_PRIX32 "X"
	#define FB_PRIX64 "I64X"

#ifndef FB_FSU64
#error Format string for uint64_t not defined
#endif
