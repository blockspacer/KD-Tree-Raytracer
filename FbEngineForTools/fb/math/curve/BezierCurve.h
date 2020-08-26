#pragma once

#include "fb/container/PodVector.h"

FB_PACKAGE1(math)

struct BezierCurveKnot
{
	typedef float PointType;

	PointType point = 0;
	float time = 0;
	PointType tangentEntering = 0;
	PointType tangentLeaving = 0;

	bool operator<(const BezierCurveKnot &o) const
	{
		return time < o.time;
	}
};

struct BezierCurve
{
	typedef BezierCurveKnot KnotType;
	typedef KnotType::PointType PointType;

	const PodVector<KnotType> &getKnots() const
	{
		return knots;
	}
	void setKnots(const PodVector<KnotType> &knotsParam);

	static float bezier4(float t, PointType a, PointType b, PointType c, PointType d)
	{
		float u = (1 - t);
		float tt = t*t;
		float uu = u*u;
		float uuu = uu*u;
		float ttt = tt*t;
		float tuu3 = 3 * t*uu;
		float ttu3 = 3 * tt*u;
		return a*uuu + b*tuu3 + c*ttu3 + d*ttt;
	}

	float evaluate(float t)
	{
		for (SizeType i = 1; i < knots.getSize(); ++i)
		{
			const KnotType &k1 = knots[i];
			if (t > k1.time)
				continue;

			const KnotType &k0 = knots[i - 1];

			float startSegment = k0.time;
			float endSegment = k1.time;
			if (startSegment == endSegment)
				return k0.point;

			float tDist = endSegment - startSegment;

			float tSegment = (t - startSegment) / tDist;
			float t0 = k0.point + k0.tangentLeaving * tDist; // Tangents divided by the time difference to preserve continuity for knots
			float t1 = k1.point - k1.tangentEntering * tDist;
			return bezier4(tSegment, k0.point, t0, t1, k1.point);
		}
		return 0;
	}

	typedef BezierCurve Evaluator;
	Evaluator getEvaluator() const
	{
		return *this;
	}

private:
	PodVector<KnotType> knots;
};

FB_END_PACKAGE1()
