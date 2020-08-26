#include "Precompiled.h"
#include "HeapString.h"

#include "fb/lang/platform/Likely.h"
#include "fb/math/Color3.h"
#include "fb/math/Quaternion.h"
#include "fb/math/Vec2.h"
#include "fb/math/Vec3.h"
#include "fb/math/Vec4.h"
#include "fb/string/CommonImpl.h"

#include <stdarg.h> // For va_start, va_end
#include <stdio.h> // For vsnprintf
#include <cstring> // For strlen and strcmp

#define FB_TEMPORARYSTRING_REGRESSION_MODE FB_FALSE

FB_PACKAGE0()

#define FB_STRING_THIS_CLASS HeapString

FB_STRING_BOOL_PARSE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_CASE_DETECTION_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_COMPARE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_FIND_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_NUMBER_PARSE_IMPL(FB_STRING_THIS_CLASS);

FB_STRING_CASE_CONVERSION_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_FILE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_MODIFY_IMPL(FB_STRING_THIS_CLASS);

#undef FB_STRING_THIS_CLASS


static char *getEmptyTemporaryString()
{
	static char emptyTemporaryStringBuffer[1] = { '\0' };
	return emptyTemporaryStringBuffer;
}

const HeapString HeapString::empty;

HeapString::HeapString(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve)
:	ImpByteElementArray(staticPointer, staticSizeInElements, assertOnReserve)
{
	fb_assert(staticPointer && staticSizeInElements > 0);
	staticPointer[0] = '\0';
	size = 1;
}

void HeapString::moveImp(HeapString &other)
{
	if (this == &other)
		return;

	if (other.getLength() == 0)
	{
		clear();
		return;
	}

	ImpByteElementArray::impMoveOrdered(other, 1);
	validate();
}

void HeapString::initIfNeededImp()
{
	if (!getBytePointer())
	{
		char nullChar = 0;
		impResizeOrdered(1, 1, &nullChar);
	}

	validate();
}

HeapString::HeapString()
{
}

HeapString::HeapString(const char *ptr)
{
	append(ptr, (SizeType) strlen(ptr));
}

HeapString::HeapString(const char *ptr, SizeType ptrLength)
{
	append(ptr, ptrLength);
}

HeapString::HeapString(const StringRef &other)
{
	append(other.getPointer(), other.getLength());
}

HeapString::HeapString(HeapString &&other)
{
	moveImp(other);
}

HeapString::~HeapString()
{
	validate();
#if FB_TEMPORARYSTRING_REGRESSION_MODE == FB_TRUE
	memset(getBytePointer(), '~', getLength());
#endif
	impReset(1);
}

const char *HeapString::getPointer() const
{
	const char *result = getBytePointer();
	if (!result)
		result = getEmptyTemporaryString();

	return result;
}

bool HeapString::isEqual(const char *ptr, SizeType ptrLength) const
{
	SizeType localLength = getLength();
	if (localLength != ptrLength)
		return false;

	if (ptr == NULL)
		return false;

	if (memcmp(getPointer(), ptr, localLength) != 0)
		return false;

	return true;
}


int32_t HeapString::getOrdering(const char *ptr, SizeType ptrLength) const
{
	return strcmp(getPointer(), ptr);
}


HeapString& HeapString::trimLeft(SizeType charactersToTrim)
{
	// Must not trim trailing zero
	fb_assert(charactersToTrim <= getLength());

	impEraseRangeOrdered(0, charactersToTrim, 1);
	validate();
	return *this;
}


HeapString& HeapString::trimRight(SizeType charactersToTrim)
{
	// Must not trim trailing zero
	uint32_t localLength = getLength();
	fb_assert(charactersToTrim <= localLength);

	impEraseRangeOrdered(localLength - charactersToTrim, charactersToTrim, 1);
	validate();
	return *this;
}


HeapString& HeapString::trimChars(const PodVector<char> &charsToTrim)
{
	trimCharsLeft(charsToTrim);
	return trimCharsRight(charsToTrim);
}


HeapString& HeapString::trimChars(char charToTrim)
{
	StaticPodVector<char, 1> charVector({ charToTrim });
	return trimChars(charVector);
}


