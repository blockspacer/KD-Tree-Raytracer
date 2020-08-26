#include "Precompiled.h"
#include "GetSystemThreadPriority.h"

#include "fb/lang/IncludeWindows.h"

FB_PACKAGE1(task)

/* This should be part of Thread */
/* FIXME: this used to return int, but since thread takes priority as uint32_t, that didn't make much sense. Some of
 * the Windows thread priorities are negative ints though (basically, we are returning priority offset here), so
 * however you look at this, the end result is bad. We should have internal presentation for priorities (basically, as
 * the previous comment says), instead of this mess. */
uint32_t GetSystemThreadPriority::getSystemThreadPriority(int priorityOffset)
{
	if (priorityOffset == -2)
		return uint32_t(THREAD_PRIORITY_LOWEST);
	if (priorityOffset == -1)
		return uint32_t(THREAD_PRIORITY_BELOW_NORMAL);
	if (priorityOffset == 0)
		return uint32_t(THREAD_PRIORITY_NORMAL);
	if (priorityOffset == 1)
		return uint32_t(THREAD_PRIORITY_ABOVE_NORMAL);
	if (priorityOffset == 2)
		return uint32_t(THREAD_PRIORITY_HIGHEST);
	fb_assert(0 && !"Unsupported priority level");
	return 0;
}

FB_END_PACKAGE1()
