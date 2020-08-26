#pragma once

#include "fb/lang/ByteOrderSwap.h"
#include "fb/lang/IntTypes.h"

FB_PACKAGE0()

static inline uint32_t getNumberHashValue(uint32_t number)
{
	// Murmurhash3's integer finaliser
	// ToDo: Would need to actually check how well this performs
	number ^= number >> 16;
	number *= 0x85ebca6b;
	number ^= number >> 13;
	number *= 0xc2b2ae35;
	number ^= number >> 16;
	return number;
}

static inline uint32_t getConsistentNumberHashValue(uint32_t number)
{
	#if FB_BYTE_ORDER == FB_BIG_ENDIAN
		lang::swapByteOrder4(&number);
	#endif

	return getNumberHashValue(number);
}

static inline uint32_t getNumberHashValue(uint64_t number)
{
	// Murmurhash3's integer finaliser
	// ToDo: Would need to actually check how well this performs
	number ^= number >> 33;
	number *= uint64_t(0xff51afd7ed558ccd);
	number ^= number >> 33;
	number *= uint64_t(0xc4ceb9fe1a85ec53);
	number ^= number >> 33;
	return (uint32_t) number;
}

static inline uint32_t getConsistentNumberHashValue(uint64_t number)
{
	#if FB_BYTE_ORDER == FB_BIG_ENDIAN
		lang::swapByteOrder8(&number);
	#endif

	return getNumberHashValue(number);
}

/* Signed versions */
static inline uint32_t getNumberHashValue(int32_t number)
{
	return getNumberHashValue(uint32_t(number));
}

static inline uint32_t getNumberHashValue(int64_t number)
{
	return getNumberHashValue(uint64_t(number));
}

static inline uint32_t getConsistentNumberHashValue(int32_t number)
{
	return getConsistentNumberHashValue(uint32_t(number));
}

static inline uint32_t getConsistentNumberHashValue(int64_t number)
{
	return getConsistentNumberHashValue(uint64_t(number));
}

// General hash for a buffer. Result is not guaranteed to be endian independent.
uint32_t getHashValue(const void *ptr, uint32_t size, uint32_t seed = 0);
// General hash for a buffer. Result is endian independent.
uint32_t getConsistentHashValue(const void *ptr, uint32_t size, uint32_t seed = 0);
// General hash for a buffer. Result is not guaranteed to be endian independent.
uint64_t getHashValue64(const void *ptr, uint32_t size, uint64_t seed = 0);
// General hash for a buffer. Result is endian independent.
uint64_t getConsistentHashValue64(const void *ptr, uint32_t size, uint64_t seed = 0);

FB_END_PACKAGE0()
