#pragma once

#include "fb/string/DynamicString.h"
#include "fb/string/HeapString.h"

FB_DECLARE(task, Scheduler);

FB_PACKAGE1(profiling)

/**
 * Class for profiling cpu/memory usage. 
 * Supports named states to keep track of important application phases such as "startup", "load", "cleanup" etc.
 */
class Profiler
{
	struct Data;
	Data *data;

	Profiler(const Profiler &) = delete;
	void operator = (const Profiler &) = delete;

public:
	Profiler();
	~Profiler();

	void setScheduler(task::Scheduler *scheduler);

	/**
	 * Reset the profiler data. 
	 */
	void reset();

	void setPrintDebugOutput(bool printDebugOutput);
	bool getPrintDebugOutput() const;

	HeapString getDump(const DynamicString &state, bool curlyScopedFormat);

	/**
	 * Output current profiling data. Use empty string to get current data. 
	 */
	void outputData(const DynamicString &state);

	/**
	 * Set breakpoint to current thread. Returns true if given entry was found.
	 */
	bool addBreakPoint(const DynamicString *stackTrace, SizeType stackSize);
	bool removeBreakPoint(const DynamicString *stackTrace, SizeType stackSize);
	
	struct MostRecentTimeEntry
	{
		const char *className;
		const char *functionName;
		uint64_t time;
	};
	void getMostRecentTimes(MostRecentTimeEntry *entries, SizeType numEntries);

};

FB_END_PACKAGE1()
