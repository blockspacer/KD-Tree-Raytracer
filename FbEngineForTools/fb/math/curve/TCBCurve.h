#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/MinMax.h"
#include "fb/math/curve/TCBCurveDecl.h"

FB_DECLARE0(HeapString)

#if FB_BUILD != FB_FINAL_RELEASE
#define FB_TCBCURVE_SANITY_CHECKS FB_TRUE
#else
#define FB_TCBCURVE_SANITY_CHECKS FB_FALSE
#endif

/* Turn this on to catch broken knots or invalid knot order as soon as they are inserted */
#define FB_TCBCURVE_AGGRESSIVE_SANITY_CHECKS FB_FALSE

FB_PACKAGE2(math, curve)

/**
* Templated TCB Curve class. Should be able to handle any POD type that can be added, subtracted and multiplied
* (namely math::VCX, float and double). Base class is not meant to be used as is, but typedeffed (or if necessary,
* wrapped) to more easy to handle form in math namespace.
*
* There are two sets of T, C, and B provided. Setting both to same values is the way to get nice, continuous and
* break free curves. Setting different values allows more control and edges. "Entering" set is used when knot is on
* the left. "Leaving" set is used when the knot is on the right.
*
* Some notes:
*     1) Knots (keyframes) added to curve must be far enough apart (Knot struct has timeEpsilon that should be
*        minimum distance between knots)
*     2) When evaluating curve, zero knots produce zero. One knot produces point of that knot
*     3) Knots must be sorted before evaluation. This is currently done automatically during insert, unless specific
*        methods without sorting are used. See FB_TCBCURVE_AGGRESSIVE_SANITY_CHECKS above
*     4) Some or all of the limitations listed could be lifted with more or better code and different compromises.
*/

template<typename T>
class Knot
{
public:
	typedef T PointType;

	Knot()
	{
	}

	Knot(const PointType &point, float time, float t, float c, float b)
		: point(point)
		, tEntering(t)
		, cEntering(c)
		, bEntering(b)
		, tLeaving(t)
		, cLeaving(c)
		, bLeaving(b)
		, time(time)
	{
#if FB_TCBCURVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
	}

	Knot(const PointType &point, float time, float tEntering, float cEntering, float bEntering, float tLeaving, float cLeaving, float bLeaving)
		: point(point)
		, tEntering(tEntering)
		, cEntering(cEntering)
		, bEntering(bEntering)
		, tLeaving(tLeaving)
		, cLeaving(cLeaving)
		, bLeaving(bLeaving)
		, time(time)
	{
#if FB_TCBCURVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
	}

	/* For sorting containers of Knots */
	bool operator < (const Knot &other) const
	{
		return this->time < other.time;
	}

	bool operator==(const Knot &other) const
	{
		return point == other.point && time == other.time &&
			tEntering == other.tEntering && cEntering == other.cEntering && bEntering == other.bEntering &&
			tLeaving == other.tLeaving && cLeaving == other.cLeaving && bLeaving == other.bLeaving;
	}

	bool operator!=(const Knot &other) const
	{
		return !(*this == other);
	}

	void sanityCheck() const
	{
		/* Seems that these aren't really sensible for general curve */
		//fb_assert(tEntering >= -1.0f && tEntering <= 1.0f && "Entering T value out of bounds");
		//fb_assert(tLeaving >= -1.0f && tLeaving <= 1.0f && "Leaving T value out of bounds");
		//fb_assert(cEntering >= -1.0f && cEntering <= 1.0f && "Entering C value out of bounds");
		//fb_assert(cLeaving >= -1.0f && cLeaving <= 1.0f && "Leaving C value out of bounds");
		//fb_assert(bEntering >= -1.0f && bEntering <= 1.0f && "Entering B value out of bounds");
		//fb_assert(bLeaving >= -1.0f && bLeaving <= 1.0f && "Leaving B value out of bounds");
	}

	/* Note: point may be uninitialized */
	PointType point;
	float tEntering = 0.0f;
	float cEntering = 0.0f;
	float bEntering = 0.0f;
	float tLeaving = 0.0f;
	float cLeaving = 0.0f;
	float bLeaving = 0.0f;
	float time = 0.0f;
	static constexpr float timeEpsilon = 0.00001f;
};

class TCBCurveCommonBase
{
public:
	enum WrapMode : uint8_t
	{
		WrapModeClamp,
		WrapModeLoop,
		WrapModeMirror
	};

