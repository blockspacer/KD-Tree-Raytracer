#pragma once

#include "fb/lang/time/HighResolutionTime.h"
#include "fb/lang/time/Time.h"

FB_PACKAGE0()

class ScopedTimer
{
public:
	ScopedTimer()
		: startTime(getHighResolutionTimeValue())
	{
	}

	Time getTime() const
	{
		int64_t delta = (int64_t)(getHighResolutionTimeValue() - startTime);
		return Time::fromTicks(delta * Time::getTicksInMillisecond() * 1000 / (int64_t)getHighResolutionTimeFrequency());
	}

	uint64_t getMicroseconds() const
	{
		int64_t delta = (int64_t)(getHighResolutionTimeValue() - startTime);
		if (delta < 0)
			return 0;
		return (uint64_t)delta * 1000 * 1000 / getHighResolutionTimeFrequency();
	}
	
	uint64_t getMilliseconds() const
	{
		int64_t delta = (int64_t)(getHighResolutionTimeValue() - startTime);
		if (delta < 0)
			return 0;
		return (uint64_t)delta * 1000 / getHighResolutionTimeFrequency();
	}
	
	void reset()
	{
		startTime = getHighResolutionTimeValue();
	}

	void resetToValue(Time offset)
	{
		startTime = getHighResolutionTimeValue() - offset.getTicks() * getHighResolutionTimeFrequency() / (Time::getTicksInMillisecond() * 1000);
	}

private:
	uint64_t startTime;
};

FB_END_PACKAGE0()