#include "Precompiled.h"

#include "fb/lang/thread/Mutex.h"

#include "fb/lang/FBSingleThreadAssert.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/IncludeWindows.h"

FB_PACKAGE0()

static CRITICAL_SECTION *castToCriticalSection(void *data)
{
	return reinterpret_cast<CRITICAL_SECTION*>(data);
}


Mutex::Mutex()
{
	fb_static_assert(alignof(fb::Mutex::CriticalSectionData) >= alignof(CRITICAL_SECTION));
	fb_static_assert(sizeof(criticalSectionData) == sizeof(CRITICAL_SECTION));
	InitializeCriticalSectionAndSpinCount(castToCriticalSection(&criticalSectionData), 100);
}


Mutex::~Mutex()
{
	DeleteCriticalSection(castToCriticalSection(&criticalSectionData));
}


bool Mutex::tryEnter()
{
	startEnteringMutexDebug();
	if (TryEnterCriticalSection(castToCriticalSection(&criticalSectionData)) != 0)
	{
		stopEnteringMutexDebug();
		enterMutexDebug();
		return true;
	}
	stopEnteringMutexDebug();
	return false;
}


void Mutex::enter()
{
	startEnteringMutexDebug();
	EnterCriticalSection(castToCriticalSection(&criticalSectionData));
	stopEnteringMutexDebug();
	enterMutexDebug();
}


void Mutex::leave()
{
	LeaveCriticalSection(castToCriticalSection(&criticalSectionData));
	leaveMutexDebug();
}

FB_END_PACKAGE0()
