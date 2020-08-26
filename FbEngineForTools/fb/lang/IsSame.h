#pragma once

FB_PACKAGE1(lang)

template<typename T, typename U>
struct IsSame
{
	static const bool value = false;
};

template<typename T>
struct IsSame<T, T>
{
	static const bool value = true;
};

FB_END_PACKAGE1()
