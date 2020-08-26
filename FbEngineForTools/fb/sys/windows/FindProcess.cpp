#include "Precompiled.h"

#include "FindProcess.h"

#include "fb/lang/IncludeWindows.h"
#include "fb/sys/util/LibraryLoaderUtil.h"

FB_PACKAGE2(sys, windows)

static TempString getNameFromFileName(const StringRef &filename)
{
	TempString tmp(filename);
	tmp.replace("\\", "/");
	return TempString().appendFileNameFromString(tmp);
}

static HeapString getCurrentProcessNameImpl()
{
	char moduleFilename[MAX_PATH];
	GetModuleFileName(nullptr, moduleFilename, MAX_PATH);
	return HeapString(moduleFilename);
}

const char *getCurrentProcessName()
{
	static HeapString result(getCurrentProcessNameImpl());
	return result.getPointer();
}

static SizeType openProcessInstanceIndexMutex()
{
	// find first free global mutex
	TempString processName = getNameFromFileName(StringRef(getCurrentProcessName()));
	for (SizeType processIndex = 0; processIndex < 10000; processIndex++)
	{
		TempString mutexName;
		mutexName << "Global\\" << processName << "_" << processIndex;
		HANDLE processIndexMutex = CreateMutexA(nullptr, TRUE, mutexName.getPointer());
		DWORD lastError = GetLastError();
		if (processIndexMutex && lastError != ERROR_ALREADY_EXISTS)
		{
			return processIndex;
		}
	}
	return 0;

}

SizeType getProcessInstanceIndex()
{
	static SizeType result = openProcessInstanceIndexMutex();
	return result;
}

FB_END_PACKAGE2()
