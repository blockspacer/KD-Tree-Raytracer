#include "Precompiled.h"
#include "StaticString.h"

#include <cstring>

FB_PACKAGE0()

const StaticString StaticString::empty("", 0);

StaticString::StaticString()
	: DynamicString(DynamicString::getEmpty())
{
}


StaticString::StaticString(const char *ptr)
	: DynamicString(ptr, SizeType(strlen(ptr)), true)
{
}


StaticString::StaticString(const char *ptr, SizeType length)
	: DynamicString(ptr, length, true)
{
}

StaticString::StaticString(const StringRef &other)
	: DynamicString(other.getPointer(), other.getLength(), true)
{
}

StaticString::StaticString(const StaticString &other)
	: DynamicString()
{
	imp.unsafeCopy(other.imp);
}


StaticString::StaticString(const DynamicString &other)
	: DynamicString(other)
{
	convertToStatic();
}


StaticString::StaticString(CreateInPlaceWrapper createInPlaceWrapper)
	: DynamicString(createInPlaceWrapper)
{
}


void StaticString::operator= (const StaticString &other)
{
	// No need to be thread safe, static instances don't update
	imp.unsafeCopy(other.imp);
}


void StaticString::operator= (const DynamicString &other)
{
	StaticString tmpStr(other);
	/* Normal unsafe copy of StaticString now possible */
	imp.unsafeCopy(tmpStr.imp);
}

StaticString::operator StringRef() const
{
	return StringRef(getPointer(), getLength());
}

FB_END_PACKAGE0()
