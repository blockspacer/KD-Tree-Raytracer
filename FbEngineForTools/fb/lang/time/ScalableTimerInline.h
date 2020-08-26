
template<typename InternalTimerT> class ScalableTimer<InternalTimerT>::Impl
{
private:
	Impl(const InternalTimerT *systemTimer)
		: systemTimer(systemTimer)
	{
		// we really need to have a 64 bit int for the calculations, if the system timer does not return such, 
		// need to change this class to use them internally. therefore, this assert.
		fb_static_assert(sizeof(TimeTypeMilliseconds) >= 8);
		fb_assert(systemTimer != nullptr);
	}

	~Impl()
	{
		fb_assert(!started);
	}

	bool started = false;
	bool paused = false;
	const InternalTimerT *systemTimer = nullptr;
	typename InternalTimerT::TimeTypeSeconds startTime = 0.0f;
	typename InternalTimerT::TimeTypeSeconds currentTime = 0.0f;
	typename InternalTimerT::TimeTypeSeconds currentUnfactoredTime = 0.0f;
	typename InternalTimerT::TimeTypeSeconds factorTimeAdd = 0.0f;
	float factor = 1.0f;
	typename InternalTimerT::TimeTypeSeconds fixedTimeStep = 0.0f;
	typename InternalTimerT::TimeTypeSeconds fixedStepCurrentTime = 0.0f;

	friend class ScalableTimer<InternalTimerT>;
};

template<typename InternalTimerT> ScalableTimer<InternalTimerT>::ScalableTimer(const InternalTimerT *systemTimer)
{
	fb_single_thread_assert();
	fb_assert(systemTimer != nullptr);

	impl = new typename ScalableTimer<InternalTimerT>::Impl(systemTimer);
}


