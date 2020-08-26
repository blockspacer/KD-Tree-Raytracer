#include "Precompiled.h"
#include "UnicodeConverter.h"

#include "fb/lang/platform/Likely.h"
#include "fb/profiling/ZoneProfiler.h"

FB_PACKAGE1(string)

int UnicodeConverter::convertUTF16ToUTF32(UTF32Unit &destination, UTF16Unit utf16, UTF16Unit surrogate)
{
	if (utf16 >= getHighSurrogateStart() && utf16 <= getHighSurrogateEnd())
	{
		/* We seem to have surrogate pair */
		if (surrogate >= getLowSurrogateStart() && surrogate <= getLowSurrogateEnd())
		{
			/* Construct complete codepoint from high and low surrogates */
			destination = (uint16_t(utf16) - getHighSurrogateStart()) << 10;
			destination += uint16_t(surrogate) - getLowSurrogateStart();
			destination += 0x10000;
			return 2;
		}
		else
		{
			/* Surrogate character is not low surrogate */
			destination = getReplacementCharCodePoint();
			return getUTF16LowSurrogateMissingError();
		}
	}
	else if (utf16 >= getLowSurrogateStart() && utf16 <= getLowSurrogateEnd())
	{
		/* Low surrogate shouldn't be here alone */
		destination = getReplacementCharCodePoint();
		return getInvalidUTF16UnitError();
	}
	else
	{
		destination = utf16;
		return 1;
	}
}

int UnicodeConverter::convertUTF32ToUTF16(UTF16MultiWord &destination, UTF32Unit utf32)
{
	if (utf32 <= 0xFFFF)
	{
		if (!(utf32 >= getHighSurrogateStart() && utf32 <= getLowSurrogateEnd()))
		{
			destination.data[0] = UTF16Unit(utf32);
			destination.data[1] = 0;
			return 1;
		}
		else
		{
			destination.data[0] = UTF16Unit(getReplacementCharCodePoint());
			destination.data[1] = 0;
			return getInvalidUTF32CodePointError();
		}
	}
	else if (utf32 <= getMaxLegalUTF32())
	{
		/* Subtract 0x10000. This reflects that we are encoding code points from 0x10000 up (to 0x10FFFF). */
		utf32 -= 0x10000;
		/* Split between high and low ten bits. */
		destination.data[0] = UTF16Unit((utf32 >> 10) + getHighSurrogateStart());
		destination.data[1] = UTF16Unit((utf32 & 0x3FF) + getLowSurrogateStart());
		return 2;
	}
	else
	{
		destination.data[0] = UTF16Unit(getReplacementCharCodePoint());
		destination.data[1] = 0;
		return getInvalidUTF32CodePointError();
	}
}

static const UTF32Unit offsetsFromUTF8[5] = { 0x0, 0x0, 0x00003080, 0x000E2080, 0x03C82080 };

int UnicodeConverter::convertUTF8ToUTF32(UTF32Unit &destination, unsigned char octet1, unsigned char octet2, unsigned char octet3, unsigned char octet4, ErrorHandling errorHandling)
{
	destination = 0;
	UTF8MultiByte utf8mb;
	utf8mb.data[0] = (UTF8Unit)octet1;
	utf8mb.data[1] = (UTF8Unit)octet2;
	utf8mb.data[2] = (UTF8Unit)octet3;
	utf8mb.data[3] = (UTF8Unit)octet4;

	SizeType dataLength = 4;

	return convertUTF8ToUTF32(destination, utf8mb.data, dataLength, errorHandling);
}


int UnicodeConverter::convertUTF8ToUTF32(UTF32Unit &destination, UTF8Unit octet1, UTF8Unit octet2, UTF8Unit octet3, UTF8Unit octet4, ErrorHandling errorHandling)
{
	destination = 0;
	UTF8MultiByte utf8mb;
	utf8mb.data[0] = octet1;
	utf8mb.data[1] = octet2;
	utf8mb.data[2] = octet3;
	utf8mb.data[3] = octet4;

	SizeType dataLength = 4;

	return convertUTF8ToUTF32(destination, utf8mb.data, dataLength, errorHandling);
}

