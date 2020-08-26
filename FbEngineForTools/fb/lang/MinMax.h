#pragma once

#include "fb/lang/platform/ForceInline.h"

FB_PACKAGE1(lang)

template<class T>
FB_FORCEINLINE T min(T a, T b)
{
	return a < b ? a : b;
}

template<class T, class... Rest>
FB_FORCEINLINE T min(T a, T b, Rest... rest)
{
	return min(a, min(b, rest...));
}

template<class T>
FB_FORCEINLINE T max(T a, T b)
{
	return a > b ? a : b;
}

template<class T, class... Rest>
FB_FORCEINLINE T max(T a, T b, Rest... rest)
{
	return max(a, max(b, rest...));
}

template<class T>
FB_FORCEINLINE T clamp(T value, T min, T max)
{
	return value > min ? (value < max ? value : max) : min;
}

template<class T>
FB_FORCEINLINE T abs(T value)
{
	return value >= 0 ? value : -value;
}

FB_END_PACKAGE1()
