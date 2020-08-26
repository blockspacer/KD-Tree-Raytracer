#pragma once

#include "BitInputStream.h"

FB_PACKAGE0()

// Float compressed to N bits for range -MaxValue,MaxValue. One bit is reserved for sign, so the resulting precision is MaxValue/(1<<(NumBits-1))
template<int NumBits, int MaxValue>
class CompressedFloat
{
public:
	CompressedFloat()
	{
	}

	CompressedFloat(float f)
	{
		set(f);
	}

	void set(float f)
	{
		const float invRadius = 1.0f / (float)MaxValue;
		const float scale = (float)((1 << (NumBits-1)) - 1);
		const float normF = lang::max(lang::min(f * invRadius, 1.0f), -1.0f);
		int32_t value = (int32_t)(normF * scale);
		value &= (1 << NumBits) - 1;
		lang::MemCopy::copy(bytes, &value, sizeof(bytes));
	}

	float get() const
	{
		const float radius = (float)MaxValue;
		const float scale = (float)((1 << (NumBits-1)) - 1);
		const float invScale = 1.0f / scale;
		int32_t value = 0;
		lang::MemCopy::copy(&value, bytes, sizeof(bytes));
		// extend sign
		if (value & (1<<(NumBits-1)))
			value |= ~((1<<NumBits)-1);
		return value * invScale * radius;
	}
	
	template<class FixedPointType>
	FixedPointType getAsFixedPoint() const
	{
		int32_t value = 0;
		lang::MemCopy::copy(&value, bytes, sizeof(bytes));
		// extend sign
		if (value & (1 << (NumBits - 1)))
			value |= ~((1 << NumBits) - 1);
		return FixedPointType::fromLargeIntDivision(value * MaxValue, (1 << (NumBits - 1)) - 1);
	}

	template<class T>
	bool stream(T &strm)
	{
		return streamImp(strm, typename T::StreamClass());
	}

	template<class T>
	bool streamImp(T &strm, StreamClassBit)
	{
		fb_stream(strm, bytes, sizeof(bytes), SizeInBits(NumBits));
		return true;
	}

	template<class T>
	bool streamImp(T &strm, StreamClassByte)
	{
		fb_stream(strm, bytes, sizeof(bytes));
		return true;
	}

	bool operator==(const CompressedFloat &other) const { return memcmp(bytes, other.bytes, sizeof(bytes)) == 0; }
	bool operator!=(const CompressedFloat &other) const { return memcmp(bytes, other.bytes, sizeof(bytes)) != 0; }

private:
	uint8_t bytes[(NumBits + 7) / 8] = { 0 };
};

// Unsigned float compressed to N bits for range 0,MaxValue. The resulting precision is MaxValue/(1<<NumBits)
template<int NumBits, int MaxValue>
class CompressedUFloat
{
public:
	CompressedUFloat()
	{
	}

	CompressedUFloat(float f)
	{
		set(f);
	}

	void set(float f)
	{
		const float invRadius = 1.0f / (float)MaxValue;
		const float scale = (float)((1 << NumBits) - 1);
		const float normF = lang::max(lang::min(f * invRadius, 1.0f), 0.0f);
		uint32_t value = (uint32_t)(normF * scale);
		value &= (1 << NumBits) - 1;
		lang::MemCopy::copy(bytes, &value, sizeof(bytes));
	}

	float get() const
	{
		const float radius = (float)MaxValue;
		const float scale = (float)((1 << NumBits) - 1);
		const float invScale = 1.0f / scale;
		uint32_t value = 0;
		lang::MemCopy::copy(&value, bytes, sizeof(bytes));
		return value * invScale * radius;
	}
	
	template<class FixedPointType>
	FixedPointType getAsFixedPoint() const
	{
		uint32_t value = 0;
		lang::MemCopy::copy(&value, bytes, sizeof(bytes));
		return FixedPointType::fromLargeIntDivision((int)(value * MaxValue), (1 << NumBits) - 1);
	}

	template<class T>
	bool stream(T &strm)
	{
		return streamImp(strm, typename T::StreamClass());
	}

	template<class T>
	bool streamImp(T &strm, StreamClassBit)
	{
		fb_stream(strm, bytes, sizeof(bytes), SizeInBits(NumBits));
		return true;
	}

	template<class T>
	bool streamImp(T &strm, StreamClassByte)
	{
		fb_stream(strm, bytes, sizeof(bytes));
		return true;
	}

	bool operator==(const CompressedUFloat &other) const { return memcmp(bytes, other.bytes, sizeof(bytes)) == 0; }
	bool operator!=(const CompressedUFloat &other) const { return memcmp(bytes, other.bytes, sizeof(bytes)) != 0; }

private:
	uint8_t bytes[(NumBits + 7) / 8] = { 0 };
};

// Compressed VC3
template<class FloatType>
class CompressedVC3
{
public:
	CompressedVC3()
	{
	}

	CompressedVC3(const math::VC3 &v)
	{
		set(v);
	}

	void set(const math::VC3 &v)
	{
		x.set(v.x);
		y.set(v.y);
		z.set(v.z);
	}

	math::VC3 get() const
	{
		return math::VC3(x.get(), y.get(), z.get());
	}
	
	template<class FixedPointVectorType, class FixedPointType>
	FixedPointVectorType getAsFixedPoint() const
	{
		return FixedPointVectorType(x.template getAsFixedPoint<FixedPointType>(), y.template getAsFixedPoint<FixedPointType>(), z.template getAsFixedPoint<FixedPointType>());
	}
	
	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, x);
		fb_stream(strm, y);
		fb_stream(strm, z);
		return true;
	}
	
	bool operator==(const CompressedVC3 &other) const { return x == other.x && y == other.y && z == other.z; }
	bool operator!=(const CompressedVC3 &other) const { return x != other.x || y != other.y || z != other.z; }

private:
	FloatType x,y,z;
};

FB_END_PACKAGE0()