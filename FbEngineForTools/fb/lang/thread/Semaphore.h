#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE0()

class Semaphore
{
	Semaphore(const Semaphore& other) = delete;
	Semaphore& operator=(const Semaphore& other) = delete;

public:
	/* NOTE: debugMaxValue is only for debugging. It may be completely ignored on platforms that don't natively support 
	 * max values for semaphores (value of 0 means no debugging). */
	Semaphore(uint32_t initialValue, uint32_t debugMaxValue = 0);
	~Semaphore();
	/* Wait for semaphore (decrease count) */
	void wait();
	/* Wait for until timeout in milliseconds. Returns false if time out reached */
	bool waitFor(SizeType milliseconds);
	/* Post semaphore (increase count) */
	void post(uint32_t postCount);

private:
	struct SemaphoreImpl;
	SemaphoreImpl* impl;
};

FB_END_PACKAGE0()
