#include "Precompiled.h"
#include "CubicBezierCurve.h"

#include "fb/algorithm/Sort.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/NumericLimits.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/math/Lerp.h"
#include "fb/string/HeapString.h"

#define FB_CUBIC_BEZIER_CURVE_STATISTICS FB_FALSE

#if FB_CUBIC_BEZIER_CURVE_STATISTICS == FB_TRUE
#include "fb/lang/FBSingleThreadAssert.h"
#endif

FB_PACKAGE1(math)

CubicBezierCurveKnot::CubicBezierCurveKnot()
{
}

CubicBezierCurveKnot::CubicBezierCurveKnot(math::VC2 point, math::VC2 controlPoint, math::VC2 mirrorControlPoint)
	: point(point)
	, controlPoint(controlPoint)
	, mirrorControlPoint(mirrorControlPoint)
{
}

CubicBezierCurveKnot::CubicBezierCurveKnot(math::VC2 point, math::VC2 controlPoint)
	: point(point)
	, controlPoint(controlPoint)
	, mirrorControlPoint(point + (point - controlPoint))
{
}

void CubicBezierCurve::addKnot(const CubicBezierCurveKnot &knot)
{
	knots.pushBack(knot);
	algorithm::sort(knots.getBegin(), knots.getEnd());
}


void CubicBezierCurve::addKnotWithoutSorting(const CubicBezierCurveKnot &knot)
{
	knots.pushBack(knot);
}

void CubicBezierCurve::setKnots(const PodVector<CubicBezierCurveKnot> &newKnots)
{
	knots = newKnots;
	algorithm::sort(knots.getBegin(), knots.getEnd());
}


void CubicBezierCurve::setKnotsWithoutSorting(const PodVector<CubicBezierCurveKnot> &newKnots)
{
	knots = newKnots;
}


void CubicBezierCurve::setKnotsWithoutSorting(PodVector<CubicBezierCurveKnot> &&newKnots)
{
	knots.swap(newKnots);
}


const PodVector<CubicBezierCurveKnot> &CubicBezierCurve::getKnots() const
{
	return knots;
}

void CubicBezierCurve::splitCurve(float time)
{
	if (knots.isEmpty())
	{
		knots.pushBack(CubicBezierCurveKnot(math::VC2(time, 0.0f), math::VC2(0.2f, 0.0f)));
		return;
	}

	if (time <= knots.getFront().point.x)
	{
		/* Add to front */
		CubicBezierCurveKnot knot(math::VC2(time, knots.getFront().point.x), knots.getFront().controlPoint);
		knots.insert(knots.getBegin(), knot);
		return;
	}
	if (time >= knots.getBack().point.x)
	{
		/* Add to back */
		CubicBezierCurveKnot knot(math::VC2(time, knots.getBack().point.x), knots.getBack().controlPoint);
		knots.insert(knots.getBegin(), knot);
		return;
	}
	/* We actually need to split something */
	SizeType knot0Index = 0xFFFFFFFF;
	SizeType knot1Index = 0xFFFFFFFF;
	getKnotIndexes(time, knot0Index, knot1Index);
	if (knot0Index == knot1Index)
	{
		/* We hit keyframe. Wiggle time a bit */
		if (time < knots[knot0Index].point.x)
		{
			fb_assert(knot0Index > 0 && "Hitting before first knot should have been handled above");
			time += (knots[knot0Index].point.x - knots[knot0Index - 1].point.x) * 0.1f;
		}
		else
		{
			fb_assert(knot0Index + 1 < knots.getSize() && "Hitting after last knot should have been handled above");
			time -= (knots[knot0Index + 1].point.x - knots[knot0Index].point.x) * 0.1f;
		}
		getKnotIndexes(time, knot0Index, knot1Index);
		if (knot0Index == knot1Index)
		{
			fb_assert(0 && "Knots are too close. Can't continue");
			return;
		}
	}
	CubicBezierCurveKnot &knot0 = knots[knot0Index];
	CubicBezierCurveKnot &knot1 = knots[knot1Index];
	/* We are splitting by coordinate. Evaluate to get actual time at split point */
	Decimal finalTimeFractionDecimal = 0.0f;
	CubicBezierCurveKnot newKnot;
	newKnot.point = evaluateImpl(time, knot0, knot1, finalTimeFractionDecimal);

	/* There's a nice image (and animation) at https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Quadratic_curves on how 
	 * the interpolation is done (https://en.wikipedia.org/wiki/B%C3%A9zier_curve#/media/File:B%C3%A9zier_3_big.svg). 
	 * Basically we just stop the evaluation a step too early to get new control points instead of final point on the 
	 * curve */

	float finalTimeFraction = float(finalTimeFractionDecimal);
	math::VC2 q0 = math::lerp(knot0.point, knot0.controlPoint, finalTimeFraction);
	math::VC2 q1 = math::lerp(knot0.controlPoint, knot1.mirrorControlPoint, finalTimeFraction);
	math::VC2 q2 = math::lerp(knot1.mirrorControlPoint, knot1.point, finalTimeFraction);
	math::VC2 r0 = math::lerp(q0, q1, finalTimeFraction);
	math::VC2 r1 = math::lerp(q1, q2, finalTimeFraction);
	newKnot.controlPoint = r1;
	newKnot.mirrorControlPoint = r0;

	/* The control points in knot0 and 1 need to be adjusted based on new knot's distance) */
	knot0.controlPoint = math::lerp(knot0.point, knot0.controlPoint,finalTimeFraction);
	knot1.mirrorControlPoint = math::lerp(knot1.point, knot1.mirrorControlPoint, 1.0f - finalTimeFraction);
	knots.insertIndex(knot1Index, newKnot);
}

