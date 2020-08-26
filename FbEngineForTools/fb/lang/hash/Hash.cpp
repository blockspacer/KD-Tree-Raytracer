#include "Precompiled.h"
#include "Hash.h"

#include "xxhash.h"
#include "fb/lang/platform/EnableDevOptimisations.h"

#if FB_COMPILER == FB_MSC
	#pragma warning(disable: 4804)
#endif
#include "xxhash.c"

FB_PACKAGE0()

// General hash
uint32_t getHashValue(const void *ptr, uint32_t size, uint32_t seed)
{
	return XXH32(ptr, size, seed);
}

uint32_t getConsistentHashValue(const void *ptr, uint32_t size, uint32_t seed)
{
	return XXH32(ptr, size, seed);
}


uint64_t getHashValue64(const void *ptr, uint32_t size, uint64_t seed)
{
	return XXH64(ptr, size, seed);
}

uint64_t getConsistentHashValue64(const void *ptr, uint32_t size, uint64_t seed)
{
	return XXH64(ptr, size, seed);
}

FB_END_PACKAGE0()
