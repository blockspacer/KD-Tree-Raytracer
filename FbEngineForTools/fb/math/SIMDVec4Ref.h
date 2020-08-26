#pragma once

#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/ForceInline.h"
#include "fb/math/util/IsFinite.h"
#include "fb/math/util/IsInf.h"
#include "fb/math/util/IsNaN.h"
#include "fb/math/Vec3.h"
#include "fb/math/Vec4.h"

FB_PACKAGE1(math)
// reference implementation
class SIMDVec4Ref
{
public:
	FB_FORCEINLINE SIMDVec4Ref()
	{
	}

	FB_FORCEINLINE SIMDVec4Ref(float x, float y, float z, float w)
		: x(x)
		, y(y)
		, z(z)
		, w(w)
	{
	}

	FB_FORCEINLINE SIMDVec4Ref(const VC3 &vec)
		: x(vec.x)
		, y(vec.y)
		, z(vec.z)
		, w(0.0f)
	{
	}

	FB_FORCEINLINE SIMDVec4Ref(const VC4 &vec)
		: x(vec.x)
		, y(vec.y)
		, z(vec.z)
		, w(vec.w)
	{
	}

	FB_FORCEINLINE SIMDVec4Ref getNormalized() const
	{
		return *this / getLength();
	}

	FB_FORCEINLINE SIMDVec4Ref getNormalizedWithZeroFailsafe(const SIMDVec4Ref &failsafeValue) const
	{
		float len = getLength();
		if (len != 0.0f)
			return *this / len;

		return failsafeValue;
	}

	FB_FORCEINLINE void normalize()
	{
		float len = getLength();
		*this /= len;
	}

	FB_FORCEINLINE void normalizeWithZeroFailsafe(const SIMDVec4Ref &failsafeValue)
	{
		float len = getLength();
		if (len != 0.0f)
			*this /= len;
		else
			*this = failsafeValue;
	}

	FB_FORCEINLINE SIMDVec4Ref getAbs() const
	{
		return SIMDVec4Ref(FB_FABS(x), FB_FABS(y), FB_FABS(z), FB_FABS(w));
	}

	FB_FORCEINLINE SIMDVec4Ref operator-() const
	{
		return SIMDVec4Ref(-x, -y, -z, -w);
	}

