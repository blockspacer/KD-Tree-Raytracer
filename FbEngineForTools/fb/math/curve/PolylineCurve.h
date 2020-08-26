#pragma once

#include "fb/container/PodVector.h"
#include "fb/math/Lerp.h"

FB_PACKAGE1(math)

struct PolylineCurveKnot
{
	typedef float PointType;

	PointType point = 0;
	float time = 0;

	bool operator<(const PolylineCurveKnot &o) const
	{
		return time < o.time;
	}
};

struct PolylineCurve
{
	typedef PolylineCurveKnot KnotType;
	typedef KnotType::PointType PointType;

	const PodVector<KnotType> &getKnots() const
	{
		return knots;
	}
	void setKnots(const PodVector<KnotType> &knotsParam);

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

			float tSegment = (t - startSegment) / (endSegment - startSegment);
			return lerp(k0.point, k1.point, tSegment);
		}
		return 0;
	}

	typedef PolylineCurve Evaluator;
	Evaluator getEvaluator() const
	{
		return *this;
	}

private:
	PodVector<KnotType> knots;
};

FB_END_PACKAGE1()
