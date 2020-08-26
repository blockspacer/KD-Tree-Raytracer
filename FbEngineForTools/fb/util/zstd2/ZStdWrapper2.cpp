#include "Precompiled.h"

#include <cstring>

FB_PACKAGE2(util, zstd)

#if defined(ERROR) && ERROR == 0
	#undef ERROR
#endif

#pragma warning(disable: 4365 4005)

// Common
#include "entropy_common.c"
#include "pool.c"
#include "debug.c"
#include "error_private.c"
#include "zstd_common.c"

FB_END_PACKAGE2()
