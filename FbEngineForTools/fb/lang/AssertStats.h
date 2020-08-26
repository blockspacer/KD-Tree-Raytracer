#pragma once

/* Use dumpAssertStatistics() in ProfilerModule to see the stats. Use clearAssertCounts() to reset counts. */

/* This has about 10 % performance penalty for Release build if enabled (tested with big level loading in Space) */
#ifndef FB_ASSERT_COUNTER_ENABLED
#error "FB_ASSERT_COUNTER_ENABLED is not defined. Include fb/lang/FBAssert.h before AssertStats.h to rectify this."
#endif

/* This has about 50 % performance penalty for Release build if enabled (tested with big level loading in Space) */
/* AssertStats are safe even without this in a sense that they won't crash or totally corrupt the counters. Some 
	* counters may miss some increments though */
#define FB_ASSERT_COUNTER_THREADSAFE FB_TRUE


#define FB_ASSERT_COUNTER_COUNTER_TYPE uint64_t
#define FB_ASSERT_COUNTER_INCREMENTOPERATION(p_var) ++p_var
#define FB_ASSERT_COUNTER_GET_CALL_COUNT(p_var) *(p_var)

#if FB_ASSERT_COUNTER_ENABLED == FB_TRUE
	#if FB_ASSERT_COUNTER_THREADSAFE == FB_TRUE
		#undef FB_ASSERT_COUNTER_COUNTER_TYPE
		#undef FB_ASSERT_COUNTER_INCREMENTOPERATION
		#undef FB_ASSERT_COUNTER_GET_CALL_COUNT
		#define FB_ASSERT_COUNTER_COUNTER_TYPE volatile LONG64
		#define FB_ASSERT_COUNTER_INCREMENTOPERATION(p_var) InterlockedIncrement64(&p_var)
		#define FB_ASSERT_COUNTER_GET_CALL_COUNT(p_var) InterlockedAdd64(p_var, 0)
	#endif

	#define FB_ASSERT_COUNTER(predicate) \
		static FB_ASSERT_COUNTER_COUNTER_TYPE assertCallCounter = 0; \
		static const bool assertRegistered = fb::lang::asserts::AssertStats::getSingleton().registerAssertCounter(fb::lang::asserts::AssertStats::AssertCounterRegisterable(#predicate, __FUNCTION__, __FILE__, uint32_t(__LINE__), &assertCallCounter)); \
		FB_ASSERT_COUNTER_INCREMENTOPERATION(assertCallCounter);
#endif


FB_PACKAGE0()
	template<typename T> struct PodVector;
FB_END_PACKAGE0()


/* Need to have some other namespace than lang, otherwise lang::swap will cause trouble with algorithm::sort */
FB_PACKAGE2(lang, asserts)

class AssertStats
{
	/* Copying not implemented */
	AssertStats(const AssertStats&);
	AssertStats& operator=(const AssertStats&);

	/* Private constructor. Use getSingleton() */
	AssertStats();

public:
	~AssertStats();

	class AssertCounterRegisterable
	{
		friend class AssertStats;
	public:
		AssertCounterRegisterable() = default;
		AssertCounterRegisterable(const char* predicate, const char* functionName, const char* fileName, uint32_t lineNumber, FB_ASSERT_COUNTER_COUNTER_TYPE *callCount);
		AssertCounterRegisterable &operator=(const AssertCounterRegisterable &other) = default;

		const char* getPredicate() const { return predicate; }
		const char* getFunctionName() const { return functionName; }
		const char* getFileName() const { return fileName; }
		uint32_t getLineNumber() const { return lineNumber; }
		uint64_t getCallCount() { return uint64_t(FB_ASSERT_COUNTER_GET_CALL_COUNT(callCount)); }

	private:
		const char* predicate;
		const char* functionName;
		const char* fileName;
		uint32_t lineNumber;
		FB_ASSERT_COUNTER_COUNTER_TYPE *callCount;
	};

	class AssertCounter
	{
	public:
		AssertCounter() = default;
		AssertCounter(AssertCounterRegisterable& registerableCounter);
		AssertCounter &operator=(const AssertCounter &other) = default;

		const char* predicate;
		const char* functionName;
		const char* fileName;
		uint32_t lineNumber;
		uint64_t callCount;
	};
	bool registerAssertCounter(const AssertCounterRegisterable &assertCounter);
	uint32_t getNumCounters() const;
	void getCounters(PodVector<AssertCounter>& outCounters) const;

	void clearAssertCounts();

	static AssertStats &getSingleton();

private:
	void prepareForDoom();

	class Impl;
	Impl* impl;
	static bool isBeingInitialized;
	static bool hasBeenDoomed;
};

FB_END_PACKAGE2()
