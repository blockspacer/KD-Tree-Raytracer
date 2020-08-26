#pragma once

#include "fb/lang/ClassId.h"
#include "fb/stream/CommonStream2.h"
#include "fb/stream/InputStream2Macro.h"
#include "fb/stream/OutputStream2Macro.h"
#include "fb/stream/StreamMacro.h"

FB_PACKAGE0()

// For optimized streaming of small uint32_t values
// <= 127 : 1 byte
// <= 16383 : 2 bytes
// <= 2097151 : 3 bytes
// <= 268435455 : 4 bytes
// <= 0xFFFFFFFF : 5 bytes
class VariableSizeU32
{
public:
	VariableSizeU32()
	{
	}

	VariableSizeU32(uint32_t value)
		: value(value)
	{
	}

	uint32_t get() const { return value; }

	static ClassId getStaticClassId() { return FB_FOURCC('V', 'U', '3', '2'); }

	template<class T>
	bool stream(T &strm)
	{
		return streamImpl(strm, typename T::StreamType());
	}

	template<class T>
	bool streamImpl(T &strm, OutputStreamType)
	{
		uint32_t i = value;
		while (i > 127)
		{
			uint8_t sevenBits = (i & 127) | (128);
			fb_stream_write(strm, sevenBits);
			i >>= 7;
		}
		uint8_t sevenBits = (i & 127);
		fb_stream_write(strm, sevenBits);
		return true;
	}

	template<class T>
	bool streamImpl(T &strm, InputStreamType)
	{
		uint8_t sevenBits;
		fb_stream_read(strm, sevenBits);
		value = (sevenBits & 127U);
		uint32_t bitsRead = 7;
		while (sevenBits & 128)
		{
			fb_stream_check(bitsRead <= 32);
			fb_stream_read(strm, sevenBits);
			value |= (sevenBits & 127) << bitsRead;
			bitsRead += 7;
		}
		return true;
	}

	template<class T>
	bool tryToReadFromStream(T &strm, bool &waitForMoreData)
	{
		if (!strm.getBytesLeft())
		{
			waitForMoreData = true;
			return true;
		}
		uint8_t sevenBits;
		fb_stream_read(strm, sevenBits);
		value = (sevenBits & 127U);
		uint32_t bitsRead = 7;
		while (sevenBits & 128)
		{
			if (!strm.getBytesLeft())
			{
				waitForMoreData = true;
				return true;
			}
			fb_stream_check(bitsRead <= 32);
			fb_stream_read(strm, sevenBits);
			value |= (sevenBits & 127) << bitsRead;
			bitsRead += 7;
		}
		waitForMoreData = false;
		return true;
	}
	
	bool operator==(const VariableSizeU32 &other) const { return value == other.value; }
	bool operator!=(const VariableSizeU32 &other) const { return value != other.value; }

private:
	uint32_t value = 0;
};

FB_END_PACKAGE0()
