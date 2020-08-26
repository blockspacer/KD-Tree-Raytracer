#include "Precompiled.h"

#include "fb/file/DeleteDirectory.h"

#include "fb/file/DeleteFile.h"
#include "fb/file/FileList.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/string/HeapString.h"
#include "fb/string/util/UnicodeConverter.h"
#include "fb/string/util/CreateTemporaryHeapString.h"
#include "fb/lang/logger/GlobalLogger.h"

FB_PACKAGE1(file)


bool deleteEmptyDirectory(const StringRef &dirname)
{
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(dirname);
	if (RemoveDirectoryW(fileWide.getPointer()) == 0)
	{
		TempString msg(FB_MSG("Failed to remove directory ", dirname, ". "));
		DebugHelp::addGetLastErrorCode(msg);
		FB_LOG_ERROR(msg);
		return false;
	}
	return true;
}


bool deleteDirectoryAndFiles(const StringRef &dirname, HeapString &errorStr)
{
	FileList files(dirname, FileList::FlagIncludeSpecialDirs);
	FileList::Entry* file = files.next();
	bool errors = false;
	while (file != nullptr)
	{
		TempString filename(dirname);
		filename += filename.doesEndWith("/") ? "" : "/";
		filename += file->name;
		if (file->directory)
		{
			if (!deleteDirectoryAndFiles(filename, errorStr))
				errors = true;
		}
		else
		{
			if (!deleteFile(filename))
				errorStr << "Unable to delete file " << filename << "\n";
		}
		file = files.next();
	}
	return deleteEmptyDirectory(dirname) && !errors;
}

FB_END_PACKAGE1()
