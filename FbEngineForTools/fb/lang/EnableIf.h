#pragma once

FB_PACKAGE1(lang)

template<bool C, typename T = void>
struct EnableIf
{
	typedef T type;
};

template<typename T>
struct EnableIf<false, T>
{
};

FB_END_PACKAGE1()
