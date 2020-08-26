#pragma once

#include <float.h>
#include <math.h>

FB_PACKAGE2(math, util)

static inline bool isNaN(float f)
{
	return _isnan(f) != 0;
}

static inline bool isNaN(double d)
{
	return _isnan(d) != 0;
}

FB_END_PACKAGE2()
