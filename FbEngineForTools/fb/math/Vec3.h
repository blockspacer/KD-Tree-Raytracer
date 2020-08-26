#pragma once

#undef max
#undef min


#include "fb/lang/FBAssert.h"
#include "fb/lang/platform/Sel.h"

#include <cmath>

FB_DECLARE0(HeapString)

FB_PACKAGE1(math)

template <class A>
class Vec3
{
public:
	static const Vec3 zero;

	typedef A ValueType;

	// Data (public for speed)
	union
	{
		A v[3];
		struct
		{
			A x,y,z;
		};
	};

	// Constructors
	FB_FORCEINLINE Vec3() : x(0),y(0),z(0) {};
	FB_FORCEINLINE Vec3(A _x,A _y,A _z) : x(_x),y(_y),z(_z) {};
	FB_FORCEINLINE explicit Vec3(const A a[3]) : x(a[0]),y(a[1]),z(a[2]) {};

	// Normalization functions only declared for float and double.
	Vec3<A> getNormalized() const;
	Vec3<A> getNormalizedWithZeroFailsafe(const Vec3<A> &failsafeValue) const;
	void normalize();
	void normalizeWithZeroFailsafe(const Vec3<A> &failsafeValue);

	// Operators
	Vec3<A> operator-() const
	{
		return Vec3<A>(-x,-y,-z);
	}
	
	Vec3<A> operator+(const Vec3<A>& other) const
	{
		return Vec3<A>(x+other.x,y+other.y,z+other.z);
	}

	Vec3<A> operator-(const Vec3<A>& other) const
	{
		return Vec3<A>(x-other.x,y-other.y,z-other.z);
	}
	
	Vec3<A> operator*(const Vec3<A>& other) const
	{
		return Vec3<A>(x*other.x,y*other.y,z*other.z);
	}

	Vec3<A> operator/(const Vec3<A>& other) const
	{
		fb_expensive_assert(other.x != A(0));
		fb_expensive_assert(other.y != A(0));
		fb_expensive_assert(other.z != A(0));

		return Vec3<A>(x/other.x,y/other.y,z/other.z);
	}

	Vec3<A> operator*(A num) const
	{
		return Vec3<A>(x*num,y*num,z*num);
	}

	Vec3<A> operator/(A num) const
	{
		fb_expensive_assert(num != A(0));

		return Vec3<A>(x/num,y/num,z/num);
	}
	
	void operator+=(const Vec3<A>& other)
	{
		x+=other.x;
		y+=other.y;
		z+=other.z;
	}

	void operator-=(const Vec3<A>& other)
	{
		x-=other.x;
		y-=other.y;
		z-=other.z;
	}

	void operator*=(const Vec3<A>& other)
	{
		x*=other.x;
		y*=other.y;
		z*=other.z;
	}

	void operator/=(const Vec3<A>& other)
	{
		fb_expensive_assert(other.x != A(0));
		fb_expensive_assert(other.y != A(0));
		fb_expensive_assert(other.z != A(0));

		x/=other.x;
		y/=other.y;
		z/=other.z;
	}

	void operator*=(A num)
	{
		x*=num;
		y*=num;
		z*=num;
	}

	void operator/=(A num)
	{
		fb_expensive_assert(num != A(0));

		x/=num;
		y/=num;
		z/=num;
	}

	A getLength() const
	{
		return sqrt(x*x+y*y+z*z);
	}

	A getSquareLength() const
	{
		return x*x+y*y+z*z;
	}

	A getDotWith(const Vec3<A>& other) const
	{
		return x*other.x+y*other.y+z*other.z;
	}

	Vec3<A> getCrossWith(const Vec3<A>& other) const
	{
		return Vec3<A>(y*other.z-z*other.y,z*other.x-x*other.z,x*other.y-y*other.x);
	}

	A getRangeTo(const Vec3<A> &other) const
	{
		return sqrt((x-other.x)*(x-other.x)+(y-other.y)*(y-other.y)+(z-other.z)*(z-other.z));
	}

	A getSquareRangeTo(const Vec3<A> &other) const
	{
		return (x-other.x)*(x-other.x)+(y-other.y)*(y-other.y)+(z-other.z)*(z-other.z);
	}

	A getAngleTo(const Vec3<A> &other) const
	{
		A f = getDotWith(other) / (getLength() * other.getLength());
		if (f < -1.0)
			f = -1.0;
		else if (f > 1.0)
			f = 1.0;

		return (A)acos(f);
	}

	bool isDenormal() const;
	bool isNaN() const;
	bool isInf() const;
	bool isFinite() const;

	const A &operator[](SizeType index) const
	{
		fb_expensive_assert(index < 3 && "Invalid index for Vec3 operator[]");
		return v[index];
	}

