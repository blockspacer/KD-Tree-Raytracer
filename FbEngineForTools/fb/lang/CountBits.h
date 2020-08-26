#pragma once

#include "fb/lang/platform/IncludeIntrin.h" // For SIMD operations


#define FB_POPCNT_ENABLED FB_FALSE

#if FB_POPCNT_ENABLED == FB_TRUE

FB_PACKAGE1(lang)

bool lang::checkPopcntSupport();

FB_END_PACKAGE1()

#elif !defined(FB_PLATFORM)

#error "FB_PLATFORM not defined"

#endif

FB_PACKAGE0()

template<class T>
static inline T countBits8(T value)
{
#if FB_POPCNT_ENABLED == FB_TRUE
	return (T)__popcnt16((uint16_t)value);
#else
	uint32_t v = (uint32_t)value;
	v = v - ((v >> 1) & 0x55);
	v = (v & 0x33) + ((v >> 2) & 0x33);
	v = (v + (v >> 4)) & 0x0F;
	return (T)(v & 0x7F);
#endif
}

template<class T>
static inline T countBits32(T value)
{
#if FB_POPCNT_ENABLED == FB_TRUE
	return (T)__popcnt((uint32_t)value);
#else
	uint32_t v = (uint32_t)value;
	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	v = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
	return (T)v;
#endif
}

// If value is zero, result is undefined
template<class T>
static inline T lowestBitSet32(T value)
{
#if FB_COMPILER == FB_MSC
	unsigned long ret = 0;
	_BitScanForward(&ret, (unsigned long)value);
	return (T)ret;
#elif FB_COMPILER == FB_GNUC || FB_COMPILER == FB_CLANG
	return (T)__builtin_ctz(value);
#endif
}

// If value is zero, result is undefined
template<class T>
static inline T highestBitSet32(T value)
{
#if FB_COMPILER == FB_MSC
	unsigned long ret = 0;
    _BitScanReverse(&ret, (unsigned long)value);
    return (T)ret;
#elif FB_COMPILER == FB_GNUC || FB_COMPILER == FB_CLANG
    return (T)(31 - __builtin_clz(value));
#else
	// is this actually correct? should be 31-clz, right?
	/*
    static const uint32_t deBruijinClzTable[32] = {
		0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30, 8,
		12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
	};

    uint32_t v = (uint32_t)value;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return (T)deBruijinClzTable[(uint32_t)(v * 0x07C4ACDDU) >> 27];
	*/
#endif
}

// Most significant bit (MSB)
static inline uint32_t highestBitSet64(uint64_t value)
{
	uint32_t result = 0;
	if (value > 0xFFFFFFFF)
	{
		result = 32;
		value >>= 32;
	}
	return result + highestBitSet32((uint32_t)value);
}

FB_END_PACKAGE0()
