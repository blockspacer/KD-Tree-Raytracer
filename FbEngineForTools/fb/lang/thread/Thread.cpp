#include "Precompiled.h"
#include "Thread.h"

#include "fb/lang/DebugHelp.h"
#include "fb/lang/thread/IThreadEntry.h"
#include "fb/lang/FBSingleThreadAssert.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/time/Time.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/HeapString.h"

#if FB_VS2017_IN_USE == FB_TRUE
#pragma warning(push)
	/* 'argument': conversion from 'const int64_t' to 'const uint64_t', signed/unsigned mismatch */
	#pragma warning( disable: 4365 )
#endif
#include <thread>
#if FB_VS2017_IN_USE == FB_TRUE
	#pragma warning(pop)
#endif

/* Note: Most implementation in platform specific classes */

FB_PACKAGE0()

static const StaticString &getMainthreadName()
{
#if FB_ENGINE_FOR_TOOLS == FB_TRUE
	/* Initialize static strings here,
	 * Then we can actually destroy the sting impl at the end */
	string::StringImpHelper::initializeData();
#endif
	FB_STATIC_CONST_STRING(mainthreadName, "Mainthread");
	return mainthreadName;
}
static const StaticString &getNonFBThreadName()
{
	FB_STATIC_CONST_STRING(nonFBThreadName, "Non-FB thread");
	return nonFBThreadName;
}

/* Static */
/* Note: runningThreadCount must be initialized before mainThread */
lang::AtomicInt32 Thread::runningThreadCount;
Thread Thread::firstThread(nullptr, getMainthreadName());
Thread *Thread::mainThread = &firstThread;

/* Thread local */
Thread &Thread::getNonFBThread()
{
	thread_local Thread nonFBThread(nullptr, getNonFBThreadName());
	return nonFBThread;

}
thread_local Thread *Thread::currentThread = nullptr;

int32_t Thread::joinThread(Thread *thread)
{
	waitThreadExit(thread);
	int32_t returnValue = disposeThread(thread);
	return returnValue;
}


Thread::ThreadId Thread::getCurrentThreadId()
{
	return getCurrentThread().threadId;
}


Thread &Thread::getCurrentThread()
{
	if (currentThread == nullptr)
		currentThread = &getNonFBThread();

	return *Thread::currentThread;
}


bool Thread::isFBThread()
{
	return currentThread != nullptr;
}


Thread &Thread::getMainThread()
{
	/* This should always be set. Usually it is the same as firstThread, but could be different on some platforms */
	fb_assert(mainThread != nullptr);
	return *mainThread;
}


Thread::ThreadId Thread::getMainThreadId()
{
	fb_assert(mainThread != &firstThread || getMainThread().threadId == 0 && "Unexpected main thread id");
	return getMainThread().threadId;
}


bool Thread::isCurrentThreadMainThread()
{
	return &getCurrentThread() == &getMainThread();
}


void Thread::yield()
{
	std::this_thread::yield();
}


void Thread::sleep(Time time)
{
	if (time > Time::zero)
		sleep(uint32_t(time.getMillisecondsAsUnsafeInt()));
}

void Thread::sleep(uint32_t milliseconds)
{
	FB_ZONE("Thread::sleep", profiling::ZoneBlock);

	// don't use sleep_for on MSVC as it depends on system time, changing the system clock will freeze threads
	Sleep(milliseconds);
}


bool Thread::hasCurrentThread()
{
	return currentThread != nullptr;
}


void Thread::setMainThread(Thread &thread)
{
	mainThread = &thread;
}


void Thread::commonInit(IThreadEntry *threadEntry, const StaticString &threadName)
{
	this->entry = threadEntry;
	this->name = threadName;
	threadId = getNextThreadId();
	if (entry != nullptr)
	{
		lang::atomicIncRelaxed(runningThreadCount);
		fb_assert(entry->thread == nullptr && "Entry->thread should be null at this point");
		entry->thread = this;
	}
	else
	{
		/* This is main thread or non-FB thread*/
		if (threadName == getMainthreadName())
		{
			lang::atomicIncRelaxed(runningThreadCount);
			FB_SINGLE_THREAD_ASSERT_INIT();
			fb_assert(threadId == 0 && "Unexpected ID for mainthread");
			currentThread = this;
		}
		else if (currentThread == nullptr)
		{
			currentThread = this;
		}
		running = true;
	}
}


void Thread::commonUninit()
{
	if (name == getNonFBThreadName())
		return;

	if (&getMainThread() == this)
	{
		/* This is main thread */
		running = false;
		/* This should be last thread */
#if FB_ASSERT_ENABLED == FB_TRUE
		if (lang::atomicLoadRelaxed(runningThreadCount) != 1)
		{
			TempString str;
			str << lang::atomicLoadRelaxed(runningThreadCount) << " threads still exist. Only one thread should remain.";
			if (!DebugHelp::settings.printInsteadOfAssert)
			{
				DebugHelp::showMessageBox("Thread leak detected", str);
			}
		}
#else
		fb_assert(lang::atomicLoadRelaxed(runningThreadCount) == 1);
#endif
		FB_SINGLE_THREAD_ASSERT_UNINIT();
	}
	// make sure this thread has finished / has been terminated.
	if (running && this != &firstThread)
	{
		fb_assert(0 && "Thread::~Thread - Destructor called when thread has not finished or has not been terminated yet.");
	}
	lang::atomicDecRelaxed(runningThreadCount);
}


void Thread::startingUp()
{
	fb::DebugHelp::checkStackAllocation();
	fb_assert(currentThread == nullptr);
	currentThread = this;
	running = true;
}


Thread::ThreadId Thread::getNextThreadId()
{
	static lang::AtomicInt32 runningId;
	int32_t value = lang::atomicIncRelaxed(runningId);
	return ThreadId(value);
}


FB_END_PACKAGE0()