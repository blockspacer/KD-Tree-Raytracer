#ifndef FB_STREAM_OUTPUTSTREAMINLINE_H
#define FB_STREAM_OUTPUTSTREAMINLINE_H

#include "fb/lang/MemoryFunctions.h"
#include <cstring>

FB_PACKAGE1(stream)

template<class StreamByteOrder>
OutputStream<StreamByteOrder>::OutputStream()
	: writePointer(0)
	, buffer(0)
	, bufferSize(0)
	, bufferCapacity(0)
	, chunkStackSize(0)
	, firstChunkStackEntry(0)
	, syncCounter(0)
	, fixedAllocation(false)
{
}

template<class StreamByteOrder>
OutputStream<StreamByteOrder>::OutputStream(const OutputStream &other)
	: writePointer(0)
	, buffer(0)
	, bufferSize(0)
	, bufferCapacity(0)
	, chunkStackSize(0)
	, firstChunkStackEntry(0)
	, syncCounter(0)
	, fixedAllocation(false)
{
	(*this) = other;
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::operator=(const OutputStream<StreamByteOrder> &other)
{
	writePointer = other.writePointer;
	bufferSize = other.bufferSize;
	setBufferCapacity(bufferSize);
	lang::MemCopy::copy(buffer, other.buffer, other.bufferSize);
	
	chunkStackSize = other.chunkStackSize;
	firstChunkStackEntry = other.firstChunkStackEntry;
	chunkStackMultipleEntries = other.chunkStackMultipleEntries;
	syncCounter = other.syncCounter;
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::enableFixedAllocation(void *fixedBuff, SizeType fixedBuffSize)
{
	fb_assert(buffer == 0 && bufferSize == 0);
	buffer = (uint8_t *)fixedBuff;
	bufferCapacity = fixedBuffSize;
	fixedAllocation = true;
}

template<class StreamByteOrder>
OutputStream<StreamByteOrder>::~OutputStream()
{
	// you forgot to call endChunk() after beginChunk()
	fb_assert(chunkStackSize == 0);

	if (!fixedAllocation)
	{
		lang::freeFixed(buffer, bufferCapacity);
	}
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::setBufferCapacity(SizeType newCapacity)
{
	if (!fixedAllocation)
	{
		buffer = (uint8_t*) lang::reallocateFixed(buffer, bufferCapacity, newCapacity);
		bufferCapacity = newCapacity;

		if (bufferSize > bufferCapacity)
			bufferSize = bufferCapacity;
	}
	else
	{
		fb_assert(newCapacity <= bufferCapacity && "Exceeded fixed allocation size");

		if (newCapacity > bufferCapacity)
		{
			// failsafe work around for final release
			fixedAllocation = false;
			uint8_t *newBuffer = (uint8_t*) lang::allocateFixed(newCapacity);
			bufferCapacity = newCapacity;

			if (bufferSize > bufferCapacity)
				bufferSize = bufferCapacity;
			lang::MemCopy::copy(newBuffer, buffer, bufferSize);
			buffer = newBuffer;
		}
	}
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::prepareWritingData(SizeType bytes)
{
	if (writePointer + bytes > bufferSize)
	{
		bufferSize = writePointer + bytes;
		if (bufferSize > bufferCapacity)
		{
			// allocate at least 32 bytes, double capacity on every allocation
			uint64_t newCapacity = bufferCapacity;
			if (newCapacity < 32)
				newCapacity = 32;
			while (bufferSize > newCapacity)
			{
				newCapacity <<= 1;
				if (newCapacity > 0xFFFFFFFF)
				{
					fb_assert(0 && "Stream size over 4 GB, something is wrong");
					newCapacity = 0xFFFFFFFF;
					break;
				}
			}
			setBufferCapacity((uint32_t)newCapacity);
		}
	}
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(const void *ptr, SizeType size)
{
	if (size)
	{
		prepareWritingData(size);
		lang::MemCopy::copy(buffer + writePointer, ptr, size);
		writePointer += size;
	}
}

template<class StreamByteOrder> template<class T>
void OutputStream<StreamByteOrder>::writeImpl(const T &t)
{
	prepareWritingData(sizeof(T));
	*((T *)&buffer[writePointer]) = t;
	writePointer += sizeof(T);
}

template<class StreamByteOrder> template<class T>
void OutputStream<StreamByteOrder>::writeOrderedImpl(T t)
{
	StreamByteOrder::convertFromNative(t);
	prepareWritingData(sizeof(T));
	*((T *)&buffer[writePointer]) = t;
	writePointer += sizeof(T);
}

template<class StreamByteOrder> template<class T>
void OutputStream<StreamByteOrder>::writeOrderedAlignedImpl(T t)
{
	StreamByteOrder::convertFromNative(t);
	// need to use memcpy due to alignment
	write(&t, sizeof(T));
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::writeTo(SizeType offset, const void *ptr, SizeType size)
{
	SizeType oldBufferSize = bufferSize;
	bufferSize += size;
	setBufferCapacity(bufferSize);
	if (offset < oldBufferSize)
	{
		memmove(buffer + offset + size, buffer + offset, oldBufferSize - offset);
	}
	else
	{
		fb_assert(offset == oldBufferSize);
	}
	lang::MemCopy::copy(buffer + offset, ptr, size);
}

template<class StreamByteOrder>
uint8_t *OutputStream<StreamByteOrder>::getData()
{
	return buffer;
}

template<class StreamByteOrder>
const uint8_t *OutputStream<StreamByteOrder>::getData() const
{
	return buffer;
}

template<class StreamByteOrder>
SizeType OutputStream<StreamByteOrder>::getSize() const
{
	return bufferSize;
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::truncateTo(SizeType size)
{
	if (size < bufferSize)
		bufferSize = size;
	if (writePointer > size)
		writePointer = size;
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::setCurrentSize(SizeType size)
{
	fb_assert(size <= bufferCapacity);
	if (size <= bufferCapacity)
	{
		bufferSize = size;
		writePointer = size;
	}
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::reserve(SizeType size)
{
	if (size > bufferCapacity)
		setBufferCapacity(size);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::trim()
{
	setBufferCapacity(bufferSize);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::beginChunk(IChunkHeader &header)
{
	SizeType headerSize = header.getHeaderSize();
	prepareWritingData(headerSize);

	// avoid using Vector unless more than 1 entry is needed
	if (chunkStackSize == 0)
	{
		firstChunkStackEntry = writePointer;
	}
	else
	{
		chunkStackMultipleEntries.resize(chunkStackSize+1);
		chunkStackMultipleEntries[0] = firstChunkStackEntry;
		chunkStackMultipleEntries[chunkStackSize] = writePointer;
	}
	chunkStackSize++;

	writePointer += headerSize;
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::endChunk(IChunkHeader &header)
{
	fb_assert(chunkStackSize > 0);

	SizeType originalWritePointer = writePointer;

	// avoid using TinyVector unless more than 1 entry is needed
	if (chunkStackSize == 1)
	{
		writePointer = firstChunkStackEntry;
	}
	else
	{
		writePointer = chunkStackMultipleEntries.getBack();
		chunkStackMultipleEntries.popBack();
	}
	chunkStackSize--;

	SizeType chunkSize = originalWritePointer - writePointer - header.getHeaderSize();
	SizeType headerWritePointer = writePointer;
	(void)headerWritePointer; // Silences compile warning
	header.writeHeader(this, chunkSize);

	// must write exactly the amount in getHeaderSize!
	fb_expensive_assert(writePointer == headerWritePointer + header.getHeaderSize());

	writePointer = originalWritePointer;
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7)
{
	unsigned char mask = ((((int)b0)&1)<<0)
		| ((((int)b1)&1)<<1)
		| ((((int)b2)&1)<<2)
		| ((((int)b3)&1)<<3)
		| ((((int)b4)&1)<<4)
		| ((((int)b5)&1)<<5)
		| ((((int)b6)&1)<<6)
		| ((((int)b7)&1)<<7);
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6)
{
	unsigned char mask = ((((int)b0)&1)<<0)
		| ((((int)b1)&1)<<1)
		| ((((int)b2)&1)<<2)
		| ((((int)b3)&1)<<3)
		| ((((int)b4)&1)<<4)
		| ((((int)b5)&1)<<5)
		| ((((int)b6)&1)<<6);
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5)
{
	unsigned char mask = ((((int)b0)&1)<<0)
		| ((((int)b1)&1)<<1)
		| ((((int)b2)&1)<<2)
		| ((((int)b3)&1)<<3)
		| ((((int)b4)&1)<<4)
		| ((((int)b5)&1)<<5);
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1, bool b2, bool b3, bool b4)
{
	unsigned char mask = ((((int)b0)&1)<<0)
		| ((((int)b1)&1)<<1)
		| ((((int)b2)&1)<<2)
		| ((((int)b3)&1)<<3)
		| ((((int)b4)&1)<<4);
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1, bool b2, bool b3)
{
	unsigned char mask = ((((int)b0)&1)<<0)
		| ((((int)b1)&1)<<1)
		| ((((int)b2)&1)<<2)
		| ((((int)b3)&1)<<3);
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1, bool b2)
{
	unsigned char mask = ((((int)b0)&1)<<0)
		| ((((int)b1)&1)<<1)
		| ((((int)b2)&1)<<2);
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::write(bool b0, bool b1)
{
	uint8_t u0 = b0 ? 1U : 0;
	uint8_t u1 = b1 ? 1U : 0;
	uint8_t mask = uint8_t((u0 << 0) | (u1 << 1));
	write(mask);
}

template<class StreamByteOrder>
void OutputStream<StreamByteOrder>::writeSync()
{
	write(syncCounter);
	write(~syncCounter);
	syncCounter++;
}

FB_END_PACKAGE1()

#endif
