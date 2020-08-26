#pragma once

#undef max
#undef min

#include "fb/lang/FBAssert.h"

FB_DECLARE0(HeapString)

FB_PACKAGE1(math)

template <class A> class Vec4
{
public:
	static const Vec4 zero;

	// Data (public for speed)
	union {
		struct {
			A x, y, z, w;
		};
		A v[4];
	};

	// Constructors
	Vec4() : x(0), y(0), z(0), w(0) {};
	Vec4(A _x, A _y, A _z, A _w) : x(_x), y(_y), z(_z), w(_w) {};
	explicit Vec4(A a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {};

    // Operators
	Vec4<A> operator-() const
	{
		return Vec4<A>(-x,-y,-z,-w);
	}

	Vec4<A> operator+(const Vec4<A>& other) const
	{
		return Vec4<A>(x+other.x,y+other.y,z+other.z,w+other.w);
	}

	Vec4<A> operator-(const Vec4<A>& other) const
	{
		return Vec4<A>(x-other.x,y-other.y,z-other.z,w-other.w);
	}
	
	Vec4<A> operator*(const Vec4<A>& other) const
	{
		return Vec4<A>(x*other.x,y*other.y,z*other.z,w*other.w);
	}
	
	Vec4<A> operator/(const Vec4<A>& other) const
	{
		return Vec4<A>(x/other.x,y/other.y,z/other.z,w/other.w);
	}
	
	Vec4<A> operator*(A num) const
	{
		return Vec4<A>(x*num,y*num,z*num,w*num);
	}

	Vec4<A> operator/(A num) const
	{
		A inum=(A)1/num;
		return Vec4<A>(x*inum,y*inum,z*inum,w*inum);
	}
	
	void operator+=(const Vec4<A>& other)
	{
		x+=other.x;
		y+=other.y;
		z+=other.z;
		w+=other.w;
	}

	void operator-=(const Vec4<A>& other)
	{
		x-=other.x;
		y-=other.y;
		z-=other.z;
		w-=other.w;
	}

	void operator*=(const Vec4<A>& other)
	{
		x*=other.x;
		y*=other.y;
		z*=other.z;
		w*=other.w;
	}

	void operator/=(const Vec4<A>& other)
	{
		x/=other.x;
		y/=other.y;
		z/=other.z;
		w/=other.w;
	}

	void operator*=(A num)
	{
		x*=num;
		y*=num;
		z*=num;
		w*=num;
	}

	void operator/=(A num)
	{
		A inum=(A)1/num;
		x*=inum;
		y*=inum;
		z*=inum;
		w*=inum;
	}

	// Get contents as array
	const A *getAsFloat() const
	{
		return v;
	}

	A getLength() const
	{
		return A(sqrt(x*x + y*y + z*z + w*w));
	}

	A getDotWith(const Vec4<A> &other) const
	{
		return x*other.x + y*other.y + z*other.z + w*other.w;
	}

	void normalize()
	{
		A length = getLength();
		if (length == 0)
			return;

		A ilen = (A)1 / length;
		x *= ilen;
		y *= ilen;
		z *= ilen;
		w *= ilen;
	}

	void unsafeNormalize()
	{
		A length = getLength();
		A ilen = (A)1 / length;
		x *= ilen;
		y *= ilen;
		z *= ilen;
		w *= ilen;
	}

	bool isDenormal() const;
	bool isNaN() const;
	bool isInf() const;
	bool isFinite() const;

	const A &operator[](SizeType index) const
	{
		fb_expensive_assert(index < 4 && "Invalid index for Vec4 operator[]");
		return v[index];
	}

	A &operator[](SizeType index)
	{
		fb_expensive_assert(index < 4 && "Invalid index for Vec4 operator[]");
		return v[index];
	}
	
	bool operator == (const Vec4<A> &other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	bool operator != (const Vec4<A> &other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}

	template <typename T>
	inline Vec4<T> convert() const
	{
		return Vec4<T>(T(x), T(y), T(z), T(w));
	}

};

template <typename A>
Vec4<A> operator*(A num, const Vec4<A> &v)
{
	return Vec4<A>(v.x*num,v.y*num,v.z*num,v.w*num);
}

template<class T>
T dot(const Vec4<T> &a, const Vec4<T> &b)
{
	return a.getDotWith(b);
}

template<class T>
Vec4<T> lerp(const Vec4<T> &a, const Vec4<T> &b, T f)
{
	return a + (b - a) * f;
}

template<class T>
Vec4<T> normalize(const Vec4<T> &a)
{
	return a.getNormalized();
}

template<class T>
T length(const Vec4<T> &a)
{
	return a.getLength();
}

template<typename A> const Vec4<A> Vec4<A>::zero(0, 0, 0, 0);

typedef Vec4<float> VC4;
typedef Vec4<double> DVC4;
typedef Vec4<int32_t> VC4I;

static Vec4<double> toDouble(const VC4 &v)
{
	return Vec4<double>(v.x, v.y, v.z, v.w);
}

static VC4 fromDouble(const DVC4 &v)
{
	return VC4((float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

static VC4I toInt(const VC4 &v)
{
	return VC4I((int32_t)v.x, (int32_t)v.y, (int32_t)v.z, (int32_t)v.w);
}

static VC4 fromInt(const VC4I &v)
{
	return VC4((float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

HeapString &debugAppendToString(HeapString &result, VC4 val);
HeapString &debugAppendToString(HeapString &result, VC4I val);
HeapString &debugAppendToString(HeapString &result, const DVC4 &val);

FB_END_PACKAGE1()