	static void sortKnotsStatic(Knot<float> *begin, Knot<float> *end);

protected:
	static float getNormalizedTime(float startTime, float endTime, float time, WrapMode wrapModePre, WrapMode wrapModePost);
	static float calculateTimeFraction(float knot1Time, float knot2Time, float timeEpsilon, float time);
};

/* See http://www.cubic.org/docs/hermite.htm */

template<typename KnotT>
class TCBCurveBase : public TCBCurveCommonBase
{
public:
	typedef KnotT KnotType;
	typedef typename KnotT::PointType PointType;

private:
	static const SizeType invalidIndex = 0xFFFFFFFF;
	struct CachedEvalData
	{
		PointType ts;
		PointType td;
		SizeType knot1Index = invalidIndex;
		SizeType knot2Index = invalidIndex;
	};

public:
	class Evaluator
	{
	public:
		friend class TCBCurveBase;
		PointType evaluate(float timeParam)
		{
			/* Start by checking edge cases */
			/* Note: for some values of insanity we may crash */
#if FB_TCBCURVE_SANITY_CHECKS == FB_TRUE
			curve.sanityCheck();
#endif
			if (curve.knots.isEmpty())
				return PointType(0);

			if (curve.knots.getSize() == 1)
				return curve.knots.getFront().point;

			/* Calculate time considering wrap modes */
			const float startTime = curve.knots.getFront().time;
			const float endTime = curve.knots.getBack().time;
			const float time = getNormalizedTime(startTime, endTime, timeParam, wrapModePre, wrapModePost);
			/* Check if we can use cache */
			if (cachedData.knot1Index != invalidIndex)
			{
				const KnotType &knot1 = curve.knots[cachedData.knot1Index];
				const KnotType &knot2 = curve.knots[cachedData.knot2Index];
				if (knot1.time <= time && knot2.time >= time)
					return curve.evaluateWithCachedData(time, cachedData);
			}
			/* Cache didn't help */
			cachedData = curve.calculateCachedEvalData(time);
			return curve.evaluateWithCachedData(time, cachedData);
		}

		void setWrapModePre(WrapMode newWrapMode)
		{
			wrapModePre = newWrapMode;
		}

		void setWrapModePost(WrapMode newWrapMode)
		{
			wrapModePost = newWrapMode;
		}

	private:
		Evaluator(const TCBCurveBase &curve, WrapMode wrapModePre, WrapMode wrapModePost)
			: curve(curve)
			, wrapModePre(wrapModePre)
			, wrapModePost(wrapModePost)
		{
		}


		const TCBCurveBase &curve;
		WrapMode wrapModePre = WrapModeClamp;
		WrapMode wrapModePost = WrapModeClamp;
		CachedEvalData cachedData;
	};

	/* Adds new knot to curve. Sorts the knots to make sure they are in order */
	void addKnot(const KnotType &knot)
	{
		knots.pushBack(knot);
		sortKnotsStatic(knots.getBegin(), knots.getEnd());
#if FB_TCBCURVE_AGGRESSIVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
	}

	void addKnot(const PointType &point, float t, float c, float b, float time)
	{
		return addKnot(KnotType(point, t, c, b, time));
	}

	/* Add knot without sorting */
	void addKnotWithoutSorting(const KnotType &knot)
	{
		knots.pushBack(knot);
	}

	void addKnotWithoutSorting(const PointType &point, float t, float c, float b, float time)
	{
		return addKnotWithoutSorting(KnotType(point, t, c, b, time));
	}

	/* Sets all knots and sorts them */
	void setKnots(const PodVector<KnotType> &newKnots)
	{
		knots = newKnots;
		sortKnotsStatic(knots.getBegin(), knots.getEnd());
#if FB_TCBCURVE_AGGRESSIVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
	}

	/* Sets all knots skipping the sorting */
	void setKnotsWithoutSorting(const PodVector<KnotType> &newKnots)
	{
		knots = newKnots;
	}

	void setKnotsWithoutSorting(PodVector<KnotType> &&newKnots)
	{
		knots.swap(newKnots);
	}

	/* Add knots to internal list and sorts it */
	void addKnots(const PodVector<KnotType> &newKnots)
	{
		knots.insert(knots.getEnd(), newKnots.getBegin(), newKnots.getEnd());
		sortKnotsStatic(knots.getBegin(), knots.getEnd());
#if FB_TCBCURVE_AGGRESSIVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
	}

	/* Adds knots to internal list skipping sorting */
	void addKnotsWithoutSorting(const PodVector<KnotType> &newKnots)
	{
		knots.insert(knots.getEnd(), newKnots.getBegin(), newKnots.getEnd());
	}