int UnicodeConverter::convertUTF8ToUTF32(UTF32Unit &destination, const UTF8Unit *data, SizeType dataLength, ErrorHandling errorHandling)
{
	UTF32Unit value = 0;
	int numOctets = getNumOctetsInUTF8Char(*data, errorHandling);

	if ((SizeType)numOctets > dataLength)
	{
		destination = getReplacementCharCodePoint();
		return getInvalidUTF8DataLengthError();
	}

	switch (numOctets)
	{
	/* Normal cases fall through */
	case 4:
		value += (unsigned char)(*data);
		value <<= 6;
		++data;
	case 3:
		value += (unsigned char)(*data);
		value <<= 6;
		++data;
	case 2:
		value += (unsigned char)(*data);
		value <<= 6;
		++data;
	case 1:
		value += (unsigned char)(*data);
		break;
	default:
		destination = getReplacementCharCodePoint();
		return getInvalidUTF8LengthError();
	}

	/* Magic values one of that must be subtracted from result (UTF-8 octet marker bits), depending
	 * on total length of sequence. */
	value -= offsetsFromUTF8[numOctets];
	destination = value;

	/* Check the final result */
	if (value <= 0xFFFF)
		return numOctets;

	if (value <= getMaxLegalUTF32() && !(value >= getHighSurrogateStart() && value <= getLowSurrogateEnd()))
		return numOctets;

	/* Something wrong. Not a valid code point. */
	destination = getReplacementCharCodePoint();
	return getInvalidUTF32CodePointError();
}

int UnicodeConverter::convertUTF32ToUTF8(UTF8MultiByte &destination, UTF32Unit utf32, ErrorHandling errorHandling)
{
	int length = getNumOctetsInCorrespondingUTF8Char(utf32);
	if (length <= 0 && errorHandling == UseReplacementChar)
	{
		length = 3;
		utf32 = getReplacementCharCodePoint();
	}

	/* Number of one bits at the beginning of first octet determines number of octets to follow */
	static const int lengthMarker[4] = { 0x00, 0xC0, 0xE0, 0xF0 };
	switch (length)
	{
	/* Note: everything falls through. Processing one octet in each step */
	case 4:
		destination.data[3] = UTF8Unit((utf32 | 0x80) & 0xBF);
		utf32 >>= 6;
	case 3:
		destination.data[2] = UTF8Unit((utf32 | 0x80) & 0xBF);
		utf32 >>= 6;
	case 2:
		destination.data[1] = UTF8Unit((utf32 | 0x80) & 0xBF);
		utf32 >>= 6;
	case 1:
		destination.data[0] = UTF8Unit(utf32 | lengthMarker[length - 1]);
		return length;
	default:
		fb_assertf((errorHandling != AssertOnError), "Invalid UTF-32 code point: %d.", utf32);
		return -100;
	}
}

bool UnicodeConverter::addUTF32CPToUTF8String(UTF32Unit source, HeapString &destination, ErrorHandling errorHandling)
{
	UTF8MultiByte tmpUTF8;
	int numChars = convertUTF32ToUTF8(tmpUTF8, source, errorHandling);
	if (numChars > 0)
	{
		for (int i = 0; i < numChars && tmpUTF8.data[i] != 0; ++i)
			destination += tmpUTF8.data[i];

		return true;
	}

	return false;
}

bool UnicodeConverter::addUTF32StrToUTF8String(const PodVector<UTF32Unit> &source, HeapString &destination, ErrorHandling errorHandling)
{
	bool fullSuccess = true;
	for (SizeType i = 0; i < source.getSize(); ++i)
	{
		if (addUTF32CPToUTF8String(source[i], destination, errorHandling))
			continue;

		fullSuccess = false;
	}

	return fullSuccess;
}

bool UnicodeConverter::addUTF16StrToUTF8String(const UTF16Unit *source, SizeType sourceLen, HeapString &destination, ErrorHandling errorHandling)
{
	bool fullSuccess = true;
	for (SizeType sourceIndex = 0; sourceIndex < sourceLen; ++sourceIndex)
	{
		UTF32Unit utf32 = 0;

		int numChars = sourceIndex < sourceLen - 1 ? convertUTF16ToUTF32(utf32, source[sourceIndex], source[sourceIndex + 1]) : convertUTF16ToUTF32(utf32, source[sourceIndex]);
		if (numChars > 0)
		{
			sourceIndex += numChars - 1;
			UTF8MultiByte utf8mb;
			int numU8chars = convertUTF32ToUTF8(utf8mb, utf32, errorHandling);
			for (int charIndex = 0; charIndex < numU8chars; ++charIndex)
				destination += utf8mb.data[charIndex];
		}
		else
		{
			/* Replace broken UTF-16 with replacement char and move on */
			addUTF32CPToUTF8String(getReplacementCharCodePoint(), destination);
			fullSuccess = false;
		}
	}
	return fullSuccess;
}

bool UnicodeConverter::addUTF16StrToUTF8String(const UTF16Unit *source, HeapString &destination, ErrorHandling errorHandling)
{
	return addUTF16StrToUTF8String(source, getLength(source), destination, errorHandling);
}

