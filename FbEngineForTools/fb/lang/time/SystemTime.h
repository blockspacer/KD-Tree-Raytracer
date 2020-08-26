#pragma once

#include "fb/lang/Types.h"
#include "fb/string/StringRef.h"

FB_DECLARE0(HeapString)
FB_DECLARE0(StaticString)
FB_DECLARE0(Time)

FB_PACKAGE0()

/**
 * Class for creating and storing time stamps. Not monotonic, so don't use this for timing purposes. Use one of the 
 * timer classes instead.
 */

class SystemTime
{
	/* Don't use the constructor directly, use the now() instead */
	SystemTime(uint64_t initTimestamp);

public:
	/* Initializes to epoch by default */
	SystemTime();
	SystemTime(const SystemTime &other);

	/* Return the current system time */
	static SystemTime now();
	static SystemTime fromTimestamp(uint64_t timestamp);

	/* Returns the time in seconds since epoch (this is the normal unix timestamp) */
	uint32_t getAsLegacyTimestamp() const;
	/* Returns the time in milliseconds since epoch, instead of seconds like the legacy timestamp */
	uint64_t getAsTimestamp() const;
	/* Returns the time since epoch as FB Time */
	Time getTime() const;

	/* Standard separators for ISO 8601 format */
	static const StringRef &getStandardDateSeparator();
	static const StringRef &getStandardTimeSeparator();
	static const StringRef &getStandardMillisecondSeparator();
	static const StringRef &getStandardDateTimeSeparator();
	/* Adds by default ISO 8601 timestamp to string (YYYY-MM-DDTHH:MM:SS). Optionally adds milliseconds (.XXX by
	 * default) and custom separators */
	void addCustomTimeStampToString(HeapString &target, bool includeYears, bool includeMonths, 
		bool includeDays, bool includeHours, bool includeMinutes, bool includeSeconds, bool includeMilliseconds, 
		const StringRef &dateSeparator = getStandardDateSeparator(),
		const StringRef &timeSeparator = getStandardTimeSeparator(),
		const StringRef &millisecondSeparator = getStandardMillisecondSeparator(),
		const StringRef &dateTimeSeparator = getStandardDateTimeSeparator());

	/* File time stamps don't contain spaces or evil characters. Human interpretable. Good for filenames */
	void addFileTimeStampToString(HeapString &target, bool includeMilliseconds = false)
	{
		addCustomTimeStampToString(target, true, true, true, true, true, true, includeMilliseconds, 
			StringRef(""), StringRef(""), StringRef("_"), StringRef("_"));
	}

	void addTimeOnlyFileTimeStampToString(HeapString &target, bool includeMilliseconds = false)
	{
		addCustomTimeStampToString(target, false, false, false, true, true, true, includeMilliseconds, 
			StringRef(""), StringRef(""), StringRef("_"), StringRef("_"));
	}

	void addDateOnlyFileTimeStampToString(HeapString &target)
	{
		addCustomTimeStampToString(target, true, true, true, false, false, false, false, 
			StringRef(""), StringRef(""), StringRef("_"), StringRef("_"));
	}

	/* Human readable time stamps contain spaces and evil characters. Easy to read */
	void addHumanReadableTimeStampToString(HeapString &target, bool includeMilliseconds = false)
	{
		addCustomTimeStampToString(target, true, true, true, true, true, true, includeMilliseconds, 
			StringRef("-"), StringRef(":"), StringRef("."), StringRef(" "));
	}

	void addTimeOnlyHumanReadableTimeStampToString(HeapString &target, bool includeMilliseconds = false)
	{
		addCustomTimeStampToString(target, false, false, false, true, true, true, includeMilliseconds, 
			StringRef("-"), StringRef(":"), StringRef("."), StringRef(" "));
	}

	void addDateOnlyHumanReadableTimeStampToString(HeapString &target)
	{
		addCustomTimeStampToString(target, true, true, true, false, false, false, false, 
			StringRef("-"), StringRef(":"), StringRef("."), StringRef(" "));
	}

private:
	uint64_t rawTime = 0;
};

FB_END_PACKAGE0()
