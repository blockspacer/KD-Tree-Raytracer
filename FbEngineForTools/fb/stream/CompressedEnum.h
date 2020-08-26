#pragma once

#include "fb/lang/CountBits.h"
#include "fb/stream/BitInputStream.h"
#include "fb/stream/StreamMacro.h"

FB_PACKAGE0()

// Enum compressed to [0,NumValues[ range
template<class Enum, int NumValues>
class CompressedEnum
{
public:
	CompressedEnum()
	{
	}

	CompressedEnum(Enum t)
	{
		set(t);
	}

	void set(Enum t)
	{
		value = (uint32_t)t;
		fb_static_assert(sizeof(Enum) <= sizeof(value));
		fb_assert(value < NumValues);
	}

	Enum get() const
	{
		return (Enum)value;
	}

	static SizeInBits getSizeInBits()
	{
		uint32_t sizeInBits = 1;
		while ((1U << sizeInBits) < NumValues)
			sizeInBits++;
		return SizeInBits(sizeInBits);
	}

	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, value, getSizeInBits());
		fb_stream_check(value < NumValues);
		return true;
	}

	bool operator==(const CompressedEnum &other) const { return value == other.value; }
	bool operator!=(const CompressedEnum &other) const { return value != other.value; }
	
	bool operator==(uint32_t other) const { return value == other; }
	bool operator!=(uint32_t other) const { return value != other; }

private:
	uint32_t value = 0;
};

FB_END_PACKAGE0()
