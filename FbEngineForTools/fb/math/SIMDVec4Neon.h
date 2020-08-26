#pragma once

#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/ForceInline.h"
#include "fb/math/util/IsFinite.h"
#include "fb/math/Vec3.h"
#include "fb/math/Vec4.h"

#define FB_NEON_ACCURATE_MODE_ENABLED FB_TRUE

FB_PACKAGE1(math)

#if FB_SIMD_NEON == FB_TRUE && FB_SIMD_VEC4_AVAILABLE == FB_TRUE

class SIMDVec4Neon
{
public:
	FB_FORCEINLINE SIMDVec4Neon()
		: vec(vdupq_n_f32(0.0f))
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4Neon(float x, float y, float z, float w)
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
		vec = vsetq_lane_f32(x, vec, 0);
		vec = vsetq_lane_f32(y, vec, 1);
		vec = vsetq_lane_f32(z, vec, 2);
		vec = vsetq_lane_f32(w, vec, 3);
	}

	FB_FORCEINLINE SIMDVec4Neon(const VC3 &src)
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
		vec = vsetq_lane_f32(src.x, vec, 0);
		vec = vsetq_lane_f32(src.y, vec, 1);
		vec = vsetq_lane_f32(src.z, vec, 2);
		vec = vsetq_lane_f32(0.0f, vec, 3);
	}

	/* Apparently, same instruction is used in both aligned and unaligned case */
	FB_FORCEINLINE SIMDVec4Neon(const VC4 &src)
		: vec(vld1q_f32(&src.x))
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4Neon(const float32x4_t &src)
		: vec(src)
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4Neon getNormalized() const
	{
		#if FB_NEON_ACCURATE_MODE_ENABLED == FB_TRUE
			float32x4_t sqrtVec = vsqrtq_f32(getVectorDotWith(*this).vec);
			return SIMDVec4Neon(vdivq_f32(vec, sqrtVec));
		#else
			/* Calculate reciprocal squareRoot and multiply by that */
			float32x4_t recSqrt = vrsqrteq_f32(getVectorDotWith(*this).vec);
			return SIMDVec4Neon(vmulq_f32(vec, recSqrt));
		#endif
	}

	FB_FORCEINLINE SIMDVec4Neon getNormalizedWithZeroFailsafe(const SIMDVec4Neon &failsafeValue) const
	{
		#if FB_NEON_ACCURATE_MODE_ENABLED == FB_TRUE
			SIMDVec4Neon squareLen = getVectorDotWith(*this);
			if (vgetq_lane_f32(squareLen.vec, 0) != 0.0f)
				return SIMDVec4Neon(vdivq_f32(vec, vsqrtq_f32(squareLen.vec)));
			else
				return failsafeValue;
		#else
			SIMDVec4Neon squareLen = getVectorDotWith(*this);
			if (vgetq_lane_f32(squareLen.vec, 0) != 0.0f)
				return SIMDVec4Neon(vmulq_f32(vec, vrsqrteq_f32(squareLen.vec)));
			else
				return failsafeValue;
		#endif
	}

	FB_FORCEINLINE void normalize()
	{
		#if FB_NEON_ACCURATE_MODE_ENABLED == FB_TRUE
			float32x4_t sqrtVec = vsqrtq_f32(getVectorDotWith(*this).vec);
			vec = vdivq_f32(vec, sqrtVec);
		#else
			float32x4_t recSqrt = vrsqrteq_f32(getVectorDotWith(*this).vec);
			vec = vmulq_f32(vec, recSqrt);
		#endif
	}

	FB_FORCEINLINE void normalizeWithZeroFailsafe(const SIMDVec4Neon &failsafeValue)
	{
		#if FB_NEON_ACCURATE_MODE_ENABLED == FB_TRUE
			SIMDVec4Neon squareLen = getVectorDotWith(*this);
			if (vgetq_lane_f32(squareLen.vec, 0) != 0.0f)
				vec = vdivq_f32(vec, vsqrtq_f32(squareLen.vec));
		#else
			SIMDVec4Neon squareLen = getVectorDotWith(*this);
			if (vgetq_lane_f32(squareLen.vec, 0) != 0.0f)
				vec = vmulq_f32(vec, vrsqrteq_f32(squareLen.vec));
			else
				*this = failsafeValue;
		#endif
	}

	FB_FORCEINLINE SIMDVec4Neon getAbs() const
	{
		return SIMDVec4Neon(vabsq_f32(vec));
	}

	FB_FORCEINLINE SIMDVec4Neon operator-() const
	{
		return SIMDVec4Neon(vnegq_f32(vec));
	}

	FB_FORCEINLINE SIMDVec4Neon operator+(const SIMDVec4Neon &other) const
	{
		return SIMDVec4Neon(vaddq_f32(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4Neon operator-(const SIMDVec4Neon &other) const
	{
		return SIMDVec4Neon(vsubq_f32(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4Neon operator*(const SIMDVec4Neon &other) const
	{
		return SIMDVec4Neon(vmulq_f32(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4Neon operator/(const SIMDVec4Neon &other) const
	{
		fb_expensive_assert(other[0] != 0.0f);
		fb_expensive_assert(other[1] != 0.0f);
		fb_expensive_assert(other[2] != 0.0f);
		fb_expensive_assert(other[3] != 0.0f);
		return SIMDVec4Neon(vdivq_f32(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4Neon operator*(float value) const
	{
		return SIMDVec4Neon(vmulq_n_f32(vec, value));
	}

	FB_FORCEINLINE SIMDVec4Neon operator/(float value) const
	{
		fb_expensive_assert(value != 0.0f);
		/* Apparently no vdivq_n_f32 available */
		return SIMDVec4Neon(vdivq_f32(vec, vdupq_n_f32(value)));
	}

	FB_FORCEINLINE SIMDVec4Neon& operator+=(const SIMDVec4Neon &other)
	{
		vec = vaddq_f32(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Neon& operator-=(const SIMDVec4Neon &other)
	{
		vec = vsubq_f32(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Neon& operator*=(const SIMDVec4Neon &other)
	{
		vec = vmulq_f32(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Neon& operator/=(const SIMDVec4Neon &other)
	{
		fb_expensive_assert(other[0] != 0.0f);
		fb_expensive_assert(other[1] != 0.0f);
		fb_expensive_assert(other[2] != 0.0f);
		fb_expensive_assert(other[3] != 0.0f);
		vec = vdivq_f32(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Neon operator*=(float value)
	{
		vec = vmulq_n_f32(vec, value);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Neon operator/=(float value)
	{
		fb_expensive_assert(value != 0.0f);
		/* Apparently no vdivq_n_f32 available */
		vec = vdivq_f32(vec, vdupq_n_f32(value));
		return *this;
	}

	FB_FORCEINLINE float getLength() const
	{
		#if FB_NEON_ACCURATE_MODE_ENABLED == FB_TRUE
			float32x4_t res = vsqrtq_f32(getVectorDotWith(*this).vec);
			return vgetq_lane_f32(res, 0);
		#else
			/* Note that this is really a pretty bad approximation */
			float32x4_t recSqrt = vrsqrteq_f32(getVectorDotWith(*this).vec);
			float32x4_t res = vrecpeq_f32(recSqrt);
			return vgetq_lane_f32(res, 0);
		#endif
	}

	FB_FORCEINLINE float getSquareLength() const
	{
		return getDotWith(*this);
	}

	FB_FORCEINLINE float getDotWith(const SIMDVec4Neon &other) const
	{
		SIMDVec4Neon res = getVectorDotWith(other);
		// Extract the first lane
		return vgetq_lane_f32(res.vec, 0);
	}

	FB_FORCEINLINE SIMDVec4Neon getVectorDotWith(const SIMDVec4Neon &other) const
	{
		// Multiply the two vectors together (c[i] = a[i] * b[i])
		float32x4_t prod = vmulq_f32(vec, other.vec);
		/*
		vrev64q_f32 reverses 32-bit lanes within a 64-bit set,
		thus vrev64q_f32({x, y, z, w}) => {y, x, w, z}.
		*/
		float32x4_t res1 = vaddq_f32(prod, vrev64q_f32(prod));
		/*
		vget_low_f32 and vget_high_f32 from a float32x4_t extracts the lowest and highest lanes.
		Now flip the high and low lanes and add them together with the previous result
		*/
		float32x4_t res2 = vaddq_f32(res1, vcombine_f32(vget_high_f32(res1), vget_low_f32(res1)));
		return res2;
	}

	FB_FORCEINLINE void storeTo(float *p) const
	{
		/* Seems that there's no separate instruction for unaligned store */
		vst1q_f32(p, vec);
	}

	FB_FORCEINLINE void storeTo(math::VC4 &result) const
	{
		/* Seems that there's no separate instruction for unaligned store */
		vst1q_f32(&result.x, vec);
	}
	
	FB_FORCEINLINE bool isFinite() const
	{
		return util::isFinite((*this)[0])
			&& util::isFinite((*this)[1])
			&& util::isFinite((*this)[2])
			&& util::isFinite((*this)[3]);
	}

	FB_FORCEINLINE bool anyLessThanZero() const
	{
		return vminvq_f32(vec) < 0.0f;
	}

	FB_FORCEINLINE float operator[](SizeType i) const
	{
		switch (i)
		{
		case 0:
			return vgetq_lane_f32(vec, 0);
		case 1:
			return vgetq_lane_f32(vec, 1);
		case 2:
			return vgetq_lane_f32(vec, 2);
		case 3:
			return vgetq_lane_f32(vec, 3);
		default:
			fb_assert(0 && "Out of bounds");
			return 0.0f;
		}
	}

	FB_FORCEINLINE VC4 toVC4()
	{
		float temp[4];
		storeTo(temp);
		return VC4(temp[0], temp[1], temp[2], temp[3]);
	}

	FB_FORCEINLINE VC3 toVC3()
	{
		float temp[4];
		storeTo(temp);
		return VC3(temp[0], temp[1], temp[2]);
	}

	FB_FORCEINLINE static SIMDVec4Neon zero()
	{
		return SIMDVec4Neon();
	}

	FB_FORCEINLINE static SIMDVec4Neon setXYZW(float value)
	{
		return SIMDVec4Neon(vdupq_n_f32(value));
	}

	FB_FORCEINLINE static SIMDVec4Neon loadAligned(const float *alignedPointer)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		return SIMDVec4Neon(vld1q_f32(alignedPointer));
	}

	FB_FORCEINLINE static SIMDVec4Neon loadUnaligned(const float *unalignedPointer)
	{
		/* Apparently, same instruction is used in either case */
		return SIMDVec4Neon(vld1q_f32(unalignedPointer));
	}

	FB_FORCEINLINE static void storeAligned(float *alignedPointer, const SIMDVec4Neon& a)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		vst1q_f32(alignedPointer, a.vec);
	}

	FB_FORCEINLINE static SIMDVec4Neon loadXYXY(const float *alignedPointer)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		float32x2_t xy = vld1_f32(alignedPointer);
		return vcombine_f32(xy, xy);
	}

	FB_FORCEINLINE static SIMDVec4Neon setXYXY(float x, float y)
	{
		float32x2_t xy = { x, y };
		return vcombine_f32(xy, xy);
	}

	FB_FORCEINLINE static void storeXY(float *alignedPointer, const SIMDVec4Neon& a)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		float32x2_t xy = vget_low_f32(a.vec);
		vst1_f32(alignedPointer, xy);
	}

	FB_FORCEINLINE static SIMDVec4Neon swapToYXWZ(const SIMDVec4Neon &a)
	{
		return SIMDVec4Neon(vrev64q_f32(a.vec));
	}

	FB_FORCEINLINE static SIMDVec4Neon swapToZWXY(const SIMDVec4Neon &a)
	{
		float32x2_t lo = vget_low_f32(a.vec);
		float32x2_t hi = vget_high_f32(a.vec);
		float32x4_t comb = vcombine_f32(hi, lo);
		return SIMDVec4Neon(comb);
	}

	FB_FORCEINLINE static SIMDVec4Neon combineXYAndZW(const SIMDVec4Neon &xy, const SIMDVec4Neon &zw)
	{
		float32x2_t lo = vget_low_f32(xy.vec);
		float32x2_t hi = vget_high_f32(zw.vec);
		float32x4_t comb = vcombine_f32(lo, hi);
		return SIMDVec4Neon(comb);
	}

	FB_FORCEINLINE static SIMDVec4Neon min(const SIMDVec4Neon &a, const SIMDVec4Neon &b)
	{
		return SIMDVec4Neon(vminq_f32(a.vec, b.vec));
	}

	FB_FORCEINLINE static SIMDVec4Neon max(const SIMDVec4Neon &a, const SIMDVec4Neon &b)
	{
		return SIMDVec4Neon(vmaxq_f32(a.vec, b.vec));
	}

	static const uint32_t alignment = 16;
	static const uint32_t simdAlignmentMask = alignment - 1;

	float32x4_t vec;
};

#endif

FB_END_PACKAGE1()