bool UnicodeConverter::addUTF16StrToUTF8String(const SimpleUTF16String &source, HeapString &destination, ErrorHandling errorHandling)
{
	return addUTF16StrToUTF8String(source.getPointer(), source.getSize(), destination, errorHandling);
}

bool UnicodeConverter::addUTF32CPToUTF16String(UTF32Unit source, SimpleUTF16String &destination)
{
	fb_assert(destination.data.getSize() > 0 && destination.data.getBack() == getUTF16NulChar() && "Destination is broken.");
	UTF16MultiWord tmpUTF16;
	bool success = convertUTF32ToUTF16(tmpUTF16, source) > 0;
	destination.data.popBack();
	for (int i = 0, iend = tmpUTF16.getDataLength(); i < iend; ++i)
		destination.data.pushBack(tmpUTF16.data[i]);

	destination.data.pushBack(getUTF16NulChar());
	return success;
}

bool UnicodeConverter::addUTF8StrToUTF16String(const StringRef &source, SimpleUTF16String &destination, ErrorHandling errorHandling)
{
	return addUTF8StrToUTF16String(source.getPointer(), source.getLength(), destination, errorHandling);
}

bool UnicodeConverter::addUTF8StrToUTF16String(const UTF8Unit *source, SizeType sourceLen, SimpleUTF16String &destination, ErrorHandling errorHandling)
{
	bool fullSuccess = true;
	if (sourceLen)
	{
		destination.reserve(sourceLen + 2);

		for (SizeType i = 0; i < sourceLen; ++i)
		{
			int numOctets = getNumOctetsInUTF8Char(source[i], errorHandling);
			if (numOctets != getInvalidUTF8StartOctet())
			{
				if (i + numOctets - 1 < sourceLen)
				{
					UTF32Unit utf32 = 0;
					int result = 0;
					switch (numOctets)
					{
					case 1:
						result = convertUTF8ToUTF32(utf32, source[i]);
						break;
					case 2:
						result = convertUTF8ToUTF32(utf32, source[i], source[i + 1]);
						break;
					case 3:
						result = convertUTF8ToUTF32(utf32, source[i], source[i + 1], source[i + 2]);
						break;
					case 4:
						result = convertUTF8ToUTF32(utf32, source[i], source[i + 1], source[i + 2], source[i + 3]);
						break;
					default:
						// This is not an error in the UTF-8 string but getNumOctetsInUTF8Char() returned garbage,
						// so let's just ignore errorHandling and assert anyway
						fb_assert(0 && "Should never get here. Check what's wrong with getNumOctetsInUTF8Char.");
						return false;
					}
					if (result > 0 || errorHandling == UseReplacementChar)
					{
						// if the conversion was successful, or it wasn't but we want to write the replacement character...
						i += numOctets - 1;
						addUTF32CPToUTF16String(utf32, destination);
					}
					else if (errorHandling == AssertOnError)
					{
						fb_assert(0 && "Converting UTF-8 character to UTF-32 failed");
					}
				}
				else
				{
					if (errorHandling == UseReplacementChar)
					{
						addUTF32CPToUTF16String(getReplacementCharCodePoint(), destination);
					}
					return false;
				}
			}
			else
			{
				if (errorHandling == UseReplacementChar)
				{
					addUTF32CPToUTF16String(getReplacementCharCodePoint(), destination);
				}
				fullSuccess = false;
			}
		}
	}
	return fullSuccess;
}

bool UnicodeConverter::addUTF8StrToUTF16String(const UTF8Unit *source, SimpleUTF16String &destination, ErrorHandling errorHandling)
{
	return addUTF8StrToUTF16String(source, getLength(source), destination, errorHandling);
}

bool UnicodeConverter::addUTF8StrToUTF32String(StringRef utf8Str, PodVector<UTF32Unit>& outUtf32String, ErrorHandling errorHandling)
{
	FB_ZONE("UnicodeConverter::addUTF8ToUTF32String");

	bool fullSuccess = true;
	for (SizeType i = 0; i < utf8Str.getLength(); )
	{
		UTF32Unit utf32Unit = 0;
		int err = convertUTF8ToUTF32(utf32Unit, utf8Str.getPointer() + i, utf8Str.getLength() - i, errorHandling);
		if (FB_LIKELY(err > 0))
		{
			SizeType numOctets = (SizeType)err;
			i += numOctets;
			outUtf32String.pushBack(utf32Unit);
		}
		else
		{
			fullSuccess = false;
			if (errorHandling == AssertOnError)
			{
				fb_assertf(false, "Failed to convert UTF-8 character to UTF-32 at position %d (utf-8 string length: %d). Octet at place is: %d", i, utf8Str.getLength(), (SizeType)utf8Str[i]);
			}
			else if (errorHandling == UseReplacementChar)
			{
				outUtf32String.pushBack(getReplacementCharCodePoint());
			}

			i += 1;
		}
	}
	return fullSuccess;
}

