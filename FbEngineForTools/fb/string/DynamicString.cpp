#include "Precompiled.h"
#include "DynamicString.h"

#include "fb/string/CommonImpl.h"
#include "fb/string/StaticString.h"

#include <string.h>

FB_PACKAGE0()

#define FB_STRING_THIS_CLASS DynamicString

FB_STRING_CASE_DETECTION_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_COMPARE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_FIND_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_BOOL_PARSE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_NUMBER_PARSE_IMPL(FB_STRING_THIS_CLASS);

#undef FB_STRING_THIS_CLASS

const DynamicString DynamicString::empty("", 0, true);

DynamicString::DynamicString(const char *ptr)
	: imp(ptr, SizeType(strlen(ptr)), false)
{
}

DynamicString::DynamicString(char *ptr)
	: imp(ptr, SizeType(strlen(ptr)), false)
{
}

DynamicString::DynamicString(const char *ptr, SizeType length)
	: imp(ptr, length, false)
{
}

DynamicString::DynamicString(const char *ptr, SizeType length, bool createAsStatic)
	: imp(ptr, length, createAsStatic)
{
}

DynamicString::DynamicString(const DynamicString &str)
	: imp(str.imp)
{
}


DynamicString::CreateInPlaceWrapper::CreateInPlaceWrapper(char *ptr, SizeType rawLength)
	: ptr(ptr)
{
	/* On x86, alignment doesn't matter. For the code below, two would be enough. Later, getting length will also
	 * require alignment of two, but nothing else but that or one byte static flag needs to be accessed. Four bytes is
	 * what StringImp requires in general, so that would be nice.
	 *
	 * If this assert triggers on a platform, where unaligned loads aren't allowed, getLength() (in both StringImp.h
	 * and .cpp) needs to be made to work byte at a time */
	//fb_assert(uintptr_t(ptr) % 4 == 0 && "Pointer is not aligned");
	SizeType length = (rawLength - string::StringImpHelper::OverheadBytes);
	string::StringImpHelper::setLength(ptr + string::StringImpHelper::ShiftBytes, length);
	fb_assert(strlen(ptr + string::StringImpHelper::ShiftBytes) == rawLength - string::StringImpHelper::OverheadBytes && "Wrong length for createInPlace string");
}

DynamicString::DynamicString(CreateInPlaceWrapper &createInPlaceWrapper)
	: imp(createInPlaceWrapper.ptr)
{
}

DynamicString DynamicString::createAsStatic(const char* ptr)
{
	return DynamicString(ptr, SizeType(strlen(ptr)), true);
}

bool DynamicString::isStatic() const
{
	return imp.isStatic();
}

void DynamicString::convertToStatic() const
{
	// This is not the most efficient way to do this, but work for const string.
	if (imp.isStatic())
		return;
	string::StringImp staticImp(imp.getPointer(), imp.getLength(), true);
}

void DynamicString::operator= (const DynamicString &other)
{
	imp = other.imp;
}

void DynamicString::operator= (const char *ptr)
{
	imp = DynamicString(ptr).imp;
}

bool DynamicString::operator== (const char *other) const
{
	if (other != NULL)
		return strcmp(imp.getPointer(), other) == 0;
	else
		return false;
}

bool DynamicString::operator== (const StaticString &other) const
{
	return imp == other.imp;
}

bool DynamicString::operator!= (const char *other) const
{
	return !(*this == other);
}

bool DynamicString::operator< (const char *other) const
{
	return strcmp(imp.getPointer(), other) < 0;
}

bool DynamicString::operator> (const char *other) const
{
	return strcmp(imp.getPointer(), other) > 0;
}

const DynamicString &DynamicString::getEmpty()
{
	static const StaticString emptyString("", 0);
	return emptyString;
}

DynamicString::operator StringRef() const
{
	return StringRef(getPointer(), getLength());
}

FB_END_PACKAGE0()
