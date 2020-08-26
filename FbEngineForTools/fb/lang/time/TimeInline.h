#pragma once

FB_PACKAGE0()

Time::Time()
	: value(0)
{
}


Time Time::fromTicks(int32_t ticks)
{
	return fromTicks(int64_t(ticks));
}


Time Time::fromTicks(uint32_t ticks)
{
	return fromTicks(int64_t(ticks));
}


Time Time::fromTicks(int64_t ticks)
{
	Time t;
	t.setTicks(ticks);
	return t;
}


Time Time::fromTicks(uint64_t ticks)
{
	return fromTicks(int64_t(ticks));
}


Time Time::fromSeconds(double seconds)
{
	Time t;
	t.setSeconds(seconds);
	return t;
}


Time Time::fromSeconds(int32_t seconds)
{
	return fromSeconds(int64_t(seconds));
}


Time Time::fromSeconds(uint32_t seconds)
{
	return fromSeconds(int64_t(seconds));
}


Time Time::fromSeconds(int64_t seconds)
{
	Time t;
	t.setSeconds(seconds);
	return t;
}


Time Time::fromSeconds(uint64_t seconds)
{
	return fromSeconds(int64_t(seconds));
}


Time Time::fromMilliseconds(int32_t milliseconds)
{
	return fromMilliseconds(int64_t(milliseconds));
}


Time Time::fromMilliseconds(uint32_t milliseconds)
{
	return fromMilliseconds(int64_t(milliseconds));
}


Time Time::fromMilliseconds(int64_t milliseconds)
{
	Time t;
	t.setMilliseconds(milliseconds);
	return t;
}


Time Time::fromMilliseconds(uint64_t milliseconds)
{
	return fromMilliseconds(int64_t(milliseconds));
}


void Time::setTicks(int32_t ticks)
{
	value = ticks;
}


void Time::setTicks(uint32_t ticks)
{
	value = ticks;
}


void Time::setTicks(int64_t ticks)
{
	value = ticks;
}


void Time::setTicks(uint64_t ticks)
{
	value = int64_t(ticks);
}


void Time::setSeconds(double seconds)
{
	value = int64_t(seconds * 1000 * ticksInMillisecond);
}


void Time::setSeconds(int32_t seconds)
{
	value = int64_t(seconds) * 1000 * ticksInMillisecond;
}


void Time::setSeconds(uint32_t seconds)
{
	value = int64_t(seconds) * 1000 * ticksInMillisecond;
}


void Time::setSeconds(int64_t seconds)
{
	value = seconds * 1000 * ticksInMillisecond;
}


void Time::setSeconds(uint64_t seconds)
{
	value = int64_t(seconds) * 1000 * ticksInMillisecond;
}


void Time::setMilliseconds(int32_t milliseconds)
{
	value = int64_t(milliseconds) * ticksInMillisecond;
}


void Time::setMilliseconds(uint32_t milliseconds)
{
	value = int64_t(milliseconds) * ticksInMillisecond;
}


void Time::setMilliseconds(int64_t milliseconds)
{
	value = milliseconds * ticksInMillisecond;
}


void Time::setMilliseconds(uint64_t milliseconds)
{
	value = int64_t(milliseconds) * ticksInMillisecond;
}


int64_t Time::getTicks() const
{
	return value;
}


double Time::getSeconds() const
{
	return value / double(1000 * ticksInMillisecond);
}


int64_t Time::getMilliseconds() const
{
	/* This could be done more efficiently (without if), if we presumed two's complement or counted on compiler 
	 * dependent behaviour. */
	if (value < 0)
	{
		uint64_t valueInMilliseconds = uint64_t(-value);
		valueInMilliseconds >>= bitsInMillisecond;
		return -int64_t(valueInMilliseconds);
	}
	else
	{
		return int64_t(uint64_t(value) >> bitsInMillisecond);
	}
}


int64_t Time::getTms() const
{
	return value * 10 / getTicksInMillisecond();
}


float Time::getMillisecondsAsFloat() const
{
	return value / float(ticksInMillisecond);
}


int32_t Time::getMillisecondsAsInt() const
{
	// float precision limit (adding 1.0f no longer increases value)
	int32_t maxValue = (1<<24) - 1;
	return getMilliseconds() % maxValue;
}


Time Time::operator+(const Time &other) const
{
	return fromTicks(value + other.value);
}


Time Time::operator-(const Time &other) const
{
	return fromTicks(value - other.value);
}


Time Time::operator*(int32_t multiplier) const
{
	return fromTicks(value * multiplier);
}


Time Time::operator*(uint32_t multiplier) const
{
	return fromTicks(value * int64_t(multiplier));
}


Time Time::operator*(int64_t multiplier) const
{
	return fromTicks(value * multiplier);
}


Time Time::operator*(uint64_t multiplier) const
{
	return fromTicks(value * int64_t(multiplier));
}


Time Time::operator*(double multiplier) const
{
	return fromTicks(int64_t(value * multiplier));
}


Time Time::operator/(int32_t divider) const
{
	return fromTicks(value / divider);
}


Time Time::operator/(uint32_t divider) const
{
	return fromTicks(value / int64_t(divider));
}


Time Time::operator/(int64_t divider) const
{
	return fromTicks(value / divider);
}


Time Time::operator/(uint64_t divider) const
{
	return fromTicks(value / int64_t(divider));
}


Time Time::operator/(double divider) const
{
	return fromTicks(int64_t(value / divider));
}


double Time::operator/(const Time &other) const
{
	return double(value) / other.value;
}


void Time::operator+=(const Time &other)
{
	value += other.value;
}


void Time::operator-=(const Time &other)
{
	value -= other.value;
}


void Time::operator*=(int32_t multiplier)
{
	value = value * multiplier;
}


void Time::operator*=(uint32_t multiplier)
{
	value = value * int64_t(multiplier);
}


void Time::operator*=(int64_t multiplier)
{
	value = value * multiplier;
}


void Time::operator*=(uint64_t multiplier)
{
	value = value * int64_t(multiplier);
}


void Time::operator*=(double multiplier)
{
	value = int64_t(value * multiplier);
}


void Time::operator/=(int32_t divider)
{
	value = value / divider;
}


void Time::operator/=(uint32_t divider)
{
	value = value / int64_t(divider);
}


void Time::operator/=(int64_t divider)
{
	value = int64_t(value / divider);
}


void Time::operator/=(uint64_t divider)
{
	value = value / int64_t(divider);
}


void Time::operator/=(double divider)
{
	value = int64_t(value / divider);
}


Time Time::operator%(const Time &other) const
{
	return fromTicks(value % other.value);
}


void Time::operator%=(const Time &other)
{
	value = value % other.value;
}


bool Time::operator==(const Time &other) const
{
	return value == other.value;
}


bool Time::operator!=(const Time &other) const
{
	return value != other.value;
}


bool Time::operator<(const Time &other) const
{
	return value < other.value;
}


bool Time::operator>(const Time &other) const
{
	return value > other.value;
}


bool Time::operator<=(const Time &other) const
{
	return value <= other.value;
}


bool Time::operator>=(const Time &other) const
{
	return value >= other.value;
}


Time Time::operator-() const
{
	return fromTicks(-value);
}

FB_END_PACKAGE0()
