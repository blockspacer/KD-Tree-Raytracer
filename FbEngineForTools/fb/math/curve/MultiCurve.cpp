#include "Precompiled.h"
#include "MultiCurve.h"

#include "fb/algorithm/Sort.h"
#include "fb/lang/platform/FBMath.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/math/curve/CubicBezierCurve.h"
#include "fb/profiling/ZoneProfiler.h"

FB_PACKAGE1(math)

static math::VC2 getLinearTangentEnter(const PodVector<MultiCurveKnot> &knots, SizeType index)
{
	if (index >= knots.getSize() || index == 0)
		return math::VC2(0.1f, 0.0f);

	return (knots[index].point - knots[index - 1U].point) * 0.5f;
}
static math::VC2 getLinearTangentLeave(const PodVector<MultiCurveKnot> &knots, SizeType index)
{
	if (index + 1 >= knots.getSize())
		return math::VC2(0.1f, 0.0f);

	return (knots[index + 1U].point - knots[index].point) * 0.5f;
}

static math::VC2 getSmoothedTangent(const PodVector<MultiCurveKnot> &knots, SizeType index)
{
	fb_assertf(index < knots.getSize(), "%d < %d", index, knots.getSize());

	if (index > 0 && index + 1 < knots.getSize())
		return (knots[index + 1].point - knots[index - 1U].point) * 0.25f;

	if (index > 0)
		return getLinearTangentEnter(knots, index);

	if (index + 1 < knots.getSize())
		return getLinearTangentLeave(knots, index);

	return math::VC2::zero;
}

static MultiCurveSegmentType getSegmentTypeImpl(MultiCurveTangentType t0, MultiCurveTangentType t1)
{
	switch (t0)
	{
	case MultiCurveTangentType::Manual:
		return MultiCurveSegmentType::Curve;

	case MultiCurveTangentType::Smoothed:
		return MultiCurveSegmentType::Curve;

	case MultiCurveTangentType::Linear:
		if (t1 == MultiCurveTangentType::Linear)
			return MultiCurveSegmentType::Linear;
		else if (t1 == MultiCurveTangentType::Smoothed)
			return MultiCurveSegmentType::Linear;
		else
			return MultiCurveSegmentType::Curve;

	case MultiCurveTangentType::Stepped:
		return MultiCurveSegmentType::Stepped;

	case MultiCurveTangentType::SteppedNext:
		return MultiCurveSegmentType::SteppedNext;
	}

	fb_assert(!"Invalid switch statement result");
	return MultiCurveSegmentType::Curve;
}

static MultiCurveSegmentType getSegmentType(const PodVector<MultiCurveKnot> &knots, SizeType index)
{
	fb_assertf(index + 1 < knots.getSize(), "0 < %d < %d", index, knots.getSize());
	if (index + 1 == knots.getSize() && knots[index].leaveType == MultiCurveTangentType::Linear && knots[index + 1].enterType == MultiCurveTangentType::Smoothed)
		return MultiCurveSegmentType::Linear;

	return getSegmentTypeImpl(knots[index].leaveType, knots[index + 1].enterType);
}

static math::VC2 getEnterTangentForCurve(const PodVector<MultiCurveKnot> &knots, SizeType index)
{
	fb_assertf(index > 0 && index < knots.getSize(), "0 < %d < %d", index, knots.getSize());
	fb_assertf(getSegmentType(knots, index - 1) == MultiCurveSegmentType::Curve, "Enter tangent is being asked when the previous segment doesn't require it. %d", int(getSegmentType(knots, index - 1)));

	const MultiCurveKnot &knot = knots[index];
	if (knot.enterType == MultiCurveTangentType::Linear)
		return getLinearTangentEnter(knots, index);

	if (knot.enterType == MultiCurveTangentType::Manual)
		return knot.enterTangent;

	fb_assertf(knot.enterType == MultiCurveTangentType::Smoothed, "Invalid enter tangent type %d", int(knot.enterType));

	if (index + 1 < knots.getSize())
	{
		// See what this knot's segment is doing

		switch (knot.leaveType)
		{
		case MultiCurveTangentType::Manual:
			return knot.leaveTangent;

		case MultiCurveTangentType::Smoothed:
			return getSmoothedTangent(knots, index);

		case MultiCurveTangentType::Linear:
			return getLinearTangentLeave(knots, index);

		case MultiCurveTangentType::Stepped:
			return math::VC2(0.1f, 0.0f);

		case MultiCurveTangentType::SteppedNext:
			return math::VC2(0.0f, knots[index + 1].point.y - knot.point.y);
		}
		fb_assert(!"switch fail");
		return math::VC2::zero;
	}
	else
	{
		return getLinearTangentEnter(knots, index);
	}
}

