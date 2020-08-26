#pragma once

#include "fb/lang/platform/Config.h"
#include "fb/lang/platform/FBConstants.h"

#ifndef FB_TRUE
	#error "FB_TRUE define is missing?"
#endif

/* FLT_DECIMAL_DIG and DBL_DECIMAL_DIG are constants that tell the required number of digits for lossless float (or 
 * double) to string to float conversion using %.*g formatting. They are available in C++17, but can be calculated as 
 * below in C++11. Note that this is the number of significant digits, and there's potentially a "-0.000" at the beginning, 
 * or "E-012" or so at the end, so add 6 (7 for double) if calculating needed size of string (or use FB_FLT_PRINTF_G_STR_LEN).
 *
 * Unfortunately, at least MSC can't handle that calculation at compile time, so better just trust that all platforms 
 * have something close enough to IEEE 754 floats and hard code. */

 //#define FB_FLT_DECIMAL_DIG (int(std::ceil(std::numeric_limits<float>::digits * std::log10(2) + 1)))
//#define FB_DBL_DECIMAL_DIG (int(std::ceil(std::numeric_limits<double>::digits * std::log10(2) + 1)))

#ifdef FLT_DECIMAL_DIG
#define FB_FLT_DECIMAL_DIG FLT_DECIMAL_DIG
#else
#define FB_FLT_DECIMAL_DIG 9
#endif
#ifdef DBL_DECIMAL_DIG
#define FB_DBL_DECIMAL_DIG DBL_DECIMAL_DIG
#else
#define FB_DBL_DECIMAL_DIG 17
#endif

/* Max exponent for float is around 127, for double around 1023. */
#define FB_FLT_PRINTF_G_STR_LEN (FB_FLT_DECIMAL_DIG + 6)
#define FB_DBL_PRINTF_G_STR_LEN (FB_DBL_DECIMAL_DIG + 7)

#if (FB_BUILD == FB_DEBUG)

	#define FB_STRING_DEBUG_STATS_ENABLED FB_FALSE

#elif (FB_BUILD == FB_RELEASE)

	#define FB_STRING_DEBUG_STATS_ENABLED FB_FALSE

#elif (FB_BUILD == FB_FINAL_RELEASE)

	#define FB_STRING_DEBUG_STATS_ENABLED FB_FALSE

#else
	#error "Unknown build."
#endif
