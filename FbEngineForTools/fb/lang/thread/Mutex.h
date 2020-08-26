#pragma once

#ifndef FB_MUTEX_DEBUGGING
	#if FB_BUILD == FB_FINAL_RELEASE
		#define FB_MUTEX_DEBUGGING FB_FALSE
	#else
		#define FB_MUTEX_DEBUGGING FB_TRUE
	#endif
#endif

/**
 * This is a basic mutex. It should be performant and re-entrant (or recursive), but it's best not to count on the 
 * latter. It has been said that algorithms requiring re-entrant mutexes are where spaghetti comes.
 */

FB_PACKAGE0()

class Mutex
{
	Mutex(const Mutex &) = delete;
	Mutex& operator=(const Mutex &) = delete;

public:
	Mutex();
	~Mutex();

	bool tryEnter();
	void enter();
	void leave();

	/* If your algorithm is counting on mutex being re-entrant (or not) drop some checks for this in for future */
	static const bool isReentrant = true;

private:
	struct alignas(8) CriticalSectionData
	{
		static const uint32_t criticalSectionDataSize = 40;
		char data[criticalSectionDataSize];
	};
	CriticalSectionData criticalSectionData;

	// Some debugging helper functions
#if FB_MUTEX_DEBUGGING == FB_TRUE
	void startEnteringMutexDebug();
	void stopEnteringMutexDebug();
	void enterMutexDebug();
	void leaveMutexDebug();
#elif FB_MUTEX_DEBUGGING == FB_FALSE
	inline void startEnteringMutexDebug() {}
	inline void stopEnteringMutexDebug() {}
	inline void enterMutexDebug() {}
	inline void leaveMutexDebug() {}
#else
	#error "FB_MUTEX_DEBUGGING is not FB_TRUE or FB_FALSE. It's probably undefined."
#endif
};

FB_END_PACKAGE0()