static math::VC2 getLeaveTangentForCurve(const PodVector<MultiCurveKnot> &knots, SizeType index)
{
	const MultiCurveKnot &knot = knots[index];
	fb_assertf(getSegmentType(knots, index) == MultiCurveSegmentType::Curve, "Leave tangent is being asked when the previous segment doesn't require it. %d", int(getSegmentType(knots, index)));
	if (knot.leaveType == MultiCurveTangentType::Linear)
		return getLinearTangentLeave(knots, index);

	if (knot.leaveType == MultiCurveTangentType::Manual)
		return knot.leaveTangent;

	fb_assertf(knot.leaveType == MultiCurveTangentType::Smoothed, "Invalid leave tangent type %d", int(knot.leaveType));
	if (index > 0)
	{
		// See what the previous segment is doing

		MultiCurveSegmentType prevSegmentType = getSegmentType(knots, index - 1U);
		switch (prevSegmentType)
		{
		case MultiCurveSegmentType::Curve:
		{
			switch (knot.enterType)
			{
			case MultiCurveTangentType::Manual:
				return knot.enterTangent;
			case MultiCurveTangentType::Linear:
				return getLinearTangentEnter(knots, index);
			case MultiCurveTangentType::Smoothed:
				return getSmoothedTangent(knots, index);


			case MultiCurveTangentType::Stepped:
			case MultiCurveTangentType::SteppedNext:
				fb_assert(!"Invalid enter tangent type somehow");
				return math::VC2::zero;
			}
		}

		case MultiCurveSegmentType::Linear:
			return getLinearTangentEnter(knots, index);

		case MultiCurveSegmentType::Stepped:
			return math::VC2(0.0f, knot.point.y - knots[index - 1U].point.y);

		case MultiCurveSegmentType::SteppedNext:
			return math::VC2(0.1f, 0.0f);
		}
		fb_assert(!"switch fail");
		return math::VC2::zero;
	}
	else
	{
		return getLinearTangentLeave(knots, index);
	}

}

void MultiCurve::setKnots(const PodVector<MultiCurve::KnotType> &knotsParam)
{
#if FB_ASSERT_ENABLED == FB_TRUE
	for (const MultiCurve::KnotType &k : knotsParam)
	{
		fb_assert(k.enterType != MultiCurveTangentType::Stepped && "Enter tangent type can't be stepped");
		fb_assert(k.enterType != MultiCurveTangentType::SteppedNext && "Enter tangent type can't be stepped next");
	}
#endif
	knots = knotsParam;
	algorithm::stableSort(knots.getBegin(), knots.getEnd());
}

void MultiCurve::addKnot(const KnotType &k)
{
	for (SizeType i = 0; i < knots.getSize(); ++i)
	{
		if (knots[i] < k)
			continue;

		knots.insertIndex(i, k);
		return;
	}
	knots.pushBack(k);
}

void MultiCurve::addKnotWithoutSorting(const KnotType &k)
{
	knots.pushBack(k);
}

void MultiCurve::sortKnots()
{
	algorithm::sort(knots.getBegin(), knots.getEnd());
}

float MultiCurve::evaluate(float t) const
{
	for (SizeType i = 1; i < knots.getSize(); ++i)
	{
		const KnotType &k1 = knots[i];
		if (t > k1.point.x)
			continue;

		const KnotType &k0 = knots[i - 1];

		float diff = k1.point.x - k0.point.x;
		fb_assertf(diff > -0.0001f, "%f", diff);
		if (diff <= 0.0f) // Avoid dividing by zero and evaluating negative values
			return k0.point.y;

		MultiCurveSegmentType segmentType = getSegmentType(knots, i - 1U);
		switch (segmentType)
		{
		case MultiCurveSegmentType::Stepped:
			return k0.point.y;
		case MultiCurveSegmentType::SteppedNext:
			return k1.point.y;
		case MultiCurveSegmentType::Linear:
		{
			float normalizedTime = (t - k0.point.x) / diff;
			fb_assertf(normalizedTime > -0.00001f && normalizedTime < 1.00001f, "%f", normalizedTime);
			return lerp(k0.point.y, k1.point.y, normalizedTime);
		}
		case MultiCurveSegmentType::Curve:
		{
			CubicBezierCurve::KnotType c0;
			CubicBezierCurve::KnotType c1;
			c0.point = k0.point;
			c1.point = k1.point;

			c0.controlPoint = k0.point + getLeaveTangentForCurve(knots, i - 1);
			c1.mirrorControlPoint = k1.point - getEnterTangentForCurve(knots, i);

			if (t <= k0.point.x)
				return k0.point.y;

			return CubicBezierCurve::evaluate(t, c0, c1);
		}
		}
	}

	if (knots.getSize() > 0)
		return knots.getBack().point.y;

	return 0.0f;
}

