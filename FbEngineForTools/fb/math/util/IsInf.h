#pragma once

#include <float.h>
#include <math.h>

FB_PACKAGE2(math, util)

static inline bool isInf(float f)
{
	return _finite(f) == 0 && _isnan(f) == 0;
}

static inline bool isInf(double d)
{
	return _finite(d) == 0 && _isnan(d) == 0;
}

FB_END_PACKAGE2()
