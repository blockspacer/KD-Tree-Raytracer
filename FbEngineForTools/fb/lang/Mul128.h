#pragma once

#if FB_COMPILER == FB_MSC
#include <intrin.h>
#endif

FB_PACKAGE0()

// warning: low is not actually signed, instead it contains the low bit pattern of the signed 128bit value. eg in int64_t:
// a = 2, b = 0x8000000000000000 -> high = -1, low = 0
// a = 2, b = 0x4000000000000000 -> high = 0, low = 0x8000000000000000
inline void mul128(int64_t a, int64_t b, int64_t &high, int64_t &low)
{
#if FB_COMPILER == FB_MSC
	low = _mul128(a, b, &high);
#else
	// this is implementation-defined and may not work like the above version. every replacement i can think of is also implementation-defined.
	__int128 result = (__int128)a * (__int128)b;
	low = (int64_t)result;
	high = result >> 64;
#endif
}

FB_END_PACKAGE0()
