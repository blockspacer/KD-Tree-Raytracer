#pragma once

/* Common stuff for strings. Particularly implementations for utils */

#include "fb/string/Config.h"

#include "fb/string/detail/CaseConversion.h"
#include "fb/string/detail/CompareString.h"
#include "fb/string/detail/Filename.h"
#include "fb/string/detail/FindString.h"
#include "fb/string/detail/ReplaceChar.h"
#include "fb/string/detail/ReplaceSubstring.h"
#include "fb/string/detail/StringToNumberConversion.h"

FB_PACKAGE1(string)

/* Use macros here in .cpp to add implementations of various methods to string. See Common.h for declaration macros */

#define FB_STRING_CASE_DETECTION_IMPL(p_thisclass) \
	bool p_thisclass::isUpper(SizeType index) const \
	{ \
		return string::isUpper((*this)[index]); \
	} \
	bool p_thisclass::isLower(SizeType index) const \
	{ \
		return string::isLower((*this)[index]); \
	}


#define FB_STRING_CASE_CONVERSION_IMPL(p_thisclass) \
	void p_thisclass::toUpper(SizeType index) \
	{ \
		(*this)[index] = string::toUpper((*this)[index]); \
	} \
	void p_thisclass::toLower(SizeType index) \
	{ \
		(*this)[index] = string::toLower((*this)[index]); \
	} \
	void p_thisclass::toUpper() \
	{ \
		for (SizeType i = 0, len = getLength(); i < len; ++i) \
			(*this)[i] = string::toUpper((*this)[i]); \
	} \
	void p_thisclass::toLower() \
	{ \
		for (SizeType i = 0, len = getLength(); i < len; ++i) \
			(*this)[i] = string::toLower((*this)[i]); \
	} \
	void p_thisclass::fromUnderScoredToCamelCase() \
	{ \
		string::detail::convertLowerCaseToCamelCase(*this); \
	} \
	void p_thisclass::fromCamelCaseToUnderScored() \
	{ \
		string::detail::convertCamelCaseToLowerCase(*this); \
	} \
	void p_thisclass::fromUnderScoredToSpacedCapitalized() \
	{ \
		string::detail::convertUnderScoredToSpacedCapitalized(*this); \
	}


#define FB_STRING_COMPARE_IMPL(p_thisclass) \
	bool p_thisclass::compareCaseInsensitive(const StringRef &other) const \
	{ \
		return string::detail::compareStringCaseInsensitive(*this, other); \
	} \
	string::CompareResult p_thisclass::compareWithSorting(const StringRef &other) const \
	{ \
		return string::detail::compareStringWithSorting(*this, other); \
	} \
	string::CompareResult p_thisclass::compareWithSortingCaseInsensitive(const StringRef &other) const \
	{ \
		return string::detail::compareStringWithSortingCaseInsensitive(*this, other); \
	} \
	bool p_thisclass::compareSubstring(SizeType index, const StringRef &subStringToFind) const \
	{ \
		return string::detail::compareSubstring(*this, index, subStringToFind); \
	} \
	bool p_thisclass::compareSubstringCaseInsensitive(SizeType index, const StringRef &subStringToFind) const \
	{ \
		return string::detail::compareSubstringCaseInsensitive(*this, index, subStringToFind); \
	}


#define FB_STRING_MODIFY_IMPL(p_thisclass) \
		void p_thisclass::replace(char lookForChar, char replaceWithChar) \
		{ \
			string::detail::replaceChar(*this, lookForChar, replaceWithChar); \
		} \
		void p_thisclass::replace(const StringRef &lookForString, const StringRef &replaceWithString) \
		{ \
			string::detail::replaceSubstring(*this, lookForString, replaceWithString); \
		} \
		void p_thisclass::convertToQuotedString() \
		{ \
			/* Convert slashes to double slashes */ \
			string::detail::replaceSubstring(*this, "\\", "\\\\"); \
			/* Convert quotes to slashed quotes */ \
			string::detail::replaceSubstring(*this, "\"", "\\\""); \
			/* Add quotes */ \
			insert(0, "\"", 1); \
			*this << "\""; \
		}


