#include "Precompiled.h"
#include "HighResolutionTime.h"

#include "fb/lang/IncludeWindows.h"

FB_PACKAGE0()

uint64_t getHighResolutionTimeValue()
{
	LARGE_INTEGER r;
	r.QuadPart = 0;
	QueryPerformanceCounter(&r);
	uint64_t timeNow = uint64_t(r.QuadPart);
	return timeNow;
}

static uint64_t getHighResolutionTimeFrequencyImpl()
{
	LARGE_INTEGER r;
	r.QuadPart = 0;
	QueryPerformanceFrequency(&r);
	uint64_t frequency = uint64_t(r.QuadPart);
	return frequency;
}

uint64_t getHighResolutionTimeFrequency()
{
	/* It is not said in Nintendo docs that frequency would stay the same all the time, but if it doesn't we are in 
	 * trouble anyway and need another timer */
	thread_local const uint64_t frequency = getHighResolutionTimeFrequencyImpl();
	return frequency;
}

uint64_t getHighResolutionDeltaAsTms(uint64_t start, uint64_t end)
{
	uint64_t delta = end - start;
	delta *= 10000;
	delta /= getHighResolutionTimeFrequency();

	return delta;
}

FB_END_PACKAGE0()

