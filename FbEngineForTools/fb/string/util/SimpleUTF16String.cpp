#include "Precompiled.h"
#include "SimpleUTF16String.h"

#include "UnicodeConverter.h"

FB_PACKAGE1(string)

const UTF16Unit *SimpleUTF16String::getPointer() const
{
	fb_assert(data.getSize() > 0);
	return &(data[0]);
}


const UTF16Unit SimpleUTF16String::operator[](SizeType index) const
{
	fb_assert(data.getSize() > index);
	return data[index];
}


UTF16Unit &SimpleUTF16String::operator[](SizeType index)
{
	fb_assert(data.getSize() > index);
	return data[index];
}


void SimpleUTF16String::append(const UTF16Unit *str)
{
	fb_assert(!data.isEmpty());
	fb_assert(data.getBack() == getUTF16NulChar());
	data.popBack();
	while (*str != getUTF16NulChar())
	{
		data.pushBack(*str);
		++str;
	}
	data.pushBack(getUTF16NulChar());
}


void SimpleUTF16String::append(const SimpleUTF16String &other)
{
	/* There should always be at least NUL terminator */
	fb_assert(!other.data.isEmpty());
	fb_assert(other.data.getBack() == getUTF16NulChar());
	append(&other.data.getFront());
}


void SimpleUTF16String::appendUTF8(const StringRef &other)
{
	UnicodeConverter::addUTF8StrToUTF16String(other, *this);
}


SimpleUTF16String &SimpleUTF16String::operator+=(const UTF16Unit *str)
{
	append(str);
	return *this;
}


SimpleUTF16String &SimpleUTF16String::operator+=(const SimpleUTF16String &other)
{
	append(other);
	return *this;
}


SimpleUTF16String &SimpleUTF16String::operator<<(const UTF16Unit *str)
{
	append(str);
	return *this;
}

SimpleUTF16String &SimpleUTF16String::operator<<(const SimpleUTF16String &other)
{
	append(other);
	return *this;
}


void SimpleUTF16String::clear()
{
	data.clear();
	data.pushBack(getUTF16NulChar());
}

FB_END_PACKAGE1()
