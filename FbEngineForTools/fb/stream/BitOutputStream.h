#pragma once

#include "fb/stream/BitInputStream.h"
#include "fb/stream/BitTypes.h"
#include "fb/stream/OutputStream2.h"

FB_PACKAGE0()

template<class StreamByteOrderT = lang::LittleEndian>
class BitOutputStream
{
public:
	typedef StreamByteOrderT StreamByteOrder;

	void writeImpl(const bool i) { writeImpl(&i, sizeof(i), OffsetInBits(0), SizeInBits(1)); }

	void writeImpl(const uint8_t i) { writeImpl(&i, sizeof(i)); }
	void writeImpl(const uint16_t i) { swapWriteImpl(i); }
	void writeImpl(const uint32_t i) { swapWriteImpl(i); }
	void writeImpl(const uint64_t i) { swapWriteImpl(i); }
	
	void writeImpl(const int8_t i) { writeImpl(&i, sizeof(i)); }
	void writeImpl(const int16_t i) { swapWriteImpl(i); }
	void writeImpl(const int32_t i) { swapWriteImpl(i); }
	void writeImpl(const int64_t i) { swapWriteImpl(i); }
	
	void writeImpl(const float i) { swapWriteImpl(i); }
	void writeImpl(const double i) { swapWriteImpl(i); }

	void writeImpl(const Time i) { swapWriteImpl(i.getTicks()); }

	template<class T>
	void swapWriteImpl(T t)
	{
		StreamByteOrder::convertFromNative(t);
		writeImpl(&t, sizeof(t));
	}
	
	template<class T>
	void writeImpl(T &t)
	{
		streamValue(*this, t);
	}
	
	void writeImpl(const void *ptr, SizeType size)
	{
		writeImpl(ptr, size, OffsetInBits(0), SizeInBits(size * 8));
	}
	
	template<class T>
	void writeImpl(T &t, SizeInBits numBits)
	{
		writeImpl(&t, SizeType(sizeof(t)), OffsetInBits(0), numBits);
	}

	void writeImpl(const void *ptr, SizeType sizeInBytes, SizeInBits numBitsParam)
	{
		writeImpl(ptr, sizeInBytes, OffsetInBits(0), numBitsParam);
	}

	void writeImpl(const void *ptr, OffsetInBits offsetInBitsParam, SizeInBits numBitsParam)
	{
		SizeType sizeInBytes = (offsetInBitsParam.get() + numBitsParam.get() + 7) / 8;
		writeImpl(ptr, sizeInBytes, offsetInBitsParam, numBitsParam);
	}

	// write numBits from ptr starting from offsetInBits
	void writeImpl(const void *ptr, SizeType sizeInBytes, OffsetInBits offsetInBitsParam, SizeInBits numBitsParam)
	{
		SizeType offsetInBits = offsetInBitsParam.get();
		SizeType numBits = numBitsParam.get();
		fb_assert(offsetInBits + numBits <= sizeInBytes * 8);
		fb_assert((positionInBits + 7) / 8 == data.getSize());

		// positionInBits as a 32 bit value cannot exceed this
		fb_assert(data.getSize() + (numBits + 7) / 8 <= uint32_t(0xFFFFFFFFU) / 8);

		if ((offsetInBits & 7) + (positionInBits & 7) + (numBits & 7) == 0)
		{
			// byte write
			const uint8_t *src = (const uint8_t *)ptr + (offsetInBits / 8);
			SizeType oldSize = data.getSize();
			data.reserveRounded(data.getSize() + (numBits / 8));
			data.uninitialisedResize(data.getSize() + (numBits / 8));
			lang::MemCopy::copy(data.getPointer() + oldSize, src, (numBits / 8));
			positionInBits = data.getSize() * 8;
			return;
		}

		// bit write

		SizeType oldSize = data.getSize();
		data.reserveRounded((positionInBits + numBits + 7) / 8);
		data.uninitialisedResize((positionInBits + numBits + 7) / 8);
		lang::MemSet::set(data.getPointer() + oldSize, 0, data.getSize() - oldSize);

		/*
		// slow bit-by-bit version
		const uint8_t *src = (const uint8_t *)ptr;
		uint32_t numBitsWritten = 0;
		while (numBitsWritten < numBits)
		{
			// clear old bit
			const uint32_t outputShift = positionInBits & 7;
			data[positionInBits / 8] &= ~(1 << outputShift);

			// read new bit
			const uint32_t inputShift = (numBitsWritten + offsetInBits) & 7;
			const uint32_t inputBit = (src[(numBitsWritten + offsetInBits) / 8] >> inputShift) & 1U;
			data[positionInBits / 8] |= (inputBit << outputShift);

			positionInBits++;
			numBitsWritten++;
		}
		*/

		// clear bits at the end of allocation
		if ((positionInBits & 7))
		{
			const uint32_t mask = (1U << (positionInBits & 7)) - 1U;
			data[positionInBits / 8] &= mask;
		}

		// write 64-bit chunks
		uint32_t numBitsWritten = offsetInBits;
		uint32_t numBitsToWrite = numBits + offsetInBits;
		{
			uint64_t *dst = (uint64_t *)data.getPointer();
			const uint64_t *src = (const uint64_t *)ptr;
			while (numBitsWritten + 64 <= numBitsToWrite)
			{
				const uint32_t remainingInputBits = 64 - (numBitsWritten & 63);
				const uint32_t remainingOutputBits = 64 - (positionInBits & 63);
				const uint32_t bitsToWrite = lang::min(lang::min(numBitsToWrite - numBitsWritten, remainingInputBits), remainingOutputBits);

				const uint64_t mask = bitsToWrite == 64 ? ~0ULL : (1LLU << bitsToWrite) - 1U;
				const uint32_t inputShift = numBitsWritten & 63;
				const uint32_t outputShift = positionInBits & 63;

				dst[positionInBits / 64] |= ((src[numBitsWritten / 64] >> inputShift) & mask) << outputShift;
				positionInBits += bitsToWrite;
				numBitsWritten += bitsToWrite;
			}
		}

		// write 8-bit chunks
		{
			uint8_t *dst = data.getPointer();
			const uint8_t *src = (const uint8_t *)ptr;
			while (numBitsWritten < numBitsToWrite)
			{
				const uint32_t remainingInputBits = 8 - (numBitsWritten & 7);
				const uint32_t remainingOutputBits = 8 - (positionInBits & 7);
				const uint32_t bitsToWrite = lang::min(lang::min(numBitsToWrite - numBitsWritten, remainingInputBits), remainingOutputBits);

				const uint32_t mask = (1U << bitsToWrite) - 1U;
				const uint32_t inputShift = numBitsWritten & 7;
				const uint32_t outputShift = positionInBits & 7;

				dst[positionInBits / 8] |= ((src[numBitsWritten / 8] >> inputShift) & mask) << outputShift;
				positionInBits += bitsToWrite;
				numBitsWritten += bitsToWrite;
			}
		}
	}
	
