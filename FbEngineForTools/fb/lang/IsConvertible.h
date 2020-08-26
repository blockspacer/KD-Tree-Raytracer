#pragma once

#if FB_COMPILER == FB_MSC || FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC || FB_COMPILER == FB_ICC

#define FB_IS_CONVERTIBLE(p_from, p_to) __is_convertible_to(p_from, p_to)

#else

#error "Unhandled compiler option for IsConvertible<T>"

#endif

FB_PACKAGE1(lang)

template<typename T, typename U>
struct IsConvertible
{
	static const bool value = FB_IS_CONVERTIBLE(T, U);
};

FB_END_PACKAGE1()
