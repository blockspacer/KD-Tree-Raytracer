#include "Precompiled.h"
#include "RunningSystemTimer.h"

FB_PACKAGE1(lang)

double RunningSystemTimer::getTimeInSeconds() const
{
	return timer.getMicroseconds() * 0.000001;
}

RunningSystemTimer::TimeTypeMilliseconds RunningSystemTimer::getTimeInMilliseconds() const
{
	return TimeTypeMilliseconds(timer.getMilliseconds());
}

FB_END_PACKAGE1()
