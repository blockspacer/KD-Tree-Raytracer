#pragma once

#include "fb/lang/Types.h"
#include "fb/lang/time/ScopedTimer.h"

FB_PACKAGE1(lang)

/**
 * A simple class for getting the value of the running system timer. 
 * This class simply returns the system time (probably since process startup or last OS boot, but you should not 
 * rely on that).
 *
 * Notice, this is probably not the timer you want to use - this is a very low-level timer.
 * If you want to deal with the actual calendar date and the day of time, see the SystemTime class instead.
 * If you want to deal with the application timer, see the ScalableTimer class instead.
 *
 * RunningSystemTimer is somewhat legacy. Consider using ScopedTimer (which is backing RunningSystemTimer) instead.
 *
 * @see fb::ScalableTimer
 */

class RunningSystemTimer
{
public:
	/* Deprecated. Should use Time via getTime(), no need for these */
	typedef int64_t TimeTypeMilliseconds;
	typedef double TimeTypeSeconds;

	/* Returns current system time */
	Time getTime() const
	{
		return timer.getTime();
	}

	/* Deprecated. Returns the current system time in seconds */
	double getTimeInSeconds() const;

	/* Deprecated. Just an alias to make all the timers accessible the same way */
	TimeTypeMilliseconds getTimeInMilliseconds() const;

private:
	ScopedTimer timer;
};

FB_END_PACKAGE1()
