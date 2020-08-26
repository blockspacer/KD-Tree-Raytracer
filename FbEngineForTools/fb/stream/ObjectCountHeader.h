#pragma once

#include "fb/lang/NumericLimits.h"

FB_PACKAGE1(stream)

template<typename StreamType, typename StorageType>
class ObjectCountHeader : public StreamType::IChunkHeader
{
public:
	ObjectCountHeader() : numObjects(0)
	{
	}

	SizeType getHeaderSize()
	{
		return sizeof(StorageType);
	}

	SizeType getNumObjects() const
	{
		return numObjects;
	}

	void addObject()
	{
		numObjects++;
	}

	void writeHeader(typename StreamType::BaseClass::BaseClass *strm, SizeType chunkSize)
	{
		fb_assert(numObjects <= lang::NumericLimits<StorageType>::getMax());
		StorageType c = (StorageType)numObjects;
		strm->write(c);
	}

	SizeType numObjects;
};

FB_END_PACKAGE1()
