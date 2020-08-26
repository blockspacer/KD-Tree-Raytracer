#ifndef FB_MATH_RECTANGLE_MIN_MAX_H
#define FB_MATH_RECTANGLE_MIN_MAX_H

#include "fb/math/RectangleMinMaxDecl.h"
#include "fb/math/Vec2.h"
#include "fb/lang/FastMinMax.h"
#include "fb/lang/NumericLimits.h"

FB_PACKAGE1(math)

template <class A>
class RectangleMinMax
{
public:
	Vec2<A> min, max;

	static const RectangleMinMax<A> none, zero, all;

	RectangleMinMax()
	{
	}
	RectangleMinMax(const Vec2<A>& min, const Vec2<A>& max)
		: min(min)
		, max(max)
	{
	}

	static RectangleMinMax<A> fromMinAndSize(const math::Vec2<A>& min, const math::Vec2<A>& size)
	{
		return RectangleMinMax<A>(min, min + size);
	}
	static RectangleMinMax<A> fromCenterAndSize(const math::Vec2<A>& center, const math::Vec2<A>& size)
	{
		math::Vec2<A> halfSize = size / A(2);
		return RectangleMinMax<A>(center - halfSize, center + halfSize);
	}

	bool intersects(const RectangleMinMax<A>& other) const
	{
		return !(min.x > other.max.x || min.y > other.max.y || max.x < other.min.x || max.y < other.min.y);
	}
	bool contains(const math::VC2& point) const
	{
		return !(point.x < min.x || point.y < min.y || point.x > max.x || point.y > max.y);
	}

	bool hasArea() const
	{
		return min.x <= max.x && min.y <= max.y;
	}

	A getWidth() const { return max.x - min.x; }
	A getHeight() const { return max.y - min.y; }
	Vec2<A> getSize() const { return max - min; }
	Vec2<A> getCenter() const { return (max + min) / (A)2; }
	Vec2<A> getHalfSize() const { return (max - min) / (A)2; }

	void move(const Vec2<A>& offset)
	{
		min += offset;
		max += offset;
	}
	math::RectangleMinMax<A> getMoved(const Vec2<A>& offset) const
	{
		return math::RectangleMinMax<A>(min + offset, max + offset);
	}

	void expand(const math::RectangleMinMax<A>& other)
	{
		min += other.min;
		max += other.max;
	}
	void expand(const math::Vec2<A>& other)
	{
		max += other.max;
	}
	math::RectangleMinMax<A> getExpanded(const math::RectangleMinMax<A>& other) const
	{
		return math::RectangleMinMax<A>(min + other.min, max + other.max);
	}
	math::RectangleMinMax<A> getExpanded(const math::Vec2<A>& other) const
	{
		return math::RectangleMinMax<A>(min, max + other);
	}

	void inflate(A amount)
	{
		inflate(math::VC2I(amount, amount));
	}
	void inflate(const Vec2<A>& amount)
	{
		min -= amount;
		max += amount;
	}
	void addPoint(const Vec2<A>& point)
	{
		min.x = lang::fastMin(min.x, point.x);
		min.y = lang::fastMin(min.y, point.y);
		max.x = lang::fastMax(max.x, point.x);
		max.y = lang::fastMax(max.y, point.y);
	}
	void addRect(const RectangleMinMax<A>& other)
	{
		min.x = lang::fastMin(min.x, other.min.x);
		min.y = lang::fastMin(min.y, other.min.y);
		max.x = lang::fastMax(max.x, other.max.x);
		max.y = lang::fastMax(max.y, other.max.y);
	}

	void getCorners(math::Vec2<A>* corners) const
	{
		corners[0] = min;
		corners[1] = math::Vec2<A>(max.x, min.y);
		corners[2] = math::Vec2<A>(min.x, max.y);
		corners[3] = max;
	}

	bool operator==(const math::RectangleMinMax<A>& other) const
	{
		return min == other.min && max == other.max;
	}
	bool operator!=(const math::RectangleMinMax<A>& other) const
	{
		return !(*this == other);
	}

	template <typename T>
	RectangleMinMax<T> convert() const
	{
		return RectangleMinMax<T> ( min.template convert<T>(), max.template convert<T>() );
	}

	RectangleMinMax<A> getIntersection(const RectangleMinMax<A>& other) const
	{
		return RectangleMinMax<A>(
			Vec2<A>(lang::fastMax(min.x, other.min.x), lang::fastMax(min.y, other.min.y)),
			Vec2<A>(lang::fastMin(max.x, other.max.x), lang::fastMin(max.y, other.max.y)));
	}

	RectangleMinMax<A> getRelativeTo(const RectangleMinMax<A>& outer) const
	{
		Vec2<A> newMin = min - outer.min;
		Vec2<A> newMax = max - outer.min;
		Vec2<A> size = outer.getSize();
		return RectangleMinMax<A>(newMin / size, newMax / size);
	}

	RectangleMinMax<A> getRelative(const RectangleMinMax<A>& relative) const
	{
		Vec2<A> size = max - min;
		Vec2<A> newMin(min.x + size.x * relative.min.x, min.y + size.y * relative.min.y);
		Vec2<A> newMax(min.x + size.x * relative.max.x, min.y + size.y * relative.max.y);
		return RectangleMinMax<A>(newMin, newMax);
	}
};

template<typename A>
const RectangleMinMax<A> RectangleMinMax<A>::none = RectangleMinMax<A>(
	math::Vec2<A>(lang::NumericLimits<A>::getHighest(), lang::NumericLimits<A>::getHighest()),
	math::Vec2<A>(lang::NumericLimits<A>::getLowest(), lang::NumericLimits<A>::getLowest()));

template<typename A>
const RectangleMinMax<A> RectangleMinMax<A>::zero(math::Vec2<A>::zero, math::Vec2<A>::zero);

template<typename A>
const RectangleMinMax<A> RectangleMinMax<A>::all = RectangleMinMax<A>(
	math::Vec2<A>(lang::NumericLimits<A>::getLowest(), lang::NumericLimits<A>::getLowest()),
	math::Vec2<A>(lang::NumericLimits<A>::getHighest(), lang::NumericLimits<A>::getHighest()));

FB_END_PACKAGE1()

#endif
