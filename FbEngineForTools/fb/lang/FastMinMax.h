#pragma once

#include "fb/lang/platform/FBMinMax.h"

FB_PACKAGE1(lang)

template<class T>
T fastMin(T a, T b)
{
	return a < b ? a : b;
}

template<class T>
T fastMax(T a, T b)
{
	return a > b ? a : b;
}

template<class T>
T fastClamp(T value, T min, T max)
{
	return value < min ? min : (value > max ? max : value);
}

template<>
inline float fastMin(float a, float b)
{
	return FB_FMIN(a, b);
}

template<>
inline float fastMax(float a, float b)
{
	return FB_FMAX(a, b);
}

template<>
inline float fastClamp(float value, float min, float max)
{
	return FB_FCLAMP(value, min, max);
}

FB_END_PACKAGE1()
