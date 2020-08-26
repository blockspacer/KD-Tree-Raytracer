#include "Precompiled.h"
#include "Alignment.h"
#include "AlignmentFunctions.h"

FB_PACKAGE1(lang)

/// Align given value
size_t alignValue(size_t value, size_t alignment)
{
	if (!alignment)
		alignment = DefaultAlignment;

	/*
	size_t offset = alignment - (value % alignment);
	if (offset != alignment)
		result += offset;
	*/

	size_t result = FB_ALIGN_VALUE(value, alignment);
	fb_expensive_assert(result >= value && result % alignment == 0);
	return result;
}

FB_END_PACKAGE1()
