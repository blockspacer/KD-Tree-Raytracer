#include "Precompiled.h"

#include "fb/file/CreateDirectory.h"

#include "fb/file/DoesFileExist.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/string/util/UnicodeConverter.h"
#include "fb/string/util/CreateTemporaryHeapString.h"
#include "fb/lang/logger/GlobalLogger.h"

FB_PACKAGE1(file)

bool createDirectory(const StringRef &dir, bool errorIfAlreadyExists)
{
	if (!errorIfAlreadyExists)
	{
		if (file::doesFileExist(dir, true) && file::isFileFolder(dir))
			return true;
	}
	string::SimpleUTF16String dirWide;
	dirWide.appendUTF8(dir);
	SetLastError(0);
	BOOL result = CreateDirectoryW(dirWide.getPointer(), nullptr);
	if (result == 0)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			/* Make sure it is directory */
			DWORD attribs = GetFileAttributesW(dirWide.getPointer());
			if (attribs != INVALID_FILE_ATTRIBUTES)
			{
				if ((attribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					/* It is not a directory */
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
					TempString msg(FB_MSG("Failed to create directory ", dir, ". File with such name already exists"));
					FB_LOG_ERROR(msg);
#endif
					return false;
				}
			}
			else
			{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
				TempString msg(FB_MSG("Failed to get attributes of presumable created directory ", dir, ". "));
				DebugHelp::addGetLastErrorCode(msg);
				FB_LOG_ERROR(msg);
#endif
				return false;
			}

			if (errorIfAlreadyExists)
			{
				FB_LOG_ERROR(FB_MSG("Failed to create directory ", dir, ", because it already exits"));
				return false;
			}
		}
		else
		{
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
			TempString msg(FB_MSG("Failed to create directory ", dir, ". "));
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
#endif
			return false;
		}
	}
	return true;
}

FB_END_PACKAGE1()