math::VC2 MultiCurve::getLeaveTangent(const PodVector<KnotType> &knots, SizeType index)
{
	if (index + 1 >= knots.getSize())
		return math::VC2(0.1f, 0.0f);

	const KnotType &k0 = knots[index];
	const KnotType &k1 = knots[index + 1];
	MultiCurveSegmentType segmentType = getSegmentType(knots, index);
	switch (segmentType)
	{
	case MultiCurveSegmentType::Curve:
		return getLeaveTangentForCurve(knots, index);

	case MultiCurveSegmentType::Linear:
		return getLinearTangentLeave(knots, index);

	case MultiCurveSegmentType::Stepped:
		return math::VC2(0.1f, 0.0f);

	case MultiCurveSegmentType::SteppedNext:
		return math::VC2(0.0f, k1.point.y < k0.point.y ? 0.1f : -0.1f);
	}
	fb_assert(!"Switch fail.");
	return math::VC2::zero;
}

math::VC2 MultiCurve::getEnterTangent(const PodVector<KnotType> &knots, SizeType index)
{
	if (index <= 0)
		return math::VC2(0.1f, 0.0f);

	const KnotType &k0 = knots[index - 1];
	const KnotType &k1 = knots[index];
	MultiCurveSegmentType segmentType = getSegmentType(knots, index - 1U);
	switch (segmentType)
	{
	case MultiCurveSegmentType::Curve:
		return getEnterTangentForCurve(knots, index);

	case MultiCurveSegmentType::Linear:
		return getLinearTangentEnter(knots, index);

	case MultiCurveSegmentType::Stepped:
		return math::VC2(0.0f, k1.point.y < k0.point.y ? 0.1f : -0.1f);

	case MultiCurveSegmentType::SteppedNext:
		return math::VC2(0.1f, 0.0f);
	}
	fb_assert(!"Switch fail.");
	return math::VC2::zero;
}

