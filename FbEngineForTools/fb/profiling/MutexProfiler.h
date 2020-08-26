#pragma once

FB_DECLARE0(HeapString)
FB_DECLARE0(Mutex)
FB_DECLARE_TEMPLATED_STRUCT0(PodVector)

#if FB_BUILD != FB_FINAL_RELEASE
	#define FB_MUTEX_PROFILER_ENABLED FB_TRUE
#else
	#define FB_MUTEX_PROFILER_ENABLED FB_FALSE
#endif

#if FB_MUTEX_PROFILER_ENABLED == FB_TRUE
	#define FB_EMPTY_BLOCK
	#define FB_RETURN_BOOL
#else
	#define FB_EMPTY_BLOCK {}
	#define FB_RETURN_BOOL { return false; }
#endif

FB_PACKAGE1(profiling)

struct MutexProfilerDumpEvent
{
	double timeStampEnteringStarted;
	double timeStampEnteringFinished;
	double timeStampEntered;
	double timeStampLeft;
	double timeStampOverheadEnd;
	const Mutex *mutexPtr;
	uint32_t threadId;
};

struct MutexPairDump
{
	const Mutex *firstMutexPtr;
	const Mutex *secondMutexPtr;
};

struct MutexProfiler
{
	static void startEntering(const Mutex *mutex) FB_EMPTY_BLOCK;
	static void finishEntering(const Mutex *mutex) FB_EMPTY_BLOCK;
	static void entered(const Mutex *mutex) FB_EMPTY_BLOCK;
	static void left(const Mutex *mutex) FB_EMPTY_BLOCK;

	static void setMutexName(const Mutex *mutexPtr, const char *name) FB_EMPTY_BLOCK;
	static void appendMutexName(HeapString &result, const Mutex *mutexPtr) FB_EMPTY_BLOCK;

	static void dumpMutexEvents(uint64_t nowTimestamp, PodVector<MutexProfilerDumpEvent> &mutexEvents) FB_EMPTY_BLOCK;
	static void appendCallstack(HeapString &result, const Mutex *mutexPtr) FB_EMPTY_BLOCK;
	static void appendSingleRowCallstack(HeapString &result, const Mutex *mutexPtr) FB_EMPTY_BLOCK;

	static void dumpMutexPairs(PodVector<MutexPairDump> &mutexPairs) FB_EMPTY_BLOCK;
	static void clearMutexPairs() FB_EMPTY_BLOCK;

	static void setMutexProfilerEnabled(bool enabled) FB_EMPTY_BLOCK;
	static bool isMutexProfilerEnabled() FB_RETURN_BOOL;
};

FB_END_PACKAGE1();

#undef FB_EMPTY_BLOCK
#undef FB_RETURN_BOOL