float CubicBezierCurve::evaluate(float normalizedTime, KnotType knot0, KnotType knot1)
{
	fb_assertf(normalizedTime >= knot0.point.x - 0.00001f && normalizedTime <= knot1.point.x + 1.0001f, "%f, %f, %f", normalizedTime, knot0.point.x - 0.00001f, knot1.point.x + 1.0001f);

	Decimal finalTimeFraction = 0.0f;
	return evaluateImpl(normalizedTime, knot0, knot1, finalTimeFraction).y;
}

float CubicBezierCurve::evaluate(float time, WrapMode wrapModePre, WrapMode wrapModePost) const
{
	if (knots.isEmpty())
		return 0.0f;
	
	if (knots.getSize() < 2)
		return knots.getFront().point.y;

	Decimal normalizedTime = getNormalizedTime(knots.getFront().point.x, knots.getBack().point.x, time, wrapModePre, wrapModePost);
	SizeType knot0Index = 0xFFFFFFFF;
	SizeType knot1Index = 0xFFFFFFFF;
	getKnotIndexes(normalizedTime, knot0Index, knot1Index);
	if (knot0Index == knot1Index)
	{
		/* We hit keyframe */
		return knots[knot0Index].point.y;
	}
	Decimal finalTimeFraction = 0.0f;
	return evaluateImpl(normalizedTime, knots[knot0Index], knots[knot1Index], finalTimeFraction).y;
}


CubicBezierCurve::Decimal CubicBezierCurve::getNormalizedTime(Decimal startTime, Decimal endTime, Decimal time, WrapMode wrapModePre, WrapMode wrapModePost)
{
	/* Presume enough knots */
	/* Presume knots are sorted by time */
	/* Presume knots are not too close (floating point inaccuracies) */
	Decimal duration = endTime - startTime;

	/* We may get stuck to infinite loops if duration is negative, so better just return something if that happens */
	fb_assert(duration > 0.0f && "Knots should be sorted, errors expected");
	if (duration <= 0.0f)
		return startTime;

	if (time < startTime)
	{
		switch (wrapModePre)
		{
		case WrapModeClamp:
			time = startTime;
			break;
		case WrapModeLoop:
			while (time < startTime)
				time += duration;

			break;
		case WrapModeMirror:
			while (time < startTime - (2.0f * duration))
				time += duration;

			/* Time should now be at most one duration outside scope */
			if (time < startTime)
				time = startTime + (startTime - time);

			break;
		}
	}
	else if (time > endTime)
	{
		switch (wrapModePost)
		{
		case WrapModeClamp:
			time = endTime;
			break;
		case WrapModeLoop:
			while (time > endTime)
				time -= duration;

			break;
		case WrapModeMirror:
			while (time > endTime + (2.0f * duration))
				time -= duration;

			/* Time should now be at most one duration outside scope */
			if (time > endTime)
				time = endTime - (time - endTime);

			break;
		}
	}
	fb_assert(time >= startTime && time <= endTime && "Wrapped time doesn't match curve limits. Check the math");
	return time;
}


