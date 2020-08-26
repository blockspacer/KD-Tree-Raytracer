#pragma once

#include "StreamObjectTable.h"

FB_PACKAGE1(stream)

/* For historical reasons, StaticStringStreamStringTable uses uint16_t index (meaning it can't really save as many 
 * strings as it should for e.g. type binary) while DynamicStringStreamStringTable uses 32 bit index. They really 
 * should be folded to the same class and whether to read strings as Static or Dynamic just specified with a bool.
 */

class StaticStringStreamStringTable : public stream::StreamObjectTable<StaticString, uint16_t>
{
public:
	typedef stream::StreamObjectTable<StaticString, uint16_t> BaseClass;

	StaticStringStreamStringTable(const DynamicString &id)
		: BaseClass(id)
	{
	}

	template<class StreamClass>
	void readTable(StreamClass &strm)
	{
		typename BaseClass::IndexType numObjects = BaseClass::readTableHeader(strm);

		if (numObjects == 0)
			return;

		uint32_t containerSize = BaseClass::objects.getSize();
		uint32_t containerCapacity = BaseClass::objects.getCapacity();
		if (containerSize + numObjects > containerCapacity)
			BaseClass::objects.reserve(containerSize + numObjects);

		for (SizeType i = 0; i < numObjects; i++)
		{
			const char *str = nullptr;
			SizeType len = strm.readString(&str);
			if (str)
				BaseClass::objects.pushBack(StaticString(str, len));
			else
				break;
		}
	}

	template<class StreamClass>
	void writeTable(StreamClass &strm)
	{
		BaseClass::writeTableHeader(strm);
		typename BaseClass::IndexType numObjects = (typename BaseClass::IndexType)BaseClass::objects.getSize();

		// reserve space for string data
		SizeType size = strm.getSize();
		for (SizeType i = 0; i < numObjects; i++)
			size += BaseClass::objects[i].getLength()+1;

		strm.reserve(size);

		// copy string data
		for (SizeType i = 0; i < numObjects; i++)
			strm.write(BaseClass::objects[i].getPointer(), BaseClass::objects[i].getLength()+1);
	}
};

class DynamicStringStreamStringTable : public stream::StreamObjectTable<DynamicString, uint32_t>
{
public:
	typedef stream::StreamObjectTable<DynamicString, uint32_t> BaseClass;

	DynamicStringStreamStringTable(const DynamicString &id)
		: BaseClass(id)
	{
	}

	template<class StreamClass>
	void readTable(StreamClass &strm, bool readAsStatic)
	{
		typename BaseClass::IndexType numObjects = BaseClass::readTableHeader(strm);

		if (numObjects == 0)
			return;

		uint32_t containerSize = BaseClass::objects.getSize();
		uint32_t containerCapacity = BaseClass::objects.getCapacity();
		if (containerSize + numObjects > containerCapacity)
			BaseClass::objects.reserve(containerSize + numObjects);

		for (SizeType i = 0; i < numObjects; i++)
		{
			const char *str = NULL;
			SizeType len = strm.readString(&str);
			if (str)
			{
				BaseClass::objects.pushBack(readAsStatic ? StaticString(str, len) : DynamicString(str, len));
			}
			else
			{
				break;
			}
		}
	}

	template<class StreamClass>
	void writeTable(StreamClass &strm)
	{
		BaseClass::writeTableHeader(strm);
		typename BaseClass::IndexType numObjects = (typename BaseClass::IndexType)BaseClass::objects.getSize();

		// reserve space for string data
		SizeType size = strm.getSize();
		for (SizeType i = 0; i < numObjects; i++)
			size += BaseClass::objects[i].getLength()+1;

		strm.reserve(size);

		// copy string data
		for (SizeType i = 0; i < numObjects; i++)
			strm.write(BaseClass::objects[i].getPointer(), BaseClass::objects[i].getLength()+1);
	}
};

FB_END_PACKAGE1()
