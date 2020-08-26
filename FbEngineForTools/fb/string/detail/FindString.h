#pragma once

#include "fb/lang/Types.h"

FB_DECLARE0(StringRef)

FB_PACKAGE2(string, detail)

	/* Searches for second string inside the first string, starting from given position. Returns index where the first 
	 * occurrence of the given string was found or string::NoPosition, if not found */
	SizeType findString(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType startingAtPosition = 0);

	/* Searches for second string inside the first string, starting from given position. Does case-insensitive 
	 * comparison. Returns index where the first occurrence of the given string was found or string::NoPosition, if 
	 * not found */
	SizeType findStringCaseInsensitive(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType startingAtPosition = 0);

	/* Same as findString, but returns starting position of the last occurrence of the searched for string, instead of 
	 * the first */
	SizeType findStringRight(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType start = ~0U);

	/* Counts and returns the number times the given string is found in string.
	 * The matches can be overlapping, f.ex. findStringCountOf("aaa", "aa") returns 2. */
	SizeType findStringCountOf(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType start = ~0U);

FB_END_PACKAGE2()
