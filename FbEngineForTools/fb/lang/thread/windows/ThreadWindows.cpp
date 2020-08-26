#include "Precompiled.h"

#include "fb/lang/thread/Thread.h"

#include "fb/lang/CallStack.h"
#include "fb/lang/thread/IThreadEntry.h"
#include "fb/profiling/ZoneProfiler.h"

unsigned __stdcall windows_thread_func_impl(void*);

#include "fb/lang/IncludeWindows.h"

#include <process.h>
#include <errno.h>

FB_PACKAGE0()

class Thread::ThreadImpl
{
public:
	// --- copied and modified from code by Patrick Hoffmann, possibly based on some MSDN source? ---
	typedef struct
	{
	  DWORD dwType; // must be 0x1000
	  LPCSTR szName; // pointer to name (in user addr space)
	  DWORD dwThreadID; // thread ID (-1=caller thread)
	  DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;

	static void setThreadName(DWORD dwThreadID, LPCSTR szThreadName)
	{
	  THREADNAME_INFO info;
	  {
		info.dwType = 0x1000;
		info.szName = szThreadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;
	  }
	  __try
	  {
		RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (const ULONG_PTR*)&info);
	  }
	  __except (EXCEPTION_CONTINUE_EXECUTION)
	  {
	  }
	}
	// ---


	/* Actual entry point function */
	static unsigned __stdcall windowsThreadFuncImpl(void* pArguments)
	{
		Thread *thread = reinterpret_cast<Thread*>(pArguments);

		if (thread == nullptr)
		{
			fb_assert(0 && "Null thread.");
			return 0;
		}

		if (thread->entry == nullptr)
		{
			fb_assert(0 && "Null thread entry.");
			return 0;
		}

		// go run the actual code...
		{
			FB_STACK_CUSTOM("IThreadEntry");
			profiling::beginThread(thread->name.getPointer());
			thread->entry->runner();
			/* Note that thread may have been destructed at this point. Use thread->entry->shouldSelfDestruct() 
			 * beforehand to find out */
			thread = nullptr;
			profiling::endThread();
		}

		// end the thread for real.
		_endthreadex(0);

		// ...and never get here.
		fb_assert(0 && "Reached unreachable code");
		return 0;
	}

	DWORD windowsThreadId = 0;
	HANDLE threadHandle = 0;
};


Thread::Thread(IThreadEntry *entry, const StaticString &name)
{
	commonInit(entry, name);
}


Thread::~Thread()
{
	commonUninit();
}


Thread *Thread::createNewThread(IThreadEntry *entry, const StaticString &threadName)
{
	if (entry == nullptr)
		return nullptr;

	Thread *thread = new Thread(entry, threadName);
	thread->impl.reset(new ThreadImpl());
	
	unsigned idRet;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 16*1024*1024, &ThreadImpl::windowsThreadFuncImpl, thread, 0, &idRet);

	if (hThread == 0)
	{
		// I'm not sure if these error numbers really apply to _beginthreadex, let's just assume so.
		int e = errno;
		if (e == EAGAIN)
		{
			fb_assert(0 && "Thread::createNewThread - Returned EAGAIN. Too many threads.");
		}
		if (e == EINVAL)
		{
			fb_assert(0 && "Thread::createNewThread - Returned EINVAL. Argument invalid or stack size incorrect.");
		}

		fb_assert(0 && "Thread::createNewThread - Thread creation failed.");
		delete thread;
		return NULL;
	}

	thread->impl->threadHandle = hThread;
	thread->impl->windowsThreadId = idRet;

	ThreadImpl::setThreadName(idRet, thread->name.getPointer());

	return thread;
}


void Thread::waitThreadExit(Thread *thread)
{
	fb_assert(thread != NULL);

	if (thread->impl->threadHandle != 0)
	{
		FB_ZONE("Thread::waitThreadExit - WaitForSingleObject", profiling::ZoneBlock);

		HANDLE th = thread->impl->threadHandle;
		WaitForSingleObject(th, INFINITE);
	}
	else
	{
		fb_assert(0 && "Thread::waitThreadExit - Thread did not have a proper handle.");
	}
}


int32_t Thread::disposeThread(Thread *thread)
{
	fb_assert(thread != NULL);
	if (thread->running)
	{
		fb_assert(0 && "Thread::disposeThread - Thread is still running.");
		return 0;
	}

	FB_ZONE("Thread::disposeThread - delete thread");

	delete thread;
	return 0;
}


void Thread::nanosleep(uint32_t nanoseconds)
{
	sleepAccurately(nanoseconds);
}


void Thread::sleepAccurately(uint32_t nanoseconds)
{
	// Timer handle
	HANDLE timer = CreateWaitableTimer(NULL, true, NULL);
	if (!(timer))
		return;

	// Set timer properties
	LARGE_INTEGER li;
	li.QuadPart = -int32_t(nanoseconds) / 100;
	if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, false))
	{
		CloseHandle(timer);
		return;
	}
		
	// Start and wait for timer
	WaitForSingleObject(timer, INFINITE);

	// Clean resources
	CloseHandle(timer);
}


void Thread::setAffinity(Thread *thread, uint32_t affinity)
{
	// No-op for now .. no point on pc(?)
}


void Thread::setPriority(Thread *thread, uint32_t priority)
{
	SetThreadPriority(thread->impl->threadHandle, (int) priority);
}


void Thread::suspend(Thread *thread)
{
	SuspendThread(thread->impl->threadHandle);
}


void Thread::resume(Thread *thread)
{
	ResumeThread(thread->impl->threadHandle);
}

FB_END_PACKAGE0()
