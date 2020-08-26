#pragma once

#include "DynamicString.h"

FB_PACKAGE0()

// Static strings which NEVER get deallocated.
// Thread safe without overhead, constructor excluded.
// Constructing string does include overhead and should be avoided as much as possible.

class StaticString : public DynamicString
{
public:
	StaticString();
	StaticString(const StaticString &other);
	StaticString(StaticString &&other) { imp.moveImp(other.imp); }

	/* We don't want to accidentally convert other string types to StaticStrings */
	explicit StaticString(const char *ptr);
	explicit StaticString(const char *ptr, SizeType length);
	explicit StaticString(const StringRef &other);
	explicit StaticString(const DynamicString &other);
	~StaticString() { }

	/* This creates StaticString using given pointer.  */
	template<size_t len>
	static StaticString createInPlace(char(&str)[len])
	{
		static_assert(len < 0xFFFF + string::StringImpHelper::OverheadBytes, "StaticString maximum length exceeded");
		static_assert(len >= string::StringImpHelper::OverheadBytes, "Malformed createInPlace string");
		return StaticString(CreateInPlaceWrapper(str, len));
	}
	static StaticString createFromConstChar(const char *str)
	{
		return StaticString(str, StringRef::strLen(str));
	}
	static StaticString createFromAnyString(const StringRef &str)
	{
		return StaticString(str.getPointer(), str.getLength());
	}

	void operator= (const StaticString &other);
	void operator= (StaticString &&other) { imp.moveImp(other.imp); }
	void operator= (const DynamicString &other);
	operator StringRef() const;

	static const StaticString empty;

private:
	/* Private constructor for createInPlace */
	explicit StaticString(CreateInPlaceWrapper createInPlaceWrapper);
};

#define FB_STATIC_INPLACE_STRINGS FB_TRUE

#if FB_STATIC_INPLACE_STRINGS == FB_TRUE

	#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
		/* [ID][RefCount][StringLength][StaticFlag][Actual string] */
		/* Zero ID isn't in use, RefCount set to max, Lenght zero at this point, static flag and in place flags set */
		#define FB_STATIC_CONST_STRING_DATA "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x03"
	#else
		/* [RefCount][StringLength][StaticFlag][Actual string] */
		/* RefCount set to max, Lenght zero at this point, static flag and in place flags set */
		#define FB_STATIC_CONST_STRING_DATA "\xFF\xFF\xFF\xFF\x00\x00\x03"
	#endif

	#define FB_STATIC_CONST_STRING(p_name, p_str) \
		static char p_name##CharArray_DoNotTouchMe[] = FB_STATIC_CONST_STRING_DATA p_str; \
		static const fb::StaticString p_name(fb::StaticString::createInPlace(p_name##CharArray_DoNotTouchMe));

#else

	#define FB_STATIC_CONST_STRING(p_name, p_str) \
		static const StaticString p_name(p_str);
#endif

FB_END_PACKAGE0()

FB_COMPARABLE(fb::StaticString);
FB_DEFAULT_COMPARATOR_FOR(fb::StaticString);
