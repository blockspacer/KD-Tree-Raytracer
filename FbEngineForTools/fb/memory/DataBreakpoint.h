#ifndef FB_MEMORY_DATABREAKPOINT_H
#define FB_MEMORY_DATABREAKPOINT_H

#include "fb/lang/IncludeWindows.h"
#include <process.h>

FB_PACKAGE1(memory)

class DataBreakpoint
{
public:
	DataBreakpoint(void *address)
		: address(address)
	{
		DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &threadToDebug, 0, FALSE, DUPLICATE_SAME_ACCESS);

		unsigned int threadId;
		HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, threadEntry, this, 0, &threadId);
		DWORD status = WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
		
		CloseHandle(threadToDebug);
		address = 0;
	}

	~DataBreakpoint()
	{
	}

	static unsigned int WINAPI threadEntry(void *arg)
	{
		DataBreakpoint *self = (DataBreakpoint *)arg;
		SuspendThread(self->threadToDebug);

		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_CONTROL|CONTEXT_DEBUG_REGISTERS;
		GetThreadContext(self->threadToDebug, &ctx);

		// enable write break point 0 with 4 bytes
#if (FB_PLATFORM_BITS == 32)
		ctx.Dr0 = (DWORD)self->address;
#else
		ctx.Dr0 = (DWORD64)self->address;
#endif
		ctx.Dr7 = (1|(1<<16)|(3<<18));

		SetThreadContext(self->threadToDebug, &ctx);

		ResumeThread(self->threadToDebug);
		return 1;
	}

	HANDLE threadToDebug;
	void *address;
};

FB_END_PACKAGE1()

#endif
