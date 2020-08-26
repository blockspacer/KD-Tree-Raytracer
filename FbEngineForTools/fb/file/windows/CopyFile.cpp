#include "Precompiled.h"

#include "fb/file/CopyFile.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/util/UnicodeConverter.h"

#include <direct.h>

FB_PACKAGE1(file)

bool copyFile(const StringRef &oldFile, const StringRef &newFile)
{
	string::SimpleUTF16String oldFileWide;
	oldFileWide.appendUTF8(oldFile);
	string::SimpleUTF16String newFileWide;
	newFileWide.appendUTF8(newFile);
	BOOL result = CopyFileExW(oldFileWide.getPointer(), newFileWide.getPointer(), nullptr, nullptr, nullptr, 0);
	if (result == 0 && FB_SYS_LOGGER_ENABLED == FB_TRUE)
	{
		TempString msg;
		msg << "Failed to copy file " << oldFile << " to " << newFile << ". ";
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
	}
	return result != 0;
}

bool hardLinkFile(const StringRef &sourceFile, const StringRef &targetFile)
{
	string::SimpleUTF16String sourceFileWide;
	sourceFileWide.appendUTF8(sourceFile);
	string::SimpleUTF16String targetFileWide;
	targetFileWide.appendUTF8(targetFile);
	BOOL result = CreateHardLinkW(targetFileWide.getPointer(), sourceFileWide.getPointer(), nullptr);
	if (result == 0 && FB_SYS_LOGGER_ENABLED == FB_TRUE)
	{
		TempString msg;
		msg << "Failed to hard link file " << sourceFile << " to " << targetFile << ". ";
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
	}
	return result != 0;
}

FB_END_PACKAGE1()
