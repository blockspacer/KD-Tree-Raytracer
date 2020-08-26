#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE0()

extern SizeType fbStrLen(const char *str);

inline SizeType cstrlen(const char* str)
{
	return str ? fbStrLen(str) : 0U;
}

template<SizeType N>
inline SizeType arrlen(const char(&charArray)[N])
{
	return N - 1U;
}

FB_END_PACKAGE0()
