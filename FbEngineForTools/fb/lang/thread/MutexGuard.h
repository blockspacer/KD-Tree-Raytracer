#pragma once

#include "fb/lang/thread/Mutex.h"

FB_PACKAGE0()

/**
 * Mutex guard for scope.
 *
 * usage example:
 * Mutex mutex;
 * {
 *   MutexGuard guard(mutex);
 *   // do stuff here
 * }
 * // mutex released at end of scope
 */
class InvertedMutexGuard;

class MutexGuard
{
	friend class InvertedMutexGuard;
	// Not implemented
	void operator = (MutexGuard &) = delete;
	MutexGuard(const MutexGuard&) = delete;

public:
	MutexGuard(Mutex &m)
		:	mutex(m)
	{
		mutex.enter();
	}

	~MutexGuard()
	{
		mutex.leave();
	}

	Mutex &getMutex()
	{
		return mutex;
	}

private:
	Mutex &mutex;
};


/**
 * Like MutexGuard, but releases mutex on enter and locks it on exit
 */
class InvertedMutexGuard
{
public:
	InvertedMutexGuard(MutexGuard &guard)
		: mutex(guard.mutex)
	{
		mutex.leave();
	}

	InvertedMutexGuard(Mutex &m)
		: mutex(m)
	{
		mutex.leave();
	}

	~InvertedMutexGuard()
	{
		mutex.enter();
	}

	Mutex &getMutex()
	{
		return mutex;
	}

private:
	// Not implemented
	void operator = (MutexGuard &);

	Mutex &mutex;
};

FB_END_PACKAGE0()