SizeType UnicodeConverter::getLength(const UTF8Unit *source)
{
	for (SizeType i = 0; /* Loop as long as it takes */; ++i)
	{
		if (source[i] == getUTF8NulChar())
			return i;
	}
}

SizeType UnicodeConverter::getLength(const UTF16Unit *source)
{
	for (SizeType i = 0; /* Loop as long as it takes */; ++i)
	{
		if (source[i] == getUTF16NulChar())
			return i;
	}
}


bool UnicodeConverter::hasBOM(StringRef utf8Source)
{
	if (utf8Source.getLength() < 3)
		return false;

	return uint8_t(utf8Source[0]) == 0xEF && uint8_t(utf8Source[1]) == 0xBB && uint8_t(utf8Source[2]) == 0xBF;
}

int UnicodeConverter::getNumOctetsInUTF8Char(UTF8Unit firstOctet, ErrorHandling errorHandling)
{
	unsigned char octet = (unsigned char)firstOctet;

	if (errorHandling == AssertOnError)
	{
		// 0b11000000 and 0b11000001 are never valid octets, they would imply the beginning of a 7-bit ASCII character encoded to a double byte UTF-8 sequence
		fb_expensive_assertf(octet != 0b11000000 && octet != 0b11000001, "Invalid UTF-8 octet: 0x%x. See https://en.wikipedia.org/wiki/UTF-8#Codepage_layout for explanation.", octet);
	}

	/*
	 * 0xxxxxxx = 1 bytes
	 * 110xxxxx = 2 bytes
	 * 1110xxxx = 3 bytes
	 * 11110xxx = 4 bytes
	 * other    -> error
	 *(10xxxxxx = byte in the middle of a character)
	 *(x represents the payload which is a unicode codepoint aka. a number from 0 to 2.097.151 = (2^21 - 1) )
	 */

	if ((octet & 0b10000000) == 0b00000000)
		return 1;
	if ((octet & 0b11100000) == 0b11000000)
		return 2;
	if ((octet & 0b11110000) == 0b11100000)
		return 3;
	if ((octet & 0b11111000) == 0b11110000)
		return 4;

	if (errorHandling == AssertOnError)
	{
		if (octet == 0b11111111)
		{
			fb_assertf(false, "Invalid UTF-8 octet. Char: 0x%x", octet);
		}
		else if ((octet & 0b11111000) == 0b11111000)
		{
			fb_assertf(false, "More than four octets in UTF-8 sequence not supported. Char: 0x%x", octet);
		}
		else if ((octet & 0b11000000) == 0b10000000)
		{
			fb_assertf(false, "Given octet is not the start octet of a UTF-8 sequence. Char: 0x%x", octet);
		}
		else
		{
			fb_assertf(false, "Invalid UTF-8 start octet. Char: 0x%x", octet);
		}
	}
	return getInvalidUTF8StartOctet();
}

int UnicodeConverter::getNumOctetsInCorrespondingUTF8Char(UTF32Unit codePoint)
{
	if (codePoint < 0x80)
	{
		return 1;
	}
	else if (codePoint < 0x800)
	{
		return 2;
	}
	else if (codePoint < 0x10000)
	{
		return 3;
	}
	else if (codePoint < 0x110000)
	{
		return 4;
	}
	else
	{
		/* Non-valid UTF-32 code point */
		return -1;
	}
}

int UnicodeConverter::getNumUnitsInUTF16Char(UTF16Unit firstUnit)
{
	/* First test for surrogates, then return 1 if this is not a one */
	if (firstUnit >= getHighSurrogateStart() && firstUnit <= getHighSurrogateEnd())
		return 2;
	else if (firstUnit >= getLowSurrogateStart() && firstUnit <= getLowSurrogateEnd())
		return getInvalidUTF16UnitError();
	else
		return 1;
}

int UnicodeConverter::getNumUnitsInCorrespondingUTF16Char(UTF32Unit codePoint)
{
	if (codePoint <= 0xFFFF)
	{
		/* Quick check validity (I seem to recall real validity test is more complex) */
		if (!(codePoint >= getHighSurrogateStart() && codePoint <= getLowSurrogateEnd()))
			return 1;
		else
			return getInvalidUTF32CodePointError();
	}
	else if (codePoint <= getMaxLegalUTF32())
		return 2;
	else
		return getInvalidUTF32CodePointError();
}

FB_END_PACKAGE1()