	const uint8_t *getData() const { return data.getPointer(); }
	uint8_t *getData() { return data.getPointer(); }
	SizeType getSizeInBytes() const { return data.getSize(); }
	SizeInBits getSizeInBits() const { return SizeInBits(positionInBits); }

	void resizeBytes(SizeType newSize)
	{
		SizeType oldSize = data.getSize();
		data.uninitialisedResize(newSize);
		if (oldSize < newSize)
			memset(data.getPointer() + oldSize, 0, data.getSize() - oldSize);
		positionInBits = data.getSize() * 8;
	}

	void reserveBytes(SizeType newSize) { data.reserve(newSize); }

	void clear()
	{
		data.clear();
		positionInBits = 0;
	}

	void trimMemory() { data.trimMemory(); }

	bool isInputStream() const { return false; }
	bool isOutputStream() const { return true; }
	bool isReading() const { return false; }
	bool isWriting() const { return true; }

	template<class T>
	bool streamImpl(T &t) { writeImpl(t); return true; }
	bool streamImpl(const void *ptr, SizeType size) { writeImpl(ptr, size); return true; }
	void streamTextImpl(const char *) {}
	
	template<class T>
	bool streamImpl(T &t, SizeInBits numBits) { writeImpl(t, numBits); return true; }
	bool streamImpl(void *ptr, SizeType sizeInBytes, SizeInBits numBits) { writeImpl(ptr, sizeInBytes, numBits); return true; }

	typedef OutputStreamType StreamType;
	typedef StreamPersistentTrue StreamPersistent;
	typedef StreamClassBit StreamClass;

	void swap(BitOutputStream<StreamByteOrderT> &other)
	{
		data.swap(other.data);
		lang::swap(positionInBits, other.positionInBits);
	}

	void swapDataVector(PodVector<uint8_t> &other)
	{
		data.swap(other);
		positionInBits = data.getSize() * 8;
	}

	void swapDataVector(PodVector<uint8_t> &other, SizeInBits &sizeInBits)
	{
		data.swap(other);
		uint32_t temp = positionInBits;
		positionInBits = sizeInBits.get();
		sizeInBits = SizeInBits(temp);
	}

	void popBitsFront(SizeInBits numBits)
	{
		fb_assert(positionInBits >= numBits);

		if (positionInBits == numBits)
		{
			data.clear();
			positionInBits = 0;
			return;
		}

		// erase whole bytes
		SizeType numFullBytes = numBits.get() / 8;
		data.eraseIndex(0, numFullBytes);

		// shift all remaining bits
		SizeType shiftAmount = numBits.get() & 7;
		if (shiftAmount)
		{
			for (SizeType i = 0; i + 1 < data.getSize(); i++)
			{
				data[i] >>= shiftAmount;
				data[i] |= data[i + 1] << (8 - shiftAmount);
			}
			data[data.getSize() - 1] >>= shiftAmount;
		}
		
		positionInBits = positionInBits - numBits.get();
	}
	
private:
	PodVector<uint8_t> data;
	SizeType positionInBits = 0;
};

FB_END_PACKAGE0()
