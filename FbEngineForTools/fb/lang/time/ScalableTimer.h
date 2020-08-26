#pragma once

#include "fb/lang/time/RunningSystemTimer.h"
#include "fb/lang/FBSingleThreadAssert.h"
#include "fb/lang/time/Time.h"
#include "fb/lang/Types.h"
#include "fb/lang/Megaton.h"

#if FB_BUILD != FB_FINAL_RELEASE
#define FB_SCALABLE_TIMER_DEBUGGING FB_TRUE
#else
#define FB_SCALABLE_TIMER_DEBUGGING FB_FALSE
#endif

FB_PACKAGE0()

class Timer
{
	FB_MEGATON_CLASS_DECL();
public:
	virtual ~Timer() {}

	virtual Time getTime() const = 0;
};

/**
 * A wrapper for the SystemTimer (or some other timer type), which can be used to calculate the relative time from the initialization,
 * instead of using the operating systems internal zero point of time.
 * Also adds support for optimization (different update and getTime calls), the time factor setting and pause feature.
 *
 * @version 1.0, 2.9.2009
 * @version 2.0, 27.9.2010 - significant redo to be usable as a template class.
 * @version 2.1, 30.9.2010 - added support for fixed time stepping.
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 *
 */
template<typename InternalTimerT> class ScalableTimer : public Timer
{
public:
	class Impl;

	typedef double TimeTypeSeconds;
	typedef uint64_t TimeTypeMilliseconds;

	/**
	 * @param systemTimer, the system timer to be used internally. Notice that this class will not 
	 *        claim ownership of the system timer, neither will it initialize it. You must do
	 *        that before calling the start method. Also, you should not uninitialize the
	 *        system timer before you have called stop. This timer will not will not update 
	 *        the internal timer - it needs to be done elsewhere.
	 */
	ScalableTimer(const InternalTimerT *internalTimer);

	virtual ~ScalableTimer();

	/** 
	 * Should be called to start the timer.
	 * Before any update or getTime calls.
	 * This also does resetAll, so all previous factors, etc. will be lost.
	 */
	void start();

	/** 
	 * Should be called when done with the timer.
	 * Don't use this to pause the progress of time, use the setPaused method instead. 
	 * This is intended to be called only once done with the timer. 
	 */
	void stop();

	/** 
	 * Resets the timer so that values returned will begin running from given value.
	 * The value returned by getTime after this may be slightly past it as a few
	 * milliseconds may passed even during the reset call.
	 * The timer must have been started before calling this.
	 */
	void resetTime(Time newTime = Time());

	/**
	 * Like resetTime, but resets the time factor as well.
	 */
	void resetAll();

	/** 
	 * Returns the current time.
	 * Value will be the time when update was last called!
	 * (Changing the time factor may or may not have and effect on the returned value,
	 * however, due to possible rounding errors, etc.)
	 */
	Time getTime() const;

	/** 
	 * Updates the time. Should be called regularly.
	 * All values returned by the getTime function will remain the
	 * same until the next update call. 
	 */
	void update();

	/**
	 * This can be used to pause or unpause the timer. 
	 * Pausing has a similar effect as setting the 
	 */
	void setPaused(bool paused);
	bool getPaused() const;

	/**
	 * Setting this to any other value than zero, will cause the timer to return fixed time 
	 * stepped values when calling getTime. Setting a zero value means the normal, variable 
	 * time step functionality.
	 * (Fixed time step means that the returned time value will always increase in a single 
	 * or multiple steps of given milliseconds - or it will not increase at all if not enough 
	 * time has passed.)
	 */
	void setFixedTimeStep(TimeTypeSeconds timeStepSeconds);

	/**
	 * This can be used to set the scale for the timer.
	 * The time returned by the timer will appear to either slow down or speed up to this factor.
	 * A change of the time scale does not have a significant effect on the returned time value,
	 * only an effect on the update of the time (time delta seen between updates). 
	 * Small rounding errors may occur though, so you should not rely on getTime to return 
	 * the exact same time after calling this.
	 *
	 * Notice that it may not be possible to use the time factor to stop the progress of the timer,
	 * (in other words, zero factor may not be acceptable) if you want to stop the time, 
	 * you should use setPaused instead.
	 *
	 * @param factor, this will be the scaling factor for the timer, the given factor may not exceed 
	 *        the range that can be obtained with getMinTimeFactor and getMaxTimeFactor.
	 */
	void setTimeFactor(float factor);
	float getTimeFactor() const;

	/**
	 * Use this to query the minimum allowed time factor.
	 * Attempting to set a time factor below this value may result in an undefined behaviour. 
	 *
	 * @return minimum value for the factor accepted by setTimeFactor.
	 */
	float getMinTimeFactor() const;

	/**
	 * Use this to query the maximum allowed time factor.
	 * Attempting to set a time factor above this value may result in an undefined behaviour. 
	 *
	 * @return maximun value for the factor accepted by setTimeFactor.
	 */
	float getMaxTimeFactor() const;

	/**
	 * Dummy method.
	 */
	bool isInitialized() const { return true; }

	/** 
	 * Returns the current time in milliseconds. 
	 * Value will be the time when update was last called!
	 * You are usually supposed to use the getTime() instead! This is provided for ScalableTimer chaining purposes only.
	 * (I think these are actually provided, because some refactoring is due.)
	 */
	TimeTypeMilliseconds getTimeInMilliseconds() const;
	TimeTypeSeconds getTimeInSeconds() const;

private:
	Impl *impl;
};

#include "ScalableTimerInline.h"

/**
 * A system timer wrapped for scaling (time factor functionality).
 * a.k.a. "the application timer"
 */
class ScalableRunningSystemTimer : public ScalableTimer<fb::lang::RunningSystemTimer>
{
public:
	ScalableRunningSystemTimer(const fb::lang::RunningSystemTimer *systemTimer) : ScalableTimer<fb::lang::RunningSystemTimer>(systemTimer) { }
	virtual ~ScalableRunningSystemTimer() { }	
};

/**
 * A system timer that has been wrapped for scaling, wrapped again for scaling.
 * a.k.a. "the state timer"
 */
class ScalableScalableTimer : public ScalableTimer<ScalableRunningSystemTimer>
{
public:
	ScalableScalableTimer(const ScalableRunningSystemTimer *internalTimer) : ScalableTimer<ScalableRunningSystemTimer>(internalTimer) { }
	virtual ~ScalableScalableTimer() { }	
};

FB_END_PACKAGE0()
