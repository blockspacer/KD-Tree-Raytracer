#pragma once

#include "fb/algorithm/CompareResult.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/Types.h"

/* Common stuff for strings. Like common constanst and declarations for utils */


FB_PACKAGE1(string)

/* Index marking e.g. not found for searches. Every string type should use the same value */
static const SizeType NoPosition = 0xFFFFFFFF;

/* Compare result enum aliased from algorithm */
typedef algorithm::CompareResult CompareResult;
static const CompareResult CompareResultEqual = algorithm::CompareResultEqual;
static const CompareResult CompareResultFirstGreaterThanSecond = algorithm::CompareResultFirstGreaterThanSecond;
static const CompareResult CompareResultFirstLessThanSecond = algorithm::CompareResultFirstLessThanSecond;

/* Case detection and conversion for single characters */

static inline bool isLower(char c)
{
	return c >= 'a' && c <= 'z';
}

static inline bool isUpper(char c)
{
	return c >= 'A' && c <= 'Z';
}

static inline char toLower(char c)
{
	/* This isn't really necessary. Offset would work fine wrapped around */
	fb_static_assert('A' < 'a' && "Something wrong with ASCII?");
	const uint8_t offset = 'a' - 'A';
	if (isUpper(c))
		return c + offset;
	else
		return c;
}

static inline char toUpper(char c)
{
	const uint8_t offset = 'a' - 'A';
	if (isLower(c))
		return c - offset;
	else
		return c;
}

/* Use macros here in .h to add various methods to string. See CommonImpl.h for implementation macros */

#define FB_STRING_CASE_DETECTION_DECL() \
	public: \
		/* Returns true if character in given index is upper case */ \
		bool isUpper(SizeType index) const; \
		/* Returns true if character in given index is lower case */ \
		bool isLower(SizeType index) const; \
	private:


#define FB_STRING_CASE_CONVERSION_DECL() \
	public: \
		/* Converts character in given index to upper case */ \
		void toUpper(SizeType index); \
		/* Converts character in given index to lower case */ \
		void toLower(SizeType index); \
		/* Converts whole string to upper case */ \
		void toUpper(); \
		/* Converts whole string to lower case */ \
		void toLower(); \
		/* Converts whole string from underscored (lower_case) to camelCase */ \
		void fromUnderScoredToCamelCase(); \
		/* Converts whole string from camelCase to underscored (lower_case) */ \
		void fromCamelCaseToUnderScored(); \
		/* Converts whole string from underscored (lower_case) to normal with spaces and capitalized first letters (Lower Case) */ \
		void fromUnderScoredToSpacedCapitalized(); \
	private:


#define FB_STRING_COMPARE_DECL() \
	public: \
		/* Does case-insensite comparison */ \
		bool compareCaseInsensitive(const StringRef &other) const; \
		/* Instead of true/false comparison, returns CompareResult that can be used to order strings */ \
		string::CompareResult compareWithSorting(const StringRef &other) const; \
		/* Instead of true/false comparison, returns CompareResult that can be used to order strings */ \
		string::CompareResult compareWithSortingCaseInsensitive(const StringRef &other) const; \
		/* Substring comparison. Compare given string to this string, starting from given index. Ignores tail, that is, \
		 * always compares at most the number of characters in subStringToFind */ \
		bool compareSubstring(SizeType index, const StringRef &subStringToFind) const; \
		/* Substring comparison. Compare given string to this string, ignoring case, starting from given index. Ignores \
		 * tail, that is, always compares at most the number of characters in subStringToFind */ \
		bool compareSubstringCaseInsensitive(SizeType index, const StringRef &subStringToFind) const; \
	private:


#define FB_STRING_MODIFY_DECL() \
	public: \
		/* Replaces every found instance with another char */ \
		void replace(char lookForString, char replaceWithString); \
		/* Replaces every found instance with another string */ \
		void replace(const StringRef &lookForString, const StringRef &replaceWithString); \
		/* Escapes quotes (\") and backslashes (\\). Adds quote (") to start and end of string */ \
		void convertToQuotedString(); \
	private:


