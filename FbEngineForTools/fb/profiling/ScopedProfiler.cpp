#include "Precompiled.h"
#include "ScopedProfiler.h"

#include "fb/lang/FBPrintf.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/time/HighResolutionTime.h"
#include "fb/lang/time/Time.h"
#include "fb/string/HeapString.h"

#if FB_PRINTF_ENABLED == FB_FALSE
#include <cstdio>
#endif

FB_PACKAGE1(profiling)

#if FB_PRINTF_ENABLED == FB_TRUE
#define FB_SP_PRINTF(...) \
do \
{ \
	if (!logMessages) \
	{ \
		FB_PRINTF(__VA_ARGS__); \
	} \
	else \
	{ \
		TempString msg; \
		msg.doSprintf(__VA_ARGS__); \
		/* Trim line feed */ \
		msg.trimRight(1); \
		FB_LOG_INFO(msg); \
	} \
} while (false) 
#elif FB_PRINTF_ENABLED == FB_FALSE
#define FB_SP_PRINTF(...) \
	printf(__VA_ARGS__);
#else
#error "FB_PRINTF_ENABLED not defined."
#endif

ScopedProfilerCounters ScopedProfiler::dummyCounters;


ScopedProfiler::ScopedProfiler(const char* id, bool logMessages)
	: id(id)
	, counters(dummyCounters)
	, mode(ModeBasic)
	, logMessages(logMessages)
	, threadSafe(false)
{
	FB_SP_PRINTF("ScopedProfiler: %s created\n", id);
	startTime = getHighResolutionTimeValue();
}


ScopedProfiler::ScopedProfiler(const char* id, Mode mode, ScopedProfilerCounters &counters, bool threadSafe, bool logMessages)
	: id(id)
	, counters(counters)
	, mode(mode)
	, logMessages(logMessages)
	, threadSafe(threadSafe)
{
	/* Note: if printInterval is 0, you'll notice it soon enough. */
	if (counters.printInterval == 1)
		FB_SP_PRINTF("ScopedProfiler: %s created\n", id);

	startTime = getHighResolutionTimeValue();
}


LoggingScopedProfiler::LoggingScopedProfiler(const char* id)
	: ScopedProfiler(id, true)
{
}


LoggingScopedProfiler::LoggingScopedProfiler(const char* id, Mode mode, ScopedProfilerCounters &counters, bool threadSafe)
	: ScopedProfiler(id, mode, counters, threadSafe, true)
{
}


static bool updateCounters(ScopedProfilerCounters& countersToUpdate, ScopedProfilerCounters& countersToPrint, ScopedProfiler::Mode mode, uint64_t rawDeltaTime)
{
	countersToUpdate.cumulativeTime += rawDeltaTime;
	countersToUpdate.maxTime = lang::max(countersToUpdate.maxTime, rawDeltaTime);
	countersToUpdate.minTime = lang::min(countersToUpdate.minTime, rawDeltaTime);
	++countersToUpdate.hitCount;
	++countersToUpdate.intervalCounter;
	/* This is never called with ModeBasic, so no need to care about that */
	if (mode == ScopedProfiler::ModeAboveAverage)
	{
		const float averageTime = float(countersToUpdate.cumulativeTime) / countersToUpdate.hitCount;
		const float targetTime = averageTime + averageTime * (countersToUpdate.printInterval / 100.0f);
		if (rawDeltaTime > targetTime)
		{
			countersToPrint = countersToUpdate;
			return true;
		}
		return false;
	}
	if (countersToUpdate.intervalCounter == countersToUpdate.printInterval)
	{
		countersToPrint = countersToUpdate;
		if (mode == ScopedProfiler::ModeCumulativeIntervalled)
		{
			/* Zero cumulative counters */
			countersToUpdate.cumulativeTime = 0;
			countersToUpdate.maxTime = 0;
			countersToUpdate.minTime = (0U - 1);
		}
		return true;
	}
	return false;
}


