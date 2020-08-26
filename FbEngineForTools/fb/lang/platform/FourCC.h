#pragma once

#include "Platform.h"
#include "fb/lang/IntTypes.h"
#include "fb/lang/platform/ByteOrder.h"

FB_PACKAGE3(lang, platform, detail)

// This is now deprecated and non-preferred, as it won't be a to compile time constant (possible performance issue
// as well as inability to use in traits, etc. that would require a compile time value.
static inline int toFourCC(const char *str)
{
#if (FB_BYTE_ORDER == FB_LITTLE_ENDIAN)
	// optimized for PC / little endian. NOTE: this is unsafe though, it does not consider any aliasing rules.
	// Theoretically could use the same for big endians as well, but then the value should never be saved to disk 
	// or it should be converted.
	return *((int *)(str));
#else
	return (int32_t)(str[0]) | ((int32_t)(str[1])<<8) | ((int32_t)(str[2])<<16) | ((int32_t)(str[3])<<24);
#endif
}

FB_END_PACKAGE3()

#if FB_COMPILER == FB_MSC
	// compile time length-check supported on MSC
	#define FB_FOURCC_STR(str) fb::lang::platform::detail::toFourCC((const char [5])str)
#elif FB_COMPILER == FB_ICC
	// compile time length-check supported on ICC?
	#define FB_FOURCC_STR(str) fb::lang::platform::detail::toFourCC((const char [5])str)
#elif FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC
	#define FB_FOURCC_STR(str) fb::lang::platform::detail::toFourCC(str)
#else
	#error "Unsupported compiler."
#endif

// This allows the FOURCC to be used as compile time constant expression
#define FB_FOURCC(c1,c2,c3,c4) ((int)(c1) | ((int)(c2)<<8) | ((int)(c3)<<16) | ((int)(c4)<<24))
