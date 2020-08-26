#pragma once

#include "fb/lang/Atomics.h"

FB_PACKAGE0()

template<class T>
class TempOutputStream
{
public:
	T strm;
	lang::AtomicInt32 locked;
};

class TempOutputStreamGuard
{
public:
	template<class T>
	TempOutputStreamGuard(T &tempOutputStream)
		: locked(tempOutputStream.locked)
	{
		int32_t wasLocked = lang::atomicIncRelaxed(locked);
		fb_assert(wasLocked == 0);
		tempOutputStream.strm.clear();
	}

	~TempOutputStreamGuard()
	{
		int32_t wasLocked = lang::atomicDecRelaxed(locked);
		fb_assert(wasLocked == 1);
	}

	lang::AtomicInt32 &locked;
};

FB_END_PACKAGE0()