void CubicBezierCurve::getKnotIndexes(Decimal normalizedTime, SizeType &outKnot0Index, SizeType &outKnot1Index) const
{
	for (SizeType i = 0, num = knots.getSize(); i < num; ++i)
	{
		const CubicBezierCurveKnot &knot = knots[i];
		if (FB_DABS(knot.point.x - normalizedTime) < timeEpsilon)
		{
			/* We happened to hit key frame value. Just return it directly */
			outKnot0Index = i;
			outKnot1Index = i;
			return;
		}
		if (knot.point.x > normalizedTime)
		{
			fb_assert(i > 0 && "Time should be normalized");
			outKnot0Index = i - 1;
			outKnot1Index = i;
			return;
		}
	}
	fb_assert(0 && "Time should be normalized. Should never get here");
}


CubicBezierCurve::Point CubicBezierCurve::bezier4(Decimal timeFraction, Point a, Point b, Point c, Point d, Decimal normalizedTime, Decimal &outErrorAmount, Decimal &outRelativeError)
{
	Point result;
	Decimal timeFraction2 = timeFraction * timeFraction;
	Decimal timeFraction3 = timeFraction2 * timeFraction;
	Decimal inverseTimeFraction = (1 - timeFraction);
	Decimal inverseTimeFraction2 = inverseTimeFraction * inverseTimeFraction;
	Decimal inverseTimeFraction3 = inverseTimeFraction2 * inverseTimeFraction;
	result = a * inverseTimeFraction3 + b * (timeFraction * inverseTimeFraction2 * 3) + c * (timeFraction2 * inverseTimeFraction * 3) + d * timeFraction3;
	outErrorAmount = result.x - normalizedTime;
	outRelativeError = FB_DABS(outErrorAmount / normalizedTime);
	fb_assert(result.isFinite() && "Math error");
	return result;
}


