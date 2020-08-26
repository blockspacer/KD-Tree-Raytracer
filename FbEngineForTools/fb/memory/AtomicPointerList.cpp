#include "Precompiled.h"
#include "AtomicPointerList.h"

#include "fb/lang/Alignment.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/time/ScopedTimer.h"

#include <cstring>
#pragma intrinsic(memcpy)

FB_PACKAGE1(memory)

fb_static_assert(sizeof(lang::AtomicPointer) == sizeof(void*));
fb_static_assert(sizeof(AtomicPointerList) == lang::CacheLineAlignment);

#define FB_USE_ABA_POINTER FB_TRUE

#if FB_USE_ABA_POINTER == FB_TRUE
	fb_static_assert(alignof(AtomicPointerList) == 16);
	fb_static_assert((sizeof(AtomicPointerList) & 15u) == 0);
	fb_static_assert((offsetof(struct AtomicPointerList, headNode) & 15u) == 0);
#elif FB_USE_ABA_POINTER == FB_FALSE
	static const lang::AtomicPointerIntType lockBit = 1;
#endif

#define FB_APL_PRINT_BACKOFF_STATS FB_FALSE

#if FB_APL_PRINT_BACKOFF_STATS == FB_TRUE
	static lang::AtomicUInt32 totalAccesses;
	/* Number of accesses that require sleeping */
	static lang::AtomicUInt32 longAccesses;
	/* Number of accesses that require some spinning, but no sleeping */
	static lang::AtomicUInt64 shortAccesses;
	/* Number of spins in short accesses (excludes accesses that required sleeping */
	static lang::AtomicUInt64 shortAccessSpinCount;
#endif

struct APLExponentialBackoff
{
	/* Following based on spinsBeforeSleep = 1000, FB_USE_ABA_POINTER == FB_FALSE, PS4.
	 * 
	 * Best values depend on platform, but also depend on load. In Trine 4, where livelocks actually happened, the 
	 * sleep and spin times were very different for normal case vs. a map where livelocks happened all the time (that 
	 * map was spamming an error every frame, which may have been the cause). For problematic map shorter less 
	 * spinning before sleeping would have been better. For both cases the most common situation by far is not to spin 
	 * at all. Average number of spins for a case that required spinning, but didn't require sleeping, was 3. Number 
	 * of cases requiring sleeping is around one in a million for problematic map, less for normal.
	 *
	 * Using the ABA pointer (after fixes by Samuli not yet committed, was crashing before those) there doesn't seem 
	 * to be any live-locking problems.
	 */

	static const uint32_t spinsBeforeSleep = 1000;
	/* Typical time in case sleeping is required is tens or hundreds of microseconds. Too small times are ignored by 
	 * nanosleep, at least on PS4 and NX, so makes sense to start with relatively large value. */
	static const uint32_t initialSleepNs = 10 * 1000;
	/* Two milliseconds is crazy long, but at least you can be sure OS will really put the thread to sleep */
	static const uint32_t maxSleepNs = 2 * 1000 * 1000;

	uint32_t spinsLeft = spinsBeforeSleep;
	uint32_t sleepNs = initialSleepNs;

	#if FB_APL_PRINT_BACKOFF_STATS == FB_TRUE
		ScopedTimer timer;
		uint32_t numSleeps = 0;

		~APLExponentialBackoff()
		{
			uint32_t totalAccessesSoFar = atomicIncRelaxed(totalAccesses) + 1;
			if (spinsLeft == 0)
			{
				uint32_t longAccessesSoFar = atomicIncRelaxed(longAccesses) + 1;
				float spinsPerShortAccess = float(atomicLoadRelaxed(shortAccessSpinCount)) / atomicLoadRelaxed(shortAccesses);
				FB_PRINTF("Long spin, took %d sleeps and %" FB_FSU64 " microseconds. Long spins per total accesses: %d / %d, average spins per normal access: %f\n", 
					numSleeps, timer.getMicroseconds(), longAccessesSoFar, totalAccessesSoFar, spinsPerShortAccess);
			}
			else if (spinsLeft != spinsBeforeSleep)
			{
				atomicIncRelaxed(shortAccesses);
				atomicAddRelaxed(shortAccessSpinCount, spinsBeforeSleep - spinsLeft);
			}
		}
	#endif

