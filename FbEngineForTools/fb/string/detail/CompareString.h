#pragma once

#include "fb/string/Common.h"
#include "fb/lang/Types.h"

FB_DECLARE0(StringRef)

FB_PACKAGE2(string, detail)

/**
 * Compares the given two strings for equality only, and doing it case-insensitively.
 * @return returns true if the strings are equal when ignoring alphabetic (note, possibly A-Z only) letter case.
 */
bool compareStringCaseInsensitive(const StringRef &string1, const StringRef &string2);

/**
 * Compares the given two strings for equality and sorting order.
 *
 * @return returns CompareResultEqual if the strings are equal,
 *                 CompareResultFirstLessThanSecond if the first string is "less than" (sorted before) the second string,
 *                 CompareResultFirstGreaterThanSecond if the first string is "greater than" (sorted after) the second string.
 */
CompareResult compareStringWithSorting(const StringRef &string1, const StringRef &string2);

/**
 * Compares the given two strings for equality and sorting order, ignoring case.
 *
 * @return returns CompareResultEqual if the strings are equal,
 *                 CompareResultFirstLessThanSecond if the first string is "less than" (sorted before) the second string,
 *                 CompareResultFirstGreaterThanSecond if the first string is "greater than" (sorted after) the second string.
 */
CompareResult compareStringWithSortingCaseInsensitive(const StringRef &string1, const StringRef &string2);

/**
 * Compares if the given (second) string is found as a substring at specified index of another string.
 *
 * NOTICE, check the string for the length of the given substring only, the string being looked up for the substring
 * may still have a tail after the substring. Any remaining tail will not affect the result.
 * In other words (pseudocode): compareSubstring("abcd", 1, "bc") == true, but still, &"abcd"[1] != "bc" (it is "bcd")
 */
bool compareSubstring(const StringRef &string, SizeType index, const StringRef &substring);

/**
 * Compares if the given (second) string is found as a substring at specified index of another string.
 *
 * Compares the given two strings for equality only, and doing it case-insensitively.
 *
 * NOTICE, check the string for the length of the given substring only, the string being looked up for the substring
 * may still have a tail after the substring. Any remaining tail will not affect the result.
 * In other words (pseudocode): compareSubstring("abcd", 1, "bc") == true, but still, &"abcd"[1] != "bc" (it is "bcd")
 */
bool compareSubstringCaseInsensitive(const StringRef &string, SizeType index, const StringRef &substring);

FB_END_PACKAGE2()