	/* Clears knots */
	void clearKnots()
	{
		knots.clear();
	}

	/* Sorts internal list of knots. If knots have been added using non-sorting methods, then this must be called
	* before calling evaluate (or order has to be established other way) */
	void sortKnots()
	{
		sortKnotsStatic(knots.getBegin(), knots.getEnd());
#if FB_TCBCURVE_AGGRESSIVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
	}

	const PodVector<KnotType> &getKnots() const
	{
		return knots;
	}

	bool operator==(const TCBCurveBase &other) const
	{
		if (knots.getSize() != other.knots.getSize())
			return false;

		for (SizeType i = 0, num = knots.getSize(); i < num; ++i)
		{
			if (knots[i] != other.knots[i])
				return false;
		}
		return true;
	}

	bool operator!=(const TCBCurveBase &other) const
	{
		return !(*this == other);
	}

	PointType evaluate(float time) const
	{
		return evaluate(time, WrapModeClamp, WrapModeClamp);
	}

	PointType evaluate(float time, WrapMode wrapMode) const
	{
		return evaluate(time, wrapMode, wrapMode);
	}

	PointType evaluate(float time, WrapMode wrapModePre, WrapMode wrapModePost) const
	{
		/* Note: for some values of insanity we may crash */
#if FB_TCBCURVE_SANITY_CHECKS == FB_TRUE
		sanityCheck();
#endif
		if (knots.isEmpty())
			return PointType(0);

		if (knots.getSize() == 1)
			return knots.getFront().point;

		float startTime = knots.getFront().time;
		float endTime = knots.getBack().time;
		time = getNormalizedTime(startTime, endTime, time, wrapModePre, wrapModePost);

		/* Actually evaluate */
		CachedEvalData data = calculateCachedEvalData(time);
		if (data.knot1Index == data.knot2Index)
		{
			const KnotType &knot1 = knots[data.knot1Index];
			fb_assert(lang::abs(knot1.time - time) <= KnotType::timeEpsilon);
			return knot1.point;
		}
		return evaluateWithCachedData(time, data);
	}

	Evaluator getEvaluator() const
	{
		return getEvaluator(WrapModeClamp, WrapModeClamp);
	}

	Evaluator getEvaluator(WrapMode wrapMode) const
	{
		return getEvaluator(wrapMode, wrapMode);
	}

	Evaluator getEvaluator(WrapMode wrapModePre, WrapMode wrapModePost) const
	{
		return Evaluator(*this, wrapModePre, wrapModePost);
	}

private:
	PointType evaluateWithCachedData(float normalizedTime, const CachedEvalData &data) const
	{
		const KnotType &knot1 = knots[data.knot1Index];
		const KnotType &knot2 = knots[data.knot2Index];
		fb_assert(knot1.time <= normalizedTime && knot2.time >= normalizedTime && "This should only be called when evaluation can and needs to be done with cached data");

		float timeFraction = calculateTimeFraction(knot1.time, knot2.time, KnotType::timeEpsilon, normalizedTime);
		/* Shortcut: if time fraction is 0 or 1, we have hit directly at (or close enough) the keyframe */
		if (timeFraction == 0.0f)
			return knot1.point;
		else if (timeFraction == 1.0f)
			return knot2.point;

		/* Interpolate using cached data */

		const float timeFractionSquared = timeFraction * timeFraction;
		const float timeFractionCubed = timeFractionSquared * timeFraction;

		const float h1 = 2.0f * timeFractionCubed - 3.0f * timeFractionSquared + 1.0f;
		const float h2 = -2.0f * timeFractionCubed + 3.0f * timeFractionSquared;
		const float h3 = timeFractionCubed - 2.0f * timeFractionSquared + timeFraction;
		const float h4 = timeFractionCubed - timeFractionSquared;

		const PointType result = knot1.point * h1 + knot2.point * h2 + data.td * h3 + data.ts * h4;
		return result;
	}

