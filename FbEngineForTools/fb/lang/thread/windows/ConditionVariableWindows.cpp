#include "Precompiled.h"

#include "fb/lang/thread/ConditionVariable.h"

#include "fb/lang/IncludeWindows.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/time/Time.h"
#include "fb/container/AFList.h"
#include "fb/container/PodVector.h"

FB_PACKAGE0()


/* FIXME: Doing condition variable this way isn't exactly super efficient and doesn't really make sense expect to be 
 * semantically compatible with other platform, but this is only needed for Windows XP. For Vista and up we could just 
 * wrap build-in condition variable. */


struct ConditionVariable::ConditionVariableImpl
{
	ConditionVariableImpl()
		: conditionEvent(INVALID_HANDLE_VALUE)
	{
		/* Default security attributes, manual-reset event, initial state is nonsignaled, object name */
		conditionEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("FB ConditionEvent"));
	}


	~ConditionVariableImpl()
	{
		CloseHandle(conditionEvent);
	}


	bool isThreadOnWaitingList(Thread::ThreadId id)
	{
		for (WaitingThreadList::ConstIterator it = waitingThreads.getBegin(); it != waitingThreads.getEnd(); ++it)
		{
			if (id == *it)
				return true;
		}
		return false;
	}


	bool isThreadOnContinueList(Thread::ThreadId id)
	{
		for (SizeType i = 0; i < threadsAllowedToContinue.getSize(); ++i)
		{
			if (threadsAllowedToContinue[i] == id)
				return true;
		}
		return false;
	}


	bool checkAndRemoveFromContinueList(Thread::ThreadId id)
	{
		for (SizeType i = 0; i < threadsAllowedToContinue.getSize(); ++i)
		{
			if (threadsAllowedToContinue[i] == id)
			{
				threadsAllowedToContinue[i] = threadsAllowedToContinue.getBack();
				threadsAllowedToContinue.popBack();
				return true;
			}
		}
		return false;
	}


	Mutex mutex;
	HANDLE conditionEvent;
	typedef AFList<Thread::ThreadId, true> WaitingThreadList;
	WaitingThreadList waitingThreads;
	/* Would be nice to be fair, but it's also pretty complicated */
	typedef PodVector<Thread::ThreadId> ContinueThreadList;
	ContinueThreadList threadsAllowedToContinue;
};


ConditionVariable::ConditionVariable()
	: impl(new ConditionVariableImpl())
{

}


ConditionVariable::~ConditionVariable()
{
	delete impl;
}


void ConditionVariable::signal()
{
	MutexGuard(impl->mutex);
	if (!impl->waitingThreads.isEmpty())
	{
		/* Try to be fair. Signal oldest waiting thread. */
		fb_expensive_assert(!impl->isThreadOnContinueList(impl->waitingThreads.getFront()));
		impl->threadsAllowedToContinue.pushBack(impl->waitingThreads.getFront());
		impl->waitingThreads.popFront();
		/* Manual event will stay set until someone resets it */
		SetEvent(impl->conditionEvent);
	}
}


void ConditionVariable::broadcast()
{
	MutexGuard(impl->mutex);
	while (!impl->waitingThreads.isEmpty())
	{
		fb_expensive_assert(!impl->isThreadOnContinueList(impl->waitingThreads.getFront()));
		impl->threadsAllowedToContinue.pushBack(impl->waitingThreads.getFront());
		impl->waitingThreads.popFront();
	}
	if (!impl->threadsAllowedToContinue.isEmpty())
	{
		/* Manual event will stay set until someone resets it */
		SetEvent(impl->conditionEvent);
	}
}


void ConditionVariable::wait(Mutex& mutex)
{
	wait(mutex, Time::fromMilliseconds(INFINITE));
}


void ConditionVariable::wait(MutexGuard& mutexGuard)
{
	wait(mutexGuard.getMutex());
}


bool ConditionVariable::wait(Mutex& mutex, Time timeout)
{
	DWORD timeoutMs = DWORD(lang::min(timeout.getMilliseconds(), int64_t(INFINITE)));
	/* Release given mutex as we start waiting. Grab it again when returning */
	InvertedMutexGuard parameterMutexInvertedGuard(mutex);

	MutexGuard implGuard(impl->mutex);
	fb_expensive_assert(!impl->isThreadOnWaitingList(Thread::getCurrentThreadId()));
	impl->waitingThreads.pushBack(Thread::getCurrentThreadId());
	Thread::ThreadId id = Thread::getCurrentThreadId();
	bool multiWaiting = false;
	bool timeouted = false;
	while (!impl->checkAndRemoveFromContinueList(id))
	{
		/* Release impl's mutex when we start waiting */
		InvertedMutexGuard implInvertedGuard(impl->mutex);
		DWORD result = WaitForSingleObject(impl->conditionEvent, timeoutMs);
		if (result == WAIT_TIMEOUT)
		{

			timeouted = true;
			break;
		}

		/* Since in signaling situation we are busy waiting here, yield thread that isn't allowed to continue */
		fb_expensive_assert(id == Thread::getCurrentThreadId());
		if (multiWaiting)
			Thread::yield();

		multiWaiting = true;
	}
	if (timeouted)
	{
		for (ConditionVariableImpl::WaitingThreadList::ConstIterator it = impl->waitingThreads.getBegin(); it != impl->waitingThreads.getEnd(); ++it)
		{
			if (id == *it)
			{
				impl->waitingThreads.erase(it);
				return false;
			}
		}
		fb_assert(0 && "Thread missing from waiting list");
		return false;
	}
	/* No one is allowed to continue, may reset event and make busy waiting less busy */
	if (impl->threadsAllowedToContinue.isEmpty())
		ResetEvent(impl->conditionEvent);

	return true;
}


bool ConditionVariable::wait(MutexGuard& mutexGuard, Time timeout)
{
	return wait(mutexGuard.getMutex(), timeout);
}

FB_END_PACKAGE0()