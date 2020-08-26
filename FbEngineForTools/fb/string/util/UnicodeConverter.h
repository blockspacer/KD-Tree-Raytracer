#pragma once

#include "SimpleUTF16String.h"

#include "fb/container/PodVector.h"
#include "fb/lang/Types.h"
#include "fb/string/HeapString.h"

FB_DECLARE_STRUCT(string, UTF8MultiByte)
FB_DECLARE_STRUCT(string, UTF16MultiWord)

FB_PACKAGE1(string)

/**
 * Class to convert between Unicode encodings. Currently just houses few static methods.
 * Not lightning fast, but doesn't do excessive heap allocation or other really stupid things.
 * TODO: Integrate better with string lib
 */
class UnicodeConverter
{
public:
	/* Error handling strategies used in UTF-8 conversion:
	 *   AssertOnError: assert whenever an error is encountered
	 *   UseReplacementChar: if there is an error during conversion, use the unicode replacement character instead
	 *   IgnoreErrors: skip the character if there is an error during conversion */
	enum ErrorHandling
	{
		AssertOnError,
		UseReplacementChar,
		IgnoreErrors
	};

	/* Convert UTF-16 character (or unit) or character + surrogate to UTF-32 code point. Possible
	 * surrogate may be specified even if it won't be needed. Will return number of units used (one
	 * or two) or negative values (see below), if there's an error. In case of error, destination
	 * will be Unicode replacement character. */
	static int convertUTF16ToUTF32(UTF32Unit &destination, UTF16Unit utf16, UTF16Unit surrogate = 0);
	static int getInvalidUTF16UnitError() { return -1; }
	static int convertUTF32ToUTF16(UTF16MultiWord &destination, UTF32Unit utf32);
	static int getInvalidUTF32CodePointError() { return -1; }
	static int getUTF16LowSurrogateMissingError() { return -2; }
	/* Convert 1 to 4 UTF-8 characters (or units) to UTF-32 code point. As per UTF-8, the first
	 * octet determines whether the next ones are needed or not. Unused octets are ignored and may
	 * contain any data (so it is possible to use this to convert octet stream without knowing
	 * where each character starts and ends). Returns number of octets used or negative values if
	 * there is an error. In case of error, destination will be Unicode replacement character. */
	static int convertUTF8ToUTF32(UTF32Unit &destination, unsigned char octet1, unsigned char octet2, unsigned char octet3, unsigned char octet4, ErrorHandling errorHandling);
	static int convertUTF8ToUTF32(UTF32Unit &destination, UTF8Unit octet1, UTF8Unit octet2 = 0, UTF8Unit octet3 = 0, UTF8Unit octet4 = 0, ErrorHandling errorHandling = AssertOnError);
	static int convertUTF8ToUTF32(UTF32Unit &destination, const UTF8Unit *data, SizeType dataLength, ErrorHandling errorHandling = AssertOnError);
	static int getInvalidUTF8UnitError() { return -1; }
	static int getInvalidUTF8LengthError() { return -2; }
	static int getInvalidUTF8DataLengthError() { return -3; }
	/* Converts UTF-32 code point to UTF-8 multi-byte character array. */
	static int convertUTF32ToUTF8(UTF8MultiByte &destination, UTF32Unit utf32, ErrorHandling errorHandling = AssertOnError);
	/* Stuff that returns true on success. */
	static bool addUTF32CPToUTF8String(UTF32Unit source, HeapString &destination, ErrorHandling errorHandling = AssertOnError);
	static bool addUTF32StrToUTF8String(const PodVector<UTF32Unit> &source, HeapString &destination, ErrorHandling errorHandling = AssertOnError);
	static bool addUTF16StrToUTF8String(const UTF16Unit *source, SizeType sourceLen, HeapString &destination, ErrorHandling errorHandling = AssertOnError);
	static bool addUTF16StrToUTF8String(const UTF16Unit *source, HeapString &destination, ErrorHandling errorHandling = AssertOnError);
	static bool addUTF16StrToUTF8String(const SimpleUTF16String &source, HeapString &destination, ErrorHandling errorHandling = AssertOnError);

	static bool addUTF32CPToUTF16String(UTF32Unit source, SimpleUTF16String &destination);
	static bool addUTF8StrToUTF16String(const StringRef &source, SimpleUTF16String &destination, ErrorHandling errorHandling = AssertOnError);
	static bool addUTF8StrToUTF16String(const UTF8Unit *source, SizeType sourceLen, SimpleUTF16String &destination, ErrorHandling errorHandling = AssertOnError);
	static bool addUTF8StrToUTF16String(const UTF8Unit *source, SimpleUTF16String &destination, ErrorHandling errorHandling = AssertOnError);

	static bool addUTF8StrToUTF32String(StringRef utf8Str, PodVector<UTF32Unit>& outUtf32String, ErrorHandling errorHandling = AssertOnError);

	static SizeType getLength(const UTF8Unit *source);
	static SizeType getLength(const UTF16Unit *source);

	static bool hasBOM(StringRef utf8Source);
	template <class StringType>
	static void stripBOM(StringType &utf8Source);
	template <class StringType>
	static bool stripBOMIfNecessary(StringType &utf8Source);

	static int getNumOctetsInUTF8Char(UTF8Unit firstOctet, ErrorHandling errorHandling = AssertOnError);
	static int getInvalidUTF8StartOctet() { return -1; }
	static int getNumOctetsInCorrespondingUTF8Char(UTF32Unit codePoint);
	static int getNumUnitsInUTF16Char(UTF16Unit firstUnit);
	static int getNumUnitsInCorrespondingUTF16Char(UTF32Unit codePoint);
	static int getInvalidUTF16StartUnit() { return -1; }

	static int getNotImplementedError() { return -16; }
	/* Unicode consts, bitmasks, etc. */
	static UTF32Unit getReplacementCharCodePoint() { return 0x0000FFFD; }
	static UTF32Unit getMaxLegalUTF32() { return 0x0010FFFF; }
	static UTF32Unit getLowSurrogateStart() { return 0xDC00; }
	static UTF32Unit getLowSurrogateEnd() { return 0xDFFF; }
	static UTF32Unit getHighSurrogateStart() { return 0xD800; }
	static UTF32Unit getHighSurrogateEnd() { return 0xDBFF; }
};

struct UTF8MultiByte
{
	int getDataLength() { return UnicodeConverter::getNumOctetsInUTF8Char(data[0]); }

	UTF8Unit data[4];
};

struct UTF16MultiWord
{
	int getDataLength() { return UnicodeConverter::getNumUnitsInUTF16Char(data[0]); }
	UTF16Unit data[2];
};

template <class StringType>
void UnicodeConverter::stripBOM(StringType &utf8Source)
{
	fb_assert(hasBOM(utf8Source));
	StringType tmpString(utf8Source.getPointer() + 3, utf8Source.getLength() - 3);
	utf8Source = tmpString;
}

template <class StringType>
bool UnicodeConverter::stripBOMIfNecessary(StringType &utf8Source)
{
	if (!hasBOM(utf8Source))
		return false;

	stripBOM(utf8Source);
	return true;
}

FB_END_PACKAGE1()