MultiCurve::Evaluator MultiCurve::getEvaluator(float lossyCompression) const
{
	FB_ZONE("MultiCurve::getEvaluator");
	Evaluator evaluator;
	evaluator.knots.reserve(knots.getSize());

#define FB_MULTICURVE_DEBUG_PRINT FB_FALSE

#if FB_MULTICURVE_DEBUG_PRINT == FB_TRUE
	static bool stopSpam = false;
	static int doMoreSpam = 0;
	if (++doMoreSpam > 100)
	{
		doMoreSpam = 0;
		stopSpam = false;
	}
#endif

	float quality = FB_FMAX(0, 1 - lossyCompression);
	quality = quality * quality;
	SizeType defaultSegmentCount = 1;

	{
		const SizeType maxDefaultSegmentCount = 64;
		defaultSegmentCount = SizeType(FB_FCLAMP(quality * maxDefaultSegmentCount, 1.0f, float(maxDefaultSegmentCount)));
	}

	for (SizeType knotIndex = 1; knotIndex < knots.getSize(); ++knotIndex)
	{
		const KnotType &k0 = knots[knotIndex - 1];
		const KnotType &k1 = knots[knotIndex];

		if (evaluator.knots.getSize() == 0)
		{
			evaluator.knots.pushBack({ k0.point.x, k0.point.y });
		}
		else
		{
			Evaluator::KnotType prevKnot = evaluator.knots.getBack();
			if (prevKnot.point != k0.point.y || prevKnot.time != k0.point.x)
				evaluator.knots.pushBack({ k0.point.x, k0.point.y });
		}

		float diff = k1.point.x - k0.point.x;
		fb_assertf(diff > -0.0001f, "%f", diff);
		if (diff <= 0.0f) // Avoid dividing by zero and evaluating negative values
		{
			float time = FB_FMAX(k0.point.x, k1.point.x);
			evaluator.knots.pushBack({ time, k1.point.y });
			continue;
		}

		MultiCurveSegmentType segmentType = getSegmentType(knots, knotIndex - 1U);
		switch (segmentType)
		{
		case MultiCurveSegmentType::Stepped:
		{
			evaluator.knots.pushBack({ k1.point.x, k0.point.y });
			evaluator.knots.pushBack({ k1.point.x, k1.point.y });
			continue;
		}
		case MultiCurveSegmentType::SteppedNext:
		{
			evaluator.knots.pushBack({ k0.point.x, k1.point.y });
			evaluator.knots.pushBack({ k1.point.x, k1.point.y });
			continue;
		}
		case MultiCurveSegmentType::Linear:
		{
			evaluator.knots.pushBack({ k1.point.x, k1.point.y });
			continue;
		}
		case MultiCurveSegmentType::Curve:
		{
			CubicBezierCurve::KnotType c0;
			CubicBezierCurve::KnotType c1;
			c0.point = k0.point;
			c1.point = k1.point;

			c0.controlPoint = k0.point + getLeaveTangentForCurve(knots, knotIndex - 1);
			c1.mirrorControlPoint = k1.point - getEnterTangentForCurve(knots, knotIndex);

			float start = k0.point.x;

			SizeType segmentCount = defaultSegmentCount;

			{
				// Take a guess at how many segments there should be on this curve
				// If the y delta is much larger than x delta, use more segments

				float x0 = FB_FMIN(c0.point.x, c1.point.x);
				float x1 = FB_FMAX(c0.point.x, c1.point.x);
				float ky0 = FB_FMIN(c0.point.y, c1.point.y);
				float ky1 = FB_FMAX(c0.point.y, c1.point.y);
				float cy0 = FB_FMIN(c0.controlPoint.y, c1.mirrorControlPoint.y);
				float cy1 = FB_FMAX(c0.controlPoint.y, c1.mirrorControlPoint.y);
				float y0 = FB_FMIN(ky0, cy0);
				float y1 = FB_FMAX(ky1, cy1);
				float dx = FB_FABS(x1 - x0);
				float dy = FB_FABS(y1 - y0);

				const SizeType defaultSegmentGrow = SizeType(64 * FB_FSQRT(quality));
				const SizeType maxSegmentGrow = defaultSegmentGrow * 4;

				float grow = FB_FCLAMP(FB_FSQRT(dy / dx), 0.0f, 1000.0f);
				float growSegments = FB_FCLAMP(grow * defaultSegmentGrow, 0.0f, float(maxSegmentGrow));
				segmentCount += SizeType(growSegments);
			}

#if FB_MULTICURVE_DEBUG_PRINT == FB_TRUE
			SizeType originalKnotCount = evaluator.knots.getSize();
#endif
			evaluator.knots.reserve(evaluator.knots.getSize() + segmentCount - 1U);
			for (SizeType i = 0; i < segmentCount; ++i)
			{
				float t = 0;
				float point = 0;
				if (i + 1 < segmentCount)
				{
					t = start + diff * float(1 + i) / segmentCount;
					point = CubicBezierCurve::evaluate(t, c0, c1);
				}
				else
				{
					// Make sure the end point of the curve is represented as is
					t = c1.point.x;
					point = c1.point.y;
				}
				Evaluator::KnotType newKnot = { t, point };
				evaluator.knots.pushBack(newKnot);
			}

#if FB_MULTICURVE_DEBUG_PRINT == FB_TRUE
			for (SizeType i = originalKnotCount + 3; i < evaluator.knots.getSize(); ++i)
			{
				bool rising0 = evaluator.knots[i - 3].point < evaluator.knots[i - 2].point;
				bool rising1 = evaluator.knots[i - 2].point < evaluator.knots[i - 1].point;
				bool rising2 = evaluator.knots[i - 1].point < evaluator.knots[i].point;
				if (rising0 == rising2)
					fb_assertf(rising0 == rising1, "%f -> %f -> %f -> %f, diff: %f -> %f", evaluator.knots[i - 3].point, evaluator.knots[i - 2].point, evaluator.knots[i - 1].point, evaluator.knots[i].point, evaluator.knots[i - 2].point - evaluator.knots[i - 3].point, evaluator.knots[i - 1].point - evaluator.knots[i - 2].point);
			}
#endif
		}
		}
	}

#if FB_MULTICURVE_DEBUG_PRINT == FB_TRUE
	if (!stopSpam)
	{
		FB_PRINTF("Total evaluator knots: %d (from maximum of %d)\n", evaluator.getKnots().getSize(), evaluator.getKnots().getCapacity());
		stopSpam = true;
	}
#endif
#undef FB_MULTICURVE_DEBUG_PRINT

	evaluator.knots.trimMemory();
	return evaluator;
}

float MultiCurveEvaluator::evaluate(float t) const
{
	for (SizeType i = 1; i < knots.getSize(); ++i)
	{
		if (knots[i].time < t)
			continue;

		KnotType k0 = knots[i - 1U];
		KnotType k1 = knots[i];
		float val = lerp(k0.point, k1.point, (t - k0.time) / (k1.time - k0.time));
		return val;
	}

	if (knots.getSize() > 0)
		return knots.getBack().point;

	return 0.0f;
}

FB_END_PACKAGE1();
