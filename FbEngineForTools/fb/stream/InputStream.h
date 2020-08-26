#pragma once

#include "fb/lang/IntTypes.h"
#include "fb/lang/ByteOrderSwap.h"
#include "fb/lang/UnalignedLoad.h"
#include "fb/math/curve/MultiCurveDecl.h"
#include "fb/math/curve/TCBCurveDecl.h"
#include "fb/math/Quaternion.h"
#include "fb/math/Vec4.h"
#include "fb/math/Vec3.h"
#include "fb/math/Vec2.h"

FB_PACKAGE1(stream)

void setStreamReadFailed(bool &streamReadFailed, bool value);

/**
 * Parses data from a memory stream
 */
template<class StreamByteOrder>
class InputStream
{
public:
	typedef StreamByteOrder Endianness;

public:
	InputStream();
	InputStream(const void *data, SizeType dataSize);
	virtual ~InputStream() {}

	void read(bool &t)
	{
		int8_t c = 0;
		read(c);
		t = (c != 0);
	}

	void read(char &t) { readImpl(t); }
	void read(int8_t &t) { readImpl(t); }
	void read(int16_t &t) { readOrderedImpl(t); }
	void read(int32_t &t) { readOrderedImpl(t); }
	void read(int64_t &t) { readOrderedImpl(t); }
	void read(uint8_t &t) { readImpl(t); }
	void read(uint16_t &t) { readOrderedImpl(t); }
	void read(uint32_t &t) { readOrderedImpl(t); }
	void read(uint64_t &t) { readOrderedImpl(t); }
	void read(float &t) { readOrderedAlignedImpl(t); }
	void read(double &t) { readOrderedAlignedImpl(t); }
	void read(void *data, SizeType size);
	void read(math::QUAT &q) { read(q.x); read(q.y); read(q.z); read(q.w); }
	void read(math::DVC3 &v) { read(v.x); read(v.y); read(v.z); }
	void read(math::VC4 &v) { read(v.x); read(v.y); read(v.z); read(v.w); }
	void read(math::VC3 &v) { read(v.x); read(v.y); read(v.z); }
	void read(math::VC2 &v) { read(v.x); read(v.y); }

	void read(math::MultiCurveKnot &v);
	void read(math::MultiCurve &v);
	void read(math::TCBCurveKnot &v);
	void read(math::TCBCurve &v);

	void read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4, bool &b5, bool &b6, bool &b7);
	void read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4, bool &b5, bool &b6);
	void read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4, bool &b5);
	void read(bool &b0, bool &b1, bool &b2, bool &b3, bool &b4);
	void read(bool &b0, bool &b1, bool &b2, bool &b3);
	void read(bool &b0, bool &b1, bool &b2);
	void read(bool &b0, bool &b1);

	bool readSync();

	// Reads PodVectors by appending to current content of the vector
	// For performance intensive uses you should use datapointer version.
	// Though that one doesn't do byte order conversions so they are not interchancable
	template <typename T>
	void read(PodVector<T> &vector)
	{
		SizeType s;
		vector.clear();
		read(s);
		for (SizeType i = 0; i < s; ++i)
		{
			T t;
			read(t);
			vector.pushBack(t);
			if (getBytesLeft() == 0)
				break;
		}
	}

	/**
	 * Skips over given number of bytes
	 */
	void skipData(SizeType size);

	/**
	 * Skips over identifier if it matches, versions other than char* assume stream needs endianess swap when needed.
	 */
	bool checkId(const char *id, SizeType size);
	bool checkId(const void *id, SizeType size);

	/**
	 * Returns pointer to zero terminated string in stream and the length of it
	 */
	SizeType readString(const char **str);
	/**
	 * Returns wrapper to zero terminated string in stream or an empty string, if fail
	 */
	StringRef readString();

	// reads zero terminated string
	template<class T>
	void readRawString(T &t)
	{
		const char *str;
		SizeType len = readString(&str);
		t = T(str, len);
	}

	/**
	 * Returns pointer to beginning of stream data
	 */
	const uint8_t *getData() const { return data; }

	/**
	 * Returns number of bytes that can still be read
	 */
	SizeType getBytesLeft() const;

	/**
	 * Returns total data size
	 */
	SizeType getSize() const { return dataSize; }

	/**
	 * Returns number of bytes read so far
	 */
	SizeType getBytesRead() const { return readPointer; }
	SizeType getPosition() const { return readPointer; }
	void setPosition(SizeType pos);

	void truncateToSize(SizeType size);

	bool hasStreamReadFailed() const { return streamReadFailed; }
	void setStreamReadFailed(bool val) { stream::setStreamReadFailed(streamReadFailed, val); }

	static SizeType getMaxStreamSize() { return 1U << 31; }

private:
	template<class T> void readImpl(T &t);
	template<class T> void readOrderedImpl(T &t);
	template<class T> void readOrderedAlignedImpl(T &t);

protected:
	const uint8_t *data;
	SizeType dataSize;
	SizeType readPointer;
	uint32_t syncCounter;

private:
	bool streamReadFailed;
};

FB_END_PACKAGE1()

#include "InputStreamInline.h"
