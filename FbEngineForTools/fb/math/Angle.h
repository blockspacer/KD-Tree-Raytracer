#ifndef FB_MATH_ANGLE_H
#define FB_MATH_ANGLE_H

#include "Constants.h"
#include "fb/lang/platform/FBMath.h"
#include "fb/lang/platform/Sel.h"

FB_PACKAGE1(math)

	// Conversion between radians and angles.
inline float radToDeg(float radians) { return radians * (180.0f / Pi); }
inline float degToRad(float degrees) { return degrees * (Pi / 180.0f); }

// Normalize angle to value between -PI and PI.
inline float normalizeAngle(float angle)
{
	const float offset = Pi * FB_FSEL(angle, 1.0f, -1.0f);
	return FB_FMOD(angle + offset, Pi * 2.0f) - offset;
}

FB_END_PACKAGE1()

#endif
