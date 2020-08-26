#pragma once

#include "BitInputStream.h"
#include "fb/lang/platform/FBMath.h"

FB_PACKAGE0()

// Unit length direction vector compressed to N bits per axis
template<int NumBits>
class CompressedNormal
{
public:
	CompressedNormal()
	{
	}

	CompressedNormal(const math::VC3 &normal)
	{
		set(normal);
	}

	void set(const math::VC3 &normalParam)
	{
		math::VC3 normal(normalize(normalParam));
		float scale = (float)((1 << NumBits) - 1);
		float largest = FB_FMAX(FB_FMAX(FB_FABS(normal.x), FB_FABS(normal.y)), FB_FABS(normal.z));
		if (largest == 0.0f)
			largest = 1.0f;
		uint32_t normX = (uint32_t)(FB_FCLAMP((normal.x*0.5f / largest + 0.5f) * scale, 0.0f, scale));
		uint32_t normY = (uint32_t)(FB_FCLAMP((normal.y*0.5f / largest + 0.5f) * scale, 0.0f, scale));
		uint32_t normZ = (uint32_t)(FB_FCLAMP((normal.z*0.5f / largest + 0.5f) * scale, 0.0f, scale));
		value = normX | (normY << NumBits) | (normZ << (NumBits + NumBits));
	}

	math::VC3 get() const
	{
		int mask = ((1 << NumBits) - 1);
		float scale = (float)((1 << NumBits) - 1);
		float halfScale = scale * 0.5f;

		float normX = (float)(value & mask) - halfScale;
		float normY = (float)((value >> NumBits) & mask) - halfScale;
		float normZ = (float)((value >> (NumBits + NumBits)) & mask) - halfScale;
		float length = FB_FSQRT(normX * normX + normY * normY + normZ * normZ);
		if (length == 0.0f)
			return math::VC3();
		return math::VC3(normX, normY, normZ) / length;
	}

	template<class T>
	bool stream(T &strm)
	{
		fb_stream(strm, &value, sizeof(value), SizeInBits(NumBits + NumBits + NumBits));
		return true;
	}

	bool operator==(const CompressedNormal &other) const { return value == other.value; }
	bool operator!=(const CompressedNormal &other) const { return value != other.value; }

private:
	uint64_t value = 0;
};

FB_END_PACKAGE0()