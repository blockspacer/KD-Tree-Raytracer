#ifndef FB_MATH_UTIL_ISFINITE_H
#define FB_MATH_UTIL_ISFINITE_H

#include <float.h>
#include <math.h>

FB_PACKAGE2(math, util)

/**
 * Neither infinity nor NaN
 */

static inline bool isFinite(float f)
{
	return _finite(f) != 0;
}

static inline bool isFinite(double d)
{
	return _finite(d) != 0;
}

FB_END_PACKAGE2()

#endif