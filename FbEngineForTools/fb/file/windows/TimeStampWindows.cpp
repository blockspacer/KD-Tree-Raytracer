#include "Precompiled.h"

#include "fb/file/TimeStamp.h"
#include "fb/lang/AppendCallstack.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/util/CreateTemporaryHeapString.h"
#include "fb/string/util/UnicodeConverter.h"

#include <time.h>

FB_PACKAGE1(file)

bool fileIsZeroSize(const StringRef &file)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);
	if (GetFileAttributesExW(fileWide.getPointer(), GetFileExInfoStandard, &data))
	{
		if (data.nFileSizeLow == 0 && data.nFileSizeHigh == 0)
			return true;
	}
	else
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to determine file time stamp for ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return false;
	}
	return false;
}

TimeStamp64 getFileTimeFromTimestamp(uint64_t timestamp)
{
	return ((int64_t)timestamp + 11644473600000LL) * 10000;
}

uint64_t getTimestampFromFileTime(TimeStamp64 fileTime)
{
	return (uint64_t)fileTime / 10000 - 11644473600000ULL;
}

TimeStamp getFileTimestamp(const StringRef &file)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);
	if (GetFileAttributesExW(fileWide.getPointer(), GetFileExInfoStandard, &data))
	{
		if (data.nFileSizeLow == 0 && data.nFileSizeHigh == 0)
			return -1;

		SYSTEMTIME stUTC, stLocal;
		FileTimeToSystemTime(&data.ftLastWriteTime, &stUTC);
		SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);

		TIME_ZONE_INFORMATION tzi = { 0 };
		GetTimeZoneInformation(&tzi);

		tm timeinfo;
		memset(&timeinfo, 0, sizeof(tm));

		timeinfo.tm_year = stLocal.wYear - 1900;
		timeinfo.tm_mon = stLocal.wMonth - 1;
		timeinfo.tm_mday = stLocal.wDay;
		timeinfo.tm_sec = stLocal.wSecond;
		timeinfo.tm_min = stLocal.wMinute;
		timeinfo.tm_hour = stLocal.wHour;
		timeinfo.tm_isdst = tzi.DaylightBias ? 1 : 0;

		time_t result = mktime(&timeinfo);
		return TimeStamp(result);
	}
	else
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to determine file time stamp for ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
	}

	return -1;
}

TimeStamp64 getFileTimestamp64(const StringRef &file)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);
	if (GetFileAttributesExW(fileWide.getPointer(), GetFileExInfoStandard, &data))
	{
		if (data.nFileSizeLow == 0 && data.nFileSizeHigh == 0)
			return -1;

		FILETIME ft = data.ftLastWriteTime;
		return (TimeStamp64(ft.dwHighDateTime) << 32) | TimeStamp64(ft.dwLowDateTime);
	}
	else
	{
		#if FB_BUILD != FB_FINAL_RELEASE 
			TempString msg(FB_MSG("Failed to determine file time stamp for ", file, ". "));
			if (GetLastError() == ERROR_ACCESS_DENIED)
			{
				DebugHelp::addGetLastErrorCode(msg);
				#if FB_ENGINE_FOR_TOOLS == FB_FALSE
					msg << ". Called from: ";
					debugAppendToString(msg, AppendSingleRowCallstack());
					msg << ". Information about processes using the file: ";
					DebugHelp::addWhoHoldsFileOpenInfo(msg, file);
				#endif
			}
			else
			{
				DebugHelp::addGetLastErrorCode(msg);
			}
			FB_LOG_ERROR(msg);
		#endif
	}

	return -1;
}

bool isFileNewerThanFile(const StringRef &file, const StringRef &thanFile)
{
	TimeStamp64 fileT = getFileTimestamp64(file);
	TimeStamp64 thanFileT = getFileTimestamp64(thanFile);
	return isFileNewerThanFile(fileT, thanFileT);
}

void setFileTimestampToMatch(const StringRef &file, const StringRef &stampFile)
{
	WIN32_FILE_ATTRIBUTE_DATA stampData;

	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);
	string::SimpleUTF16String stampFileWide;
	stampFileWide.appendUTF8(stampFile);

	if (!GetFileAttributesExW(stampFileWide.getPointer(), GetFileExInfoStandard, &stampData))
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to determine file time stamp for ", stampFile, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return;
	}

	HANDLE fileHandle = CreateFileW(fileWide.getPointer(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to set file time (CreateFileW) for ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return;
	}

	if (SetFileTime(fileHandle, nullptr, &stampData.ftLastWriteTime, &stampData.ftLastWriteTime) == 0)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to set file time (SetFileTime) for ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return;
	}

	if (CloseHandle(fileHandle) == 0)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to close handle for file ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
	}
}

void setFileTimestamp(const StringRef &file, TimeStamp64 stamp)
{
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);

	HANDLE fileHandle = CreateFileW(fileWide.getPointer(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to set file time (CreateFileW) for ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return;
	}

	FILETIME fileTime;
	fileTime.dwLowDateTime = (uint32_t)stamp;
	fileTime.dwHighDateTime = (uint32_t)(stamp >> 32);
	if (SetFileTime(fileHandle, nullptr, &fileTime, &fileTime) == 0)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to set file time (SetFileTime) for ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return;
	}
	if (CloseHandle(fileHandle) == 0)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to close handle for file ", file, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
	}
}

TimeStamp64 getCurrentTimeStamp()
{
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);

	FILETIME fileTime;
	if (!SystemTimeToFileTime(&systemTime, &fileTime))
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		TempString msg(FB_MSG("Failed to convert SYSTEMTIME to FILETIME. "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
#endif
		return TimeStamp64(-1);
	}

	return (TimeStamp64(fileTime.dwHighDateTime) << 32) | TimeStamp64(fileTime.dwLowDateTime);
}

FB_END_PACKAGE1()