HeapString& HeapString::trimCharsLeft(const PodVector<char> &charsToTrim)
{
	SizeType skippingAtStart = 0;
	for (SizeType i = 0, len = getLength(); i < len; ++i)
	{
		if (findIfContains(charsToTrim, (*this)[i]))
			++skippingAtStart;
		else
			break;
	}
	if (skippingAtStart > 0)
		trimLeft(skippingAtStart);

	return *this;
}


HeapString& HeapString::trimCharsLeft(char charToTrim)
{
	StaticPodVector<char, 1> charVector({ charToTrim });
	return trimCharsLeft(charVector);
}


HeapString& HeapString::trimCharsRight(const PodVector<char> &charsToTrim)
{
	SizeType skippingAtEnd = 0;
	for (SizeType i = 0, len = getLength(); i < len; ++i)
	{
		if (findIfContains(charsToTrim, (*this)[len - i - 1]))
			++skippingAtEnd;
		else
			break;
	}
	if (skippingAtEnd > 0)
		trimRight(skippingAtEnd);

	return *this;
}


HeapString& HeapString::trimCharsRight(char charToTrim)
{
	StaticPodVector<char, 1> charVector({ charToTrim });
	return trimCharsRight(charVector);
}


const PodVector<char> &HeapString::getWhiteSpaceChars()
{
	static StaticPodVector<char, 4> whiteSpaces({ ' ', '\t', '\r', '\n' });
	return whiteSpaces;
}

HeapString& HeapString::trimWhiteSpace()
{
	return trimChars(getWhiteSpaceChars());
}


HeapString& HeapString::trimWhiteSpaceLeft()
{
	return trimCharsLeft(getWhiteSpaceChars());
}


HeapString& HeapString::trimWhiteSpaceRight()
{
	return trimCharsRight(getWhiteSpaceChars());
}


void HeapString::erase(SizeType index, SizeType count)
{
	// Must not erase trailing zero
	fb_assert(index + count <= getLength());

	impEraseRangeOrdered(index, count, 1);
	validate();
}

void HeapString::insert(SizeType index, const char *str, SizeType count)
{
	// Must not move trailing zero
	fb_assert(index <= getLength());

	initIfNeededImp();
	impInsertArrayOrdered(index, str, count, 1);
	validate();
}

void HeapString::truncateToSize(SizeType newLength)
{
	fb_assert(newLength && newLength <= getLength());
	getBytePointer()[newLength] = 0;
	size = newLength + 1;

	validate();
}

void HeapString::resizeAndFill(SizeType newSize, char fillChar)
{
	fb_assert(fillChar != '\0' && "Fill character cannot be '\\0'. HeapString will break.");
	if (newSize == 0)
	{
		clear();
		return;
	}

	reserve(newSize);

	char *str = getBytePointer();
	for (SizeType i = getLength(); i < newSize; ++i)
		str[i] = fillChar;
	str[newSize] = 0;

	size = newSize + 1;
	validate();
}

void HeapString::reserve(SizeType reserveSize)
{
	uint32_t currentCapacity = getCapacity();
	if (reserveSize > currentCapacity)
	{
		reserveSize = reserveSize < 7 ? 7 : reserveSize;
		impGrowContainerOrdered(1, reserveSize - currentCapacity + 1);
	}

	getBytePointer()[getLength()] = 0;
}

void HeapString::clear()
{
	if (size)
	{
		getBytePointer()[0] = 0;
		size = 1;
	}
	else
		size = 0;

	validate();
}

HeapString &HeapString::append(const char *ptr, SizeType ptrLength)
{
	if (!ptr || !ptrLength)
		return *this;

	fb_assertf(getBytePointer() != ptr, "Appending string to itself. Length: %u, str: '%s'", ptrLength, ptr);

	uint32_t currentLength = getLength();
	uint32_t currentCapacity = getCapacity();
	if (currentCapacity < currentLength + ptrLength)
		reserve(currentLength + ptrLength);

	char *currentPtr = getBytePointer();
	memcpy(currentPtr + currentLength, ptr, ptrLength);

	currentLength += ptrLength;
	currentPtr[currentLength] = 0;
	size = currentLength + 1;

	validate();
	return *this;
;}

