#pragma once

FB_DECLARE0(Mutex)
FB_DECLARE0(MutexGuard)
FB_DECLARE0(Time)

FB_PACKAGE0()

class ConditionVariable
{
	ConditionVariable(const ConditionVariable& other);
	ConditionVariable& operator=(const ConditionVariable& other);

public:
	ConditionVariable();
	~ConditionVariable();

	/* Signal one of the waiting threads to continue */
	void signal();
	/* Signal all of the threads */
	void broadcast();
	/* Wait for variable to be signaled. Given mutex must be locked. It will be released for wait and locked again 
	 * before function returns.
	 * NOTE: Signaling variable when no one is waiting is a nop. If variable is signaled and then waited, the waiting 
	 * thread will be stuck until someone else signals the variable. 
	 * NOTE: Waiting threads are signaled in implementation dependent order. This may lead to starvation.*/
	void wait(Mutex& mutex);
	void wait(MutexGuard& mutexGuard);
	/* Wait for until timeout. Returns false if time out reached */
	bool wait(Mutex& mutex, Time timeout);
	bool wait(MutexGuard& mutexGuard, Time timeout);

private:
	struct ConditionVariableImpl;
	ConditionVariableImpl* impl;
};

FB_END_PACKAGE0()
