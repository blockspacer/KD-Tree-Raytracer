#include "Precompiled.h"

#include <cstring>

FB_PACKAGE2(util, zstd)

#if defined(ERROR) && ERROR == 0
	#undef ERROR
#endif

#pragma warning(disable: 4365 4005)

// Legacy. Should be removed once properly integrated
#include "zstd_v04.c"

FB_END_PACKAGE2()