	void yield()
	{
		if (spinsLeft > 0)
		{
			--spinsLeft;
			lang::atomicThreadPause();
		}
		else
		{
			#if FB_APL_PRINT_BACKOFF_STATS == FB_TRUE
				++numSleeps;
			#endif
			Thread::nanosleep(sleepNs);
			sleepNs = lang::min(sleepNs * 2, maxSleepNs);
		}
	}

};


void AtomicPointerList::pushAtomic(void *ptr)
{
	fb_assert(lang::AtomicPointerIntType(ptr) % 8 == 0);
	APLExponentialBackoff backoff;
	#if FB_USE_ABA_POINTER  == FB_TRUE
		lang::AtomicAbaPointer currentHead = atomicLoadAcquire(headNode);
		for (;;)
		{
			void *currentHeadPointer = currentHead.getPointer();
			memcpy(ptr, &currentHeadPointer, sizeof(void*));

			lang::AtomicAbaPointer newHead = currentHead;
			newHead.step();
			newHead.setPointer(ptr);

			if(lang::atomicCompareExchangeWeakAcquireRelease(headNode, currentHead, newHead))
				return;

			backoff.yield();
		}
	#else
		for(;;)
		{
			// ABA safe, but creates a (quick) spin lock
			lang::AtomicPointerIntType currentValue = atomicOrAcquire(headPointer, lockBit);
			if (currentValue & lockBit)
			{
				// Already locked, need to wait
				backoff.yield();
				continue;
			}

			void *currentValuePointer = (void*)currentValue;
			memcpy(ptr, &currentValuePointer, sizeof(void*));
			fb_assert((lang::AtomicPointerIntType(ptr) & lockBit) == 0);
			atomicStoreRelease(headPointer, ptr);
			break;
		}
	#endif
}

void AtomicPointerList::pushAtomic(void *ptr, uint32_t sizeInBytes, uint32_t amountInElements)
{
	// Create linked list out of the ptr.
	fb_assert(lang::AtomicPointerIntType(ptr) % 8 == 0);
	fb_assert(sizeInBytes % 8 == 0);
	char *currentPointer = (char*) ptr;
	for (uint32_t i = 0; i < amountInElements - 1; ++i)
	{
		char *nextPointer = (char *) (currentPointer + sizeInBytes);
		memcpy(currentPointer, &nextPointer, sizeof(void*));
		currentPointer = nextPointer;
	}

	// Connect list to head pointer.
	APLExponentialBackoff backoff;
	#if FB_USE_ABA_POINTER  == FB_TRUE
		lang::AtomicAbaPointer currentHead = atomicLoadAcquire(headNode);
		for (;;)
		{
			void *currentHeadPointer = currentHead.getPointer();
			memcpy(currentPointer, &currentHeadPointer, sizeof(void*));

			lang::AtomicAbaPointer newHead = currentHead;
			newHead.step();
			newHead.setPointer(ptr);

			if(lang::atomicCompareExchangeWeakAcquireRelease(headNode, currentHead, newHead))
				return;

			backoff.yield();
		}
	#else
		for(;;)
		{
			// ABA safe, but creates a (quick) spin lock
			lang::AtomicPointerIntType currentHead = atomicOrAcquire(headPointer, lockBit);
			if (currentHead & lockBit)
			{
				// Already locked, need to wait
				backoff.yield();
				continue;
			}

			void *currentHeadPointer = (void*)currentHead;
			memcpy(currentPointer, &currentHeadPointer, sizeof(void*));
			fb_assert((lang::AtomicPointerIntType(ptr) & lockBit) == 0);
			atomicStoreRelease(headPointer, ptr);
			break;
		}
	#endif
}

