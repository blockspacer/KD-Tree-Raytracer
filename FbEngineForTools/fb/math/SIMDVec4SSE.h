#pragma once

#include "fb/math/SIMDVec4Ref.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/ForceInline.h"

/*
 * Xbox One seems to be missing _mm_hadd_ps, _mm_loaddup_pd, _mm_castpd_ps, _mm_castps_pd, _mm_store_sd, 
 *_mm_set1_epi32 and _mm_castsi128_ps, so no SIMDVec4 for Xbox
*/

#if FB_SIMD_SSE == FB_TRUE && FB_SIMD_VEC4_AVAILABLE == FB_TRUE

FB_PACKAGE1(math)

// SSE implementation

class SIMDVec4SSE
{
public:
	FB_FORCEINLINE SIMDVec4SSE()
		: vec(_mm_set1_ps(0.0f))
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4SSE(float x, float y, float z, float w)
		: vec(_mm_setr_ps(x, y, z, w))
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4SSE(const VC3 &vec)
		: vec(_mm_setr_ps(vec.x, vec.y, vec.z, 0.0f))
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4SSE(const VC4 &vec)
		: vec(_mm_loadu_ps(&vec.x))
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	explicit FB_FORCEINLINE SIMDVec4SSE(const __m128 &vec)
		: vec(vec)
	{
		fb_expensive_assert((intptr_t(this) & simdAlignmentMask) == 0);
	}

	FB_FORCEINLINE SIMDVec4SSE getNormalized() const
	{
		/* Calculate reciprocal squareRoot and multiply by that */
		return SIMDVec4SSE(_mm_mul_ps(vec, _mm_rsqrt_ps(getVectorDotWith(*this).vec)));
	}

	FB_FORCEINLINE SIMDVec4SSE getNormalizedWithZeroFailsafe(const SIMDVec4SSE &failsafeValue) const
	{
		SIMDVec4SSE squareLengthVec = getVectorDotWith(*this);
		if (_mm_cvtss_f32(squareLengthVec.vec) != 0.0f)
			return SIMDVec4SSE(_mm_mul_ps(vec, _mm_rsqrt_ps(squareLengthVec.vec)));
		else
			return failsafeValue;
	}

	FB_FORCEINLINE void normalize()
	{
		vec = _mm_mul_ps(vec, _mm_rsqrt_ps(getVectorDotWith(*this).vec));
	}

	FB_FORCEINLINE void normalizeWithZeroFailsafe(const SIMDVec4SSE &failsafeValue)
	{
		SIMDVec4SSE squareLengthVec = getVectorDotWith(*this);
		if (_mm_cvtss_f32(squareLengthVec.vec) != 0.0f)
			vec = _mm_mul_ps(vec, _mm_rsqrt_ps(squareLengthVec.vec));
		else
			*this = failsafeValue;
	}

