#include "Precompiled.h"
#include "OutputFile.h"

#include "fb/lang/DebugHelp.h"
#include "fb/lang/FBAssert.h"
#include "fb/lang/NumericLimits.h"
#include "fb/string/HeapString.h"
#include "fb/string/util/UnicodeConverter.h"
#include <stdio.h>

#include "fb/lang/IncludeWindows.h"

FB_PACKAGE1(file)

OutputFile::OutputFile()
	: file((void *)InvalidHandleValue)
{
	fb_static_assert((void *)InvalidHandleValue == INVALID_HANDLE_VALUE);
}

OutputFile::~OutputFile()
{
	close();
}

static FILE *getFilePtr(void *ptr)
{
	return reinterpret_cast<FILE *>(ptr);
}

bool OutputFile::open(const StringRef &fileNameParam, int32_t flags)
{
	close();
#if FB_BUILD != FB_FINAL_RELEASE
	fileName = DynamicString(fileNameParam);
#endif

	string::SimpleUTF16String fileWide;
	fileWide.appendUTF8(fileNameParam);

	DWORD lastError = 0;
	for (int attempts = 0; attempts < 100; attempts++)
	{
		if (flags & FlagAppend)
		{
			file = CreateFileW(fileWide.getPointer(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, 0);
		}
		else
		{
			file = CreateFileW(fileWide.getPointer(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, 0);
		}

		if (file != INVALID_HANDLE_VALUE)
			return true;

		lastError = GetLastError();
		if (lastError == ERROR_ACCESS_DENIED)
		{
			// wait for access
			Sleep(1);
		}
	}
	TempString msg;
	msg << "CreateFileW for " << fileNameParam << " failed. ";
	DebugHelp::addGetLastErrorCode(msg);
	FB_FINAL_LOG_ERROR(msg);

#if FB_BUILD != FB_FINAL_RELEASE
	fileName.clear();
#endif
	return false;
}

void OutputFile::flush()
{
	if (file != INVALID_HANDLE_VALUE)
	{
		if (!FlushFileBuffers(file))
		{
#if FB_BUILD != FB_FINAL_RELEASE
#if FB_SYS_LOGGER_ENABLED == FB_TRUE
			TempString msg("Failed to flush file ");
			msg << fileName;
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
#endif
#endif
		}
	}
}

void OutputFile::close()
{
	if (file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
		file = INVALID_HANDLE_VALUE;
	}

#if FB_BUILD != FB_FINAL_RELEASE
	fileName.clear();
#endif
}

void OutputFile::writeData(const void *buffer, BigSizeType size)
{
	if (!size)
		return;

	FB_UNUSED_NAMED_VAR(BigSizeType, bytesWritten) = tryToWriteData(buffer, size);
	fb_assertf(bytesWritten == size, "%" FB_FSU64 " != %" FB_FSU64, bytesWritten, size);
}

BigSizeType OutputFile::tryToWriteData(const void *buffer, BigSizeType size)
{
	if (file != INVALID_HANDLE_VALUE)
	{
		const BigSizeType maxSize = lang::NumericLimits<SizeType>::getMax();
		const DWORD bytesToWrite = (DWORD)lang::min(size, maxSize);
		DWORD bytesWritten = 0;
		if (!WriteFile(file, buffer, bytesToWrite, &bytesWritten, nullptr))
		{
			FB_PRINTF("WriteFile for %" FB_FSU64 " bytes failed with error %i\n", size, GetLastError());
		}
		return bytesWritten;
	}

	return 0;
}

FB_END_PACKAGE1()