#define FB_DT_MS_AND_US_FRAC "%" FB_FSU64 ".%03" FB_FSU64 " ms"
ScopedProfiler::~ScopedProfiler()
{
	if (&getGlobalProfiler() == this)
		return;

	const uint64_t endTime = getHighResolutionTimeValue();
	FB_UNUSED_NAMED_VAR(uint64_t, deltaTimeUs) = getHighResolutionDeltaAsTms(startTime * 100, endTime * 100);
	FB_UNUSED_NAMED_VAR(uint64_t, deltaTimeMs) = deltaTimeUs / 1000;
	FB_UNUSED_NAMED_VAR(uint64_t, deltaTimeFrac) = deltaTimeUs % 1000;
	if (mode == ModeBasic)
	{
		FB_SP_PRINTF("ScopedProfiler: %s took " FB_DT_MS_AND_US_FRAC "\n", id, deltaTimeMs, deltaTimeFrac);
	}
	else
	{
		ScopedProfilerCounters printCounters;
		const uint64_t rawDeltaTime = endTime - startTime;
		bool shouldPrint = false;
		if (threadSafe)
		{
			MutexGuard guard(getMutex());
			shouldPrint = updateCounters(counters, printCounters, mode, rawDeltaTime);
		}
		else
		{
			shouldPrint = updateCounters(counters, printCounters, mode, rawDeltaTime);
		}
		if (shouldPrint)
		{
			counters.intervalCounter = 0;
			const uint64_t highResolutionTimeFrequency = getHighResolutionTimeFrequency();
			const uint64_t million = 1000000;
			FB_UNUSED_NAMED_VAR(uint64_t, minTimeUs) = (printCounters.minTime * million) / highResolutionTimeFrequency;
			FB_UNUSED_NAMED_VAR(uint64_t, maxTimeUs) = (printCounters.maxTime * million) / highResolutionTimeFrequency;
			FB_UNUSED_NAMED_VAR(uint64_t, cumulativeMs) = getHighResolutionDeltaAsTms(0, printCounters.cumulativeTime / 10);
			FB_UNUSED_NAMED_VAR(uint64_t, cumulativeFrac) = getHighResolutionDeltaAsTms(0, printCounters.cumulativeTime * 100) % 1000;
			if (mode == ModeCumulative || mode == ModeAboveAverage)
			{
				FB_UNUSED_NAMED_VAR(uint64_t, avgTimeUs) = (printCounters.cumulativeTime * million) / (highResolutionTimeFrequency * printCounters.hitCount);
				FB_SP_PRINTF("ScopedProfiler: %s took " FB_DT_MS_AND_US_FRAC ". Hit count: %" FB_FSU64 ". Cumulative: " FB_DT_MS_AND_US_FRAC ". Min / max / average: %" FB_FSU64 " / %" FB_FSU64 " / %" FB_FSU64 " us.\n",
					id, deltaTimeMs, deltaTimeFrac, printCounters.hitCount, cumulativeMs, cumulativeFrac, minTimeUs, maxTimeUs, avgTimeUs);
			}
			else
			{
				/* Mode must be ModeCumulativeIntervalled */
				FB_UNUSED_NAMED_VAR(uint64_t, avgTimeUs) = (printCounters.cumulativeTime * million) / (highResolutionTimeFrequency * printCounters.printInterval);
				FB_SP_PRINTF("ScopedProfiler: %s took " FB_DT_MS_AND_US_FRAC ". Stats for %d runs. Cumulative: " FB_DT_MS_AND_US_FRAC ". Min / max / average: %" FB_FSU64 " / %" FB_FSU64 " / %" FB_FSU64 " us. Total hit count: %" FB_FSU64 "\n",
						id, deltaTimeMs, deltaTimeFrac, printCounters.printInterval, cumulativeMs, cumulativeFrac, minTimeUs, maxTimeUs, avgTimeUs, printCounters.hitCount);
			}
		}
	}
}


void ScopedProfiler::lapTime(const char* message)
{
	FB_UNUSED_NAMED_VAR(uint64_t, deltaTimeUs) = getHighResolutionDeltaAsTms(startTime * 100, getHighResolutionTimeValue() * 100);
	FB_UNUSED_NAMED_VAR(uint64_t, deltaTimeMs) = deltaTimeUs / 1000;
	FB_UNUSED_NAMED_VAR(uint64_t, deltaTimeFrac) = deltaTimeUs % 1000;
	FB_SP_PRINTF("ScopedProfiler: %s lap time %s: " FB_DT_MS_AND_US_FRAC "\n", id, message, deltaTimeMs, deltaTimeFrac);
}
#undef FB_DT_MS_AND_US_FRAC

Time ScopedProfiler::getTime()
{
	const uint64_t now = getHighResolutionTimeValue();
	uint64_t delta = now - startTime;
	return Time::fromTicks(delta * Time::getTicksInMillisecond() / (getHighResolutionTimeFrequency() * 1000));
}


Mutex& ScopedProfiler::getMutex()
{
	static Mutex mutex;
	return mutex;
}


ScopedProfiler& ScopedProfiler::getGlobalProfiler()
{
	static ScopedProfiler globalProfiler("GlobalProfiler");
	return globalProfiler;
}

FB_END_PACKAGE1()