void *AtomicPointerList::popAtomic()
{
	APLExponentialBackoff backoff;
	#if FB_USE_ABA_POINTER  == FB_TRUE
		lang::AtomicAbaPointer currentHead = atomicLoadAcquire(headNode);
		for (;;)
		{
			void *currentHeadPointer = currentHead.getPointer();
			if (!currentHeadPointer)
				return nullptr;

			void *nextHead = nullptr;
			memcpy(&nextHead, currentHeadPointer, sizeof(void*));

			lang::AtomicAbaPointer newHead = currentHead;
			newHead.step();
			newHead.setPointer(nextHead);

			if(lang::atomicCompareExchangeWeakAcquireRelease(headNode, currentHead, newHead))
				return currentHeadPointer;

			backoff.yield();
		}
	#else
		for (;;)
		{
			// ABA safe, but creates a (quick) spin lock
			lang::AtomicPointerIntType currentHead = atomicOrAcquire(headPointer, lockBit);
			if (currentHead & lockBit)
			{
				// Already locked, need to wait
				backoff.yield();
				continue;
			}

			// Empty. Clear the lock.
			if (!currentHead)
			{
				atomicStoreRelease(headPointer, nullptr);
				return nullptr;
			}

			void *nextHead = nullptr;
			void *currentHeadPointer = (void*)currentHead;
			memcpy(&nextHead, currentHeadPointer, sizeof(void*));
		
			fb_assert((lang::AtomicPointerIntType(nextHead) & lockBit) == 0);
			atomicStoreRelease(headPointer, nextHead);
			return currentHeadPointer;
		}
	#endif
}

void AtomicPointerList::pushRelaxed(void *ptr)
{
	pushAtomic(ptr);
	/*
	void *currentHead = atomicLoadRelaxed(headPointer);
	memcpy(ptr, &currentHead, sizeof(void*));
	atomicStoreRelaxed(headPointer, ptr);
	*/
}

void AtomicPointerList::pushRelaxed(void *ptr, uint32_t sizeInBytes, uint32_t amountInElements)
{
	pushAtomic(ptr, sizeInBytes, amountInElements);
	/*
	// Create linked list out of the ptr.
	char *currentPointer = (char*) ptr;
	for (uint32_t i = 0; i < amountInElements - 1; ++i)
	{
		char *nextPointer = (char *) (currentPointer + sizeInBytes);
		memcpy(currentPointer, &nextPointer, sizeof(void*));
		currentPointer = nextPointer;
	}

	// Connect list to head pointer.
	void *currentHead = atomicLoadAcquire(headPointer);
	memcpy(currentPointer, &currentHead, sizeof(void*));
	atomicStoreRelaxed(headPointer, ptr);
	*/
}

void *AtomicPointerList::popRelaxed()
{
	return popAtomic();

	/*
	void *currentHead = atomicLoadRelaxed(headPointer);
	if (!currentHead)
		return nullptr;

	void *nextHead = nullptr;
	memcpy(&nextHead, currentHead, sizeof(void*));
	atomicStoreRelaxed(headPointer, nextHead);
	return currentHead;
	*/
}

void AtomicPointerList::push(void *ptr)
{
	if (useAtomics)
		pushAtomic(ptr);
	else 
		pushRelaxed(ptr);
}

void AtomicPointerList::push(void *ptr, uint32_t sizeInBytes, uint32_t amountInElements)
{
	if (useAtomics)
		pushAtomic(ptr, sizeInBytes, amountInElements);
	else 
		pushRelaxed(ptr, sizeInBytes, amountInElements);
}

void *AtomicPointerList::pop()
{
	if (useAtomics)
		return popAtomic();
	else
		return popRelaxed();
}

FB_END_PACKAGE1()
