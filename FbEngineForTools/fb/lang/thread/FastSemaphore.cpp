#include "Precompiled.h"
#include "FastSemaphore.h"

#define FB_FASTSEMAPHORE_USE_ATOMICS FB_TRUE

FB_PACKAGE0()

bool FastSemaphore::tryWait()
{
	// See if we can dec count before preparing the wait
	int c = atomicLoadAcquire(counter);
	while (c > 0)
	{
		if (atomicCompareExchangeWeakAcquireRelease(counter, c, c - 1))
			return true;
	}

	return false;
}

void FastSemaphore::waitNoSpin()
{
	int c = atomicAddAcquireRelease(counter, -1);
	if (c < 1)
		osSemaphore.wait();
}

FastSemaphore::FastSemaphore()
:	osSemaphore(0)
{
	atomicStoreRelaxed(counter, 0);
}

FastSemaphore::~FastSemaphore()
{
	fb_assert(atomicLoadRelaxed(counter) >= 0);
}

void FastSemaphore::wait()
{
	#if FB_FASTSEMAPHORE_USE_ATOMICS == FB_TRUE
		int spinCount = 1;
		while (spinCount--)
		{
			if (tryWait())
				return;
		}

		waitNoSpin();
	#else
		osSemaphore.wait();
	#endif
}

void FastSemaphore::post(uint32_t postCount_)
{
	#if FB_FASTSEMAPHORE_USE_ATOMICS == FB_TRUE
		int postCount = (int) postCount_;

		int oldCounter = atomicAddAcquireRelease(counter, postCount);
		if (oldCounter < 0)
		{
			int numWaiters = -oldCounter;
			int numToWake = numWaiters < postCount ? numWaiters : postCount;
			osSemaphore.post(SizeType(numToWake));
		}
	#else
		osSemaphore.post(postCount_);
	#endif
}

FB_END_PACKAGE0()