math::VC2 CubicBezierCurve::evaluateImpl(Decimal normalizedTime, const CubicBezierCurveKnot &knot0, const CubicBezierCurveKnot &knot1, Decimal &outFinalTimeFraction)
{
	math::VC2 finalResult;
	const Decimal timeA = knot0.point.x;
	const Decimal timeB = knot1.point.x;
	fb_assert(timeA < timeB && "Wrong knot order or same knot");
	fb_assert(normalizedTime > timeA && "Invalid normalized time or direct hit to knot (should be separately handled");
	const Decimal duration = timeB - timeA;

	/* Find correct point by binary search. We could solve cubic equation instead, but that's probably not that much 
	 * faster[1] and rather hard */
	Decimal timeFraction = (normalizedTime - timeA) / duration;
	Decimal maxFraction = 1;
	Decimal minFraction = 0;
	Decimal relativeError = 1;
	Decimal errorAmount = 9999;

	Point a(knot0.point.x, knot0.point.y);
	Point b(knot0.controlPoint.x, knot0.controlPoint.y);
	Point c(knot1.mirrorControlPoint.x, knot1.mirrorControlPoint.y);
	Point d(knot1.point.x, knot1.point.y);

	Point result = bezier4(timeFraction, a, b, c, d, normalizedTime, errorAmount, relativeError);
	uint32_t iterationCount = 1;
	const Decimal maxAllowedError = timeEpsilon * timeB;
	while (relativeError > maxAllowedError)
	{
		fb_assert(errorAmount != 0.0f && "Math error");
		if (FB_DABS(maxFraction - minFraction) < timeEpsilon)
			break;

		if (errorAmount > 0.0f)
		{
			/* Too far. Move target to the direction of minimum fraction*/
			maxFraction = timeFraction;
			/* Move at most twice the relative error or half the total available distance */
			Decimal relativeDistanceToMove = lang::min(Decimal(0.5) * (timeFraction - minFraction), Decimal(2.0) * relativeError);
			timeFraction = timeFraction - relativeDistanceToMove;
		}
		else
		{
			/* Not far enough. Move target to the direction of maximum fraction */
			minFraction = timeFraction;
			/* Move at most twice the relative error or half the total available distance */
			Decimal relativeDistanceToMove = lang::min(0.5f * (maxFraction - timeFraction), 2.0f * relativeError);
			timeFraction = timeFraction + relativeDistanceToMove;
		}
		fb_assert(timeFraction > 0.0f && "Math error");
		result = bezier4(timeFraction, a, b, c, d, normalizedTime, errorAmount, relativeError);
		/* Quick tests (using FB_CUBIC_BEZIER_CURVE_STATISTICS) resulted in about 6 average iterations and maximum of 
		 * 88. Different curves will produce different results, but seems likely that thousand iterations points to 
		 * something being really wrong */
		++iterationCount;
		if (iterationCount > 1000)
		{
			fb_assert_once(0 && "More than thousand iterations, breaking");
			break;
		}
	}

	#if FB_CUBIC_BEZIER_CURVE_STATISTICS == FB_TRUE
		fb_main_thread_assert();
		static uint32_t highestIterationCount = 0;
		static uint32_t lowestIterationCount = 0xFFFFFFFF;
		static uint32_t numTotalIterations = 0;
		static uint32_t numEvaluations = 0;
		static Decimal totalRelativeError = relativeError;
		static Decimal maxRelativeError = 0.0f;
		static math::VC2I lastReportedAverageIterationCount;
		++numEvaluations;
		numTotalIterations += iterationCount;
		bool shouldReport = highestIterationCount < iterationCount || lowestIterationCount > iterationCount || maxRelativeError < relativeError;
		highestIterationCount = lang::max(highestIterationCount, iterationCount);
		lowestIterationCount = lang::min(lowestIterationCount, iterationCount);
		maxRelativeError = lang::max(maxRelativeError, relativeError);
		math::VC2I iterationCountToReport;
		iterationCountToReport.x = int32_t(numTotalIterations / numEvaluations);
		iterationCountToReport.y = int32_t(10 * numTotalIterations / numEvaluations) % 10;
		shouldReport = shouldReport || iterationCountToReport != lastReportedAverageIterationCount;
		if (shouldReport)
		{
			lastReportedAverageIterationCount = iterationCountToReport;
			TempString msg("Iteration counts (average / min / max): ");
			msg << iterationCountToReport.x << "." << iterationCountToReport.y << " / " << lowestIterationCount << " / " << highestIterationCount;
			msg << ". Relative error (average / max): " << totalRelativeError / numEvaluations << " / " << maxRelativeError;
			FB_LOG_DEBUG(msg);
		}
	#endif
	/* [1] Solving by iteration will require about one division, 17 multiplications and 10 additions per iteration. 
	 * Solving the cubic equation will require in best realistic case about 5 divisions, one or two square roots and 
	 * 20 multiplications. In worse cases there are additional square roots, cubic roots and divisions, dozen(s) or 
	 * more multiplications and in worst case three cosines. I have no idea at all how often different cases would 
	 * happen, but even the best case is quite expensive. */
	outFinalTimeFraction = timeFraction;
	finalResult.x = float(result.x);
	finalResult.y = float(result.y);
	return finalResult;
}