void HeapString::assign(const char *ptr, SizeType ptrLength)
{
	clear();
	append(ptr, ptrLength);
}

HeapString& HeapString::operator= (const HeapString &other)
{
	if (this == &other)
		return *this;

	clear();
	*this += other;
	return *this;
}

HeapString& HeapString::operator= (const StringRef &other)
{
	if (getPointer() == other.getPointer())
		return *this;

	clear();
	*this += other;
	return *this;
}

HeapString& HeapString::operator= (HeapString &&other)
{
	moveImp(other);
	return *this;
}

HeapString& HeapString::operator= (const char *ptr)
{
	clear();
	append(ptr, (SizeType) strlen(ptr));
	return *this;
}

bool HeapString::operator== (const StringRef &other) const
{
	return isEqual(other.getPointer(), other.getLength());
}

bool HeapString::operator== (const char *ptr) const
{
	return isEqual(ptr, (SizeType)strlen(ptr));
}

bool HeapString::operator!= (const StringRef &other) const
{
	return !isEqual(other.getPointer(), other.getLength());
}

bool HeapString::operator!= (const char *ptr) const
{
	return !isEqual(ptr, (SizeType)strlen(ptr));
}

bool HeapString::operator!= (char *ptr) const
{
	return !isEqual(ptr, (SizeType)strlen(ptr));
}

bool HeapString::operator< (const StringRef &other) const
{
	return getOrdering(other.getPointer(), other.getLength()) < 0;
}

bool HeapString::operator< (const char *ptr) const
{
	return getOrdering(ptr, (SizeType)strlen(ptr)) < 0;
}

bool HeapString::operator< (char *ptr) const
{
	return getOrdering(ptr, (SizeType)strlen(ptr)) < 0;
}

bool HeapString::operator> (const StringRef &other) const
{
	return getOrdering(other.getPointer(), other.getLength()) > 0;
}

bool HeapString::operator> (const char *ptr) const
{
	return getOrdering(ptr, (SizeType)strlen(ptr)) > 0;
}

bool HeapString::operator> (char *ptr) const
{
	return getOrdering(ptr, (SizeType)strlen(ptr)) > 0;
}

HeapString& HeapString::operator+= (const StringRef &other)
{
	append(other.getPointer(), other.getLength());
	return *this;
}

HeapString& HeapString::operator+= (const HeapString &other)
{
	append(other.getPointer(), other.getLength());
	return *this;
}

HeapString& HeapString::operator+= (const char *ptr)
{
	append(ptr, (SizeType) strlen(ptr));
	return *this;
}

HeapString& HeapString::operator+= (char c)
{
	fb_assert(c != 0);
	append(&c, 1);
	return *this;
}


HeapString& HeapString::operator+=(bool value)
{
	*this += (value ? "true" : "false");
	return *this;
}


HeapString& HeapString::operator+=(int64_t value)
{
	doSprintf("%" FB_FSI64, value);
	return *this;
}


HeapString& HeapString::operator+=(uint64_t value)
{
	doSprintf("%" FB_FSU64, value);
	return *this;
}

HeapString& HeapString::operator+=(float value)
{
	return appendAccurate(value);
}


HeapString& HeapString::operator+=(double value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::VC2 &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::VC3 &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::VC4 &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::DVC2 &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::DVC3 &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::DVC4 &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::VC2I &value)
{
	doSprintf("VC2I(%d, %d)", value.x, value.y);
	return *this;
}

HeapString& HeapString::operator+= (const math::VC3I &value)
{
	doSprintf("VC3I(%d, %d, %d)", value.x, value.y, value.z);
	return *this;
}

HeapString& HeapString::operator+= (const math::VC4I &value)
{
	doSprintf("VC4I(%d, %d, %d, %d)", value.x, value.y, value.z, value.w);
	return *this;
}

HeapString& HeapString::operator+= (const math::QUAT &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::DQUAT &value)
{
	return appendAccurate(value);
}

HeapString& HeapString::operator+= (const math::COL &value)
{
	return appendAccurate(value);
}


