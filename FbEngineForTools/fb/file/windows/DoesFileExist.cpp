#include "Precompiled.h"

#include "fb/file/DoesFileExist.h"
#include "fb/lang/ScopedArray.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)


bool isFileFolder(const StringRef &file)
{
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);
	DWORD attr = GetFileAttributesW(fileWide.getPointer());

	if (attr == INVALID_FILE_ATTRIBUTES)
		return false;

	if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return true;

	return false;
}

bool doesFileExist(const StringRef &file, bool allowDirs, bool caseSensitive)
{
	FB_ZONE("file::doesFileExist");

	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(file);
	DWORD attr = GetFileAttributesW(fileWide.getPointer());

	if (attr == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!allowDirs && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return false;
	
	if (caseSensitive)
	{
		WIN32_FIND_DATAW findFileData;
		HANDLE handle = FindFirstFileW(fileWide.getPointer(), &findFileData);
		if (handle == INVALID_HANDLE_VALUE)
			return false;
		
		const wchar_t *fileWithoutPath = wcsrchr(fileWide.getPointer(), L'\\');
		if (!fileWithoutPath)
			fileWithoutPath = wcsrchr(fileWide.getPointer(), L'/');

		if (!fileWithoutPath)
			fileWithoutPath = fileWide.getPointer();
		else
			fileWithoutPath++;

		const wchar_t *filename = findFileData.cFileName;
		bool doesFileCaseMatch = wcscmp(filename, fileWithoutPath) == 0;
		FindClose(handle);

		if (!doesFileCaseMatch)
			return false;
	}

	return true;
}

FB_END_PACKAGE1()
