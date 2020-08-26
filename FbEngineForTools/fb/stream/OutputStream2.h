#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/ByteOrderSwap.h"
#include "fb/stream/CommonStream2.h"
#include "fb/stream/OutputStream2Macro.h"
#include "fb/stream/OutputStream2Decl.h"

FB_PACKAGE0()

// OutputStream that is used through macros, eg. fb_stream_write(strm, value)
// Macros return false on stream read failure and trigger an assert. This makes code safer and debugging easier.
template<class StreamByteOrderT>
class OutputStream2
{
public:
	typedef StreamByteOrderT StreamByteOrder;

	void writeImpl(const bool i) { writeImpl(&i, sizeof(i)); }

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
		// If you get an error here, make sure you have included "fb/stream/StreamValue.h" in your cpp
		streamValue(*this, t);
	}
	
	void writeImpl(const void *ptr, SizeType size)
	{
		data.insert(data.getEnd(), (const uint8_t*)ptr, (const uint8_t*)ptr + size);
	}
	
	void patchValue(SizeType offset, const bool i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const uint8_t i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const uint16_t i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const uint32_t i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const uint64_t i) { patchValue(offset, &i, sizeof(i)); }
	
	void patchValue(SizeType offset, const int8_t i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const int16_t i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const int32_t i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const int64_t i) { patchValue(offset, &i, sizeof(i)); }
	
	void patchValue(SizeType offset, const float i) { patchValue(offset, &i, sizeof(i)); }
	void patchValue(SizeType offset, const double i) { patchValue(offset, &i, sizeof(i)); }
	
	void patchValue(SizeType offset, const void *ptr, SizeType size)
	{
		fb_assert(offset + size <= data.getSize());
		lang::MemCopy::copy(data.getPointer() + offset, ptr, size);
	}

	const uint8_t *getData() const { return data.getPointer(); }
	uint8_t *getData() { return data.getPointer(); }
	SizeType getSize() const { return data.getSize(); }
	SizeType getPosition() const { return data.getSize(); }
	void resize(SizeType newSize) { data.resize(newSize); }
	void reserve(SizeType newSize) { data.reserve(newSize); }

	void clear() { data.clear(); }
	void trimMemory() { data.trimMemory(); }

	bool isInputStream() const { return false; }
	bool isOutputStream() const { return true; }
	bool isReading() const { return false; }
	bool isWriting() const { return true; }

	template<class T>
	bool streamImpl(T &t) { writeImpl(t); return true; }
	bool streamImpl(const void *ptr, SizeType size) { writeImpl(ptr, size); return true; }
	void streamTextImpl(const char *) {}

	typedef OutputStreamType StreamType;
	typedef StreamPersistentTrue StreamPersistent;
	typedef StreamClassByte StreamClass;

	void swap(OutputStream2<StreamByteOrderT> &other)
	{
		data.swap(other.data);
	}

	PodVector<uint8_t> &getDataVector() { return data; }
	const PodVector<uint8_t> &getDataVector() const { return data; }

	void swapDataVector(PodVector<uint8_t> &other)
	{
		// swap CachePodVector in place without converting allocations
		uint8_t temp[sizeof(container::ImpByteElementArray)];
		memcpy(&temp, &data, sizeof(container::ImpByteElementArray));
		memcpy(&data, &other, sizeof(container::ImpByteElementArray));
		memcpy(&other, &temp, sizeof(container::ImpByteElementArray));
	}
	
private:
	PodVector<uint8_t> data;
};

FB_END_PACKAGE0()