template<typename InternalTimerT> ScalableTimer<InternalTimerT>::~ScalableTimer()
{
	fb_single_thread_assert();
	if (impl->started)
	{
		fb_assert(0 && "ScalableTimer - You should have stopped the timer before deleting it.");
		stop();
	}
	delete impl;
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::start()
{
	fb_single_thread_assert();
	fb_assert(!impl->started);
	impl->started = true;

	resetAll();
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::stop()
{
	fb_single_thread_assert();
	fb_assert(impl->started);
	impl->started = false;
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::resetTime(Time newTime)
{
	fb_single_thread_assert();
	fb_assert(impl->started);

	// note: this conversion is only safe because TimeType is >= 64-bit
	typename InternalTimerT::TimeTypeSeconds newTimeSeconds = (typename InternalTimerT::TimeTypeSeconds)(newTime.getSeconds());
	impl->startTime = impl->systemTimer->getTimeInSeconds() - newTimeSeconds;
	impl->currentTime = newTimeSeconds;
	impl->currentUnfactoredTime = newTimeSeconds;
	impl->factorTimeAdd = 0;

	update();
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::resetAll()
{
	fb_single_thread_assert();
	fb_assert(impl->started);

	impl->factor = 1.0f;
	impl->factorTimeAdd = 0;

	resetTime();
}

template<typename InternalTimerT> Time ScalableTimer<InternalTimerT>::getTime() const
{
	if (impl->fixedTimeStep > 0)
		return Time::fromSeconds(impl->fixedStepCurrentTime);
	else
		return Time::fromSeconds(impl->currentTime);
}

template<typename InternalTimerT> typename ScalableTimer<InternalTimerT>::TimeTypeMilliseconds ScalableTimer<InternalTimerT>::getTimeInMilliseconds() const
{
	if (impl->fixedTimeStep > 0)
		return (TimeTypeMilliseconds)(impl->fixedStepCurrentTime * 1000);
	else
		return (TimeTypeMilliseconds)(impl->currentTime * 1000);
}

template<typename InternalTimerT> typename ScalableTimer<InternalTimerT>::TimeTypeSeconds ScalableTimer<InternalTimerT>::getTimeInSeconds() const
{
	if (impl->fixedTimeStep > 0)
		return impl->fixedStepCurrentTime;
	else
		return impl->currentTime;
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::update()
{
	fb_single_thread_assert();
	fb_assert(impl->started);

	// notice, that the factor is calculated in this complex manner rather than just 
	// adding a factored time delta to previous value because such a simple approach
	// would cause significant errors with low factor values. (because of the int value
	// deltas getting rounded to floor value - at worst, rounded to zero, causing the 
	// timer to stop - and that behaviour would be based on the frequency of the update 
	// calls.)a

	if (!impl->paused)
	{
		TimeTypeSeconds sysTime = impl->systemTimer->getTimeInSeconds();
		fb_assert(sysTime >= impl->startTime);
		impl->currentUnfactoredTime = sysTime - impl->startTime;
		impl->currentTime = impl->factorTimeAdd + TimeTypeSeconds((double)(impl->currentUnfactoredTime) * (double)(impl->factor));
	}
	else
	{
		// while paused, need to move the factorTimeAdd forward to prevent a sudden warp in time once unpaused again.
		// (alternatively, could take the pause time value when entering pause, and then doing this time offset for the total time at unpausing.)
		// (or alternatively, could just set the time factor to zero while paused, but would apparently blow up the current factorTimeAdd logic)
		TimeTypeSeconds sysTime = impl->systemTimer->getTimeInSeconds();
		fb_assert(sysTime >= impl->startTime);
		impl->currentUnfactoredTime = sysTime - impl->startTime;
		TimeTypeSeconds prevTime = impl->currentTime;
		impl->currentTime = impl->factorTimeAdd + TimeTypeSeconds((double)(impl->currentUnfactoredTime) * (double)(impl->factor));

		// correct the result for pausing...
		TimeTypeSeconds resultTimeDelta = impl->currentTime - prevTime;
		impl->factorTimeAdd -= resultTimeDelta;
		impl->currentTime = impl->factorTimeAdd + TimeTypeSeconds((double)(impl->currentUnfactoredTime) * (double)(impl->factor));

		// (as a result, the time that is given out should not have changed.)
		fb_assert(fabs(prevTime - impl->currentTime) < 0.01/1000.0);
	}

	if (impl->fixedTimeStep > 0)
	{
		// (well the following will such if trying to use a floating point)
		//fb_assert(lang::NumericLimits<InternalTimerT::TimeType>::isInteger);

		impl->fixedStepCurrentTime = ((uint64_t)(impl->currentTime / impl->fixedTimeStep)) * impl->fixedTimeStep;
	}
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::setPaused(bool paused)
{
	fb_assert(impl->started);
	impl->paused = paused;
}

template<typename InternalTimerT> bool ScalableTimer<InternalTimerT>::getPaused() const
{
	fb_assert(impl->started);
	return impl->paused;
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::setTimeFactor(float factor)
{
	fb_single_thread_assert();
	fb_assert(factor >= getMinTimeFactor() && factor <= getMaxTimeFactor());

#if FB_SCALABLE_TIMER_DEBUGGING == FB_TRUE
	Time timeWasBeforeFactorSet = getTime();
#endif

	float oldFactor = impl->factor;
	TimeTypeSeconds oldFactorTimeAdd = impl->factorTimeAdd;

	TimeTypeSeconds newFactorTimeAdd =
		oldFactorTimeAdd + ((double)impl->currentUnfactoredTime * (double)(oldFactor - factor));

	impl->factor = factor;
	impl->factorTimeAdd = newFactorTimeAdd;

#if FB_SCALABLE_TIMER_DEBUGGING == FB_TRUE
	Time timeIsAfterFactorSet = getTime();

	if (timeIsAfterFactorSet < timeWasBeforeFactorSet) // Adding dummy check to get rid off "unused variable warning"
	{
	}

	// time may never go back due to factor setting. although, this might happen due rounding errors?
	fb_assert(timeIsAfterFactorSet >= timeWasBeforeFactorSet);

	// time must also remain somewhat constant. assuming that the error is at most 1000 milliseconds, 
	// an error beyond that means that we must have a bug in the above formulae
	fb_assert((timeIsAfterFactorSet - timeWasBeforeFactorSet) < Time::fromSeconds(1));
#endif
}

template<typename InternalTimerT> float ScalableTimer<InternalTimerT>::getTimeFactor() const
{
	return impl->factor;
}

template<typename InternalTimerT> float ScalableTimer<InternalTimerT>::getMinTimeFactor() const
{
	return 0.01f;
}

template<typename InternalTimerT> float ScalableTimer<InternalTimerT>::getMaxTimeFactor() const
{
	return 100.0f;
}

template<typename InternalTimerT> void ScalableTimer<InternalTimerT>::setFixedTimeStep(TimeTypeSeconds timeStepSeconds)
{
	impl->fixedTimeStep = timeStepSeconds;
}
