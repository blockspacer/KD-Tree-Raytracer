#include "Precompiled.h"
#include "Time.h"

#include "fb/lang/NumericLimits.h"
#include "fb/lang/platform/FBMath.h" // For ::fmod (for NX)
#include "fb/string/StringRef.h"

FB_PACKAGE0()

const Time Time::zero = fromTicks(0);
const Time Time::infinity = fromTicks(lang::NumericLimits<int64_t>::getMax());


Time::Time(const char *str)
	: value(0)
{
	if (str)
	{
		double parsedValue = 0.0;
		StringRef(str).parse(parsedValue);
		setSeconds(parsedValue);
	}
}


float Time::getSecondsAsFloat() const
{
	// float precision limit (adding 0.001f no longer increases value)
	int maxValue = (1 << 14);
	double wrappedValue = fmod(getSeconds(), (double)maxValue);
	return (float)wrappedValue;
}


int32_t Time::getMillisecondsAsUnsafeInt() const
{
	// int precision limit
	int32_t maxValue = lang::NumericLimits<int32_t>::getMax();
	return getMilliseconds() % maxValue;
}


FB_END_PACKAGE0()