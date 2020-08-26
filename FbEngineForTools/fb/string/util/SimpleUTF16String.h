#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/GlobalFixedAllocateFunctions.h"
#include "fb/lang/Types.h"
#include "fb/string/util/UnicodeDefs.h"

FB_DECLARE0(StringRef)

FB_PACKAGE1(string)

class SimpleUTF16String
{
public:
	friend class UnicodeConverter;
	SimpleUTF16String() { data.pushBack(getUTF16NulChar()); }
	unsigned getSize() const { return unsigned(data.getSize()) - 1; }
	SizeType getLength() const { return data.getSize() - 1; }
	const UTF16Unit *getPointer() const;
	const UTF16Unit operator[](SizeType index) const;
	UTF16Unit &operator[](SizeType index);

	void append(const UTF16Unit *str);
	void append(const SimpleUTF16String &other);
	void appendUTF8(const StringRef &other);

	SimpleUTF16String &operator+=(const UTF16Unit *str);
	SimpleUTF16String &operator+=(const SimpleUTF16String &other);
	SimpleUTF16String &operator<<(const UTF16Unit *str);
	SimpleUTF16String &operator<<(const SimpleUTF16String &other);

	// Maybe should just support assignment and copy construction, but going with a cautionary approach in case there has been
	// some specific point to leave those out.
	void replaceWith(const SimpleUTF16String &otherString) { data = otherString.data; }

	void reserve(SizeType newCapacity) { data.reserve(newCapacity); }
	void clear();

	FB_ADD_CLASS_MEMORY_OVERLOADS_SINGLETHREAD(SimpleUTF16String);

private:
	CachePodVector<UTF16Unit, 512> data;
};

FB_END_PACKAGE1()
