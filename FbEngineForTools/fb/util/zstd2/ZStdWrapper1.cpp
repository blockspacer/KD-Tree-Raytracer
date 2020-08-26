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

// Compress
#include "zstd_compress.c"
#include "huf_compress.c"
#include "fse_compress.c"
#include "zstd_double_fast.c"
#include "zstd_fast.c"
#include "zstd_lazy.c"
#include "zstd_ldm.c"
#include "zstd_opt.c"
#include "zstdmt_compress.c"
#include "hist.c"

#pragma clang diagnostic pop

#pragma warning(pop)

FB_END_PACKAGE2()
