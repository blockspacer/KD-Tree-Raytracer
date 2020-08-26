#include "Precompiled.h"

#include "fb/file/DeleteFile.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)
	

static bool deleteFile(const StringRef &filename, bool errorIfNotExists, bool suppressAllErrorMessages)
{
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(filename);
	if (DeleteFileW(fileWide.getPointer()) == 0)
	{
		DWORD errorCode = GetLastError();
		if (errorCode == ERROR_ACCESS_DENIED)
		{
			// remove read-only attribute first
			SetFileAttributesW(fileWide.getPointer(), GetFileAttributesW(fileWide.getPointer()) & ~FILE_ATTRIBUTE_READONLY);
			if (DeleteFileW(fileWide.getPointer()))
				return true;
		}
		if (!errorIfNotExists && (errorCode == ERROR_FILE_NOT_FOUND || errorCode == ERROR_PATH_NOT_FOUND))
			return true;

#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		if (!suppressAllErrorMessages)
		{
			TempString msg;
			msg << "Failed to delete file " << filename << ". ";
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
		}
#endif
		return false;
	}
	return true;
}


bool deleteFile(const StringRef &filename, bool suppressErrorMessages)
{
	return deleteFile(filename, true, suppressErrorMessages);
}

bool deleteFileIfExists(const StringRef &filename, bool suppressErrorMessages)
{
	return deleteFile(filename, false, suppressErrorMessages);
}

FB_END_PACKAGE1()
