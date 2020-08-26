#pragma once

#include "fb/string/HeapString.h"
#include "fb/string/AppendToString.h"

FB_PACKAGE1(string)

class StringBuilderEatCommaHack;

template<typename BaseStringType>
class StringBuilderBase
{
public:
	// Implements AnyString
	typedef void ImplementsStringRef;
	const char *getPointer() const
	{
		return val.getPointer();
	}
	SizeType getLength() const
	{
		return val.getLength();
	}

	StringBuilderBase()
	{
	}

	template<typename... Targs>
	StringBuilderBase(Targs... args)
	{
		appendToString(val, args...);
	}

	template<typename... Targs>
	StringBuilderBase(StringBuilderEatCommaHack *, Targs... args)
	{
		appendToString(val, args...);
	}

	BaseStringType &getString() { return val; }

private:
	BaseStringType val;
};

typedef StringBuilderBase<CacheHeapString<128 - OverheadOfHeapString>> ErrorStringBuilder;
typedef StringBuilderBase<LargeTempString> StringBuilder;

FB_END_PACKAGE1()
