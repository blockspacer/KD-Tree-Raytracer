#pragma once

#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/Types.h"

#if FB_BUILD == FB_DEBUG && FB_ASSERT_ENABLED == FB_TRUE
#define FB_ENABLE_STRING_LITERAL_LENGTH_CHECK FB_TRUE
#else
#define FB_ENABLE_STRING_LITERAL_LENGTH_CHECK FB_FALSE
#endif

FB_PACKAGE1(string)

class StringLiteral
{
public:
	typedef void ImplementsStringRef;

	void checkLength() const;

	template<int N>
	inline StringLiteral(const char(&str)[N])
		: ptr(str)
		, length(SizeType(N - 1))
	{
		fb_static_assert(N > 0);
		fb_static_assert(N < 100000);

		if (FB_ENABLE_STRING_LITERAL_LENGTH_CHECK == FB_TRUE)
			checkLength();
	}

	const char *getPointer() const { return ptr; }
	SizeType getLength() const { return length; }

	template<int N> StringLiteral(char(&str)[N]) = delete;
private:

	const char *ptr;
	SizeType length;
};

FB_END_PACKAGE1()
