#include "Precompiled.h"
#include "DataGuard.h"

#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/profiling/ZoneProfiler.h"

FB_PACKAGE0()

void ScopedRefBase::enterMutex(Mutex &mutex, const char *name)
{
	fb_static_assert(Mutex::isReentrant && "Copying ScopedRef causes recursive mutexing");
	FB_DATAGUARD_PRINTF("%s: ScopedRef - LOCK\n", name);
	
	FB_ZONE_ENTER_CONST_CHAR(name, profiling::ZoneBlock);
	mutex.enter();
	FB_ZONE_EXIT();

	FB_ZONE_ENTER_CONST_CHAR(name);
}
void ScopedRefBase::leaveMutex(Mutex &mutex, const char *name)
{
	mutex.leave();
	FB_ZONE_EXIT();
	FB_DATAGUARD_PRINTF("%s: ScopedRef - unlock\n", name ? name : "nullptr");
}

FB_END_PACKAGE0()
