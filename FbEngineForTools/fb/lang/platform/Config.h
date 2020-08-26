#pragma once

#include "fb/lang/platform/FBConstants.h"
#include "GlobalConfig.h"

// ------------------------------------------------------------------------------
// automatic platform detection, etc.

#include "fb/lang/platform/Platform.h"

// ------------------------------------------------------------------------------
// some cell assert stuff?
#undef _WIN32_WINNT
/* Windows 7 is 0x0601 */
#define _WIN32_WINNT 0x0601
#if (FB_BUILD == FB_FINAL_RELEASE)
	#undef _HAS_EXCEPTIONS
	#define _HAS_EXCEPTIONS 0
	#include <exception>
#endif
