#pragma once

#include "fb/string/HeapString.h"

FB_PACKAGE1(string)

	/**
	 * Returns characters in the range [start, start+length)
	 * start should be a positive zero based index
	 */
	inline HeapString getSubstring(StringRef str, SizeType start, SizeType length = 0xFFFFFFFF)
	{
		SizeType stringLength = str.getLength();
		if (start >= stringLength)
			return HeapString();

		if (length > stringLength || start + length > stringLength)
		{
			length = stringLength - start;
		}
		return HeapString(str.getPointer() + start, length);
	}
FB_END_PACKAGE1()
