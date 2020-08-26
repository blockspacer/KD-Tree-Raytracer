#ifndef FB_MATH_CLAMPTOLENGTH_H
#define FB_MATH_CLAMPTOLENGTH_H

#include "fb/lang/platform/FBMath.h" // For FB_FSQRT

FB_PACKAGE1(math)

inline float clampToLength(float v, float length)
{
	return FB_FCLAMP(v, -length, length);
}

inline double clampToLength(double v, float length)
{
	return FB_DCLAMP(v, -double(length), double(length));
}

inline double clampToLength(double v, double length)
{
	return FB_DCLAMP(v, -length, length);
}

inline math::VC2 clampToLength(const math::VC2 &v, float length)
{
	const float sql = v.getSquareLength();
	if (sql <= length * length)
		return v;

	const float l = FB_FSQRT(sql);
	return v * (length / l);
}

inline math::VC3 clampToLength(const math::VC3 &v, float length)
{
	const float sql = v.getSquareLength();
	if (sql <= length * length)
		return v;

	const float l = FB_FSQRT(sql);
	return v * (length / l);
}

FB_END_PACKAGE1()

#endif
