#pragma once

#include "fb/lang/time/RunningSystemTimer.h"
#include "fb/lang/time/Time.h"

FB_PACKAGE0()

class ScopedSystemTimer
{
public:
	ScopedSystemTimer()
	{
		startTime = systemTimer.getTime();
	}

	~ScopedSystemTimer()
	{
	}

	Time getTime() const
	{
		return systemTimer.getTime() - startTime;
	}

	double getTimeInSeconds() const
	{
		return (systemTimer.getTime() - startTime).getSeconds();
	}
	
	uint64_t getTimeInMilliseconds() const
	{
		return uint64_t((systemTimer.getTime() - startTime).getMilliseconds());
	}

	void reset()
	{
		startTime = systemTimer.getTime();
	}
	
private:
	lang::RunningSystemTimer systemTimer;
	Time startTime;
};

FB_END_PACKAGE0()