	FB_FORCEINLINE SIMDVec4Ref operator+(const SIMDVec4Ref &other) const
	{
		return SIMDVec4Ref(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	FB_FORCEINLINE SIMDVec4Ref operator-(const SIMDVec4Ref &other) const
	{
		return SIMDVec4Ref(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	FB_FORCEINLINE SIMDVec4Ref operator*(const SIMDVec4Ref &other) const
	{
		return SIMDVec4Ref(x * other.x, y * other.y, z * other.z, w * other.w);
	}

	FB_FORCEINLINE SIMDVec4Ref operator/(const SIMDVec4Ref &other) const
	{
		fb_expensive_assert(other.x != 0.0f);
		fb_expensive_assert(other.y != 0.0f);
		fb_expensive_assert(other.z != 0.0f);
		fb_expensive_assert(other.w != 0.0f);
		return SIMDVec4Ref(x / other.x, y / other.y, z / other.z, w / other.w);
	}

	FB_FORCEINLINE SIMDVec4Ref operator*(float value) const
	{
		return SIMDVec4Ref(x * value, y * value, z * value, w * value);
	}

	FB_FORCEINLINE SIMDVec4Ref operator/(float value) const
	{
		fb_expensive_assert(value != 0.0f);
		return SIMDVec4Ref(x / value, y / value, z / value, w / value);
	}

	FB_FORCEINLINE SIMDVec4Ref& operator+=(const SIMDVec4Ref &other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Ref& operator-=(const SIMDVec4Ref &other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Ref& operator*=(const SIMDVec4Ref &other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Ref& operator/=(const SIMDVec4Ref &other)
	{
		fb_expensive_assert(other.x != 0.0f);
		fb_expensive_assert(other.y != 0.0f);
		fb_expensive_assert(other.z != 0.0f);
		fb_expensive_assert(other.w != 0.0f);
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Ref& operator*=(float value)
	{
		x *= value;
		y *= value;
		z *= value;
		w *= value;
		return *this;
	}

	FB_FORCEINLINE SIMDVec4Ref& operator/=(float value)
	{
		fb_assert(value != 0.0f);
		x /= value;
		y /= value;
		z /= value;
		w /= value;
		return *this;
	}

	FB_FORCEINLINE float getLength() const
	{
		return std::sqrt(getSquareLength());
	}

	FB_FORCEINLINE float getSquareLength() const
	{
		return getDotWith(*this);
	}

	FB_FORCEINLINE float getDotWith(const SIMDVec4Ref &other) const
	{
		return (x * other.x + y * other.y) + (z * other.z + w * other.w);
	}

	FB_FORCEINLINE SIMDVec4Ref getVectorDotWith(const SIMDVec4Ref &other) const
	{

		float result = (x * other.x + y * other.y) + (z * other.z + w * other.w);
		return SIMDVec4Ref(result, result, result, result);
	}

	FB_FORCEINLINE void storeTo(float *p) const
	{
		p[0] = x;
		p[1] = y;
		p[2] = z;
		p[3] = w;
	}

	FB_FORCEINLINE void storeTo(math::VC4 &result) const
	{
		result.x = x;
		result.y = y;
		result.z = z;
		result.w = w;
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
		return x < 0.0f || y < 0.0f || z < 0.0f || w < 0.0f;
	}

	FB_FORCEINLINE float operator[](SizeType i) const
	{
		switch (i)
		{
		case 0 :
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		default:
			fb_assert(i <= 3 && "Out of bounds");
			return 0.0f;
		}
	}

	FB_FORCEINLINE VC4 toVC4()
	{
		return VC4(x, y, z, w);
	}

	FB_FORCEINLINE VC3 toVC3()
	{
		return VC3(x, y, z);
	}

	FB_FORCEINLINE static SIMDVec4Ref zero()
	{
		return SIMDVec4Ref();
	}

	FB_FORCEINLINE static SIMDVec4Ref setXYZW(float value)
	{
		return SIMDVec4Ref(value, value, value, value);
	}

	FB_FORCEINLINE static SIMDVec4Ref loadAligned(const float *alignedPointer)
	{
		return SIMDVec4Ref(alignedPointer[0], alignedPointer[1], alignedPointer[2], alignedPointer[3]);
	}

	FB_FORCEINLINE static SIMDVec4Ref loadUnaligned(const float *unalignedPointer)
	{
		return SIMDVec4Ref(unalignedPointer[0], unalignedPointer[1], unalignedPointer[2], unalignedPointer[3]);
	}

	FB_FORCEINLINE static void storeAligned(float *alignedPointer, const SIMDVec4Ref& a)
	{
		alignedPointer[0] = a.x;
		alignedPointer[1] = a.y;
		alignedPointer[2] = a.z;
		alignedPointer[3] = a.w;
	}

	FB_FORCEINLINE static SIMDVec4Ref loadXYXY(const float *alignedPointer)
	{
		return SIMDVec4Ref(alignedPointer[0], alignedPointer[1], alignedPointer[0], alignedPointer[1]);
	}

	FB_FORCEINLINE static SIMDVec4Ref setXYXY(float x, float y)
	{
		return SIMDVec4Ref(x, y, x, y);
	}

	FB_FORCEINLINE static void storeXY(float *alignedPointer, const SIMDVec4Ref& a)
	{
		alignedPointer[0] = a.x;
		alignedPointer[1] = a.y;
	}

	FB_FORCEINLINE static SIMDVec4Ref swapToYXWZ(const SIMDVec4Ref& a)
	{
		return SIMDVec4Ref(a.y, a.x, a.w, a.z);
	}

	FB_FORCEINLINE static SIMDVec4Ref swapToZWXY(const SIMDVec4Ref& a)
	{
		return SIMDVec4Ref(a.z, a.w, a.x, a.y);
	}

	FB_FORCEINLINE static SIMDVec4Ref combineXYAndZW(const SIMDVec4Ref &xy, const SIMDVec4Ref& zw)
	{
		return SIMDVec4Ref(xy.x, xy.y, zw.z, zw.w);
	}

	FB_FORCEINLINE static SIMDVec4Ref min(const SIMDVec4Ref &a, const SIMDVec4Ref& b)
	{
		return SIMDVec4Ref(FB_FMIN(a.x, b.x), FB_FMIN(a.y, b.y), FB_FMIN(a.z, b.z), FB_FMIN(a.w, b.w));
	}

	FB_FORCEINLINE static SIMDVec4Ref max(const SIMDVec4Ref &a, const SIMDVec4Ref& b)
	{
		return SIMDVec4Ref(FB_FMAX(a.x, b.x), FB_FMAX(a.y, b.y), FB_FMAX(a.z, b.z), FB_FMAX(a.w, b.w));
	}

	static const uint32_t alignment = 16;
	static const uint32_t simdAlignmentMask = alignment - 1;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
};

FB_END_PACKAGE1()