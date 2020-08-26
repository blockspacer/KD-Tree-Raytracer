#ifndef FB_MATH_TRIGOPTIMIZED_H
#define FB_MATH_TRIGOPTIMIZED_H

#include "Vec3.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/Sel.h"
#include <math.h>

FB_PACKAGE1(math)
	namespace imp {
		static const float pi = 3.14159265f;
	}

// Clamp angle to be valid for trig functions below
// Prolly lose a lot of the benefit if naively calling this for everything
static inline float trigClampAngle(float angle)
{
	const float pi = imp::pi;
	return FB_FMOD(angle, pi);
}

// Approximations below work from -PI to PI

static inline float wrapCosToSineAngleImp(float angle)
{
	const float pi = imp::pi;
	const float halfPi = pi * 0.5f;
	angle += halfPi;
	angle = FB_FSEL(angle - pi, angle - pi, angle);

	return angle;
}

static inline float sineUnsafeApprox0(float angle)
{
	const float pi = imp::pi;
    const float B = 4.f / pi;
    const float C = -4.f / (pi*pi);

    float y = B * angle + C * angle * FB_FABS(angle);
	return y;
}

static inline float cosineUnsafeApprox0(float angle)
{
	angle = wrapCosToSineAngleImp(angle);
	return sineUnsafeApprox0(angle);
}

static inline void sineCosineUnsafeApprox0(float angle, float *FB_RESTRICT outSine, float *FB_RESTRICT outCosine)
{
	*outSine = sineUnsafeApprox0(angle);
	*outCosine = cosineUnsafeApprox0(angle);
}

static inline float sineUnsafeApprox1(float angle)
{
	const float pi = imp::pi;
    const float B = 4.f / pi;
    const float C = -4.f / (pi*pi);
	const float P = 0.225f;

	float y = B * angle + C * angle * FB_FABS(angle);
	y = P * (y * FB_FABS(y) - y) + y;

	return y;
}

static inline float cosineUnsafeApprox1(float angle)
{
	angle = wrapCosToSineAngleImp(angle);
	return sineUnsafeApprox1(angle);
}

static inline void sineCosineUnsafeApprox1(float angle, float *FB_RESTRICT outSine, float *FB_RESTRICT outCosine)
{
	*outSine = sineUnsafeApprox1(angle);
	*outCosine = cosineUnsafeApprox1(angle);
}

FB_END_PACKAGE1()

#endif
