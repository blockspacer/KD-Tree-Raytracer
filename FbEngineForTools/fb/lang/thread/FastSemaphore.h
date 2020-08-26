#pragma once

#include "Semaphore.h"
#include "fb/lang/Atomics.h"

FB_PACKAGE0()

// Fast semaphore will use atomic counter to prevent unnecessary calls to OS.
// OS semaphore is only called during transitions.
// See http://cbloomrants.blogspot.fi/2011/12/12-08-11-some-semaphores.html
// In short, negative counter tells the amount of waiting threads that should be woken up when there is work to do.
class FastSemaphore
{
	FastSemaphore(const FastSemaphore &other) = delete;
	FastSemaphore &operator=(const FastSemaphore &other) = delete;

public:
	explicit FastSemaphore();
	~FastSemaphore();

	/* Wait for semaphore (decrease count) */
	void wait();
	/* Post semaphore (increase count) */
	void post(uint32_t postCount);

	bool tryWait();
	void waitNoSpin();

private:
	Semaphore osSemaphore;
	lang::AtomicInt32 counter;
};

FB_END_PACKAGE0()
