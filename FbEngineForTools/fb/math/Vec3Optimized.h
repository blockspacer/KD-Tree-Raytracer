#ifndef FB_MATH_VEC3OPTIMIZED_H
#define FB_MATH_VEC3OPTIMIZED_H

#include "Vec3.h"
#include "fb/lang/platform/FBMath.h"
#include "fb/lang/platform/Sel.h"

FB_PACKAGE1(math)

inline void normalize(VC3 *FB_RESTRICT vec)
{
	float mag = vec->getSquareLength();
	float len = FB_FSQRT(mag);

	float invLen = 1.f / len;
	vec->x *= invLen;
	vec->y *= invLen;
	vec->z *= invLen;

	float selector = len - 0.005f;
	vec->x = FB_FSEL(selector, vec->x, 0.f);
	vec->y = FB_FSEL(selector, vec->y, 0.f);
	vec->z = FB_FSEL(selector, vec->z, 0.f);
}

FB_END_PACKAGE1()

#endif
