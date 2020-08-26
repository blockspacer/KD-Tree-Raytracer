#include "Precompiled.h"

#include "GetProcessId.h"

#include "fb/lang/IncludeWindows.h"
#include "fb/lang/IsSame.h"

FB_PACKAGE2(sys, windows)

SizeType GetProcessId::getProcessId()
{
	fb_static_assert(SizeType(0) == DWORD(0));
	fb_static_assert(SizeType(-1) == DWORD(-1));
	fb_static_assert(SizeType(~0U) == DWORD(~0U));
	fb_static_assert(sizeof(SizeType) == sizeof(DWORD));
	return GetCurrentProcessId();
}

FB_END_PACKAGE2()
