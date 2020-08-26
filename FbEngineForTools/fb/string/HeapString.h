#pragma once

#include <stdarg.h>

#include "Config.h"
#include "fb/container/ImpByteElementArray.h"
#include "fb/container/PodVector.h"
#include "fb/algorithm/Comparator.h"
#include "fb/algorithm/traits/CompareTraits.h"
#include "fb/lang/hash/Hash.h"
#include "fb/lang/Types.h"
#include "fb/string/StringRef.h"
#include "fb/string/IsStringLiteral.h"

FB_DECLARE_TEMPLATED_CLASS(math, Vec2)
FB_DECLARE_TEMPLATED_CLASS(math, Vec3)
FB_DECLARE_TEMPLATED_CLASS(math, Vec4)
FB_DECLARE_TEMPLATED_CLASS(math, Color3)
FB_DECLARE_TEMPLATED_CLASS(math, Quaternion)

#define FB_HEAPSTRING_REGRESSION_MODE FB_FALSE

FB_PACKAGE0()

class HeapString : protected container::ImpByteElementArray
{
	// ImpByteElementArray size/capacity includes terminating 0, whereas length/capacity given to outside does not.
	// Note that local getCapacity() shadows ImpByteElementArray one and returns off-by-one result.

protected:
	HeapString(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve);
	char *getImpPointer();
	void moveImp(HeapString &other);
	void initIfNeededImp();

public:
	typedef void ImplementsStringRef;
	/* Length does not include NUL terminator. Subtrack another char just in case (if it matters, someone is probably doing something wrong) */
	static const SizeType maxSupportedLength = string::NoPosition - 2;

	HeapString();
	HeapString(const HeapString &other) : HeapString(StringRef(other)) { }
	HeapString(HeapString &&other);
	explicit HeapString(const StringRef &other);
	explicit HeapString(const char *ptr);
	explicit HeapString(char *ptr) : HeapString(StringRef(ptr)) {}
	template<typename S, FB_IS_STRING_LITERAL(S)>
	explicit HeapString(const S &other) : HeapString(StringRef::stringLiteral(other)) { }
	HeapString(const char *ptr, SizeType ptrLength);
	~HeapString();

	const char *getPointer() const;
	SizeType getLength() const { SizeType result = (SizeType) ImpByteElementArray::getSize(); return result ? result - 1 : 0; }
	SizeType getCapacity() const { SizeType result = (SizeType) ImpByteElementArray::getCapacity(); return result ? result - 1 : 0; }

	bool isEmpty() const { return (getLength() == 0); }
	bool isEqual(const char *ptr, SizeType ptrLength) const;
	int32_t getOrdering(const char *ptr, SizeType ptrLength) const;

	HeapString& trimLeft(SizeType charactersToTrim);
	HeapString& trimRight(SizeType charactersToTrim);
	HeapString& trimChars(const PodVector<char> &charsToTrim);
	HeapString& trimChars(char charToTrim);
	HeapString& trimCharsLeft(const PodVector<char> &charsToTrim);
	HeapString& trimCharsLeft(char charToTrim);
	HeapString& trimCharsRight(const PodVector<char> &charsToTrim);
	HeapString& trimCharsRight(char charToTrim);
	static const PodVector<char> &getWhiteSpaceChars();
	HeapString& trimWhiteSpace();
	HeapString& trimWhiteSpaceLeft();
	HeapString& trimWhiteSpaceRight();

	void erase(SizeType index, SizeType count);
	void insert(SizeType index, const char *str, SizeType count);
	void insert(SizeType index, const StringRef &str) { insert(index, str.getPointer(), str.getLength()); }
	void truncateToSize(SizeType newLength);
	void resizeAndFill(SizeType newSize, char fillChar);
	void reserve(SizeType reserveSize);
	void clear();
	HeapString &append(const char *ptr, SizeType ptrLength);
	void assign(const char *ptr, SizeType ptrLength);

	FB_FORCEINLINE char operator[] (SizeType index) const { fb_expensive_assert(getLength() > index); return getBytePointer()[index]; }
	FB_FORCEINLINE char &operator[] (SizeType index) { fb_expensive_assert(getLength() > index); return getBytePointer()[index]; }

