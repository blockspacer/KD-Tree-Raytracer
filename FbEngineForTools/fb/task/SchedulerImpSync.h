#ifndef FB_TASK_SCHEDULERIMPSYNC_H
#define FB_TASK_SCHEDULERIMPSYNC_H

#include "fb/lang/thread/FastSemaphore.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/thread/Semaphore.h"

FB_PACKAGE1(task)

// Implementation detail of Scheduler.h. Isolated here for easier readability.

struct Sync
{
	Mutex mutex;
	FastSemaphore semaphore;

	Semaphore userGeneralBlockThreadSemaphore;
	Semaphore userDependencyBlockThreadSemaphore;

	inline Sync();
	inline ~Sync();

	// Init for given thread amount
	inline void init(SizeType workerThreadAmount);

	// General locks
	inline void enterLock();
	inline void leaveLock();

	// Let sleeping threads know that there is a new task available
	inline void signalWorkForThreads(uint32_t postCount);
	// For worker threads only -- sleep until there is some work
	inline void blockThreadUntilMoreWork();

	inline void manualSignalWorkForThreads(uint32_t postCount);
	inline void manualBlockThreadUntilMoreWork();

	// For user thread sync
	inline void blockUserGeneralBlockThread();
	inline void resumeUserGeneralBlockThread();
	inline void blockUserDependencyBlockThread();
	inline void resumeUserDependencyBlockThread();
};

struct SyncGuard
{
	Sync &sync;

	SyncGuard(Sync &s)
	:	sync(s)
	{
		sync.enterLock();
	}

	~SyncGuard()
	{
		sync.leaveLock();
	}

};

#include "SchedulerImpSyncInline.h"

FB_END_PACKAGE1()

#endif
