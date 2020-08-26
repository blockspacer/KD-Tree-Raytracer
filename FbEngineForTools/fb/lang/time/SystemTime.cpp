#include "Precompiled.h"
#include "SystemTime.h"

#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/time/Time.h"
#include "fb/string/HeapString.h"

#if FB_VS2017_IN_USE == FB_TRUE
	#pragma warning(push)
	/* 'argument': conversion from 'const int64_t' to 'const uint64_t', signed/unsigned mismatch */
	#pragma warning( disable: 4365 )
#endif

#include <chrono>
#include <ctime>

#if FB_VS2017_IN_USE == FB_TRUE
	#pragma warning(pop)
#endif

FB_PACKAGE0()

static bool initializeIfNecessary()
{
	return true;
}

/* Make sure initialize is called before any threading commences */
static const bool initializeCalled = initializeIfNecessary();

SystemTime::SystemTime(uint64_t initTimestamp)
	: rawTime(initTimestamp)
{
}


SystemTime::SystemTime()
	: rawTime(0)
{
}


SystemTime::SystemTime(const SystemTime &other)
	: rawTime(other.rawTime)
{
}


SystemTime SystemTime::now()
{
	initializeIfNecessary();
	/* Note: We are saving milliseconds, but it would be trivial to save microseconds or nanoseconds instead */
	std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now();
	std::chrono::duration<uint64_t, std::milli> rawTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
	return SystemTime(rawTime.count());
}


SystemTime SystemTime::fromTimestamp(uint64_t timestamp)
{
	initializeIfNecessary();
	return SystemTime(timestamp);
}


uint32_t SystemTime::getAsLegacyTimestamp() const
{
	// making sure we don't warp back in time due to int64->32 conversion (around 2038 ;) )
	fb_expensive_assert(uint64_t(rawTime / 1000) == uint64_t(uint32_t(rawTime / 1000)));

	return uint32_t(rawTime / 1000);
}


uint64_t SystemTime::getAsTimestamp() const
{
	return rawTime;
}


Time SystemTime::getTime() const
{
	return Time::fromMilliseconds(rawTime);
}


const StringRef &SystemTime::getStandardDateSeparator()
{
	static StringRef str("-");
	return str;
}


const StringRef &SystemTime::getStandardTimeSeparator()
{
	static StringRef str(":");
	return str;
}


const StringRef &SystemTime::getStandardMillisecondSeparator()
{
	static StringRef str(".");
	return str;
}


const StringRef &SystemTime::getStandardDateTimeSeparator()
{
	static StringRef str("T");
	return str;
}


void SystemTime::addCustomTimeStampToString(HeapString &target, bool includeYears, bool includeMonths,
	bool includeDays, bool includeHours, bool includeMinutes, bool includeSeconds, bool includeMilliseconds, 
	const StringRef &dateSeparator, const StringRef &timeSeparator,
	const StringRef &millisecondSeparator, const StringRef &dateTimeSeparator)
{
	/* Note: This should be done differently on NX, but since this way seems to work just fine, I'll let it be */
	std::chrono::time_point<std::chrono::system_clock> epoch;
	std::chrono::duration<uint64_t, std::milli> rawDuration(rawTime);
	std::chrono::time_point<std::chrono::system_clock> timeStamp = epoch + rawDuration;
	std::time_t timeStampTime = std::chrono::system_clock::to_time_t(timeStamp);
	/* Note: std::localtime may not be thread-safe. It is on MSC, but doesn't seem to be e.g. on PS4. So we are 
	 * presuming here that this is the only place where std::localtime is used and mutex here. We could use some 
	 * platform specific alternatives, but frankly, taking an extra mutex per timestamp printing shouldn't be a huge 
	 * issue */
	std::tm calendarTimeStamp;
	{
		std::tm *tmpTm = std::localtime(&timeStampTime);
		/* This should only fail, if given time_t is broken, and I'm not sure how even in that case. */
		fb_assert(tmpTm != nullptr);
		calendarTimeStamp = *tmpTm;
	}

	TempString formatStr;
	if (includeYears)
		formatStr << "%Y" << (includeMonths || includeDays ? dateSeparator : "");

	if (includeMonths)
		formatStr << "%m" << (includeDays ? dateSeparator : "");
	
	if (includeDays)
		formatStr << "%d" << (includeHours || includeMinutes || includeSeconds ? dateTimeSeparator : "");
	
	if (includeHours)
		formatStr << "%H" << (includeMinutes || includeSeconds ? timeSeparator : "");
	
	if (includeMinutes)
		formatStr << "%M" << (includeSeconds ? timeSeparator : "");
	
	if (includeSeconds)
		formatStr << "%S";

	char tmpTarget[128] = { 0 };
	std::strftime(tmpTarget, 127, formatStr.getPointer(), &calendarTimeStamp);
	target << tmpTarget;

	if (includeMilliseconds)
	{
		target << millisecondSeparator;
		uint64_t milliseconds = rawTime % 1000;
		if (milliseconds < 100)
			target << "0";

		if (milliseconds < 10)
			target << "0";

		target << milliseconds;
	}
}

FB_END_PACKAGE0()
