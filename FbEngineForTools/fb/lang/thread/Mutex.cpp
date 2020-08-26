#include "Precompiled.h"
#include "Mutex.h"

// rest of the implementation is in the platform specific Mutex*.cpp file.

#if FB_MUTEX_DEBUGGING == FB_TRUE

#include "fb/profiling/MutexProfiler.h"

FB_PACKAGE0()

void Mutex::startEnteringMutexDebug()
{
	profiling::MutexProfiler::startEntering(this); // Never allocates memory
}

void Mutex::stopEnteringMutexDebug()
{
	profiling::MutexProfiler::finishEntering(this); // Never allocates memory
}

void Mutex::enterMutexDebug()
{
	profiling::MutexProfiler::entered(this); // Never allocates memory
}

void Mutex::leaveMutexDebug()
{
	profiling::MutexProfiler::left(this); // Never allocates memory
}

FB_END_PACKAGE0()

#endif