#define FB_STRING_FIND_DECL() \
	public: \
		/* Searches for given string, starting from given position (zero by default). Returns index where the first \
		 * occurrence of the given string was found or string::NoPosition, if not found */ \
		SizeType find(const StringRef &stringToLookFor, SizeType startAtPosition = 0) const; \
		/* Same as findString, but instead of starting position of searched for string, returns index of the first 
		 * character not belonging to searched for string (basically findString(...) + stringToLookFor.getLength(), if 
		 * string is found) */ \
		SizeType findEndOf(const StringRef &stringToLookFor, SizeType startAtPosition = 0) const; \
		/* Searches for given string, starting from given position (zero by default). Does case-insensitive comparison. \
		 * Returns index where the first occurrence of the given string was found or string::NoPosition, if not found */ \
		SizeType findCaseInsensitive(const StringRef &stringToLookFor, SizeType startAtPosition = 0) const; \
		/* Same as findString, but returns starting position of the last occurrence of the searched for string, instead \ 
		 * of the first */ \
		SizeType findRight(const StringRef &stringToLookFor, SizeType startAtPosition = ~0U) const; \
		/* Same as findStringRight, but instead of starting position of searched for string, returns index of the 
		 * first character not belonging to searched for string (basically findStringRight(...) + 
		 * stringToLookFor.getLength(), if string is found) */ \
		SizeType findRightEndOf(const StringRef &stringToLookFor, SizeType startAtPosition = ~0U) const; \
		/* Returns true, if string contains given character */ \
		bool doesContain(char character) const; \
		/* Returns true, if string contains given character. Does case-insensitive comparison */ \
		bool doesContainCaseInsensitive(char character) const; \
		/* Returns true, if string contains given string */ \
		bool doesContain(const StringRef &stringToLookFor) const; \
		/* Returns true, if string contains given string. Does case-insensitive comparison */ \
		bool doesContainCaseInsensitive(const StringRef &stringToLookFor) const; \
		/* Returns true, if string starts with given string */ \
		bool doesStartWith(const StringRef &stringToLookFor) const; \
		/* Returns true, if string starts with given string. Does case-insensitive comparison */ \
		bool doesStartWithCaseInsensitive(const StringRef &stringToLookFor) const; \
		/* Returns true, if string ends to given string */ \
		bool doesEndWith(const StringRef &stringToLookFor) const; \
		/* Returns true, if string ends to given string. Does case-insensitive comparison */ \
		bool doesEndWithCaseInsensitive(const StringRef &stringToLookFor) const; \
		/* Counts and returns the number times the given string is found in string
		 * The matches can be overlapping, f.ex. ("aaa").findCountOf("aa") returns 2. */ \
		SizeType findCountOf(const StringRef &stringToLookFor, SizeType startAtPosition = 0) const; \
	private:


#define FB_STRING_NUMBER_PARSE_DECL() \
	public: \
		/**
		 * "parse(...)"
		 *   - Normal string to number conversions that quite closely match the C language number parsing
		 *   - Some minor formatting variations may be accepted, but nothing that could be ambiguously interpreted as different value
		 *
		 * "parseLoosely(...)"
		 *   - Human-readable type of number parsing
		 *   - Whitespace removals
		 *   - Empty string is zero
		 *   - Automatically try different number format (e.g. base 16)
		 *   - Generally accept small formatting errors
		 */ \
		bool parse(int8_t &resultOut) const; \
		bool parse(uint8_t &resultOut) const; \
		bool parse(int16_t &resultOut) const; \
		bool parse(uint16_t &resultOut) const; \
		bool parse(int32_t &resultOut) const; \
		bool parse(uint32_t &resultOut) const; \
		bool parse(int64_t &resultOut) const; \
		bool parse(uint64_t &resultOut) const; \
		bool parse(float &resultOut) const; \
		bool parse(double &resultOut) const; \
		/* Ignores whitespace, accepts empty string as zero, accepts base 16 when string starts with 0x or 0X, accepts 
		 * simple arithmetic operations */ \
		bool parseLoosely(int8_t &resultOut) const; \
		bool parseLoosely(uint8_t &resultOut) const; \
		bool parseLoosely(int16_t &resultOut) const; \
		bool parseLoosely(uint16_t &resultOut) const; \
		bool parseLoosely(int32_t &resultOut) const; \
		bool parseLoosely(uint32_t &resultOut) const; \
		bool parseLoosely(int64_t &resultOut) const; \
		bool parseLoosely(uint64_t &resultOut) const; \
		/* Ignores whitespace, accepts empty string as zero */ \
		bool parseLoosely(float &resultOut) const; \
		bool parseLoosely(double &resultOut) const; \
		/* Requires the string to start with 0x */ \
		bool parseHex(uint8_t &resultOut) const; \
		/* Requires the string to start with 0x */ \
		bool parseHex(uint16_t &resultOut) const; \
		/* Requires the string to start with 0x */ \
		bool parseHex(uint32_t &resultOut) const; \
		/* Requires the string to start with 0x */ \
		bool parseHex(uint64_t &resultOut) const; \
		/* Accept simple arithmetic operations */ \
		bool parseArithmeticOperation(int32_t &resultOut) const; \
		/* Accept simple arithmetic operations */ \
		bool parseArithmeticOperation(uint32_t &resultOut) const; \
		/* Accept simple arithmetic operations */ \
		bool parseArithmeticOperation(int64_t &resultOut) const; \
		/* Accept simple arithmetic operations */ \
		bool parseArithmeticOperation(uint64_t &resultOut) const; \
		/* Accept simple arithmetic operations */ \
		bool parseArithmeticOperation(float &resultOut) const; \
		/* Accept simple arithmetic operations */ \
		bool parseArithmeticOperation(double &resultOut) const; \
	private:


