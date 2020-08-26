#include "Precompiled.h"

#include "fb/lang/FBPrintf.h"
#include "fb/lang/thread/Semaphore.h"
#include "fb/lang/IncludeWindows.h"

FB_PACKAGE0()

struct Semaphore::SemaphoreImpl
{
	SemaphoreImpl(uint32_t initialValue, uint32_t debugMaxValue)
		: semaphore(INVALID_HANDLE_VALUE)
	{
		uint32_t maxValue = debugMaxValue != 0 ? debugMaxValue : 0xFFFFF;

		// Name parameter is not some debug value - see MSDN.
		// Basically it will merge different semaphores in different processes...
		const char* name = NULL;
		/* default security attributes, initial count, maximum count, name */
		semaphore = CreateSemaphore(NULL, LONG(initialValue), LONG(maxValue), name);
		fb_assert(semaphore != NULL);
	}

	~SemaphoreImpl()
	{
		CloseHandle(semaphore);
	}

	HANDLE semaphore;
};


Semaphore::Semaphore(uint32_t initialValue, uint32_t debugMaxValue)
	: impl(new SemaphoreImpl(initialValue, debugMaxValue))
{

}

Semaphore::~Semaphore()
{
	delete impl;
}

void Semaphore::wait()
{
	WaitForSingleObject(impl->semaphore, INFINITE);
}

bool Semaphore::waitFor(SizeType milliseconds)
{
	return WaitForSingleObject(impl->semaphore, milliseconds) != WAIT_TIMEOUT;
}

void Semaphore::post(uint32_t postCount)
{
	LONG previousValue = 0;
	BOOL result = ReleaseSemaphore(impl->semaphore, (int)postCount, &previousValue);
	fb_assert(result && "Release semaphore failed");
#if FB_BUILD != FB_FINAL_RELEASE
	if (!result)
	{
		FB_PRINTF("Release semaphore failed with error %d (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381%28v=vs.85%29.aspx)", GetLastError());
	}
#endif
}

FB_END_PACKAGE0()