	FB_FORCEINLINE SIMDVec4SSE getAbs() const
	{
		return SIMDVec4SSE(_mm_andnot_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), vec));
	}

	FB_FORCEINLINE SIMDVec4SSE operator-() const
	{
		const __m128 signMask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
		return SIMDVec4SSE(_mm_xor_ps(signMask, vec));
	}

	FB_FORCEINLINE SIMDVec4SSE operator+(const SIMDVec4SSE &other) const
	{
		return SIMDVec4SSE(_mm_add_ps(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4SSE operator-(const SIMDVec4SSE &other) const
	{
		return SIMDVec4SSE(_mm_sub_ps(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4SSE operator*(const SIMDVec4SSE &other) const
	{
		return SIMDVec4SSE(_mm_mul_ps(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4SSE operator/(const SIMDVec4SSE &other) const
	{
		fb_expensive_assert(other[0] != 0.0f);
		fb_expensive_assert(other[1] != 0.0f);
		fb_expensive_assert(other[2] != 0.0f);
		fb_expensive_assert(other[3] != 0.0f);
		return SIMDVec4SSE(_mm_div_ps(vec, other.vec));
	}

	FB_FORCEINLINE SIMDVec4SSE operator*(float value) const
	{
		return SIMDVec4SSE(_mm_mul_ps(vec, _mm_set1_ps(value)));
	}

	FB_FORCEINLINE SIMDVec4SSE operator/(float value) const
	{
		fb_expensive_assert(value != 0.0f);
		return SIMDVec4SSE(_mm_div_ps(vec, _mm_set1_ps(value)));
	}

	FB_FORCEINLINE SIMDVec4SSE& operator+=(const SIMDVec4SSE &other)
	{
		vec = _mm_add_ps(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4SSE& operator-=(const SIMDVec4SSE &other)
	{
		vec = _mm_sub_ps(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4SSE& operator*=(const SIMDVec4SSE &other)
	{
		vec = _mm_mul_ps(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4SSE& operator/=(const SIMDVec4SSE &other)
	{
		vec = _mm_div_ps(vec, other.vec);
		return *this;
	}

	FB_FORCEINLINE SIMDVec4SSE operator*=(float value)
	{
		vec = _mm_mul_ps(vec, _mm_set1_ps(value));
		return *this;
	}

	FB_FORCEINLINE SIMDVec4SSE operator/=(float value)
	{
		fb_expensive_assert(value != 0.0f);
		vec = _mm_div_ps(vec, _mm_set1_ps(value));
		return *this;
	}

	FB_FORCEINLINE float getLength() const
	{
		/* This calculates length using reciprocal square root (_mm_rsqrt_ss) and _mm_rcp_ss. Both are approximate 
		 * with "maximum relative error [of] less than 1.5*2^-12" */
		return _mm_cvtss_f32(_mm_rcp_ss(_mm_rsqrt_ss(getVectorDotWith(*this).vec)));
		/* More accurate version would be this, but the above one is vastly faster on Jaguar (like 4 vs. 16 cycles). 
		 * Difference is smaller (around 8 vs. 14 cycles) on modern Intel, as reciprocals are slower. Throughput is 
		 * much better with above version (1 / cycle vs. 3 (Skylake) to 21 (Jaguar) cycles per iteration) */
		//return _mm_cvtss_f32(_mm_sqrt_ss(getVectorDotWith(*this).vec));
	}

	FB_FORCEINLINE float getSquareLength() const
	{
		return _mm_cvtss_f32(getVectorDotWith(*this).vec);
	}

	FB_FORCEINLINE float getDotWith(const SIMDVec4SSE &other) const
	{
		return _mm_cvtss_f32(getVectorDotWith(other).vec);;
	}

	FB_FORCEINLINE SIMDVec4SSE getVectorDotWith(const SIMDVec4SSE &other) const
	{
		/* With SSE4 we could do
		 *     return _mm_dp_ps(vec, other.vec, 0xFF);
		 * but that's actually slower
		 */
		__m128 r1 = _mm_mul_ps(vec, other.vec);
		__m128 r2 = _mm_hadd_ps(r1, r1);
		return SIMDVec4SSE(_mm_hadd_ps(r2, r2));
	}

	FB_FORCEINLINE void storeTo(float *p) const
	{
		_mm_storeu_ps(p, vec);
	}

	FB_FORCEINLINE void storeTo(math::VC4 &result) const
	{
		_mm_storeu_ps(&result.x, vec);
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
		int mask = _mm_movemask_ps(_mm_cmplt_ps(vec, _mm_setzero_ps()));
		return mask != 0;
	}

	FB_FORCEINLINE float operator[](SizeType i) const
	{
		fb_assert(i <= 3 && "Out of bounds");
		float temp[4];
		storeTo(temp);
		return temp[i & 3];
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

	FB_FORCEINLINE static SIMDVec4SSE zero()
	{
		return SIMDVec4SSE();
	}

	FB_FORCEINLINE static SIMDVec4SSE setXYZW(float value)
	{
		return SIMDVec4SSE(_mm_set1_ps(value));
	}

	FB_FORCEINLINE static SIMDVec4SSE loadAligned(const float *alignedPointer)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		return SIMDVec4SSE(_mm_load_ps(alignedPointer));
	}

	FB_FORCEINLINE static SIMDVec4SSE loadUnaligned(const float *unalignedPointer)
	{
		return SIMDVec4SSE(_mm_loadu_ps(unalignedPointer));
	}

	FB_FORCEINLINE static void storeAligned(float *alignedPointer, const SIMDVec4SSE& a)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		_mm_store_ps(alignedPointer, a.vec);
	}

	FB_FORCEINLINE static SIMDVec4SSE loadXYXY(const float *alignedPointer)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		return SIMDVec4SSE(_mm_castpd_ps(_mm_loaddup_pd((const double*)alignedPointer)));
	}

	FB_FORCEINLINE static SIMDVec4SSE setXYXY(float x, float y)
	{
		return SIMDVec4SSE(_mm_setr_ps(x, y, x, y));
	}

	FB_FORCEINLINE static void storeXY(float *alignedPointer, const SIMDVec4SSE& a)
	{
		fb_expensive_assert((intptr_t(alignedPointer) & simdAlignmentMask) == 0);
		_mm_store_sd((double*)alignedPointer, _mm_castps_pd(a.vec));
	}

	FB_FORCEINLINE static SIMDVec4SSE swapToYXWZ(const SIMDVec4SSE &a)
	{
		return SIMDVec4SSE(_mm_shuffle_ps(a.vec, a.vec, _MM_SHUFFLE(2, 3, 0, 1)));
	}

	FB_FORCEINLINE static SIMDVec4SSE swapToZWXY(const SIMDVec4SSE &a)
	{
		return SIMDVec4SSE(_mm_shuffle_ps(a.vec, a.vec, _MM_SHUFFLE(1, 0, 3, 2)));
	}

	FB_FORCEINLINE static SIMDVec4SSE combineXYAndZW(const SIMDVec4SSE &xy, const SIMDVec4SSE &zw)
	{
		return SIMDVec4SSE(_mm_shuffle_ps(xy.vec, zw.vec, _MM_SHUFFLE(3, 2, 1, 0)));
	}

	FB_FORCEINLINE static SIMDVec4SSE min(const SIMDVec4SSE &a, const SIMDVec4SSE &b)
	{
		return SIMDVec4SSE(_mm_min_ps(a.vec, b.vec));
	}

	FB_FORCEINLINE static SIMDVec4SSE max(const SIMDVec4SSE &a, const SIMDVec4SSE &b)
	{
		return SIMDVec4SSE(_mm_max_ps(a.vec, b.vec));
	}

	static const uint32_t alignment = 16;
	static const uint32_t simdAlignmentMask = alignment - 1;

	__m128 vec;
};

#endif

FB_END_PACKAGE1()