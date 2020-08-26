#pragma once

FB_PACKAGE0()

class Time
{
public:
	inline Time();
	/* For Lua, also allows initialization with zero */
	Time(const char *str);

	/* Constructor like static methods */
	/* Int32 versions are to fix problems with ambiguous casts from plain numbers (ints) in code */
	static inline Time fromTicks(int32_t ticks);
	static inline Time fromTicks(uint32_t ticks);
	static inline Time fromTicks(int64_t ticks);
	static inline Time fromTicks(uint64_t ticks);
	static inline Time fromSeconds(double seconds);
	static inline Time fromSeconds(int32_t seconds);
	static inline Time fromSeconds(uint32_t seconds);
	static inline Time fromSeconds(int64_t seconds);
	static inline Time fromSeconds(uint64_t seconds);
	static inline Time fromMilliseconds(int32_t milliseconds);
	static inline Time fromMilliseconds(uint32_t milliseconds);
	static inline Time fromMilliseconds(int64_t milliseconds);
	static inline Time fromMilliseconds(uint64_t milliseconds);

	/* Setters */
	inline void setTicks(int32_t ticks);
	inline void setTicks(uint32_t ticks);
	inline void setTicks(int64_t ticks);
	inline void setTicks(uint64_t ticks);
	inline void setSeconds(double seconds);
	inline void setSeconds(int32_t seconds);
	inline void setSeconds(uint32_t seconds);
	inline void setSeconds(int64_t seconds);
	inline void setSeconds(uint64_t seconds);
	inline void setMilliseconds(int32_t milliseconds);
	inline void setMilliseconds(uint32_t milliseconds);
	inline void setMilliseconds(int64_t milliseconds);
	inline void setMilliseconds(uint64_t milliseconds);

	/**
	 * Returns highest precision time in ticks. Note: you should only use this if absolutely needed. Use getTicksInMillisecond() to get precise time in human readable format.
	 */
	inline int64_t getTicks() const;

	/**
	 * Returns high precision time in seconds. Note: you should not use this
	 */
	inline double getSeconds() const;

	/**
	 * Returns high precision time in milliseconds.
	 */
	inline int64_t getMilliseconds() const;

	/**
	 * Returns high precision time in tenths of milliseconds.
	 */
	inline int64_t getTms() const;

	/**
	 * Returns low precision time in milliseconds. Note: you should not use this
	 */
	inline float getMillisecondsAsFloat() const;

	/**
	 * Returns time in seconds which wraps around if value is too large (around 4.5 hours). You should not use this.
	 */
	float getSecondsAsFloat() const;

	/**
	 * Returns rounded time in milliseconds which wraps around if value is too large (around 4.5 hours). Returned value is also safe to use as a float.
	 */
	inline int32_t getMillisecondsAsInt() const;

	/**
	 * Returns rounded time in milliseconds which wraps around if value is too large (around 24 days). Returned value is NOT safe to use as a float!
	 */
	int32_t getMillisecondsAsUnsafeInt() const;

	/* Operators. Note that division and multiplier operators are special, as time has a unit */
	inline Time operator+(const Time &other) const;
	inline Time operator-(const Time &other) const;
	inline Time operator*(int32_t multiplier) const;
	inline Time operator*(uint32_t multiplier) const;
	inline Time operator*(int64_t multiplier) const;
	inline Time operator*(uint64_t multiplier) const;
	inline Time operator*(double multiplier) const;
	inline Time operator/(int32_t divider) const;
	inline Time operator/(uint32_t divider) const;
	inline Time operator/(int64_t divider) const;
	inline Time operator/(uint64_t divider) const;
	inline Time operator/(double divider) const;
	inline double operator/(const Time &other) const;

	inline void operator+=(const Time &other);
	inline void operator-=(const Time &other);
	inline void operator*=(int32_t multiplier);
	inline void operator*=(uint32_t multiplier);
	inline void operator*=(int64_t multiplier);
	inline void operator*=(uint64_t multiplier);
	inline void operator*=(double multiplier);
	inline void operator/=(int32_t divider);
	inline void operator/=(uint32_t divider);
	inline void operator/=(int64_t divider);
	inline void operator/=(uint64_t divider);
	inline void operator/=(double divider);

	inline bool operator==(const Time &other) const;
	inline bool operator!=(const Time &other) const;
	
	inline bool operator<(const Time &other) const;
	inline bool operator<=(const Time &other) const;
	inline bool operator>(const Time &other) const;
	inline bool operator>=(const Time &other) const;
	
	inline Time operator%(const Time &other) const;
	inline void operator%=(const Time &other);
	
	inline Time operator-() const;
	
	static const Time zero;
	/* This is not technically infinite time, but longest time that can be expressed */
	static const Time infinity;

	static uint32_t getBitsInMillisecond() { return bitsInMillisecond; }
	static int64_t getTicksInMillisecond() { return ticksInMillisecond; }
	
private:
	static const uint32_t bitsInMillisecond = 10;
	static const int64_t ticksInMillisecond = int64_t(1U << bitsInMillisecond);
	int64_t value;
};


// a dummy for ScalableTimer...
inline Time timeFromMilliseconds(Time milliseconds) { return milliseconds; }

FB_END_PACKAGE0()

#include "TimeInline.h"