#pragma once

#include "fb/lang/time/RunningSystemTimer.h"
#include "fb/lang/time/Time.h"

FB_PACKAGE0()

class SystemDeltaTime
{
public:
	SystemDeltaTime()
	{
		startTime = systemTimer.getTime();
	}

	~SystemDeltaTime()
	{
	}

	Time update()
	{
		Time timeNow = systemTimer.getTime();
		Time deltaTime = timeNow - startTime;
		startTime = timeNow;
		return deltaTime;
	}
	
private:
	lang::RunningSystemTimer systemTimer;
	Time startTime;
};

FB_END_PACKAGE0()
