#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/profiling/ZoneEvent.h"

FB_PACKAGE1(profiling)

struct ZoneRing
{
	typedef ZoneEvent T;

	ZoneRing()
		: read(0)
		, write(0)
		, vec()
	{
	}

	void reset()
	{
		read = 0;
		write = 0;
		vec.releaseBuffer();
	}

	void setSize(SizeType newSize)
	{
		fb_assert(newSize > 0);
		vec.allocateBuffer(newSize);
		read = 0U;
		write = 0U;
	}

	void resize(SizeType newSize)
	{
		if (newSize != vec.getSize())
		{
			if (write < vec.getSize() && getSize() > 0 && vec.heapAllocated)
			{
				// Copy over existing zones when growing
				// TODO: Consider copying zones also when shrinking

				SizeType oldSize = vec.getSize();
				T *oldHeapBuffer = vec.buffer;

				vec.dismissHeapBuffer(); // Don't free old buffer yet
				vec.allocateBuffer(newSize); // Allocate new buffer

				SizeType dstIndex = 0U;
				for (SizeType srcIndex = read; srcIndex != write && dstIndex < newSize; )
				{
					vec[dstIndex] = oldHeapBuffer[srcIndex];
					srcIndex = (srcIndex + 1) % oldSize;
					++dstIndex;
				}

				read = 0;
				write = dstIndex;

				lang::osFree(oldHeapBuffer); // Now free the old buffer
			}
			else
			{
				setSize(newSize);
			}
		}
	}

	struct Iter
	{
		SizeType index; // This is a "raw" index to the underlying vector. No need to do modulo or add the read pointer.
		Iter(SizeType index)
			: index(index)
		{
		}
	};

	bool hasTwoHalves() const
	{
		return write < read;
	}

	Iter getBeginOnlyHalf() const
	{
		fb_assertf(!hasTwoHalves(), "!%d < %d", write, read);
		return read;
	}
	Iter getEndOnlyHalf() const
	{
		fb_assertf(!hasTwoHalves(), "!%d < %d", write, read);
		return write;
	}

	Iter getBeginFirstHalf() const
	{
		fb_assertf(hasTwoHalves(), "%d < %d", write, read);
		return read;
	}
	Iter getEndFirstHalf() const
	{
		fb_assertf(hasTwoHalves(), "%d < %d", write, read);
		return vec.getSize();
	}

	Iter getBeginSecondHalf() const
	{
		fb_assertf(hasTwoHalves(), "%d < %d", write, read);
		return 0U;
	}
	Iter getEndSecondHalf() const
	{
		fb_assertf(hasTwoHalves(), "%d < %d", write, read);
		return write;
	}

	SizeType getSize() const
	{
		if (read <= write)
			return write - read;
		else
			return vec.getSize();
	}
	SizeType getCapacity() const
	{
		return vec.getCapacity();
	}

	T &pushBack()
	{
		fb_assert(!vec.isEmpty());
		SizeType index = write;
		write = toIndex(write + 1);
		if (read == write)
			read = toIndex(write + 1);

		return vec[index];
	}

	void popFront(SizeType count)
	{
		fb_assert(!vec.isEmpty());

		// This could definitely be done without a loop, but I can't be bothered
		while (count > 0 && read != write)
		{
			--count;
			read = toIndex(read + 1);
		}
	}

	SizeType toIndex(SizeType i) const
	{
		return i % vec.getSize();
	}
	SizeType toIndexHandleNegative(int i) const
	{
		SizeType size = vec.getSize();
		int mod = i % (int)size;
		if (mod < 0)
			return SizeType(mod + size);
		else
			return SizeType(i);
	}

	T operator[](SizeType i) const
	{
		return vec[toIndex(read + i)];
	}

	T operator[](Iter it) const
	{
		return vec[toIndex(it.index)];
	}


	SizeType read = 0U;
	SizeType write = 0U;

	struct StupidVector
	{
		T stackBuffer[1];
		T *buffer;
		SizeType size;
		bool heapAllocated;

		StupidVector()
			: stackBuffer()
			, buffer(stackBuffer)
			, size(1)
			, heapAllocated(false)
		{
		}

		StupidVector(StupidVector &&o) = delete;
		StupidVector(const StupidVector &) = delete;

		void allocateBuffer(SizeType newSize)
		{
			releaseBuffer();
			size = newSize;
			buffer = (T*)lang::osAllocate(newSize * sizeof(T));
			heapAllocated = true;
		}

		void releaseBuffer()
		{
			if (heapAllocated)
			{
				heapAllocated = false;
				lang::osFree(buffer);

				buffer = stackBuffer;
				size = 1;
			}
		}

		void dismissHeapBuffer()
		{
			buffer = stackBuffer;
			size = 1;
			heapAllocated = false;
		}

		T &operator[](SizeType index) { return buffer[index]; }
		const T &operator[](SizeType index) const { return buffer[index]; }
		SizeType getSize() const { return size; }
		SizeType getCapacity() const { return size; }
		const T *getBegin() const { return buffer; }
		const T *getEnd() const { return buffer + size; }
		bool isEmpty() const { return size == 0; }

	};
	StupidVector vec;
};

FB_END_PACKAGE1();
