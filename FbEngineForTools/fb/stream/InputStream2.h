#pragma once

#include "fb/lang/ByteOrderSwap.h"
#include "fb/lang/time/Time.h"
#include "fb/stream/CommonStream2.h"
#include "fb/stream/InputStream2Macro.h"
#include "fb/stream/InputStream2Decl.h"

FB_PACKAGE0()

// InputStream that is used through macros, eg. fb_stream_read(strm, value)
// Macros return false on stream read failure and trigger an assert. This makes code safer and debugging easier.
template<class StreamByteOrderT>
class InputStream2
{
public:
	typedef StreamByteOrderT StreamByteOrder;

	InputStream2()
	{
	}

	InputStream2(const uint8_t *data, SizeType dataSize)
		: data(data)
		, dataSize(dataSize)
	{
	}
	
	bool readImpl(bool &i) { return readImpl(&i, sizeof(bool)); }
	bool readImpl(uint8_t &i) { return readImpl(&i, sizeof(i)); }
	bool readImpl(uint16_t &i) { return swapReadImpl(i); }
	bool readImpl(uint32_t &i) { return swapReadImpl(i); }
	bool readImpl(uint64_t &i) { return swapReadImpl(i); }
	
	bool readImpl(int8_t &i) { return readImpl(&i, sizeof(i)); }
	bool readImpl(int16_t &i) { return swapReadImpl(i); }
	bool readImpl(int32_t &i) { return swapReadImpl(i); }
	bool readImpl(int64_t &i) { return swapReadImpl(i); }
	
	bool readImpl(float &i) { return swapReadImpl(i); }
	bool readImpl(double &i) { return swapReadImpl(i); }

	bool readImpl(Time &i)
	{
		int64_t ticks = 0;
		if (swapReadImpl(ticks))
		{
			i.setTicks(ticks);
			return true;
		}
		return false;
	}

	bool readImpl(DynamicString &t){ return readStringValueImpl(*this, t); }
	bool readImpl(StaticString &t) { return readStringValueImpl(*this, t); }
	bool readImpl(HeapString &t) { return readStringValueImpl(*this, t); }

	template<class T>
	bool readImpl(T &t)
	{
		// If you get an error here, make sure you have included "fb/stream/StreamValue.h" in your cpp
		return streamValue(*this, t);
	}
	
	bool readImpl(void *ptr, SizeType size)
	{
		if (position + size <= dataSize)
		{
			lang::MemCopy::copy(ptr, data + position, size);
			position += size;
			return true;
		}
		else
		{
			lang::MemSet::set(ptr, 0, size);
		}
		position = dataSize;
		return false;
	}
	
	template<class T>
	bool swapReadImpl(T &t)
	{
		if (!readImpl(&t, sizeof(T)))
			return false;
		StreamByteOrder::convertToNative(t);
		return true;
	}

	const uint8_t *getData() const { return data; }
	SizeType getSize() const { return dataSize; }
	SizeType getPosition() const { return position; }
	SizeType getBytesLeft() const { return dataSize - position; }

	bool setPositionImpl(SizeType newPosition)
	{
		if (newPosition <= dataSize)
		{
			position = newPosition;
			return true;
		}
		return false;
	}
	
	bool skipDataImpl(SizeType bytes)
	{
		return setPositionImpl(position + bytes);
	}

	bool isInputStream() const { return true; }
	bool isOutputStream() const { return false; }
	bool isReading() const { return true; }
	bool isWriting() const { return false; }
	
	template<class T>
	bool streamImpl(T &t) { return readImpl(t); }
	bool streamImpl(void *ptr, SizeType size) { return readImpl(ptr, size); }
	void streamTextImpl(const char *) {}
	
	typedef InputStreamType StreamType;
	typedef StreamPersistentTrue StreamPersistent;
	typedef StreamClassByte StreamClass;
	
private:
	template<class StringType>
	static bool readStringValueImpl(InputStream2 &strm, StringType &t)
	{
		// scan for zero terminator
		const char *data = (const char *)strm.getData();
		SizeType start = strm.getPosition();
		SizeType end = strm.getSize();
		for (SizeType i = start; i < end; i++)
		{
			if (data[i] == '\0')
			{
				t = StringType(data + start, i - start);
				fb_stream_setPosition(strm, i + 1);
				return true;
			}
		}
		return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER);
	}

	const uint8_t *data = nullptr;
	SizeType dataSize = 0;
	SizeType position = 0;
};

bool streamReadFailed(const char *file, int line);

FB_END_PACKAGE0()
