#pragma once

#include <cstring>

FB_PACKAGE1(lang)

static inline void fastBigMemoryCopy(void *dst, const void *src, SizeType bytes, bool flushCacheIfPossible = true)
{
	memcpy(dst, src, bytes);
}

static inline void fastSmallMemoryCopy(void *dst, const void *src, SizeType bytes, bool flushCacheIfPossible = false)
{
	memcpy(dst, src, bytes);
}

FB_END_PACKAGE1()