HeapString &debugAppendToString(HeapString &result, const CubicBezierCurve &curve)
{
	const uint32_t width = 160;
	const uint32_t height = 40;

	const PodVector<CubicBezierCurveKnot> &knots = curve.getKnots();

	result << "CubicBezierCurve: \n";
	result << "\t" << "Knots: " << knots.getSize() << "\n";
	for (SizeType i = 0, num = knots.getSize(); i < num; ++i)
	{
		const CubicBezierCurveKnot &knot = knots[i];
		result << "\t\t" << (i < 10 ? "0" : "") << i << ": " << knot.point.x << ", " << knot.point.y << "\n";
		result << "\t\t\t" << knot.controlPoint.x << ", " << knot.controlPoint.y << "\n";
		result << "\t\t\t" << knot.mirrorControlPoint.x << ", " << knot.mirrorControlPoint.y << "\n";
	}

	if (knots.getSize() < 2)
		return result;

	result << "\n";
	const float startTime = knots.getFront().point.x;
	const float endTime = knots.getBack().point.x;
	const float duration = endTime - startTime;
	const float stepSize = duration / (width - 1);

	struct CurveValue
	{
		float value = 0.0f;
		uint32_t normalized = 0;
	};
	StaticPodVector<CurveValue, width> values;
	float minValue = lang::NumericLimits<float>::getHighest();
	float maxValue = lang::NumericLimits<float>::getLowest();
	CubicBezierCurve::Evaluator evaluator = curve.getEvaluator();
	for (uint32_t i = 0; i < width; ++i)
	{
		CurveValue &value = values.pushBack();
		value.value = evaluator.evaluate(i * stepSize);
		minValue = lang::min(minValue, value.value);
		maxValue = lang::max(maxValue, value.value);
	}
	const float dynamics = maxValue - minValue;
	for (uint32_t i = 0; i < width; ++i)
	{
		CurveValue &value = values[i];
		float rounded = std::round((value.value - minValue) / dynamics * (height - 1));
		fb_assert(rounded >= 0.0f && rounded <= height);
		value.normalized = uint32_t(rounded);
	}

	result << "\t" << "Min / Max values: " << minValue << " / " << maxValue << " (dynamics: " << dynamics << ")\n\n";
	SmallTempString maxValueStr;
	maxValueStr << maxValue << " ";
	SmallTempString minValueStr;
	minValueStr << minValue << " ";
	while (minValueStr.getLength() < maxValueStr.getLength())
		minValueStr << " ";

	while (maxValueStr.getLength() < minValueStr.getLength())
		maxValueStr << " ";

	SmallTempString leftPaddingStr;
	while (leftPaddingStr.getLength() < maxValueStr.getLength())
		leftPaddingStr << " ";

	minValueStr << "|";
	maxValueStr << "|";
	leftPaddingStr << "|";

	result << "\t" << leftPaddingStr;
	for (uint32_t i = 0; i < width; ++i)
		result << "-";

	result << "|\n";

	for (uint32_t lineIndex = 0; lineIndex < height; ++lineIndex)
	{
		result << "\t";
		if (lineIndex == 0)
			result << maxValueStr;
		else if (lineIndex == height - 1)
			result << minValueStr;
		else
			result << leftPaddingStr;

		for (uint32_t charIndex = 0; charIndex < width; ++charIndex)
		{
			const CurveValue &value = values[charIndex];
			if (value.normalized == ((height - 1) - lineIndex))
				result << "x";
			else
				result << " ";
		}
		result << "|\n";
	}
	result << "\t" << leftPaddingStr;
	for (uint32_t i = 0; i < width; ++i)
		result << "-";

	result << "|\n";

	SmallTempString startTimeStr;
	startTimeStr << startTime;
	SmallTempString endTimeStr;
	endTimeStr << endTime;
	result << "\t";
	for (SizeType i = 0, num = leftPaddingStr.getLength(); i < num; ++i)
		result << " ";

	result << startTimeStr;
	for (SizeType i = startTimeStr.getLength(), num = width - endTimeStr.getLength(); i < num; ++i)
		result << " ";

	result << endTimeStr;
	return result;
}

FB_END_PACKAGE1()
