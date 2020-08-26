#pragma once

#include <stddef.h>

FB_PACKAGE1(lang)

#define FB_ALIGN_VALUE(value, alignment) (((value) + (alignment) - 1) & (~((alignment) - 1)))

/// Align given value
size_t alignValue(size_t value, size_t alignment);

FB_END_PACKAGE1()