#define FB_STRING_FIND_IMPL(p_thisclass) \
	SizeType p_thisclass::find(const StringRef &stringToLookFor, SizeType startAtPosition) const \
	{ \
		return string::detail::findString(*this, stringToLookFor, startAtPosition); \
	} \
	SizeType p_thisclass::findEndOf(const StringRef &stringToLookFor, SizeType startAtPosition) const \
	{ \
		SizeType pos = string::detail::findString(*this, stringToLookFor, startAtPosition); \
		return pos != string::NoPosition ? pos + stringToLookFor.getLength() : string::NoPosition; \
	} \
	SizeType p_thisclass::findCaseInsensitive(const StringRef &stringToLookFor, SizeType startAtPosition) const \
	{ \
		return string::detail::findStringCaseInsensitive(*this, stringToLookFor, startAtPosition); \
	} \
	SizeType p_thisclass::findRight(const StringRef &stringToLookFor, SizeType startAtPosition) const \
	{ \
		return string::detail::findStringRight(*this, stringToLookFor, startAtPosition); \
	} \
	SizeType p_thisclass::findRightEndOf(const StringRef &stringToLookFor, SizeType startAtPosition) const \
	{ \
		SizeType pos = string::detail::findStringRight(*this, stringToLookFor, startAtPosition); \
		return pos != string::NoPosition ? pos + stringToLookFor.getLength() : string::NoPosition; \
	} \
	bool p_thisclass::doesContain(char character) const \
	{ \
		return strchr(getPointer(), character) != nullptr; \
	} \
	bool p_thisclass::doesContainCaseInsensitive(char character) const \
	{ \
		if (doesContain(string::toLower(character))) \
			return true; \
		else \
			return doesContain(string::toUpper(character)); \
	} \
	bool p_thisclass::doesContain(const StringRef &stringToLookFor) const \
	{ \
		return string::detail::findString(*this, stringToLookFor, 0) != string::NoPosition; \
	} \
	bool p_thisclass::doesContainCaseInsensitive(const StringRef &stringToLookFor) const \
	{ \
		return string::detail::findStringCaseInsensitive(*this, stringToLookFor, 0) != string::NoPosition; \
	} \
	bool p_thisclass::doesStartWith(const StringRef &stringToLookFor) const \
	{ \
		return string::detail::compareSubstring(*this, 0, stringToLookFor); \
	} \
	bool p_thisclass::doesStartWithCaseInsensitive(const StringRef &stringToLookFor) const \
	{ \
		return string::detail::compareSubstringCaseInsensitive(*this, 0, stringToLookFor); \
	} \
	bool p_thisclass::doesEndWith(const StringRef &stringToLookFor) const \
	{ \
		if (stringToLookFor.getLength() > getLength()) \
		{ \
			return false; \
		} \
		SizeType startIndex = getLength() - stringToLookFor.getLength(); \
		return string::detail::compareSubstring(*this, startIndex, stringToLookFor); \
	} \
	bool p_thisclass::doesEndWithCaseInsensitive(const StringRef &stringToLookFor) const \
	{ \
		if (stringToLookFor.getLength() > getLength()) \
		{ \
			return false; \
		} \
		SizeType startIndex = getLength() - stringToLookFor.getLength(); \
		return string::detail::compareSubstringCaseInsensitive(*this, startIndex, stringToLookFor); \
	} \
	SizeType p_thisclass::findCountOf(const StringRef &stringToLookFor, SizeType startAtPosition) const \
	{ \
		return string::detail::findStringCountOf(*this, stringToLookFor, startAtPosition); \
	} \


