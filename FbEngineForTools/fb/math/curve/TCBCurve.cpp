#include "Precompiled.h"
#include "TCBCurve.h"

#include "fb/algorithm/Sort.h"
#include "fb/lang/NumericLimits.h"
#include "fb/string/HeapString.h"
#include "fb/lang/MinMax.h"

#include <cmath>

FB_PACKAGE2(math, curve)

float TCBCurveCommonBase::getNormalizedTime(float startTime, float endTime, float time, WrapMode wrapModePre, WrapMode wrapModePost)
{
	/* Presume enough knots */
	/* Presume knots are sorted by time */
	/* Presume knots are not too close (floating point inaccuracies) */
	float duration = endTime - startTime;

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

float TCBCurveCommonBase::calculateTimeFraction(float knot1Time, float knot2Time, float timeEpsilon, float normalizedTime)
{
	if (knot2Time - knot1Time < timeEpsilon)
		return 0.0f;

	float timeBetweenBaseKnots = knot2Time - knot1Time;
	float timeFraction = (normalizedTime - knot1Time) / timeBetweenBaseKnots;
	fb_assert(timeFraction >= 0.0f && "Math error");
	fb_assert(timeFraction <= 1.0f && "Math error");
	return timeFraction;
}


HeapString &debugAppendToString(HeapString &result, const TCBCurve &curve)
{
	const uint32_t width = 160;
	const uint32_t height = 40;

	const PodVector<TCBCurveKnot> &knots = curve.getKnots();

	result << "TCBCurve: \n";
	result << "\t" << "Knots: " << knots.getSize() << "\n";
	for (SizeType i = 0, num = knots.getSize(); i < num; ++i)
	{
		const TCBCurveKnot &knot = knots[i];
		result << "\t\t" << (i < 10 ? "0" : "") << i << ": " << knot.point << ", " << knot.time << "\n";
		result << "\t\t\t" << knot.tEntering << ", " << knot.cEntering << ", " << knot.bEntering << "\n";
		result << "\t\t\t" << knot.tLeaving << ", " << knot.cLeaving << ", " << knot.bLeaving << "\n";
	}

	if (knots.getSize() < 2)
		return result;

	result << "\n";
	const float startTime = knots.getFront().time;
	const float endTime = knots.getBack().time;
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
	TCBCurve::Evaluator evaluator = curve.getEvaluator();
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

void TCBCurveCommonBase::sortKnotsStatic(Knot<float> *begin, Knot<float> *end)
{
	algorithm::stableSort(begin, end);
}

FB_END_PACKAGE2()