#define FB_STRING_BOOL_PARSE_DECL() \
	public: \
		/* Accepts case-insensitive true or false */ \
		bool parse(bool &resultOut) const; \
		/* Ignores whitespace. Accepts case-insensitive true or false, and 0 as false and any other integer as true. Empty string accepted as false */ \
		bool parseLoosely(bool &resultOut) const; \
		/* Only accepts case-sensitive true or false */ \
		bool parseStrictly(bool &resultOut) const; \
	private:


#define FB_STRING_FILE_DECL(p_thisclass) \
	public: \
		/* Appends file path with trailing slash
		 * "a/b/c.d" -> "a/b/"
		 * "a/b/" -> "a/b/"
		 * "/c.d" -> "/"
		 * "c.d" -> "" */ \
		p_thisclass &appendPathFromString(const StringRef &pathString); \
		/* Returns (or appends) file extension without dot
		 * "a/b/c.d" -> "d"
		 * "a/b/c." -> ""
		 * "a/b/c" -> ""
		 * "a/b/c.d/" -> ""
		 * "a/b/c./d" -> "" */ \
		StringRef getFileExtension() const; \
		p_thisclass &appendFileExtensionFromString(const StringRef &pathString); \
		/* Appends file and path but no extension(and no dot)
		 * "a/b/c.d" -> "a/b/c"
		 * "a/b/c." -> "a/b/c"
		 * "a/b/c" -> "a/b/c"
		 * "a/b/c.d/" -> "a/b/c.d/"
		 * "a/b/c./d" -> "a/b/c./d"
		 * "c.d" -> "c" */ \
		p_thisclass &appendPathWithoutExtensionFromString(const StringRef &pathString); \
		/* Appends file without path and without extension (and no dot)
		 * "a/b/c.d" -> "c"
		 * "a/b/" -> ""
		 * "/c.d" -> "c"
		 * "c.d" -> "c" */ \
		p_thisclass &appendFileNameWithoutExtensionFromString(const StringRef &pathString); \
		/* Returns (or appends) file without path
		 * "a/b/c.d" -> "c.d"
		 * "a/b/" -> ""
		 * "/c.d" -> "c.d"
		 * "c.d" -> "c.d" */ \
		StringRef getFileName() const; \
		p_thisclass &appendFileNameFromString(const StringRef &pathString); \
		/* Removes ".." and "." from path if possible
		 * "a/../b/c" -> "b/c"
		 * "a/b/../c/d" -> "a/c/d"
		 * "a/.." -> ""
		 * "../a/b" -> "../a/b"
		 * "/../a/b" -> "/../a/b"
		 * "./a/b" -> "a/b" */ \
		p_thisclass &solvePath(); \
		\
	private:


FB_END_PACKAGE1()
