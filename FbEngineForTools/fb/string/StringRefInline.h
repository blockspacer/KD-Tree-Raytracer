#pragma once

#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/IsSame.h"

FB_PACKAGE0()

namespace
{
FB_FORCEINLINE static bool checkLength(const char *ptr, SizeType expectedLength)
{
	// Use this for checks because we want strlen to only be used for runtime string length calculation

	if (ptr)
	{
		SizeType i = 0;
		for (; i < 100000000; ++i)
		{
			if (ptr[i] != 0)
				continue;

			break;
		}

		if (i == expectedLength)
			return true;

		return false;
	}

	return false;
}
#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	#define FB_STRING_REF_CHECK_LENGTH_ASSERTF fb_assertf
#else
	#define FB_STRING_REF_CHECK_LENGTH_ASSERTF fb_expensive_assertf
#endif
}

FB_FORCEINLINE StringRef::StringRef(const StringRef &other)
	: ptr(other.ptr)
	, length(other.length)
#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	, hash(other.hash)
#endif
{
	// Copy constructor for the easier to inline copying without asserts (helpful in debug builds)
}

inline StringRef::StringRef(const char *strPtr, SizeType strLength)
{
	this->ptr = strPtr;
	this->length = strLength;

	fb_assert(ptr != nullptr);
	FB_STRING_REF_CHECK_LENGTH_ASSERTF(checkLength(ptr, length), "%d == %d", strLen(ptr), length);
	debugStoreHash();
}
template<typename CString, typename SFINAE_HACK>
inline StringRef::StringRef(CString other)
{
	fb_assert(other != nullptr);
	fb_static_assert(((lang::IsSame<CString, char*>::value) || (lang::IsSame<CString, const char*>::value))&& "Call StringRef constructor explicitly for C-Strings");

	this->ptr = other;
	this->length = strLen(other);
	debugStoreHash();
}

template<class StringT, typename Valid>
FB_FORCEINLINE StringRef::StringRef(const StringT &other)
{
	fb_static_assert((!lang::IsSame<StringT, StringRef>::value) && "Invalid StringRef constructor chosen");
	fb_static_assert((!lang::IsSame<StringT, char*>::value) && "Call StringRef constructor explicitly for C-Strings");
	fb_static_assert((!lang::IsSame<StringT, char* const>::value) && "Call StringRef constructor explicitly for C-Strings");
	fb_static_assert((!lang::IsSame<StringT, const char*>::value) && "Call StringRef constructor explicitly for C-Strings");
	fb_static_assert((!lang::IsSame<StringT, const char* const>::value) && "Call StringRef constructor explicitly for C-Strings");

	ptr = other.getPointer();
	length = other.getLength();

	fb_expensive_assertf(checkLength(ptr, length), "%d == %d", strLen(ptr), length);
	debugStoreHash();
}

template<size_t N>
FB_FORCEINLINE StringRef::StringRef(char(&charBuffer)[N])
{
	fb_static_assert(N > 0 && "Char array is 0-length (including the null-terminator)");
	fb_static_assert(N < string::NoPosition && "Too long string literal");
	fb_expensive_assert(charBuffer != nullptr);
	this->ptr = charBuffer;
	this->length = strLen(charBuffer);

	fb_assert(this->length < N && "Length of string is longer than the allocated buffer");
	debugStoreHash();
}

template<size_t N>
FB_FORCEINLINE StringRef::StringRef(const char(&charArray)[N])
{
	fb_static_assert(N > 0 && "Char array is 0-length (including the null-terminator)");
	fb_static_assert(N < string::NoPosition && "Too long string literal");
	fb_expensive_assert(charArray != nullptr);
	FB_STRING_REF_CHECK_LENGTH_ASSERTF(checkLength(charArray, SizeType(N) - 1U), "This overload is reserved for string-literals. Use the explicit overload with string length as parameter. Or just don't use const buffers. %d + 1 != %d", strLen(charArray), SizeType(N));

	this->ptr = charArray;
	this->length = SizeType(N) - 1U;
	debugStoreHash();
}

template<size_t N>
FB_FORCEINLINE StringRef StringRef::stringLiteral(const char(&charArray)[N])
{
	fb_static_assert(N > 0 && "Char array is 0-length (including the null-terminator)");
	fb_static_assert(N < string::NoPosition && "Too long string literal");
	FB_STRING_REF_CHECK_LENGTH_ASSERTF(checkLength(charArray, SizeType(N) - 1), "This overload is reserved for string-literals. Use the explicit overload with string length as parameter. Or just don't use const buffers. %d + 1 != %d", strLen(charArray), SizeType(N));
	return StringRef(charArray, N - 1U);
}

template<size_t N>
FB_FORCEINLINE StringRef StringRef::stringLiteral(char(&arr)[N])
{
	fb_static_assertf(N == ~123456789U, "Invalid use of StringRef::stringLiteral. Must use FB_IS_STRING_LITERAL-macro to validate template argument");
	return StringRef((const char *)arr);
}

inline const char *StringRef::getPointer() const
{
	fb_expensive_assert(this->ptr != nullptr);
	return this->ptr;
}

inline SizeType StringRef::getLength() const
{
	fb_expensive_assert(this->ptr != nullptr);

#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	// you are supposed to use this with actual _const_ strings only.
	FB_STRING_REF_CHECK_LENGTH_ASSERTF(checkLength(this->ptr, this->length), "%d == %d", strLen(this->ptr), this->length);
#endif

	return this->length;
}

inline bool StringRef::isEmpty() const
{
	fb_expensive_assert(this->ptr != nullptr);

	// you are supposed to use this with actual _const_ strings only.
	fb_expensive_assert((this->length == 0) == (this->ptr[0] == '\0'));

	return (this->ptr[0] == '\0');
}

inline char StringRef::operator[] (SizeType index) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_assertf(index < this->length, "%d < %d", index, this->length);

	return this->ptr[index];
}

#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
inline StringRef::~StringRef()
{
	// This is just some debug functionality
	fb_assert(checkLength(ptr, length) && "Length of string referred by StringRef changed during its lifetime");
	fb_assert(hash == debugCalculateHash(ptr, length) && "String referred by StringRef changed during its lifetime");
}
FB_FORCEINLINE SizeType StringRef::debugCalculateHash(const char *ptr, SizeType length)
{
	// A simple hash algorithm that when inlined doesn't interfere with debug stepping
	// The algortihm is sdbm: http://www.cse.yorku.ca/~oz/hash.html#sdbm
	// It has relatively nice distribution for being so simple: https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633

	SizeType hashResult = 0;
	for (SizeType i = 0; i < length; ++i)
	{
		hashResult = ptr[i] + (hashResult << 6) + (hashResult << 16) - hashResult;
	}
	return hashResult;
}
#endif

FB_FORCEINLINE void StringRef::debugStoreHash()
{
#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	hash = debugCalculateHash(ptr, length);
#endif
}

inline StringRef StringRef::make(const char* ptr)
{
	fb_assert(ptr && "String is null.");
	return StringRef(ptr, strLen(ptr));
};

FB_END_PACKAGE0()
