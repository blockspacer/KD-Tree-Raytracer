#pragma once

FB_DECLARE0(Mutex)
FB_DECLARE0(Time)

FB_PACKAGE1(profiling)

struct ScopedProfilerCounters
{
	ScopedProfilerCounters()
	{
	}

	ScopedProfilerCounters(uint32_t printInterval)
		: printInterval(printInterval)
	{
	}

	uint64_t cumulativeTime = 0;
	uint64_t maxTime = 0;
	uint64_t minTime = ~(uint64_t(0));
	uint64_t hitCount = 0;
	uint32_t intervalCounter = 0;
	/* How often to print. In ModeAboveAverage this is interpreted as percentage above average that triggers printing
	 * (so to print calls that took 50 % more than average, use 50. To print calls that took three times longer than 
	 * average, use 200) */
	uint32_t printInterval = 0;
};

class ScopedProfiler
{
public:

	enum Mode
	{
		/* Just measure and print the scope once */
		ModeBasic,
		/* Measure and print average in addition to each call */
		ModeCumulative,
		/* Measure and print average and a call once per given interval (average is reset after each interval) */
		ModeCumulativeIntervalled, 
		/* Measure averages and print calls that are above average */
		ModeAboveAverage
	};

	ScopedProfiler(const char* id, bool logMessages = false);
	/* Note: counters must stay valid until ScopedProfiler has destructed */
	ScopedProfiler(const char* id, Mode mode, ScopedProfilerCounters &counters, bool threadSafe = false, bool logMessages = false);
	~ScopedProfiler();

	void lapTime(const char* message);
	Time getTime();

	static Mutex& getMutex();

	static ScopedProfiler& getGlobalProfiler();

protected:
	const char* id;
	uint64_t startTime;
	ScopedProfilerCounters &counters;
	Mode mode;
	bool logMessages;
	bool threadSafe;

	static ScopedProfilerCounters dummyCounters;
};

/* Version that writes everything to log instead of just printfing */
class LoggingScopedProfiler : public ScopedProfiler
{
public:
	LoggingScopedProfiler(const char* id);
	/* Note: counters must stay valid until ScopedProfiler has destructed */
	LoggingScopedProfiler(const char* id, Mode mode, ScopedProfilerCounters &counters, bool threadSafe = false);
};

FB_END_PACKAGE1()