HeapString & HeapString::appendAccurate(float value)
{
	doSprintf("%.*g", FB_FLT_DECIMAL_DIG, value);
	return *this;
}

HeapString & HeapString::appendAccurate(double value)
{
	doSprintf("%.*g", FB_DBL_DECIMAL_DIG, value);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::VC2 &value)
{
	doSprintf("VC2(%.*g, %.*g)", FB_FLT_DECIMAL_DIG, value.x, FB_FLT_DECIMAL_DIG, value.y);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::VC3 &value)
{
	doSprintf("VC3(%.*g, %.*g, %.*g)", FB_FLT_DECIMAL_DIG, value.x, FB_FLT_DECIMAL_DIG, value.y, FB_FLT_DECIMAL_DIG, value.z);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::VC4 &value)
{
	doSprintf("VC4(%.*g, %.*g, %.*g, %.*g)", FB_FLT_DECIMAL_DIG, value.x, FB_FLT_DECIMAL_DIG, value.y, FB_FLT_DECIMAL_DIG, value.z, FB_FLT_DECIMAL_DIG, value.w);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::DVC2 &value)
{
	doSprintf("DVC2(%.*g, %.*g)", FB_DBL_DECIMAL_DIG, value.x, FB_DBL_DECIMAL_DIG, value.y);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::DVC3 &value)
{
	doSprintf("DVC3(%.*g, %.*g, %.*g)", FB_DBL_DECIMAL_DIG, value.x, FB_DBL_DECIMAL_DIG, value.y, FB_DBL_DECIMAL_DIG, value.z);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::DVC4 &value)
{
	doSprintf("DVC4(%.*g, %.*g, %.*g, %.*g)", FB_DBL_DECIMAL_DIG, value.x, FB_DBL_DECIMAL_DIG, value.y, FB_DBL_DECIMAL_DIG, value.z, FB_DBL_DECIMAL_DIG, value.w);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::QUAT &value)
{
	doSprintf("QUAT(%.*g, %.*g, %.*g, %.*g)", FB_FLT_DECIMAL_DIG, value.x, FB_FLT_DECIMAL_DIG, value.y, FB_FLT_DECIMAL_DIG, value.z, FB_FLT_DECIMAL_DIG, value.w);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::DQUAT &value)
{
	doSprintf("DQUAT(%.*g, %.*g, %.*g, %.*g)", FB_DBL_DECIMAL_DIG, value.x, FB_DBL_DECIMAL_DIG, value.y, FB_DBL_DECIMAL_DIG, value.z, FB_DBL_DECIMAL_DIG, value.w);
	return *this;
}

HeapString &HeapString::appendAccurate(const math::COL &value)
{
	doSprintf("COL(%.*g, %.*g, %.*g)", FB_FLT_DECIMAL_DIG, value.r, FB_FLT_DECIMAL_DIG, value.g, FB_FLT_DECIMAL_DIG, value.b);
	return *this;
}

HeapString &HeapString::appendFloat(float value, SizeType numDecimals)
{
	doSprintf("%.*f", numDecimals, value);
	return *this;
}

HeapString &HeapString::appendDouble(double value, SizeType numDecimals)
{
	doSprintf("%.*f", numDecimals, value);
	return *this;
}

HeapString &HeapString::appendHexNumber(uint8_t value, bool add0x)
{
	if (add0x)
		append("0x", 2);

	doSprintf("%" FB_PRIX8, value);
	return *this;
}

HeapString &HeapString::appendHexNumber(uint16_t value, bool add0x)
{
	if (add0x)
		append("0x", 2);

	doSprintf("%" FB_PRIX16, value);
	return *this;
}

HeapString &HeapString::appendHexNumber(uint32_t value, bool add0x)
{
	if (add0x)
		append("0x", 2);

	doSprintf("%" FB_PRIX32, value);
	return *this;
}

HeapString &HeapString::appendHexNumber(uint64_t value, bool add0x)
{
	if (add0x)
		append("0x", 2);

	doSprintf("%" FB_PRIX64, value);
	return *this;
}

