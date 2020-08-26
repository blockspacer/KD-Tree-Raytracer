#pragma once

#include "fb/lang/NumericLimits.h"
#include "fb/stream/InputStream2.h"
#include "fb/string/HeapString.h"
#include "BitTypes.h"

FB_PACKAGE0()

struct StreamClassBit {};

template<class StreamByteOrderT = lang::LittleEndian>
class BitInputStream
{
public:
	typedef StreamByteOrderT StreamByteOrder;

	BitInputStream()
		: data(0)
		, dataSizeInBits(0)
		, positionInBits(0)
	{
	}

	BitInputStream(const uint8_t *data, SizeType dataSizeInBytes)
		: data(data)
		, dataSizeInBits(dataSizeInBytes * 8)
		, positionInBits(0)
	{
		// positionInBits as a 32 bit value cannot exceed this
		fb_assert(dataSizeInBytes <= lang::NumericLimits<uint32_t>::getMax() / 8);
	}
	
	BitInputStream(const uint8_t *data, OffsetInBits positionInBits, SizeInBits dataSizeInBits)
		: data(data)
		, dataSizeInBits(dataSizeInBits.get())
		, positionInBits(positionInBits.get())
	{
		fb_assert(positionInBits.get() <= dataSizeInBits.get());
	}
	
	bool readImpl(bool &i) { return readImpl(&i, sizeof(bool), SizeInBits(1)); }
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

	template<class T>
	bool readImpl(T &t) { return streamValue(*this, t); }
	
	bool readImpl(void *ptr, SizeType size)
	{
		return readImpl(ptr, size, SizeInBits(size * 8));
	}
	
	template<class T>
	bool readImpl(T &t, SizeInBits numBits)
	{
		return readImpl(&t, sizeof(t), numBits);
	}
	
	bool readImpl(void *ptr, SizeType sizeInBytes, SizeInBits numBits_)
	{
		SizeType numBits = numBits_.get();
		fb_assert(numBits <= sizeInBytes * 8);
		if (positionInBits + numBits > dataSizeInBits)
		{
			lang::MemSet::set(ptr, 0, sizeInBytes);
			positionInBits = dataSizeInBits;
			return false;
		}

		if ((positionInBits&7) + (numBits&7) == 0)
		{
			// byte read
			lang::MemCopy::copy(ptr, data + positionInBits / 8, numBits / 8);
			// zero out remainder
			lang::MemSet::set((uint8_t*)ptr + (numBits / 8), 0, sizeInBytes - (numBits / 8));
			positionInBits = positionInBits + numBits;
			return true;
		}
		else
		{
			// bit read
			lang::MemSet::set(ptr, 0, sizeInBytes);

			/*
			// slow bit-by-bit read
			uint8_t *dst = (uint8_t*)ptr;
			SizeType numBitsRead = 0;
			while (numBitsRead < numBits)
			{
				const uint32_t inputShift = positionInBits & 7;
				const uint32_t inputBit = (data[positionInBits / 8] >> inputShift) & 1U;
				const uint32_t outputShift = numBitsRead & 7;
				dst[ numBitsRead / 8 ] |= inputBit << outputShift;
				positionInBits++;
				numBitsRead++;
			}
			*/

			uint32_t numBitsRead = 0;

			// read 64-bit chunks
			{
				const uint64_t *src = (const uint64_t*)data;
				uint64_t *dst = (uint64_t*)ptr;
				while (numBitsRead + 64 <= numBits)
				{
					const uint32_t remainingInputBits = 64 - (positionInBits & 63);
					const uint32_t remainingOutputBits = 64 - (numBitsRead & 63);
					const uint32_t bitsToRead = lang::min(lang::min(numBits - numBitsRead, remainingInputBits), remainingOutputBits);

					const uint64_t mask = (1LLU << bitsToRead) - 1U;
					const uint32_t inputShift = positionInBits & 63;
					const uint32_t outputShift = numBitsRead & 63;
					dst[numBitsRead / 64] |= ((src[positionInBits / 64] >> inputShift) & mask) << outputShift;
					positionInBits = positionInBits + bitsToRead;
					numBitsRead += bitsToRead;
				}
			}
			
			// read 8-bit chunks
			{
				const uint8_t *src = data;
				uint8_t *dst = (uint8_t*)ptr;
				while (numBitsRead < numBits)
				{
					const uint32_t remainingInputBits = 8 - (positionInBits & 7);
					const uint32_t remainingOutputBits = 8 - (numBitsRead & 7);
					const uint32_t bitsToRead = lang::min(lang::min(numBits - numBitsRead, remainingInputBits), remainingOutputBits);

					const uint32_t mask = (1U << bitsToRead) - 1U;
					const uint32_t inputShift = positionInBits & 7;
					const uint32_t outputShift = numBitsRead & 7;
					dst[ numBitsRead / 8 ] |= ((src[positionInBits / 8] >> inputShift) & mask) << outputShift;
					positionInBits = positionInBits + bitsToRead;
					numBitsRead += bitsToRead;
				}
			}
			return true;
		}
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
	SizeType getSizeInBytes() const { return (dataSizeInBits + 7) / 8; }
	SizeInBits getSizeInBits() const { return SizeInBits(dataSizeInBits); }
	OffsetInBits getPositionInBits() const { return OffsetInBits(positionInBits); }
	SizeInBits getBitsLeft() const { return SizeInBits(dataSizeInBits - positionInBits); }
	SizeType getBytesLeft() const { return getBitsLeft().get() / 8; }

	bool setPositionInBitsImpl(OffsetInBits newPositionInBits)
	{
		if (newPositionInBits <= dataSizeInBits)
		{
			positionInBits = newPositionInBits.get();
			return true;
		}
		else
		{
			positionInBits = dataSizeInBits;
			return false;
		}
	}
	
	bool skipBytesImpl(SizeType bytes)
	{
		return setPositionInBitsImpl(positionInBits + bytes * 8);
	}

	bool isInputStream() const { return true; }
	bool isOutputStream() const { return false; }
	bool isReading() const { return true; }
	bool isWriting() const { return false; }
	
	template<class T>
	bool streamImpl(T &t) { return readImpl(t); }
	bool streamImpl(void *ptr, SizeType size) { return readImpl(ptr, size); }
	void streamTextImpl(const char *) {}
	
	template<class T>
	bool streamImpl(T &t, SizeInBits numBits) { return readImpl(t, numBits); }
	bool streamImpl(void *ptr, SizeType sizeInBytes, SizeInBits numBits) { return readImpl(ptr, sizeInBytes, numBits); }
		
	typedef InputStreamType StreamType;
	typedef StreamPersistentTrue StreamPersistent;
	typedef StreamClassBit StreamClass;
	
private:
	const uint8_t *data;
	SizeType dataSizeInBits;
	SizeType positionInBits;
};

bool streamReadFailed(const char *file, int line);

#define fb_stream_setPositionInBits(strm, value) do { if (!strm.setPositionInBitsImpl(value)) return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER); } while (false)

template<class S, class StringType>
static bool streamStringValue(BitInputStream<S> &strm, StringType &t, InputStreamType)
{
	SmallTempString str;
	while (true)
	{
		uint8_t c;
		fb_stream_read(strm, c);
		if (c == '\0')
		{
			t = StringType(str);
			return true;
		}
		str += (char)c;
	}
	return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER);
}

FB_END_PACKAGE0()
