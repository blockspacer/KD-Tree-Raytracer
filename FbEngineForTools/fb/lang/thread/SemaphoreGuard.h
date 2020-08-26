#pragma once

#include "FastSemaphore.h"

FB_PACKAGE0()

template<typename SemaphoreType>
class SemaphoreGuardTemplated
{
public:
	SemaphoreGuardTemplated(SemaphoreType& semaphore)
		: semaphore(semaphore)
	{
		semaphore.wait();
	}

	~SemaphoreGuardTemplated()
	{
		semaphore.post(1);
	}

private:
	/* Note implemented */
	SemaphoreGuardTemplated &operator=(const SemaphoreGuardTemplated &) = delete;

	SemaphoreType &semaphore;
};

typedef SemaphoreGuardTemplated<Semaphore> SemaphoreGuard;
typedef SemaphoreGuardTemplated<FastSemaphore> FastSemaphoreGuard;

FB_END_PACKAGE0()
