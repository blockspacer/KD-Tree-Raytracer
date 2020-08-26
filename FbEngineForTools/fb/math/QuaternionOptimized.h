#ifndef FB_MATH_QUATERNIONOPTIMIZED_H
#define FB_MATH_QUATERNIONOPTIMIZED_H

// Separate file to avoid recompile-everything cycles

#include "fb/math/Angle.h"
#include "fb/math/Quaternion.h"
#include "fb/math/Vec3.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/Sel.h"

FB_PACKAGE1(math)

	inline void makeFromAxisRotationSinCos(QUAT *FB_RESTRICT result, const VC3 *FB_RESTRICT axis, float sinHalfAngle, float cosHalfAngle)
	{
		//const float deg = angle * 0.5f;
		//const float cs = FB_FSIN(deg);
		//result->w = FB_FCOS(deg);
		result->w = cosHalfAngle;
		result->x = sinHalfAngle * axis->x;
		result->y = sinHalfAngle * axis->y;
		result->z = sinHalfAngle * axis->z;
	}

	inline void makeFromAxisRotation(QUAT *FB_RESTRICT result, const VC3 *FB_RESTRICT axis, float angle)
	{
#if (FB_LEGACY_QUATERNION_ANGLES == FB_FALSE)
		const float deg = -angle * 0.5f;
#else
		const float deg = angle * 0.5f;
#endif
		const float cs = FB_FSIN(deg);
		result->w = FB_FCOS(deg);
		result->x = cs * axis->x;
		result->y = cs * axis->y;
		result->z = cs * axis->z;
	}

	inline void rotateTowards(QUAT *FB_RESTRICT result, const VC3 *FB_RESTRICT a, const VC3 *FB_RESTRICT b)
	{
		VC3 axis = a->getCrossWith(*b);
		float dot = a->getDotWith(*b);

		float selector = 0.01f - dot;
		float x = FB_FSEL(selector, axis.x, 0.f);
		float y = FB_FSEL(selector, axis.y, 0.f);
		float z = FB_FSEL(selector, axis.z, 0.f);
		float w = FB_FSEL(selector, dot + 1.f, 1.f);

		float mag = x*x + y*y + z*z + w*w;
		float nor = FB_FSQRT(mag);
		float inorm = 1.f / FB_FSEL(nor - 0.01f, nor, 1.f);

		result->x = x * inorm;
		result->y = y * inorm;
		result->z = z * inorm;
		result->w = w * inorm;
	}

	inline void slerpWith(QUAT *FB_RESTRICT a, const QUAT *FB_RESTRICT other, float interpolation)
	{
		float ox = other->x;
		float oy = other->y;
		float oz = other->z;
		float ow = other->w;

		// Compute dot product (equal to cosine of the angle between quaternions)
		float fCosTheta=a->x*ox + a->y*oy + a->z*oz + a->w*ow;

		// Check angle to see if quaternions are in opposite hemispheres
		ox = FB_FSEL(fCosTheta, ox, -ox);
		oy = FB_FSEL(fCosTheta, oy, -oy);
		oz = FB_FSEL(fCosTheta, oz, -oz);
		ow = FB_FSEL(fCosTheta, ow, -ow);
		fCosTheta = FB_FSEL(fCosTheta, fCosTheta, -fCosTheta);

		/*
		// Set factors to do linear interpolation, as a special case where the
		// quaternions are close together.
		A fBeta=(A)1-interpolation;
    
		// If the quaternions aren't close, proceed with spherical interpolation
		if ((A)1-fCosTheta>(A)0.001) 
		{   
	        A fTheta=(A)acos(fCosTheta);
		    fBeta=(A)sin(fTheta*fBeta)/(A)sin(fTheta);
			interpolation=(A)sin(fTheta*interpolation)/(A)sin(fTheta);
		}
		*/
		float fBeta = 1.f - interpolation;
		float fTheta = FB_FACOS(fCosTheta);
		float fThetaSin = FB_FSIN(fTheta);
		float fThetaSinInv = 1.f / fThetaSin;
		fThetaSinInv = FB_FSEL(FB_FABS(fThetaSin) - 0.001f, fThetaSinInv, 0.f);
		//fBeta = FB_FSIN(fTheta*fBeta) * fThetaSinInv;
		//interpolation = FB_FSIN(fTheta*interpolation) * fThetaSinInv;
		
		float selector = 1.f - fCosTheta - 0.0001f;
		fBeta = FB_FSEL(selector, FB_FSIN(fTheta*fBeta) * fThetaSinInv, fBeta);
		interpolation = FB_FSEL(selector, FB_FSIN(fTheta*interpolation) * fThetaSinInv, interpolation);

		a->x = fBeta*a->x + interpolation*ox;
		a->y = fBeta*a->y + interpolation*oy;
		a->z = fBeta*a->z + interpolation*oz;
		a->w = fBeta*a->w + interpolation*ow;
	}

	inline void normalize(QUAT *FB_RESTRICT q)
	{
		float mag = q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w;
		float len = FB_FSQRT(mag);

		// Should prolly change invLen to 1 if length is ~0. Reverting to identity shouldn't  be needed in optimised versions ..

		float invLen = 1.f / len;
		q->x *= invLen;
		q->y *= invLen;
		q->z *= invLen;
		q->w *= invLen;

		float selector = len - 0.005f;
		q->x = FB_FSEL(selector, q->x, 0.f);
		q->y = FB_FSEL(selector, q->y, 0.f);
		q->z = FB_FSEL(selector, q->z, 0.f);
		q->w = FB_FSEL(selector, q->w, 1.f);
	}

	inline void normalizeApprox(QUAT *FB_RESTRICT q)
	{
		float mag = q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w;

		#ifdef FB_FRESSQRTE
			float invLen = FB_FRESSQRTE(mag);
		#else
			float invLen = 1.f / FB_FSQRT(mag);
		#endif

		invLen = FB_FSEL(mag - 0.0001, invLen, 1.f);
		q->x *= invLen;
		q->y *= invLen;
		q->z *= invLen;
		q->w *= invLen;
	}

	inline void lerp(QUAT *FB_RESTRICT a, const QUAT *FB_RESTRICT other, float interpolation)
	{
		float i0 = 1.f - interpolation;
		a->x = i0*a->x + interpolation*other->x;
		a->y = i0*a->y + interpolation*other->y;
		a->z = i0*a->z + interpolation*other->z;
		a->w = i0*a->w + interpolation*other->w;
		normalize(a);
	}

	inline void lerpApprox(QUAT *FB_RESTRICT a, const QUAT *FB_RESTRICT other, float interpolation)
	{
		float i0 = 1.f - interpolation;
		a->x = i0*a->x + interpolation*other->x;
		a->y = i0*a->y + interpolation*other->y;
		a->z = i0*a->z + interpolation*other->z;
		a->w = i0*a->w + interpolation*other->w;
		normalizeApprox(a);
	}

	inline void calculateTwist(QUAT *FB_RESTRICT dest, const QUAT *FB_RESTRICT src, const VC3 *FB_RESTRICT axis)
	{
		float d = src->x * axis->x + src->y * axis->y + src->z * axis->z;
		float s = FB_FSEL(d, 1.0f, -1.0f);
		d *= s;

		dest->x = axis->x * d;
		dest->y = axis->y * d;
		dest->z = axis->z * d;
		dest->w = src->w * s;

		normalize(dest);
	}

	inline float getAngle(QUAT *FB_RESTRICT src)
	{
#if (FB_LEGACY_QUATERNION_ANGLES == FB_FALSE)
		const float angle = -FB_FACOS(src->w) * 2.0f;
#else
		const float angle = FB_FACOS(src->w) * 2.0f;
#endif

		return normalizeAngle(angle);
	}

FB_END_PACKAGE1()

#endif
