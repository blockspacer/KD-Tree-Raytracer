#include "Precompiled.h"

#include "fb/util/ExpGolomb.h"

FB_PACKAGE1(util)

class BitWriter
{
public:
	BitWriter(uint8_t *buffer, SizeType bufferSizeInBytes)
		: bufferBytesRemaining(bufferSizeInBytes)
		, totalBitsWritten(0)
		, bufferPointer(buffer)
		, byteToWrite(0)
		, bitsWritten(0)
	{
	}
 
	~BitWriter()
	{
		flush();
	}
 
	void writeBit(bool bit)
	{
		fb_assert(bufferBytesRemaining != 0);
 
		byteToWrite = uint8_t((byteToWrite << 1) | (bit ? 1U : 0));
		++bitsWritten;
		++totalBitsWritten;
 
		if (bitsWritten == 8)
			flushFullByte();
	}
 
	SizeType getTotalBitsWritten() const
	{
		return totalBitsWritten;
	}
 
private:
	void flush()
	{
		if (bitsWritten == 0)
			return;
 
		for (SizeType i = bitsWritten; i != 8; ++i)
			byteToWrite <<= 1;
 
		flushFullByte();
	}
 
	void flushFullByte()
	{
		*bufferPointer = byteToWrite;
 
		++bufferPointer;
		--bufferBytesRemaining;
		byteToWrite = 0;
		bitsWritten = 0;
	}
 
	SizeType bufferBytesRemaining;
	SizeType totalBitsWritten;
 
	uint8_t *bufferPointer;
 
	uint8_t byteToWrite;
	uint8_t bitsWritten;
};
 
class BitReader
{
public:
	BitReader(const uint8_t *buffer, SizeType bufferSizeInBits)
		: bufferPointer(buffer)
		, bitsRead(0)
		, byteToRead(0)
	{
		if (bufferSizeInBits % 8 == 0)
		{
			bufferWholeBytesRemaining = (bufferSizeInBits - 1) / 8;
			bufferLastByteBits = 8;
		}
		else
		{
			bufferWholeBytesRemaining = bufferSizeInBits / 8;
			bufferLastByteBits = bufferSizeInBits % 8;
		}
 
		if (bufferSizeInBits > 0)
		{
			byteToRead = *bufferPointer;
			++bufferPointer;
		}
	}
 
	bool readBit()
	{
		fb_assert(!eof());
 
		if (bitsRead == 8 && bufferWholeBytesRemaining != 0)
		{
			byteToRead = *bufferPointer;
			++bufferPointer;
			--bufferWholeBytesRemaining;
			bitsRead = 0;
		}
 
		++bitsRead;
		bool msb = bool((byteToRead & 0x80) != 0);
		byteToRead <<= 1;
 
		return msb;
	}
 
	bool eof() const
	{
		return bufferWholeBytesRemaining == 0 && bufferLastByteBits == bitsRead;
	}
 
private:
	SizeType bufferWholeBytesRemaining;
	const uint8_t *bufferPointer;
	uint8_t bufferLastByteBits;
	uint8_t bitsRead;
	uint8_t byteToRead;
};

typedef uint32_t RemapType;
 
static uint32_t toRemap(int8_t i)
{
	int32_t twoI = 2 * i;
	return uint32_t((i >= 0) ? (twoI + 1) : (-twoI));
}
 
static int8_t fromRemap(uint32_t i)
{
	int32_t si = int32_t(i);
	bool sign = bool(si & 1);
	return int8_t(sign ? (si - 1) / 2 : (-si / 2));
}

SizeType expGolombEncode(const int8_t *in, SizeType inN, uint8_t *out, SizeType outN)
{
	BitWriter bw(out, outN);
	for (SizeType byte = 0; byte != inN; ++byte)
	{
		RemapType remapped = toRemap(in[byte]);
		fb_assert(remapped != 0);
 
		RemapType temp = remapped;
		const SizeType remapTypeBits = 8 * sizeof(RemapType);
		uint8_t len = 0;
		uint8_t bitsReversed[remapTypeBits] = {};
		while (temp != 0)
		{
			bitsReversed[len] = temp & 1;
			temp >>= 1;
			++len;
		}
 
		for (uint8_t i = 0; i != len - 1; ++i)
			bw.writeBit(0);
 
		for (uint8_t i = 0; i != len; ++i)
			bw.writeBit(bitsReversed[len - 1 - i] ? 1 : 0);
	}
	return bw.getTotalBitsWritten();
}
 
SizeType expGolombDecode(const uint8_t *in, SizeType inNBits, int8_t *out, SizeType outN)
{
	BitReader br(in, inNBits);
	SizeType bytesWritten = 0;
 
	while (!br.eof())
	{
		uint8_t zeros = 0;
		while (br.readBit() == 0 && !br.eof())
			++zeros;
 
		RemapType byte = 1;
		for (uint8_t i = 0; i != zeros; ++i)
		{
			byte <<= 1;
			byte |= br.readBit() ? 1 : 0;
		}
 
		out[bytesWritten] = fromRemap(byte);
		++bytesWritten;
	}
 
	return bytesWritten;
}
 
FB_END_PACKAGE1()