	HeapString& operator= (const HeapString &other);
	HeapString& operator= (const StringRef &other);
	HeapString& operator= (HeapString &&other);
	HeapString& operator= (const char *ptr);
	HeapString& operator= (char *ptr) { return operator=((const char *)ptr); }
	template<typename S>
	HeapString& operator= (const S &other) { return HeapString::operator= (StringRef(other)); }

	bool operator== (const StringRef &other) const;
	bool operator== (const char *ptr) const;
	bool operator!= (const StringRef &other) const;
	bool operator!= (const char *ptr) const;
	bool operator!= (char *ptr) const;
	bool operator< (const StringRef &other) const;
	bool operator< (const char *ptr) const;
	bool operator< (char *ptr) const;
	bool operator> (const StringRef &other) const;
	bool operator> (const char *ptr) const;
	bool operator> (char *ptr) const;
	operator StringRef() const;

	HeapString& operator+= (const StringRef &other);
	HeapString& operator+= (const HeapString &other);
	HeapString& operator+= (const char *ptr);
	HeapString& operator+= (char c);

	/* Some number stuff */
	HeapString& operator+= (bool value);
	HeapString& operator+= (int8_t value) { return *this += int64_t(value); }
	HeapString& operator+= (uint8_t value) { return *this += uint64_t(value); }
	HeapString& operator+= (int16_t value) { return *this += int64_t(value); }
	HeapString& operator+= (uint16_t value) { return *this += uint64_t(value); }
	HeapString& operator+= (int32_t value) { return *this += int64_t(value); }
	HeapString& operator+= (uint32_t value) { return *this += uint64_t(value); }
	HeapString& operator+= (int64_t value);
	HeapString& operator+= (uint64_t value);
	HeapString& operator+= (float value);
	HeapString& operator+= (double value);
	HeapString& operator+= (const math::Vec2<float> &value);
	HeapString& operator+= (const math::Vec3<float> &value);
	HeapString& operator+= (const math::Vec4<float> &value);
	HeapString& operator+= (const math::Vec2<double> &value);
	HeapString& operator+= (const math::Vec3<double> &value);
	HeapString& operator+= (const math::Vec4<double> &value);
	HeapString& operator+= (const math::Vec2<int32_t> &value);
	HeapString& operator+= (const math::Vec3<int32_t> &value);
	HeapString& operator+= (const math::Vec4<int32_t> &value);
	HeapString& operator+= (const math::Quaternion<float> &value);
	HeapString& operator+= (const math::Quaternion<double> &value);
	HeapString& operator+= (const math::Color3<float> &value);

	/* Accurate versions */
	/* These will append floating point numbers to string in such a way that it is possible to accurately convert back to given number. */
	HeapString &appendAccurate(float value);
	HeapString &appendAccurate(double value);
	HeapString &appendAccurate(const math::Vec2<float> &value);
	HeapString &appendAccurate(const math::Vec3<float> &value);
	HeapString &appendAccurate(const math::Vec4<float> &value);
	HeapString &appendAccurate(const math::Vec2<double> &value);
	HeapString &appendAccurate(const math::Vec3<double> &value);
	HeapString &appendAccurate(const math::Vec4<double> &value);
	HeapString &appendAccurate(const math::Quaternion<float> &value);
	HeapString &appendAccurate(const math::Quaternion<double> &value);
	HeapString &appendAccurate(const math::Color3<float> &value);

	/* These will append floating point number with given accuracy */
	HeapString &appendFloat(float value, SizeType numDecimals);
	HeapString &appendDouble(double value, SizeType numDecimals);

	/* These will append given number as hex value to string */
	HeapString &appendHexNumber(uint8_t value, bool add0x = false);
	HeapString &appendHexNumber(uint16_t value, bool add0x = false);
	HeapString &appendHexNumber(uint32_t value, bool add0x = false);
	HeapString &appendHexNumber(uint64_t value, bool add0x = false);

