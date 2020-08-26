#pragma once

FB_PACKAGE0()

// Int32 compressed to N bits
template<int NumBits>
class CompressedInt
{
public:
	CompressedInt()
	{
	}

	CompressedInt(int32_t valueParam)
	{
		set(valueParam);
	}

	static const int numBits = NumBits;
	static int32_t getMaxValue() { return (1 << (NumBits - 1)) - 1; }
	static int32_t getMinValue() { return -getMaxValue(); }
	static uint32_t getNumBits() { return NumBits; }

	void set(int32_t valueParam)
	{
		const int32_t maxValue = (1 << (NumBits-1)) - 1;
		const int32_t minValue = -maxValue;
		value = lang::max(minValue, lang::min(maxValue, valueParam));
	}

	int32_t get() const
	{
		return value;
	}

	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, &value, sizeof(value), SizeInBits(NumBits));
		if (strm.isReading())
		{
			// extend sign
			if (value & (1 << (NumBits - 1)))
				value |= ~((1 << NumBits) - 1);
		}
		return true;
	}

	bool operator==(const CompressedInt &other) const { return value == other.value; }
	bool operator!=(const CompressedInt &other) const { return value != other.value; }

private:
	int32_t value = 0;
};

// UInt32 compressed to N bits
template<int NumBits>
class CompressedUInt
{
public:
	CompressedUInt()
	{
	}

	CompressedUInt(uint32_t valueParam)
	{
		set(valueParam);
	}

	static uint32_t getMaxValue() { return (1U << NumBits) - 1; }
	static uint32_t getMinValue() { return 0; }
	static uint32_t getNumBits() { return NumBits; }

	void set(uint32_t valueParam)
	{
		const uint32_t maxValue = (1U << NumBits) - 1;
		value = lang::min(maxValue, valueParam);
	}

	uint32_t get() const
	{
		return value;
	}

	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, &value, sizeof(value), SizeInBits(NumBits));
		return true;
	}

	bool operator==(const CompressedUInt &other) const { return value == other.value; }
	bool operator!=(const CompressedUInt &other) const { return value == other.value; }

private:
	uint32_t value = 0;
};

// Compressed VC3I
template<class IntType>
class CompressedVC3I
{
public:
	CompressedVC3I()
	{
	}

	CompressedVC3I(const math::VC3I &v)
	{
		set(v);
	}

	void set(const math::VC3I &v)
	{
		x.set(v.x);
		y.set(v.y);
		z.set(v.z);
	}

	math::VC3I get() const
	{
		return math::VC3I(x.get(), y.get(), z.get());
	}
	
	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, x);
		fb_stream(strm, y);
		fb_stream(strm, z);
		return true;
	}
	
	bool operator==(const CompressedVC3I &other) const { return x == other.x && y == other.y && z == other.z; }
	bool operator!=(const CompressedVC3I &other) const { return x != other.x || y != other.y || z != other.z; }

private:
	IntType x,y,z;
};

FB_END_PACKAGE0()
