#pragma once

#undef max
#undef min

#include "fb/lang/FBAssert.h"
#include <cmath>

FB_DECLARE0(HeapString)

FB_PACKAGE1(math)

template <class A>
class Vec2
{
public:
	static const Vec2 zero;

	typedef A ValueType;

	// Data (public for speed)
	union
	{
		A v[2];
		struct
		{
			A x,y;
		};
	};

	// Constructors
	Vec2() : x(0),y(0) {};
	Vec2(A _x,A _y) : x(_x),y(_y) {};
	explicit Vec2(A a[2]) : x(a[0]),y(a[1]) {};

	// Operators
	Vec2<A> operator-() const
	{
		return Vec2<A>(-x,-y);
	}
	
	Vec2<A> operator+(const Vec2<A>& other) const
	{
		return Vec2<A>(x+other.x,y+other.y);
	}

	Vec2<A> operator-(const Vec2<A>& other) const
	{
		return Vec2<A>(x-other.x,y-other.y);
	}
	
	Vec2<A> operator*(const Vec2<A>& other) const
	{
		return Vec2<A>(x*other.x,y*other.y);
	}
	
	Vec2<A> operator/(const Vec2<A>& other) const
	{
		return Vec2<A>(x/other.x,y/other.y);
	}
	
	Vec2<A> operator*(A num) const
	{
		return Vec2<A>(x*num,y*num);
	}

	Vec2<A> operator/(A num) const
	{
		return Vec2<A>(x/num,y/num);
	}
	
	void operator+=(const Vec2<A>& other)
	{
		x+=other.x;
		y+=other.y;
	}

	void operator-=(const Vec2<A>& other)
	{
		x-=other.x;
		y-=other.y;
	}

	void operator*=(const Vec2<A>& other)
	{
		x*=other.x;
		y*=other.y;
	}

	void operator/=(const Vec2<A>& other)
	{
		x/=other.x;
		y/=other.y;
	}

	void operator*=(A num)
	{
		x*=num;
		y*=num;
	}

	void operator/=(A num)
	{
		x/=num;
		y/=num;
	}

	A getLength() const
	{
		return sqrt(x*x+y*y);
	}

	A getSquareLength() const
	{
		return x*x+y*y;
	}

	A getDotWith(const Vec2<A>& other) const
	{
		return x*other.x+y*other.y;
	}

	Vec2<A> getNormalized() const
	{
		A l = getLength();
		fb_expensive_assert(l != 0);
		A ilen=(A)1/l;
		return Vec2<A>(x*ilen,y*ilen);
	}

	Vec2<A> getNormalizedWithZeroFailsafe(const Vec2<A> &failsafeValue) const
	{
		A l = getLength();
		if (l == 0)
			return failsafeValue;

		A ilen=(A)1/l;
		return Vec2<A>(x*ilen,y*ilen);
	}

	void normalizeWithZeroFailsafe(const Vec2<A> &failsafeValue)
	{	
		A l = getLength();

		if (l == 0)
		{
			x = failsafeValue.x;
			y = failsafeValue.y;
			return;
		}

		A ilen=(A)1/l;
		x*=ilen;
		y*=ilen;
	}

	void normalize()
	{
		A l = getLength();

		fb_expensive_assert(l != 0);

		A ilen=(A)1/l;
		x*=ilen;
		y*=ilen;
	}

	A getRangeTo(const Vec2<A> &other) const
	{
		return std::sqrt((x-other.x)*(x-other.x)+(y-other.y)*(y-other.y));
	}

	A getSquareRangeTo(const Vec2<A> &other) const
	{
		return (x-other.x)*(x-other.x)+(y-other.y)*(y-other.y);
	}

	A getAngleTo(const Vec2<A> &other) const
	{
		A f=getDotWith(other)/(getLength()*other.getLength());
		return (A)acos(f);
	}

	void rotateCCW ( A angleRad )
	{
		float a_x = x, a_y = y;
		x = a_x * cos ( angleRad ) - a_y * sin ( angleRad ) ;
		y = a_y * cos ( angleRad ) + a_x * sin ( angleRad ) ;
	}

	// Calculates angle (-PI, PI): (0,1) = 0, angle increases clockwise.
	float getClockwiseAngle() const
	{
		return atan2(x, y);
	}

	bool isDenormal() const;
	bool isNaN() const;
	bool isInf() const;
	bool isFinite() const;

	const A &operator[](SizeType index) const
	{
		fb_expensive_assert(index < 2 && "Invalid index for Vec2 operator[]");
		return v[index];
	}

	A &operator[](SizeType index)
	{
		fb_expensive_assert(index < 2 && "Invalid index for Vec2 operator[]");
		return v[index];
	}

	bool operator == (const Vec2<A> &other) const
	{
		return x == other.x && y == other.y;
	}

	bool operator != (const Vec2<A> &other) const
	{
		return x != other.x || y != other.y;
	}

	template <typename T>
	inline Vec2<T> convert() const
	{
		return Vec2<T>(T(x), T(y));
	}

};

template <typename A>
Vec2<A> operator*(A num, const Vec2<A> &v)
{
	return Vec2<A>(v.x*num,v.y*num);
}

template<class T>
T dot(const Vec2<T> &a, const Vec2<T> &b)
{
	return a.getDotWith(b);
}

template<class T>
Vec2<T> lerp(const Vec2<T> &a, const Vec2<T> &b, T f)
{
	return a + (b - a) * f;
}

template<class T>
Vec2<T> normalize(const Vec2<T> &a)
{
	return a.getNormalized();
}

template<class T>
T length(const Vec2<T> &a)
{
	return a.getLength();
}

template<typename A> const Vec2<A> Vec2<A>::zero(0, 0);

typedef Vec2<double> DVC2;
typedef Vec2<float> VC2;
typedef Vec2<int32_t> VC2I;

static math::Vec2<double> toDouble(const math::VC2 &v)
{
	return math::Vec2<double>(v.x, v.y);
}

static math::VC2 fromDouble(const math::Vec2<double> &v)
{
	return math::VC2((float)v.x, (float)v.y);
}

static math::Vec2<int> toInt(const math::VC2 &v)
{
	return math::Vec2<int>((int)v.x, (int)v.y);
}

static math::VC2 fromInt(const math::Vec2<int> &v)
{
	return math::VC2((float)v.x, (float)v.y);
}

HeapString &debugAppendToString(HeapString &result, VC2 val);
HeapString &debugAppendToString(HeapString &result, VC2I val);
HeapString &debugAppendToString(HeapString &result, const DVC2 &val);

FB_END_PACKAGE1()

#include "fb/math/traits/Vec2Traits.h"
