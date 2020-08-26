#include "Precompiled.h"
#include "IThreadEntry.h"

#include "fb/lang/thread/Thread.h"

FB_PACKAGE0()

void IThreadEntry::runner()
{
	fb_assert(thread != nullptr);
	thread->startingUp();
	entry();
	thread->running = false;
	if (shouldSelfDestruct())
	{
		thread->disposeThread(thread);
		delete this;
	}
}


FB_END_PACKAGE0()