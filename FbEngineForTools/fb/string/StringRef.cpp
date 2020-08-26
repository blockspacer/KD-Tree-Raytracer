#include "Precompiled.h"
#include "StringRef.h"

#include "fb/lang/Cstrlen.h"
#include "fb/string/CommonImpl.h"
#include <cstring> // For strcmp

FB_PACKAGE0()

#define FB_STRING_THIS_CLASS StringRef

FB_STRING_CASE_DETECTION_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_COMPARE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_FIND_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_BOOL_PARSE_IMPL(FB_STRING_THIS_CLASS);
FB_STRING_NUMBER_PARSE_IMPL(FB_STRING_THIS_CLASS);

#undef FB_STRING_THIS_CLASS

static const char *nothing = "";
StringRef StringRef::empty = StringRef(nothing, 0);


bool StringRef::operator== (const StringRef &other) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_expensive_assert(other.ptr != nullptr);

	return getLength() == other.getLength() && (strcmp(this->ptr, other.getPointer()) == 0);
}


bool StringRef::operator== (const char *other) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_assert(other != nullptr);

	return (strcmp(this->ptr, other) == 0);
}


bool StringRef::operator!= (const StringRef &other) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_expensive_assert(other.ptr != nullptr);

	return getLength() != other.getLength() || (strcmp(this->ptr, other.getPointer()) != 0);
}


bool StringRef::operator!= (const char *other) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_assert(other != nullptr);

	return (strcmp(this->ptr, other) != 0);
}


bool StringRef::operator< (const StringRef &other) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_expensive_assert(other.ptr != nullptr);

	return (strcmp(this->ptr, other.getPointer()) < 0);
}


bool StringRef::operator< (const char *other) const
{
	fb_expensive_assert(this->ptr != nullptr);
	fb_assert(other != nullptr);

	return (strcmp(this->ptr, other) < 0);
}


SizeType StringRef::strLen(const char *str)
{
	return fbStrLen(str);
}


FB_END_PACKAGE0()
