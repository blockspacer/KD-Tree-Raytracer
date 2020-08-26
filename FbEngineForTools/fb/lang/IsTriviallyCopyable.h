#pragma once

#if FB_COMPILER == FB_MSC || FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC || FB_COMPILER == FB_ICC

#define FB_IS_TRIVIALLY_COPYABLE(p_type) __is_trivially_copyable(p_type)

#else

#error "Unhandled compiler option for IsTriviallyCopyable<T>"

#endif

FB_PACKAGE1(lang)

template<typename T>
struct IsTriviallyCopyable
{
	static const bool value = FB_IS_TRIVIALLY_COPYABLE(T);
};

FB_END_PACKAGE1()
