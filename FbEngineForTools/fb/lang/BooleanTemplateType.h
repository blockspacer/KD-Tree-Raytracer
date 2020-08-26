#pragma once

FB_PACKAGE1(lang)
template<class T, T Val>
struct IntegralConstant
{
	static constexpr T value = Val;
};

typedef IntegralConstant<bool, true> TrueType;
typedef IntegralConstant<bool, false> FalseType;

FB_END_PACKAGE1()
