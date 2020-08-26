#ifndef FB_UTIL_EXPGOLOMB_H
#define FB_UTIL_EXPGOLOMB_H

FB_PACKAGE1(util)

// Exponential-Golomb encoding for signed 8-bit integers.

SizeType expGolombEncode(const int8_t *in, SizeType inN, uint8_t *out, SizeType outN);
SizeType expGolombDecode(const uint8_t *in, SizeType inNBits, int8_t *out, SizeType outN);

FB_END_PACKAGE1()

#endif
