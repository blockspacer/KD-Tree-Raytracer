#pragma once

FB_PACKAGE1(math)

enum class MultiCurveSegmentType : uint8_t
{
	Curve,
	Linear,
	Stepped,
	SteppedNext,
};
enum class MultiCurveTangentType : uint8_t
{
	Manual,
	Smoothed,
	Linear,
	Stepped,
	SteppedNext,
};

FB_END_PACKAGE1()