	A &operator[](SizeType index)
	{
		fb_expensive_assert(index < 3 && "Invalid index for Vec3 operator[]");
		return v[index];
	}

	bool operator == (const Vec3<A> &other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	bool operator != (const Vec3<A> &other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	template <typename T>
	inline Vec3<T> convert() const
	{
		return Vec3<T>(T(x), T(y), T(z));
	}

};


template <>
inline Vec3<float> Vec3<float>::getNormalized() const
{
	float len = getLength();
	float iLen = FB_FSEL(len - 0.0001f, 1.0f / len, 0.0f);
	return Vec3<float>(x * iLen, y * iLen, z * iLen);
}
template <>
inline Vec3<float> Vec3<float>::getNormalizedWithZeroFailsafe(const Vec3<float> &failsafeValue) const
{
	float len = getLength();
	if (len != 0.0f)
	{
		float iLen = 1.0f / len;
		return Vec3<float>(x * iLen, y * iLen, z * iLen);
	}

	return failsafeValue;
}
template <>
inline void Vec3<float>::normalize()
{
	float len = getLength();
	float iLen = FB_FSEL(len - 0.0001f, 1.0f / len, 0.0f);
	x *= iLen;
	y *= iLen;
	z *= iLen;
}
template <>
inline void Vec3<float>::normalizeWithZeroFailsafe(const Vec3<float> &failsafeValue)
{
	float len = getLength();
	if (len != 0.0f)
	{
		float iLen = 1.0f / len;
		x *= iLen;
		y *= iLen;
		z *= iLen;
		return;
	}

	*this = failsafeValue;
}

template <>
inline Vec3<double> Vec3<double>::getNormalized() const
{
	double len = getLength();
	double iLen = FB_DSEL(len - 0.0001, 1.0 / len, 0.0);
	return Vec3<double>(x * iLen, y * iLen, z * iLen);
}
template <>
inline Vec3<double> Vec3<double>::getNormalizedWithZeroFailsafe(const Vec3<double> &failsafeValue) const
{
	double len = getLength();
	if (len != 0.0)
	{
		double iLen = 1.0 / len;
		return Vec3<double>(x * iLen, y * iLen, z * iLen);
	}

	return failsafeValue;
}
template <>
inline void Vec3<double>::normalize()
{
	double len = getLength();
	double iLen = FB_DSEL(len - 0.0001, 1.0 / len, 0.0);
	x *= iLen;
	y *= iLen;
	z *= iLen;
}
template <>
inline void Vec3<double>::normalizeWithZeroFailsafe(const Vec3<double> &failsafeValue)
{
	double len = getLength();
	if (len != 0.0)
	{
		double iLen = 1.0 / len;
		x *= iLen;
		y *= iLen;
		z *= iLen;
		return;
	}

	*this = failsafeValue;
}

template<class T>
Vec3<T> cross(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.getCrossWith(b);
}

template<class T>
T dot(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.getDotWith(b);
}

template<class T>
Vec3<T> lerp(const Vec3<T> &a, const Vec3<T> &b, T f)
{
	return a + (b - a) * f;
}

template<class T>
Vec3<T> normalize(const Vec3<T> &a)
{
	return a.getNormalized();
}

template<class T>
T length(const Vec3<T> &a)
{
	return a.getLength();
}

template <typename A>
Vec3<A> operator*(A num, const Vec3<A> &v)
{
	return Vec3<A>(v.x*num,v.y*num,v.z*num);
}

template <typename A>
bool isEqualWithinEpsilon(const Vec3<A> &first, const Vec3<A> &second, A epsilon = 0.00001f)
{
	return (first - second).getSquareLength() < epsilon*epsilon;
}

template<typename A> const Vec3<A> Vec3<A>::zero(0, 0, 0);

typedef Vec3<double> DVC3;
typedef Vec3<float> VC3;
typedef Vec3<int32_t> VC3I;

static math::Vec3<double> toDouble(const math::VC3 &v)
{
	return math::Vec3<double>(v.x, v.y, v.z);
}

static math::VC3 fromDouble(const math::Vec3<double> &v)
{
	return math::VC3((float)v.x, (float)v.y, (float)v.z);
}

static math::Vec3<int> toInt(const math::VC3 &v)
{
	return math::Vec3<int>((int)v.x, (int)v.y, (int)v.z);
}

static math::VC3 fromInt(const math::Vec3<int> &v)
{
	return math::VC3((float)v.x, (float)v.y, (float)v.z);
}

HeapString &debugAppendToString(HeapString &result, const math::VC3 &val);
HeapString &debugAppendToString(HeapString &result, const math::VC3I &val);
HeapString &debugAppendToString(HeapString &result, const math::DVC3 &val);

FB_END_PACKAGE1()

#include "fb/math/traits/Vec3Traits.h"
