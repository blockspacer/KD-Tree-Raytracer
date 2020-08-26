#ifndef FB_TASK_SCHEDULERIMPSYNCINLINE_H
#define FB_TASK_SCHEDULERIMPSYNCINLINE_H

Sync::Sync()
:	userGeneralBlockThreadSemaphore(0)
,	userDependencyBlockThreadSemaphore(0)
{

}

Sync::~Sync()
{
}

void Sync::init(SizeType workerThreadAmount)
{
}

void Sync::enterLock()
{
	mutex.enter();
}

void Sync::leaveLock()
{
	mutex.leave();
}

void Sync::signalWorkForThreads(uint32_t postCount)
{
	semaphore.post(postCount);
}


void Sync::blockThreadUntilMoreWork()
{
	semaphore.wait();
}

// --

void Sync::blockUserGeneralBlockThread()
{
	userGeneralBlockThreadSemaphore.wait();
}


void Sync::resumeUserGeneralBlockThread()
{
	userGeneralBlockThreadSemaphore.post(1);
}

void Sync::blockUserDependencyBlockThread()
{
	userDependencyBlockThreadSemaphore.wait();
}

void Sync::resumeUserDependencyBlockThread()
{
	userDependencyBlockThreadSemaphore.post(1);
}

#endif // FB_TASK_SCHEDULERIMPSYNCINLINE_H