	/* Append string converting something like 1248514.31 to 1,248,514.31 */
	HeapString &appendNumberStringWithCommas(const StringRef &numberString);
	/* Append number in comma separated form so something like 1248514.31 becomes 1,248,514.31 */
	HeapString &appendNumberWithCommas(int8_t value) { appendNumberWithCommas(int64_t(value)); return *this; }
	HeapString &appendNumberWithCommas(uint8_t value) { appendNumberWithCommas(uint64_t(value)); return *this; }
	HeapString &appendNumberWithCommas(int16_t value) { appendNumberWithCommas(int64_t(value)); return *this; }
	HeapString &appendNumberWithCommas(uint16_t value) { appendNumberWithCommas(uint64_t(value)); return *this; }
	HeapString &appendNumberWithCommas(int32_t value) { appendNumberWithCommas(int64_t(value)); return *this; }
	HeapString &appendNumberWithCommas(uint32_t value) { appendNumberWithCommas(uint64_t(value)); return *this; }
	HeapString &appendNumberWithCommas(int64_t value);
	HeapString &appendNumberWithCommas(uint64_t value);
	HeapString &appendNumberWithCommas(float value, SizeType numDecimals = 2);
	HeapString &appendNumberWithCommas(double value, SizeType numDecimals = 2);

	#if FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC
		#define FB_HEAP_STRING_DOSPRINTF_ARGUMENT_CHECK_ATTRIBUTE __attribute__((format(printf, 2, 3)))
	#else
		#define FB_HEAP_STRING_DOSPRINTF_ARGUMENT_CHECK_ATTRIBUTE
	#endif
	HeapString &doSprintf(const char* formatStr, ...) FB_HEAP_STRING_DOSPRINTF_ARGUMENT_CHECK_ATTRIBUTE;
	#undef FB_HEAP_STRING_DOSPRINTF_ARGUMENT_CHECK_ATTRIBUTE

	HeapString &doVSprintf(const char* formatStr, va_list args);

	static const HeapString empty;

	/* Utils */
	FB_STRING_BOOL_PARSE_DECL();
	FB_STRING_CASE_DETECTION_DECL();
	FB_STRING_COMPARE_DECL();
	FB_STRING_FIND_DECL();
	FB_STRING_NUMBER_PARSE_DECL();

	FB_STRING_CASE_CONVERSION_DECL();
	FB_STRING_FILE_DECL(HeapString);
	FB_STRING_MODIFY_DECL();

protected:
	void validate() const;
};

// Global operators to add anything that TempString happens to support with += operator.
// There was a lot of code which degeneraated to operator += string::HeapString(value) due to missing + operator overloads
// (even const char* was first created to dummy TempString causing heap allocation)
template<typename T>
HeapString operator+ (const HeapString &str1, const T &other) { HeapString copy(str1); copy += other; return copy; }
template<typename T>
HeapString &operator<<(HeapString &str1, const T &other) { str1 += other; return str1; }

// --------------------------------------------------------------------------------------

// Version which uses local buffer for initial storage.

template<uint32_t LocalCapacity>
struct CacheHeapString: public HeapString
{
protected:
	fb_static_assert(LocalCapacity > 0 && "Zero sized cache");
	char buffer[LocalCapacity + 1];

public:
	// This is annoying, but we have to duplicate some defined constructors/operator
	CacheHeapString() : HeapString(buffer, LocalCapacity + 1, false) {}
	CacheHeapString(const CacheHeapString &other) : HeapString(buffer, LocalCapacity + 1, false) { append(other.getPointer(), other.getLength()); }
	CacheHeapString(CacheHeapString &&other) : HeapString(buffer, LocalCapacity + 1, false) { moveImp(other); }

	CacheHeapString(const HeapString &other) : HeapString(buffer, LocalCapacity + 1, false) { append(other.getPointer(), other.getLength()); }
	CacheHeapString(HeapString &&other) : HeapString(buffer, LocalCapacity + 1, false) { moveImp(other); }
	CacheHeapString(const StringRef &other) : HeapString(buffer, LocalCapacity + 1, false) { append(other.getPointer(), other.getLength()); }
	explicit CacheHeapString(const char *ptr) : HeapString(buffer, LocalCapacity + 1, false) { *this = StringRef(ptr); }
	explicit CacheHeapString(char *ptr) : HeapString(buffer, LocalCapacity + 1, false) { *this = StringRef(ptr); }
	CacheHeapString(const char *ptr, SizeType ptrLength) : HeapString(buffer, LocalCapacity + 1, false) { append(ptr, ptrLength); }

