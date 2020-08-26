#pragma once

#include "fb/container/LinearHashMap.h"
#include "fb/string/StringHash.h"

FB_PACKAGE1(stream)

/**
 * Helper for streaming objects such as strings that may appear multiple times in a stream
 */
template<class ObjectType, class IndexT>
class StreamObjectTable
{
public:
	typedef IndexT IndexType;

	template<class StreamClass>
	void read(StreamClass &strm, ObjectType &obj)
	{
		IndexType index = IndexType(-1);
		strm.read(index);
		fb_expensive_assert(SizeType(index) < objects.getSize());
		if (SizeType(index) < objects.getSize())
		{
			obj = objects[index];
		}
	}

	template<class StreamClass>
	void write(StreamClass &strm, const ObjectType &obj)
	{
		// find existing entry
		typename ObjectToIndexMap::ConstIterator it = objectToIndexMap.find(obj);
		if (it != objectToIndexMap.getEnd())
		{
			IndexType index = it.getValue();
			strm.write(index);
			return;
		}

		// add new entry
		IndexType index = (IndexType)objects.getSize();
		objects.pushBack(obj);
		objectToIndexMap[obj] = index;
		// check for too many entries in container (if this fails IndexType must be larger)
		fb_assert(SizeType(index) == objects.getSize()-1);
		strm.write(index);
	}

	void clear()
	{
		objects.clear();
		objectToIndexMap.clear();
	}

	bool isEmpty() const
	{
		return objects.isEmpty();
	}

protected:
	StreamObjectTable(const DynamicString &id)
		: id(id)
	{
	}

	template<class StreamClass>
	IndexType readTableHeader(StreamClass &strm)
	{
		if (strm.checkId(id.getPointer(), id.getLength() + 1))
		{
			unsigned char version = 0;
			strm.read(version);
			if (version != 1)
				return 0;

			IndexType numObjects = 0;
			strm.read(numObjects);
			return numObjects;
		}
		return 0;
	}

	template<class StreamClass>
	void writeTableHeader(StreamClass &strm)
	{
		if (!objects.isEmpty())
		{
			// write ID
			strm.write(id.getPointer(), id.getLength() + 1);
			// write version
			uint8_t version = 1;
			strm.write(version);
			// write number of objects
			IndexType numObjects = (IndexType)objects.getSize();
			fb_assert(SizeType(numObjects) == objects.getSize() && "Conversion to IndexType failed. Too many objects?");
			strm.write(numObjects);
		}
	}

protected:
	Vector<ObjectType> objects;
	typedef LinearHashMap<DynamicString, IndexType> ObjectToIndexMap;
	ObjectToIndexMap objectToIndexMap;
	StaticString id;
};

FB_END_PACKAGE1()
