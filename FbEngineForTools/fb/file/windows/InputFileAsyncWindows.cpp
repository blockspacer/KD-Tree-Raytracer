#include "Precompiled.h"

#include "InputFileAsyncWindows.h"

#include "fb/lang/IncludeWindows.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)

InputFileAsync::InputFileAsync()
	: fileHandle(INVALID_HANDLE_VALUE)
	, overlappedBuffer(0)
	, readPending(false)
	, readBytes(-1)
{
}

InputFileAsync::~InputFileAsync()
{
	close();
}

bool InputFileAsync::open(const StringRef &filename)
{
	fb_assert(fileHandle == INVALID_HANDLE_VALUE);

	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(filename);
	fileHandle = CreateFileW(fileWide.getPointer(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	overlappedBuffer = new char[sizeof(OVERLAPPED)];

	return fileHandle != INVALID_HANDLE_VALUE;
}

void InputFileAsync::close()
{
	delete[] overlappedBuffer;
	overlappedBuffer = 0;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fileHandle);
		fileHandle = INVALID_HANDLE_VALUE;
	}
}

InputFileAsync::SizeType InputFileAsync::getSize() const
{
	LARGE_INTEGER fs;
	if (!GetFileSizeEx(fileHandle, &fs))
		return 0;

	return SizeType(fs.QuadPart);
}

bool InputFileAsync::startReadingWithOffset(SizeType offset, void *buffer, SizeType size)
{
	if (fileHandle == INVALID_HANDLE_VALUE)
		return false;

	fb_assert(overlappedBuffer);
	OVERLAPPED *overlapped = (OVERLAPPED *) overlappedBuffer;
	ZeroMemory(overlapped, sizeof(OVERLAPPED));
	overlapped->Offset = (DWORD) offset;
	
	readBytes = -1;
	BOOL b = ReadFile(fileHandle, buffer, (DWORD) size, 0, overlapped);
	if (b)
	{
		readPending = true;
		return true;
	}
	else
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			readPending = true;
			return true;
		}
	}

	return false;
}

bool InputFileAsync::anyReadsPending() const
{
	if (fileHandle == INVALID_HANDLE_VALUE)
		return false;

	return readPending;
}

bool InputFileAsync::getReadStatus(SizeType &numberOfBytesRead) const
{
	if (fileHandle == INVALID_HANDLE_VALUE)
		return false;

	fb_assert(!readPending);
	if (readBytes >= 0)
	{
		numberOfBytesRead = (SizeType) readBytes;
		return true;
	}
	
	return false;
}

void InputFileAsync::blockUntilReady()
{
	if (!anyReadsPending())
		return;

	updateResults(true);
	fb_assert(!anyReadsPending());
}

void InputFileAsync::updateResults(bool block)
{
	if (!anyReadsPending())
		return;

	OVERLAPPED *overlapped = (OVERLAPPED *) overlappedBuffer;
	DWORD bytesReceived = 0;
	BOOL result = GetOverlappedResult(fileHandle, overlapped, &bytesReceived, (block) ? TRUE : FALSE);
	if (result)
	{
		readPending = false;
		readBytes = (int32_t) bytesReceived;
	}
}

FB_END_PACKAGE1()
