#pragma once

#include "BitInputStream.h"

FB_PACKAGE0()

// QUAT compressed to 32 bits
class CompressedQuat
{
public:
	CompressedQuat()
	{
	}

	CompressedQuat(const math::QUAT &rot)
	{
		set(rot);
	}

	explicit CompressedQuat(uint32_t data)
		: data(data)
	{
	}

	void set(const math::QUAT &rot)
	{
		float components[4] = { rot.x, rot.y, rot.z, rot.w };

		// normalize
		float length = rot.getLength();
		components[0] /= length;
		components[1] /= length;
		components[2] /= length;
		components[3] /= length;

		// find index of largest component
		uint8_t i = 0;
		if (fabsf(components[1]) > fabsf(components[i]))
			i = 1;
		if (fabsf(components[2]) > fabsf(components[i]))
			i = 2;
		if (fabsf(components[3]) > fabsf(components[i]))
			i = 3;

		// flip so that it's positive
		if (components[i] < 0.0f)
		{
			components[0] = -components[0];
			components[1] = -components[1];
			components[2] = -components[2];
			components[3] = -components[3];
		}

		const float invMaxValue = 1.0f / 0.7071067811865475244f;
		const float scale = 511.0f;
		float f1 = components[(i + 1) & 3] * invMaxValue * scale;
		float f2 = components[(i + 2) & 3] * invMaxValue * scale;
		float f3 = components[(i + 3) & 3] * invMaxValue * scale;

		// clamp to -scale,scale because of precision issues
		f1 = lang::max(-scale, lang::min(scale, f1));
		f2 = lang::max(-scale, lang::min(scale, f2));
		f3 = lang::max(-scale, lang::min(scale, f3));

		// pack 3 smallest components to 10 bits each and pack largest index to last 2 bits
		const uint32_t mask = ((1 << 10) - 1);
		uint32_t c1 = (uint32_t)(int32_t)f1;
		uint32_t c2 = (uint32_t)(int32_t)f2;
		uint32_t c3 = (uint32_t)(int32_t)f3;
		c1 &= mask;
		c2 &= mask;
		c3 &= mask;
		data = (c1 << 0) | (c2 << 10) | (c3 << 20) | (i << 30);
	}

	math::QUAT get() const
	{
		const uint32_t mask = ((1 << 10) - 1);
		int32_t c1 = (int32_t)((data >> 0) & mask);
		int32_t c2 = (int32_t)((data >> 10) & mask);
		int32_t c3 = (int32_t)((data >> 20) & mask);
		uint8_t i = (data >> 30) & 3;

		// extend signs
		if (c1 & (1 << 9))
			c1 |= ~mask;
		if (c2 & (1 << 9))
			c2 |= ~mask;
		if (c3 & (1 << 9))
			c3 |= ~mask;

		// unpack
		const float maxValue = 0.7071067811865475244f;
		const float invScale = 1.0f / 511.0f;
		float components[4];
		components[(i+1)&3] = c1 * invScale * maxValue;
		components[(i+2)&3] = c2 * invScale * maxValue;
		components[(i+3)&3] = c3 * invScale * maxValue;
		components[i] = sqrtf( 1.0f - components[(i+1)&3]*components[(i+1)&3] - components[(i+2)&3]*components[(i+2)&3] - components[(i+3)&3]*components[(i+3)&3] );
		return math::QUAT(components);
	}
	
	template<class FixedPointQuatType, class FixedPointType>
	FixedPointQuatType getAsFixedPoint() const
	{
		const uint32_t mask = ((1 << 10) - 1);
		int32_t c1 = (int32_t)((data >> 0) & mask);
		int32_t c2 = (int32_t)((data >> 10) & mask);
		int32_t c3 = (int32_t)((data >> 20) & mask);
		uint8_t i = (data >> 30) & 3;

		// extend signs
		if (c1 & (1 << 9))
			c1 |= ~mask;
		if (c2 & (1 << 9))
			c2 |= ~mask;
		if (c3 & (1 << 9))
			c3 |= ~mask;

		// unpack
		const FixedPointType maxValue = FixedPointType::fromLargeIntDivision(927538921, 1311738121); // 0.70710678080537387996 (larger than sqrt2/2 by 0.00000000038117364424)
		const FixedPointType invScale = FixedPointType::fromLargeIntDivision(1, ((1 << 9) - 1));
		FixedPointType components[4];
		components[(i+1)&3] = FixedPointType::fromInt(c1) * invScale * maxValue;
		components[(i+2)&3] = FixedPointType::fromInt(c2) * invScale * maxValue;
		components[(i+3)&3] = FixedPointType::fromInt(c3) * invScale * maxValue;
		components[i] = FixedPointType::squareRoot( FixedPointType::fromInt(1) - components[(i+1)&3]*components[(i+1)&3] - components[(i+2)&3]*components[(i+2)&3] - components[(i+3)&3]*components[(i+3)&3] );
		return FixedPointQuatType(components[0], components[1], components[2], components[3]);
	}
	
	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, data);
		return true;
	}

	bool operator==(const CompressedQuat &other) const { return data == other.data; }
	bool operator!=(const CompressedQuat &other) const { return data != other.data; }

	uint32_t getData() const { return data; }

private:
	uint32_t data = 0;
};

FB_END_PACKAGE0()
