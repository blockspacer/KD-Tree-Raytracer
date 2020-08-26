#pragma once

FB_PACKAGE1(stream)

template<class StreamByteOrder>
InputStream<StreamByteOrder>::InputStream()
	: data(0)
	, dataSize(0)
	, readPointer(0)
	, syncCounter(0)
	, streamReadFailed(false)
{
}

template<class StreamByteOrder>
InputStream<StreamByteOrder>::InputStream(const void *data, SizeType dataSize)
	: data((const uint8_t *)data)
	, dataSize(dataSize)
	, readPointer(0)
	, syncCounter(0)
	, streamReadFailed(false)
{
}

template<class StreamByteOrder>
SizeType InputStream<StreamByteOrder>::readString(const char **str)
{
	SizeType startPointer = readPointer;
	// find null terminator
	while (readPointer < dataSize)
	{
		if (data[readPointer] == '\0')
		{
			*str = (const char *)(data + startPointer);
			SizeType len = readPointer - startPointer;
			readPointer++;
			return len;
		}
		readPointer++;
	}
	// none found
	*str = "";
	setStreamReadFailed(true);
	return 0;
}

template<class StreamByteOrder>
StringRef InputStream<StreamByteOrder>::readString()
{
	SizeType startPointer = readPointer;
	// find null terminator
	while (readPointer < dataSize)
	{
		if (data[readPointer] == '\0')
		{
			const char *str = (const char *)(data + startPointer);
			SizeType len = readPointer - startPointer;
			readPointer++;
			return StringRef(str, len);
		}
		readPointer++;
	}
	// none found
	setStreamReadFailed(true);
	return StringRef::empty;
}

template<class StreamByteOrder>
bool InputStream<StreamByteOrder>::checkId(const char *id, SizeType size)
{
	if (readPointer + size <= dataSize)
	{
		if (lang::MemCompare::equals(data + readPointer, id, size))
		{
			readPointer += size;
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

template<class StreamByteOrder>
bool InputStream<StreamByteOrder>::checkId(const void *id, SizeType size)
{
	// Assuming that everything else might need their endianess flipped
	if (readPointer + size <= dataSize)
	{
		if (StreamByteOrder::compareToNative(data + readPointer, id, size))
		{
			readPointer += size;
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(void *ptr, SizeType size)
{
	fb_expensive_assert(readPointer + size <= dataSize);
	if (readPointer + size <= dataSize)
	{
		lang::MemCopy::copy(ptr, data + readPointer, size);
		readPointer += size;
	}
	else
	{
		lang::MemSet::set(ptr, 0, size);
		setStreamReadFailed(true);
		readPointer = dataSize;
	}
}

template<class StreamByteOrder> template<class T>
void InputStream<StreamByteOrder>::readImpl(T &t)
{
	fb_expensive_assert(readPointer + sizeof(T) <= dataSize);
	if (readPointer + sizeof(T) <= dataSize)
	{
		t = *(const T*)(data + readPointer);
		readPointer += sizeof(T);
	}
	else
	{
		setStreamReadFailed(true);
		readPointer = dataSize;
		t = T();
	}
}

template<class StreamByteOrder> template<class T>
void InputStream<StreamByteOrder>::readOrderedImpl(T &t)
{
	fb_expensive_assert(readPointer + sizeof(T) <= dataSize);
	if (readPointer + sizeof(T) <= dataSize)
	{
		t = *(const T*)(data + readPointer);
		StreamByteOrder::convertToNative(t);
		readPointer += sizeof(T);
	}
	else
	{
		setStreamReadFailed(true);
		readPointer = dataSize;
		t = T();
	}
}

template<class StreamByteOrder> template<class T>
void InputStream<StreamByteOrder>::readOrderedAlignedImpl(T &t)
{
	fb_expensive_assert(readPointer + sizeof(T) <= dataSize);
	if (readPointer + sizeof(T) <= dataSize)
	{
		t = lang::loadUnaligned<T>(data + readPointer);
		StreamByteOrder::convertToNative(t);
		readPointer += sizeof(T);
	}
	else
	{
		setStreamReadFailed(true);
		readPointer = dataSize;
		t = T();
	}
}

template<class StreamByteOrder>
SizeType InputStream<StreamByteOrder>::getBytesLeft() const
{
	return dataSize - readPointer;
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::skipData(SizeType size)
{
	readPointer += size;
	// tried to skip too far
	fb_expensive_assert(readPointer <= dataSize);
	if (readPointer > dataSize)
	{
		setStreamReadFailed(true);
		readPointer = dataSize;
	}
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::setPosition(SizeType size)
{
	readPointer = size;
	fb_expensive_assert(readPointer <= dataSize);
	if (readPointer > dataSize)
	{
		setStreamReadFailed(true);
		readPointer = dataSize;
	}
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4, bool &b5, bool &b6, bool &b7)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
	b2 = !!(mask & 4);
	b3 = !!(mask & 8);
	b4 = !!(mask & 16);
	b5 = !!(mask & 32);
	b6 = !!(mask & 64);
	b7 = !!(mask & 128);
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4, bool &b5, bool &b6)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
	b2 = !!(mask & 4);
	b3 = !!(mask & 8);
	b4 = !!(mask & 16);
	b5 = !!(mask & 32);
	b6 = !!(mask & 64);
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4, bool &b5)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
	b2 = !!(mask & 4);
	b3 = !!(mask & 8);
	b4 = !!(mask & 16);
	b5 = !!(mask & 32);
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
	b2 = !!(mask & 4);
	b3 = !!(mask & 8);
	b4 = !!(mask & 16);
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1, bool &b2, bool &b3)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
	b2 = !!(mask & 4);
	b3 = !!(mask & 8);
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1, bool &b2)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
	b2 = !!(mask & 4);
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::read(bool &b0, bool &b1)
{
	unsigned char mask = 0;
	read(mask);
	b0 = !!(mask & 1);
	b1 = !!(mask & 2);
}

template<class StreamByteOrder>
bool InputStream<StreamByteOrder>::readSync()
{
	uint32_t ref, complRef;
	read(ref);
	read(complRef);

	fb_assertf(complRef == ~ref, "Stream sync corruption: Expected to read a sync but didn't: 0x%x is not ~0x%x (0x%x)", complRef, ref, ~ref);
	fb_assertf(syncCounter == ref, "Stream sync mismatch: Read %u, expected %u", syncCounter, ref);

	streamReadFailed |= complRef != ~ref || syncCounter != ref;
	syncCounter = ref + 1;
	return streamReadFailed;
}

template<class StreamByteOrder>
void InputStream<StreamByteOrder>::truncateToSize(SizeType size)
{
	fb_assert(size <= dataSize);
	if (size < dataSize)
	{
		dataSize = size;
		if (readPointer > dataSize)
			readPointer = dataSize;
	}
}

FB_END_PACKAGE1()
