#pragma once

#include "fb/lang/IntTypes.h"

FB_PACKAGE1(lang)

static inline int16_t getFastS16(float value)
{
	return int16_t(value);
}

static inline uint16_t getFastU16(float value)
{
	return uint16_t(value);
}

static inline int8_t getFastS8(float value)
{
	return int8_t(value);
}

static inline uint8_t getFastU8(float value)
{
	return uint8_t(value);
}

// Int to float

static inline float getFastS16Float(int16_t value)
{
	return float(value);
}

static inline float getFastU16Float(uint16_t value)
{
	return float(value);
}

static inline float getFastS8Float(int8_t value)
{
	return float(value);
}

static inline float getFastU8Float(uint8_t value)
{
	return float(value);
}


static inline uint32_t getSortableIntegerFromFloatPositive(float value)
{
	union IntFloat
	{
		float floatValue;
		uint32_t intValue;
	};

	IntFloat u;
	u.floatValue = value;
	return u.intValue;
}

static inline uint32_t getSortableIntegerFromFloat(float value)
{
	uint32_t result = getSortableIntegerFromFloatPositive(value);
	uint32_t isNegative = result >> 31;
	uint32_t mask = 0x80000000;
	if (isNegative)
		mask |= 0xffffffff;
		
	return result ^ mask;
}


FB_END_PACKAGE1()