	// NOTE: CacheHeapString has more lenient implicit casting rules than HeapString
	template<size_t N>
	CacheHeapString(char(&arr)[N]) : CacheHeapString(StringRef((const char *)arr)) { }
	template<size_t N>
	CacheHeapString(const char(&arr)[N]) : CacheHeapString(StringRef::stringLiteral(arr)) { }
	template<typename S, SizeType SFINAE = string::NoPosition>
	CacheHeapString(const S &other) : CacheHeapString(StringRef(other)) { }

	void operator= (const CacheHeapString &other) { HeapString::operator= (other); }
	void operator= (CacheHeapString &&other) { moveImp(other); }
	void operator= (const HeapString &other) { HeapString::operator= (other); }
	void operator= (const StringRef &other) { HeapString::operator= (other); }
	void operator= (const char *ptr) { HeapString::operator= (ptr); }
	template<typename S>
	void operator= (const S &other) { HeapString::operator= (other); }
};

// Version which uses local buffer for initial storage. Asserts if having to reserve more capacity.

template<uint32_t LocalCapacity>
struct StaticHeapString: public HeapString
{
protected:
	fb_static_assert(LocalCapacity > 0 && "Zero sized cache");
	char buffer[LocalCapacity + 1];

public:
	enum { Capacity = LocalCapacity };

	// This is annoying, but we have to duplicate some defined constructors/operator
	StaticHeapString() : HeapString(buffer, LocalCapacity + 1, true) {}
	StaticHeapString(const StaticHeapString &other) : HeapString(buffer, LocalCapacity + 1, false) { append(other.getPointer(), other.getLength()); }
	StaticHeapString(StaticHeapString &&other) : HeapString(buffer, LocalCapacity + 1, false) { moveImp(other); }

	StaticHeapString(const HeapString &other) : HeapString(buffer, LocalCapacity + 1, true) { append(other.getPointer(), other.getLength()); }
	StaticHeapString(HeapString &&other) : HeapString(buffer, LocalCapacity + 1, true) { moveImp(other); }
	StaticHeapString(const StringRef &other) : HeapString(buffer, LocalCapacity + 1, false) { append(other.getPointer(), other.getLength()); }
	explicit StaticHeapString(const char *ptr) : HeapString(buffer, LocalCapacity + 1, true) { *this = StringRef(ptr); }
	explicit StaticHeapString(char *ptr) : HeapString(buffer, LocalCapacity + 1, true) { *this = StringRef(ptr); }
	template<typename S, FB_IS_STRING_LITERAL(S)>
	StaticHeapString(const S &other) : StaticHeapString(StringRef::stringLiteral(other)) { }
	StaticHeapString(const char *ptr, SizeType ptrLength) : HeapString(buffer, LocalCapacity + 1, true) { append(ptr, ptrLength); }

	void operator= (const StaticHeapString &other) { HeapString::operator= (other); }
	void operator= (StaticHeapString &&other) { moveImp(other); }
	void operator= (const HeapString &other) { HeapString::operator= (other); }
	void operator= (const StringRef &other) { HeapString::operator= (other); }
	void operator= (const char *ptr) { HeapString::operator= (ptr); }
	template<typename S>
	void operator= (const S &other) { HeapString::operator= (other); }
};


namespace string
{
	enum
	{
		OverheadOfHeapString = 1U + sizeof(HeapString)
	};
}

// TempString/LargeTempString are meant to stay in the stack when you need to do some string operations.
// If you require any specific sizes or store them to class instances, use the underlying types explicitly.
typedef CacheHeapString<64U - string::OverheadOfHeapString> SmallTempString;
typedef CacheHeapString<512U - string::OverheadOfHeapString> TempString;
typedef CacheHeapString<4096U - string::OverheadOfHeapString> LargeTempString;

static inline uint32_t global_default_hash_function(const HeapString &str) { return getHashValue(str.getPointer(), (uint32_t)str.getLength()); }

FB_END_PACKAGE0()

FB_COMPARABLE(fb::HeapString);
FB_DEFAULT_COMPARATOR_FOR(fb::HeapString);
