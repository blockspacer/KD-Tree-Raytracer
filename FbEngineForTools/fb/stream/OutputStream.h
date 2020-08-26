#pragma once

#include "fb/container/Vector.h"
#include "fb/container/PodVector.h"
#include "fb/lang/IntTypes.h"
#include "fb/lang/ByteOrderSwap.h"
#include "fb/lang/UnalignedLoad.h"
#include "fb/math/curve/MultiCurveDecl.h"
#include "fb/math/curve/TCBCurveDecl.h"
#include "fb/math/Vec3.h"
#include "fb/math/Vec2.h"
#include "fb/math/Vec4.h"
#include "fb/math/Quaternion.h"
#include "fb/lang/MemoryFunctions.h"

FB_PACKAGE1(stream)

/**
 * Writes data to a memory stream
 */
template<class StreamByteOrder>
class OutputStream
{
public:
	typedef StreamByteOrder Endianness;

public:
	OutputStream();
	OutputStream(const OutputStream &other);
	virtual ~OutputStream();
	
	void operator=(const OutputStream &other);

	void write(bool t) { write(int8_t(t ? 1 : 0)); }
	void write(char t) { writeImpl(t); }
	void write(int8_t t) { writeImpl(t); }
	void write(int16_t t) { writeOrderedImpl(t); }
	void write(int32_t t) { writeOrderedImpl(t); }
	void write(const int64_t &t) { writeOrderedImpl(t); }
	void write(uint8_t t) { writeImpl(t); }
	void write(uint16_t t) { writeOrderedImpl(t); }
	void write(uint32_t t) { writeOrderedImpl(t); }
	void write(const uint64_t &t) { writeOrderedImpl(t); }
	void write(float t) { writeOrderedAlignedImpl(t); }
	void write(double t) { writeOrderedAlignedImpl(t); }
	void write(const void *data, SizeType size);
	void write(const math::QUAT &q) { write(q.x); write(q.y); write(q.z); write(q.w); }
	void write(const math::VC4 &v) { write(v.x); write(v.y); write(v.z); write(v.w); }
	void write(const math::VC3 &v) { write(v.x); write(v.y); write(v.z); }
	void write(const math::DVC3 &v) { write(v.x); write(v.y); write(v.z); }
	void write(const math::VC2 &v) { write(v.x); write(v.y); }
	void write(const math::TCBCurveKnot &v);
	void write(const math::TCBCurve &v);
	void write(const math::MultiCurveKnot &v);
	void write(const math::MultiCurve &v);
	void write(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7);
	void write(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6);
	void write(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5);
	void write(bool b0, bool b1, bool b2, bool b3, bool b4);
	void write(bool b0, bool b1, bool b2, bool b3);
	void write(bool b0, bool b1, bool b2);
	void write(bool b0, bool b1);

	void writeSync();

	// Utility for writing pod vectors, it is just a wrapper for the data pointer version
	// that also stores the size of the vector.
	template <typename T>
	void write(const PodVector<T> &vector)
	{
		write(vector.getSize());
		for (SizeType i = 0, num = vector.getSize(); i < num; ++i)
			write(vector[i]);
	}
	// writes zero terminated string
	template<class T>
	void writeRawString(const T &t)
	{
		write(t.getPointer(), t.getLength() + 1);
	}

	/**
	 * Get the current position in the stream
	 */
	SizeType getPosition() const { return writePointer; }

	/**
	 * Pushes data to front of stream
	 */
	void writeTo(SizeType offset, const void *data, SizeType size);

	/**
	 * Returns pointer to data or NULL if nothing written
	 */
	uint8_t *getData();
	const uint8_t *getData() const;

	/**
	 * Returns number of bytes written
	 */
	SizeType getSize() const;

	/**
	 * Removes data from the end of the stream
	 */
	void truncateTo(SizeType size);

	/**
	 * Reserves space in stream
	 */
	void reserve(SizeType size);

	/**
	 * Returns number of bytes reserved for stream
	 */
	SizeType getCapacity() const { return bufferCapacity; }

	/**
	 * Class for creating chunk headers
	 */
	class IChunkHeader
	{
	public:
		virtual ~IChunkHeader() {}
		virtual SizeType getHeaderSize() = 0;
		virtual void writeHeader(OutputStream *strm, SizeType chunkSize) = 0;
	};

	/**
	 * Starts a new chunk.
	 */
	void beginChunk(IChunkHeader &header);

	/**
	 * Ends last started chunk.
	 */
	void endChunk(IChunkHeader &header);
	
	void enableFixedAllocation(void *fixedAllocation, SizeType fixedAllocationSize);

	void trim();

	void setCurrentSize(SizeType size);

	void swap(OutputStream &other)
	{
		lang::swap(writePointer, other.writePointer);
		lang::swap(buffer, other.buffer);
		lang::swap(bufferSize, other.bufferSize);
		lang::swap(bufferCapacity, other.bufferCapacity);
		lang::swap(chunkStackSize, other.chunkStackSize);
		lang::swap(firstChunkStackEntry, other.firstChunkStackEntry);
		lang::swap(chunkStackMultipleEntries, other.chunkStackMultipleEntries);
		lang::swap(fixedAllocation, other.fixedAllocation);
	}

protected:
	template<class T> void writeImpl(const T &t);
	template<class T> void writeOrderedImpl(T t);
	template<class T> void writeOrderedAlignedImpl(T t);

	void setBufferCapacity(SizeType newCapacity);
	void prepareWritingData(SizeType bytes);

protected:
	SizeType writePointer;
	uint8_t *buffer;
	SizeType bufferSize;
	SizeType bufferCapacity;
	SizeType chunkStackSize;
	SizeType firstChunkStackEntry; // avoid using Vector unless more than 1 entry is needed
	PodVector<SizeType> chunkStackMultipleEntries;
	uint32_t syncCounter;
	bool fixedAllocation;
};

/**
 * RAII class for scoped chunk header writing.
 */
template <typename StreamT>
class ScopedChunkHeader
{
public:
	ScopedChunkHeader(StreamT &strm, typename StreamT::IChunkHeader &header)
		: strm(strm)
		, header(header)
	{
		strm.beginChunk(header);
	}

	~ScopedChunkHeader()
	{
		strm.endChunk(header);
	}

protected:
	StreamT &strm;
	typename StreamT::IChunkHeader &header;
};

FB_END_PACKAGE1()

#include "OutputStreamInline.h"