	CachedEvalData calculateCachedEvalData(float normalizedTime) const
	{
		SizeType knot0Index = 0;
		SizeType knot1Index = 0;
		SizeType knot2Index = 0;
		SizeType knot3Index = 0;
		float timeAdjustmentTS = 0.0f;
		float timeAdjustmentTD = 0.0f;
		calculateKnotIndexesAndTimeAdjustment(normalizedTime, knot0Index, knot1Index, knot2Index, knot3Index, timeAdjustmentTS, timeAdjustmentTD);
		const KnotType &knot0 = knots[knot0Index];
		const KnotType &knot1 = knots[knot1Index];
		const KnotType &knot2 = knots[knot2Index];
		const KnotType &knot3 = knots[knot3Index];

		/* Actual point is between knot1 and knot2. Use leaving parameters for 0 and 1, entering for 2 and 3 */
		/* Only knots 1 and 2 are needed later */
		CachedEvalData data;
		data.knot1Index = knot1Index;
		data.knot2Index = knot2Index;

		// Incoming tangent TS(i+1)
		data.ts = ((knot2.point - knot1.point) * (1.0f - knot2.tEntering) * (1.0f - knot2.cEntering) * (1.0f + knot2.bEntering) +
			(knot3.point - knot2.point) * (1.0f - knot2.tEntering) * (1.0f + knot2.cEntering) * (1.0f - knot2.bEntering)) *
			0.5f * timeAdjustmentTS;
		// Outgoing tangent TD(i)
		data.td = ((knot1.point - knot0.point) * (1.0f - knot1.tLeaving) * (1.0f + knot1.cLeaving) * (1.0f + knot1.bLeaving) +
			(knot2.point - knot1.point) * (1.0f - knot1.tLeaving) * (1.0f - knot1.cLeaving) * (1.0f - knot1.bLeaving)) *
			0.5f * timeAdjustmentTD;

		return data;
	}

	void calculateKnotIndexesAndTimeAdjustment(float normalizedTime, SizeType &outKnot0Index, SizeType &outKnot1Index, SizeType &outKnot2Index, SizeType &outKnot3Index, float &outTimeAdjustmentTS, float &outTimeAdjustmentTD) const
	{
		/* Presume enough knots */
		/* Presume knots are sorted by time */
		/* Presume knots are not too close (floating point inaccuracies) */
		fb_assert(normalizedTime >= knots.getFront().time && normalizedTime <= knots.getBack().time && "Wrapped time doesn't match curve limits. Check the math");

		/* See http://www.cubic.org/docs/hermite.htm */
		/* We calculate TS(i+1) and TD(i), each need three consecutive points, so we need four total consecutive
		* points. Base points, so to speak, are the ones on either side of given time value. One point after and
		* before each is also needed. We use the first and last point multiple times if needed */
		for (SizeType i = 0, num = knots.getSize(); i < num; ++i)
		{
			const KnotType &knot = knots[i];
			if (lang::abs(knot.time - normalizedTime) < KnotType::timeEpsilon)
			{
				/* We happened to hit key frame value. Just return it directly */
				outKnot0Index = i;
				outKnot1Index = outKnot0Index;
				outKnot2Index = outKnot0Index;
				outKnot3Index = outKnot0Index;
				return;
			}
			if (knot.time > normalizedTime)
			{
				fb_assert(i > 0 && "First knot shouldn't be after time at this point");
				/* This is the latter knot */
				outKnot0Index = i > 1 ? i - 2 : i - 1;
				outKnot1Index = i - 1;
				outKnot2Index = i;
				outKnot3Index = lang::min(i + 1, num - 1);

				/* Adjust for variable time between keyframes */
				/* Note: TD(i) and TS(i+1) */
				float n1 = knots[outKnot1Index].time - knots[outKnot0Index].time;
				float n2 = knots[outKnot2Index].time - knots[outKnot1Index].time;
				float n3 = knots[outKnot3Index].time - knots[outKnot2Index].time;
				outTimeAdjustmentTD = 2.0f * n1 / (n1 + n2);
				outTimeAdjustmentTS = 2.0f * n2 / (n2 + n3);
				return;
			}
		}

		fb_assert(0 && "No knot found with supposed normalized time. Something wrong somewhere");
	}

	void sanityCheck() const
	{
		/* Allow no knots */
		if (knots.isEmpty())
			return;

		/* Knots must be in order */
		for (SizeType i = 0, num = knots.getSize(); i < num - 1; ++i)
		{
			const KnotType &knot1 = knots[i];
			const KnotType &knot2 = knots[i + 1];
			fb_assert(knot1.time != knot2.time && "Knots have same time. Errors expected");
			fb_assert(knot1.time < knot2.time && "Knots are not in order. Errors expected");
			fb_assert(knot2.time - knot1.time >= KnotType::timeEpsilon && "Knots are too close together for their own health");
			knot1.sanityCheck();
		}
		knots.getBack().sanityCheck();
	}

	PodVector<KnotType> knots;
};

FB_END_PACKAGE2()

FB_PACKAGE2(math, curve)

HeapString &debugAppendToString(HeapString &result, const TCBCurve &curve);

FB_END_PACKAGE2()
