#pragma once

FB_PACKAGE0()

// Round up to closest pow2
static inline uint32_t roundToPow2(uint32_t value)
{
	uint32_t pow2 = value - 1;
	pow2 |= pow2 >> 1;
	pow2 |= pow2 >> 2;
	pow2 |= pow2 >> 4;
	pow2 |= pow2 >> 8;
	pow2 |= pow2 >> 16;
	++pow2;

	return pow2;
}

FB_END_PACKAGE0()
