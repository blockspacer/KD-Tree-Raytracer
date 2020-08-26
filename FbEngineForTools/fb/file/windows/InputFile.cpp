#include "Precompiled.h"

#include "fb/file/InputFile.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/FBAssert.h"
#include "fb/profiling/ScopedProfiler.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)

InputFile::InputFile()
	: file((FILE*)INVALID_HANDLE_VALUE)
{
}

InputFile::~InputFile()
{
	close();
}

bool InputFile::open(const StringRef &fileToOpen)
{
	close();
	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(fileToOpen);
	HANDLE h = CreateFileW(fileWide.getPointer(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	file = (FILE*)h;
	if (h == INVALID_HANDLE_VALUE)
	{
		DWORD sysErrCode = GetLastError();
		// ignore the possibly expected not found cases (such should not really occur, but in reality...)
		if (sysErrCode != ERROR_FILE_NOT_FOUND && sysErrCode != ERROR_PATH_NOT_FOUND && sysErrCode != ERROR_INVALID_NAME)
		{
			/* InputFile is used as a test ("can I open this file"), so no assert here */
			//fb_assertf(false, "Error occurred during file open : %s", filename);
		}
	}
	if (h == INVALID_HANDLE_VALUE)
		return false;
#if FB_BUILD != FB_FINAL_RELEASE
	this->fileName = fileToOpen;
#endif
	return true;
}

void InputFile::close()
{
#if FB_BUILD != FB_FINAL_RELEASE
	fileName.clear();
#endif
	HANDLE h = (HANDLE)file;
	if (h != INVALID_HANDLE_VALUE)
	{
		CloseHandle(h);
		file = (FILE*)INVALID_HANDLE_VALUE;
	}
}

BigSizeType InputFile::readData(void *buffer, BigSizeType size)
{
	HANDLE h = (HANDLE)file;
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD readBytes = 0;
		BOOL returnValue = ReadFile(h, buffer, (DWORD)size, &readBytes, 0);

		fb_assert(readBytes == size);
		return readBytes;
	}
	return 0;
}

BigSizeType InputFile::readDataWithOffset(BigSizeType offset, void *buffer, BigSizeType size) const
{
	HANDLE h = (HANDLE)file;
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD readBytes = 0;

		OVERLAPPED overlapped;
		overlapped.Offset = (DWORD)(offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = 0;
		#if (FB_PLATFORM_BITS == 64)
			overlapped.OffsetHigh = (DWORD)((offset >> 32) & 0xFFFFFFFF);
		#endif
		overlapped.hEvent = nullptr;

		BOOL returnValue = ReadFile(h, buffer, (DWORD)size, &readBytes, &overlapped);

		fb_assert(readBytes == size);
		return readBytes;
	}
	return 0;
}

BigSizeType InputFile::getSize() const
{
	HANDLE h = (HANDLE)file;
	if (h != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fs;
		if (!GetFileSizeEx(h, &fs))
			return 0;

		return BigSizeType(fs.QuadPart);
	}
	return 0;
}

BigSizeType InputFile::getPosition() const
{
	HANDLE h = (HANDLE)file;
	if (h != INVALID_HANDLE_VALUE)
	{
		LONG distanceToMove = 0;
		LONG distanceToMoveHigh = 0;
		DWORD result = SetFilePointer(h, distanceToMove, &distanceToMoveHigh, FILE_CURRENT);
		if (result == 0xFFFFFFFF && GetLastError() != NO_ERROR)
		{
			// ERROR
		}
		else
		{
			fb_assert(sizeof(BigSizeType) == 8);
			return ((BigSizeType)distanceToMoveHigh << 16) | (BigSizeType)result;
		}
	}
	return 0;
}

void InputFile::seek(BigSizeType pos, SeekMode mode)
{
	HANDLE h = (HANDLE)file;
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD m = FILE_BEGIN;
		if (mode == SeekModeCur)
			m = FILE_CURRENT;
		else if (mode == SeekModeEnd)
			m = FILE_END;

		DWORD result;
		fb_assert(sizeof(BigSizeType) == 8);
		LONG distanceToMove = (LONG)(pos & 0xFFFFFFFF);
		LONG distanceToMoveHigh = (LONG)(pos >> 32);
		result = SetFilePointer(h, distanceToMove, &distanceToMoveHigh, m);

		if (result == 0xFFFFFFFF && GetLastError() != NO_ERROR)
		{
			// ERROR
		}
	}
}

FB_END_PACKAGE1()
