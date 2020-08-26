#pragma once

#include "fb/lang/Atomics.h"
#include "fb/lang/ScopedPointer.h"
#include "fb/string/StaticString.h"

FB_DECLARE0(IThreadEntry);
FB_DECLARE0(Time);

FB_PACKAGE0()

class Thread
{
	/* Use createNewThread and joinThread calls instead */
	Thread(IThreadEntry *entry, const StaticString &threadName);
	~Thread();

public:
	typedef uint32_t ThreadId;
	
	const StaticString &getName() const { return name; }

	static Thread *createNewThread(IThreadEntry *entry, const StaticString &threadName);

	/* Joins the parameter thread to current one - wait for exit and dispose. 
	 * NOTE: As disposing thread includes deleting the thread object, parameter thread here isn't valid after joinThread */
	static int32_t joinThread(Thread *thread);

	/* Return thread id of the currently running thread */
	static ThreadId getCurrentThreadId();
	/* Return currently running thread */
	static Thread &getCurrentThread();
	/* Return main thread */
	static Thread &getMainThread();
	/* Return thread id of main thread */
	static ThreadId getMainThreadId();
	/* Returns true if currently running thread is main thread */
	static bool isCurrentThreadMainThread();
	/* Returns true if the currently running thread was created with fb::Thread */
	static bool isFBThread();

	/**
	 * Suggest the thread to yield the end of its time slice to other threads.
	 * Can only be called at the currently running thread.
	 * Actual implementation specifics may vary based on system.
	 * The yield may return immediately based on operating system scheduling decisions, or may cause
	 * the thread to be put to sleep for a short period of time.
	 * You probably don't want to use this.
	 */
	static void yield();
	/**
	 * Sleep at least given amount of milliseconds.
	 * Can only be called at the currently running thread.
	 */
	static void sleep(Time time);
	static void sleep(uint32_t milliseconds);
	static void nanosleep(uint32_t nanoseconds);
	/**
	 * Try to sleep a precise amount of nanoseconds.
	 * Can only be called at the currently running thread.
	 */
	static void sleepAccurately(uint32_t nanoseconds);

	static void setAffinity(Thread *thread, uint32_t affinity);
	static void setPriority(Thread *thread, uint32_t priority);

	/* .. you don't want to know. */
	static void suspend(Thread *thread);
	static void resume(Thread *thread);

	/* Returns true, if it is safe to call getCurrentThread methods. This should be needed only during static initialization */
	static bool hasCurrentThread();
	/* This can be used to set game's main thread on a platform where first thread to run isn't the main one */
	static void setMainThread(Thread &thread);

private:
	friend class IThreadEntry;
	/* Platform-independent initialization and uninitialization */
	void commonInit(IThreadEntry *entry, const StaticString &threadName);
	void commonUninit();

	/* This is called by IThreadEntry before starting the user code */
	void startingUp();

	static void waitThreadExit(Thread *thread);
	static int32_t disposeThread(Thread *thread);

	static ThreadId getNextThreadId();

	/* Returns dummy Thread to use for current thread for non-fb threads */
	static Thread &getNonFBThread();
	/* Current thread is stored as thread_local */
	static thread_local Thread *currentThread;

	/* Note: RunningThreadCount includes threads that have finished, but haven't yet been destructed. This is for
	* debugging, no interface to check it (just use debugger) */
	static lang::AtomicInt32 runningThreadCount;
	static Thread firstThread;
	static Thread *mainThread;

	/* Platform specific stuff */
	class ThreadImpl;
	ScopedPointer<ThreadImpl> impl;

	StaticString name;
	IThreadEntry *entry;
	ThreadId threadId = 0xFFFFFFFF;
	/* Only used for debugging */
	bool running = false;
};

FB_END_PACKAGE0()