#define FB_STRING_NUMBER_PARSE_IMPL(p_thisclass) \
	bool p_thisclass::parse(int8_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(uint8_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(int16_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(uint16_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(int32_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(uint32_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(int64_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(uint64_t &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(float &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parse(double &resultOut) const \
	{ \
		return string::detail::parseNumber(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(int8_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(uint8_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(int16_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(uint16_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(int32_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(uint32_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(int64_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(uint64_t &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(float &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(double &resultOut) const \
	{ \
		return string::detail::parseNumberLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseHex(uint8_t &resultOut) const \
	{ \
		return string::detail::parseNumberAsHex(*this, resultOut); \
	} \
	bool p_thisclass::parseHex(uint16_t &resultOut) const \
	{ \
		return string::detail::parseNumberAsHex(*this, resultOut); \
	} \
	bool p_thisclass::parseHex(uint32_t &resultOut) const \
	{ \
		return string::detail::parseNumberAsHex(*this, resultOut); \
	} \
	bool p_thisclass::parseHex(uint64_t &resultOut) const \
	{ \
		return string::detail::parseNumberAsHex(*this, resultOut); \
	} \
	bool p_thisclass::parseArithmeticOperation(int32_t &resultOut) const \
	{ \
		return string::detail::parseNumberOperation(*this, resultOut); \
	} \
	bool p_thisclass::parseArithmeticOperation(uint32_t &resultOut) const \
	{ \
		return string::detail::parseNumberOperation(*this, resultOut); \
	} \
	bool p_thisclass::parseArithmeticOperation(int64_t &resultOut) const \
	{ \
		return string::detail::parseNumberOperation(*this, resultOut); \
	} \
	bool p_thisclass::parseArithmeticOperation(uint64_t &resultOut) const \
	{ \
		return string::detail::parseNumberOperation(*this, resultOut); \
	} \
	bool p_thisclass::parseArithmeticOperation(float &resultOut) const \
	{ \
		return string::detail::parseNumberOperation(*this, resultOut); \
	} \
	bool p_thisclass::parseArithmeticOperation(double &resultOut) const \
	{ \
		return string::detail::parseNumberOperation(*this, resultOut); \
	}


#define FB_STRING_BOOL_PARSE_IMPL(p_thisclass) \
	bool p_thisclass::parse(bool &resultOut) const \
	{ \
		return string::detail::parseBool(*this, resultOut); \
	} \
	bool p_thisclass::parseLoosely(bool &resultOut) const \
	{ \
		return string::detail::parseBoolLoosely(*this, resultOut); \
	} \
	bool p_thisclass::parseStrictly(bool &resultOut) const \
	{ \
		return string::detail::parseBoolStrictly(*this, resultOut); \
	}


#define FB_STRING_FILE_IMPL(p_thisclass) \
	p_thisclass &p_thisclass::appendPathFromString(const StringRef &pathString) \
	{ \
		string::detail::appendPath(*this, pathString); \
		return *this; \
	} \
	StringRef p_thisclass::getFileExtension() const \
	{ \
		return string::detail::getExtension(*this); \
	} \
	p_thisclass &p_thisclass::appendFileExtensionFromString(const StringRef &pathString) \
	{ \
		*this << string::detail::getExtension(pathString); \
		return *this; \
	} \
	p_thisclass &p_thisclass::appendPathWithoutExtensionFromString(const StringRef &pathString) \
	{ \
		string::detail::appendWithoutExtension(*this, pathString); \
		return *this; \
	} \
	p_thisclass &p_thisclass::appendFileNameWithoutExtensionFromString(const StringRef &pathString) \
	{ \
		string::detail::appendFileWithoutExtension(*this, pathString); \
		return *this; \
	} \
	StringRef p_thisclass::getFileName() const \
	{ \
		return string::detail::getFile(*this); \
	} \
	p_thisclass &p_thisclass::appendFileNameFromString(const StringRef &pathString) \
	{ \
		*this << string::detail::getFile(pathString); \
		return *this; \
	} \
	p_thisclass &p_thisclass::solvePath() \
	{ \
		string::detail::solvePath(*this); \
		return *this; \
	}



FB_END_PACKAGE1()
