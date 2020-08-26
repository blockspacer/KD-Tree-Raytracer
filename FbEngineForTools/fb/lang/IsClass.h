#pragma once

#if FB_COMPILER == FB_MSC || FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC || FB_COMPILER == FB_ICC

#define FB_IS_CLASS(p_type) __is_class(p_type)

#else

#error "Unhandled compiler option for IsClass<T>"

#endif

FB_PACKAGE1(lang)

template <class T>
struct IsClass
{
	static const bool value = FB_IS_CLASS(T);
};

FB_END_PACKAGE1()
