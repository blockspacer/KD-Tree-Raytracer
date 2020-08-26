#include "Precompiled.h"

#include "fb/file/TemporaryFile.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)

bool getTemporaryFilename(HeapString &outFilename)
{
	// Scrap that, files will be generated to the current directory, with .TMP extension

	wchar_t temp_buffer[MAX_PATH];
	UINT result = GetTempFileNameW(L".", L"fb", 0, temp_buffer);
	if (!result)
		return false;

	outFilename.clear();
	string::UnicodeConverter::addUTF16StrToUTF8String(temp_buffer, outFilename);

	return true;
}

FB_END_PACKAGE1()
