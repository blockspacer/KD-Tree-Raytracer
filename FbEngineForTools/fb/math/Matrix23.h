#ifndef FB_MATH_MATRIX_23_H
#define FB_MATH_MATRIX_23_H

#include "fb/math/Vec2.h"

FB_PACKAGE1(math)

template <class A> class Matrix23;
typedef Matrix23<float> MAT23;

template <class A> class Matrix23
{
public:
	union {
		struct {
			A _11, _12, _13;
			A _21, _22, _23;
			// 0    0    1
		};
		A m[2][3];
		A raw[6];
	};

	Matrix23()
		: _11(1), _12(0), _13(0)
		, _21(0), _22(1), _23(0)
	{ }

	Matrix23(A _11, A _12, A _13, A _21, A _22, A _23)
		: _11(_11), _12(_12), _13(_13)
		, _21(_21), _22(_22), _23(_23)
	{ }
	
	Matrix23(const A *f6)
	{
		memcpy(raw, f6, sizeof(A)*6);
	}

	const A& operator[](int i) const
	{
		return raw[i];
	}
	A& operator[](int i)
	{
		return raw[i];
	}
	const A& operator()(int row, int col) const
	{
		fb_expensive_assert(row >= 0 && col >= 0 && row < 2 && col < 3);
		return m[row][col];
	}
	A& operator()(int row, int col)
	{
		fb_expensive_assert(row >= 0 && col >= 0 && row < 2 && col < 3);
		return m[row][col];
	}
	const A* getAsFloat() const
	{
		return raw;
	}
	
	Matrix23<A> operator*(const Matrix23<A>& o) const
	{
		return Matrix23<A>(
			_11*o._11 + _12*o._21 /*+ _13*o._13*/,
			_11*o._12 + _12*o._22 /*+ _13*o._23*/,
			_11*o._13 + _12*o._23 + _13/**o._33*/,
			_21*o._11 + _22*o._21 /*+ _23*o._13*/,
			_21*o._12 + _22*o._22 /*+ _23*o._23*/,
			_21*o._13 + _22*o._23 + _23/**o._33*/);
	}

	Matrix23<A>& operator*=(const Matrix23<A>& o)
	{
		*this = *this * o;
		return *this;
	}

	Vec2<A> getTransformed(const Vec2<A>& v) const
	{
		return Vec2<A>(
			_11*v.x + _12*v.y + _13,
			_21*v.x + _22*v.y + _23);
	}
	void transformVector(Vec2<A>& v) const
	{
		v = getTransformed(v);
	}

	float getDeterminant() const
	{
		return _11*_22 - _21*_12;
	}

	Matrix23<A> getInverse() const
	{
		float id = ((A)1) / getDeterminant();

		return Matrix23<A>(
			(+_22) * id, (-_12) * id, (_23*_12 - _13*_22) * id,
			(-_21) * id, (+_11) * id, (_13*_21 - _23*_11) * id);
	}

	math::Vec2<A> getTranslation() const
	{
		return math::Vec2<A>(_13, _23);
	}

	math::Vec2<A> getScale() const
	{
		return math::Vec2<A>(
				std::sqrt(_11*_11 + _21*_21),
				std::sqrt(_12*_12 + _22*_22));
	}

	float getRotation() const
	{
		return std::atan2(_21, _22);
	}

	void setTranslation(const Vec2<A>& translation)
	{
		_13 = translation.x;
		_23 = translation.y;
	}
	void setAxes(const Vec2<A>& axis1, const Vec2<A>& axis2)
	{
		_11 = axis1.x;
		_21 = axis1.y;
		_12 = axis2.x;
		_22 = axis2.y;
	}

	void translate(const Vec2<A>& translation)
	{
		_13 += translation.x;
		_23 += translation.y;
	}

	static Matrix23<A> createScale(const Vec2<A>& scale)
	{
		return Matrix23<A>(
			scale.x, 0, 0,
			0, scale.y, 0);
	}
	static Matrix23<A> createScale(A scale)
	{
		return createScale(Vec2<A>(scale, scale));
	}
	static Matrix23<A> createTranslation(const Vec2<A>& translation)
	{
		return Matrix23<A>(
			1, 0, translation.x,
			0, 1, translation.y);
	}
	static Matrix23<A> createTranslationScale(const Vec2<A>& translation, const Vec2<A>& scale)
	{
		return Matrix23<A>(
			scale.x, 0, translation.x,
			0, scale.y, translation.y);
	}
	static Matrix23<A> createRotation(float rotation)
	{
		float s = sin(rotation);
		float c = cos(rotation);

		return Matrix23<A>(
			c, -s, 0,
			s,  c, 0);
	}
	static Matrix23<A> createFromAxes(const Vec2<A>& axis1, const Vec2<A>& axis2)
	{
		return Matrix23<A>(
			axis1.x, axis2.x, 0,
			axis1.y, axis2.y, 0);
	}
	static Matrix23<A> createFromAxes(const Vec2<A>& axis1, const Vec2<A>& axis2, const Vec2<A>& position)
	{
		return Matrix23<A>(
			axis1.x, axis2.x, position.x,
			axis1.y, axis2.y, position.y);
	}
};

FB_END_PACKAGE1()

#endif
