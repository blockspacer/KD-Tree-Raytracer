#include "Precompiled.h"

#include <cstring>

FB_PACKAGE2(util, zstd)

#if defined(ERROR) && ERROR == 0
	#undef ERROR
#endif

#pragma warning(push)
#pragma warning(disable: 4365 4005)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"

// Decompress
#include "zstd_decompress.c"
#include "huf_decompress.c"
#include "zstd_decompress_block.c"
#include "zstd_ddict.c"
// Common
#include "fse_decompress.c"

#pragma clang diagnostic pop

#pragma warning(pop)

FB_END_PACKAGE2()
