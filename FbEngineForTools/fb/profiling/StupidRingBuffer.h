#pragma once

#include "fb/lang/FBAssert.h"

FB_PACKAGE1(profiling)

template<typename T, SizeType TBufferSize = 64 * 1024>
struct StupidRingBuffer
{
	enum : SizeType
	{
		BufferSize = TBufferSize
	};

	StupidRingBuffer(T (&b)[BufferSize]) { buffer = b; }

	void pushBack(const T &t)
	{
		fb_assert(write < BufferSize);
		buffer[write] = t;
		write = (write + 1) % BufferSize;
		if (write == read)
			read = (read + 1) % BufferSize;
	};

	void popFront()
	{
		if (read != write)
			read = (read + 1) % BufferSize;
	};

	SizeType getSize() const
	{
		fb_assertf(write + BufferSize > read, "%d + %d > %d", write, BufferSize, read);
		return (write + BufferSize - read) % BufferSize;
	}

	void clear()
	{
		read = 0;
		write = 0;
	}

	struct Iterator
	{
		const T* buffer;
		SizeType place;
		Iterator(const T* buffer, SizeType place, SizeType size)
			: buffer(buffer)
			, place(place)
		{
		}
		void operator++()
		{
			place = (place + 1) % BufferSize;
		}
		const T &operator*() const
		{
			return *(buffer + place);
		}
		bool operator!=(const Iterator &other)
		{
			return other.place != place || other.buffer != buffer;
		}
	};
	Iterator getBegin() const
	{
		return Iterator(buffer, read, BufferSize);
	}
	Iterator getEnd() const
	{
		return Iterator(buffer, write, BufferSize);
	}

private:
	T *buffer = nullptr;
	SizeType read = 0;
	SizeType write = 0;
};

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename T, SizeType N
#define FB_CONTAINERIMP_CONTAINER_TYPE StupidRingBuffer<T, N>
#include "fb/container/ContainerRangeFor.h"

FB_END_PACKAGE1();
