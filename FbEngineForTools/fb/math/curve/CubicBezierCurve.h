#pragma once

#include "fb/container/PodVector.h"
#include "fb/math/Vec2.h"

FB_PACKAGE1(math)

/* See http://www.cubic.org/docs/bezier.htm */

struct CubicBezierCurveKnot
{
	CubicBezierCurveKnot();
	CubicBezierCurveKnot(math::VC2 point, math::VC2 controlPoint, math::VC2 mirrorControlPoint);
	CubicBezierCurveKnot(math::VC2 point, math::VC2 controlPoint);


	/* For sorting containers of Knots */
	bool operator < (const CubicBezierCurveKnot &other) const
	{
		return this->point.x < other.point.x;
	}

	bool operator==(const CubicBezierCurveKnot &other) const
	{
		return point == other.point && controlPoint == other.controlPoint && mirrorControlPoint == other.mirrorControlPoint;
	}

	bool operator!=(const CubicBezierCurveKnot &other) const
	{
		return !(*this == other);
	}

	math::VC2 point;
	math::VC2 controlPoint;
	math::VC2 mirrorControlPoint;
};

enum WrapMode : uint8_t
{
	WrapModeClamp,
	WrapModeLoop,
	WrapModeMirror
};

class CubicBezierCurve
{
public:
	typedef CubicBezierCurveKnot KnotType;

	void addKnot(const CubicBezierCurveKnot &knot);
	void addKnotWithoutSorting(const CubicBezierCurveKnot &knot);

	void setKnots(const PodVector<CubicBezierCurveKnot> &newKnots);
	void setKnotsWithoutSorting(const PodVector<CubicBezierCurveKnot> &newKnots);
	void setKnotsWithoutSorting(PodVector<CubicBezierCurveKnot> &&newKnots);

	const PodVector<CubicBezierCurveKnot> &getKnots() const;

	/* Adds a knot to given time so that curve is disturbed as little as possible. 
	 * Note: splitting empty curve or curve with just one knot is not well defined. A knot will be added, but nothing 
	 * more is guaranteed. Given time outside the curve a new knot will be added to beginning or end. Split positions 
	 * hitting too close to or at existing keyframe will be moved a bit to make splitting possible. If that fails, 
	 * knot is not added (at that point knot density is around timeEpsilon, so very special case) */
	void splitCurve(float normalizedTime);

	static float evaluate(float normalizedTime, KnotType knot0, KnotType knot1);

	float evaluate(float time, WrapMode wrapModePre, WrapMode wrapModePost) const;

	float evaluate(float time, WrapMode wrapMode) const
	{
		return evaluate(time, wrapMode, wrapMode);
	}

	float evaluate(float time) const
	{
		return evaluate(time, WrapModeClamp, WrapModeClamp);
	}

	/* TODO: Replace with real evaluator */
	typedef CubicBezierCurve Evaluator;
	const Evaluator &getEvaluator() const
	{
		return *this;
	}

	static constexpr float timeEpsilon = 0.00001f;

private:
	typedef float Decimal;
	typedef math::VC2 Point;

	static Decimal getNormalizedTime(Decimal startTime, Decimal endTime, Decimal time, WrapMode wrapModePre, WrapMode wrapModePost);

	void getKnotIndexes(Decimal normalizedTime, SizeType &outKnot0Index, SizeType &outKnot1Index) const;
	static Point bezier4(Decimal timeFraction, Point a, Point b, Point c, Point d, Decimal normalizedTime, Decimal &outErrorAmount, Decimal &outRelativeError);
	static math::VC2 evaluateImpl(Decimal normalizedTime, const CubicBezierCurveKnot &knot0, const CubicBezierCurveKnot &knot1, Decimal &outFinalTimeFraction);

	PodVector<CubicBezierCurveKnot> knots;
};

HeapString &debugAppendToString(HeapString &result, const CubicBezierCurve &curve);

FB_END_PACKAGE1()
