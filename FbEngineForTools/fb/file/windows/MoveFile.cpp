#include "Precompiled.h"

#include "fb/file/MoveFile.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)

bool moveFile(const StringRef &oldFile, const StringRef &newFile, bool logErrors)
{
	DWORD flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH;

	string::SimpleUTF16String oldFileWide;
	oldFileWide.appendUTF8(oldFile);
	string::SimpleUTF16String newFileWide;
	newFileWide.appendUTF8(newFile);
	if (MoveFileExW(oldFileWide.getPointer(), newFileWide.getPointer(), flags) == 0)
	{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
		if (logErrors)
		{
			DWORD errorCode = GetLastError();
			TempString msg;
			msg << "Failed to move file " << oldFile << " to " << newFile << ". Error code was " << uint32_t(errorCode);
			FB_LOG_ERROR(msg);
		}
#endif
		return false;
	}
	return true;
}

FB_END_PACKAGE1()