HeapString &HeapString::appendNumberStringWithCommas(const StringRef &numberString)
{
	SizeType digitCount = 0;
	for (SizeType i = 0; i < numberString.getLength(); i++)
	{
		if (numberString[i] == '.')
			break;

		digitCount++;
	}

	const char *digitsString = numberString.getPointer();
	if (digitsString[0] == '-')
	{
		digitsString++;
		digitCount--;
		*this += "-";
	}

	SizeType c = 2 - (digitCount % 3);
	for (SizeType i = 0; digitsString[i] != 0; i++)
	{
		if (digitsString[i] == '.')
		{
			*this += digitsString + i;
			break;
		}
		else
		{
			*this += digitsString[i];
			if (c == 1 && i + 1 < digitCount)
				*this += ',';
	
			c = (c + 1) % 3;
		}
	}
	return *this;
}

HeapString &HeapString::appendNumberWithCommas(int64_t value)
{
	appendNumberStringWithCommas(SmallTempString() += value);
	return *this;
}

HeapString &HeapString::appendNumberWithCommas(uint64_t value)
{
	appendNumberStringWithCommas(SmallTempString() += value);
	return *this;
}

HeapString &HeapString::appendNumberWithCommas(float value, SizeType numDecimals)
{
	appendNumberStringWithCommas(SmallTempString().doSprintf("%.*f", numDecimals, value));
	return *this;
}

HeapString &HeapString::appendNumberWithCommas(double value, SizeType numDecimals)
{
	/* Presuming no one wants scientific notation here */
	appendNumberStringWithCommas(SmallTempString().doSprintf("%.*f", numDecimals, value));
	return *this;
}

HeapString &HeapString::doSprintf(const char* formatStr, ...)
{
	va_list arguments;
	va_start(arguments, formatStr);
	doVSprintf(formatStr, arguments);
	va_end(arguments);
	return *this;
}

HeapString &HeapString::doVSprintf(const char* formatStr, va_list args)
{
	initIfNeededImp();
	/* If buffer size is 0, vsnprintf will write nothing. If it is 1, however, it is allowed to write terminator 
	 * character, which would be a problem, if getBytePointer() returns NULL */
	SizeType bufferSizeForPrinting = getCapacity() > 0 ? getCapacity() - getLength() + 1 : 0;
	int result = 0;
	{
		va_list arguments;
		va_copy(arguments, args);
		result = vsnprintf(getBytePointer() + getLength(), bufferSizeForPrinting, formatStr, arguments);
		va_end(arguments);
	}

	if (result < 0)
	{
		fb_assertf(0, "Failed to doSprintf with format str: %s", formatStr);
		return *this;
	}
	else if (SizeType(result) >= bufferSizeForPrinting)
	{
		// We need more space, allocate and print again.
		reserve(getLength() + SizeType(result));
		bufferSizeForPrinting = getCapacity() - getLength() + 1;

		va_list arguments;
		va_copy(arguments, args);
		result = vsnprintf(getBytePointer() + getLength(), bufferSizeForPrinting, formatStr, arguments);
		va_end(arguments);
		fb_assertf(result >= 0 && SizeType(result) < bufferSizeForPrinting, "Failed to doSprintf after resizing string with format str: %s", formatStr);
		if (result < 0)
			return *this;
	}

	if (size == 0)
	{
		// zero terminator isn't included empty string size, so add it here
		size = 1;
	}
	size += SizeType(result);
	fb_assert(size <= capacity);
	validate();
	return *this;
}


void HeapString::validate() const
{
	#if FB_TEMPORARYSTRING_REGRESSION_MODE == FB_TRUE
		fb_assert(getPointer()[getLength()] == 0);
		fb_assert(getLength() <= getCapacity());
		fb_assertf(strlen(getPointer()) == getLength(),  "%zu == %u", strlen(getPointer()), getLength());

		//fb_assert(_CrtCheckMemory());
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF);
	#endif
}

HeapString::operator StringRef() const
{
	const char *result = getBytePointer();
	SizeType resultLength = getLength();
	return FB_LIKELY(result != nullptr) ? StringRef(result, resultLength) : StringRef(getEmptyTemporaryString(), resultLength);
}

FB_END_PACKAGE0()
