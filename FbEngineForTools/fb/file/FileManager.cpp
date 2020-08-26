#include "Precompiled.h"
#include "FileManager.h"

#include "fb/algorithm/Sort.h"
#include "fb/container/PodVector.h"
#include "fb/container/LinearHashSet.h"
#include "fb/file/AbsolutePath.h"
#include "fb/file/DebugBlockWriter.h"
#include "fb/file/FileSortInfo.h"
#include "fb/file/package/IFilePackage.h"
#include "fb/lang/CallStack.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/time/HighResolutionTime.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/profiling/ScopedProfiler.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/HeapString.h"
#include "fb/task/Scheduler.h"


#if FB_EDITOR_ENABLED == FB_TRUE
	#define FB_ENABLE_HIGHLY_PARALLEL_LOADING FB_TRUE
#else
	#define FB_ENABLE_HIGHLY_PARALLEL_LOADING FB_FALSE
#endif


FB_PACKAGE1(file)

/* In Trine 4 there's at least one 16 MB texture, so no need to stay under that really */
static const uint32_t maxReadSize = 16 * 1024 * 1024;
/* Incresing preferred size from one to 4 MBs seems to have small effect. After that it doesn't really matter */
static const uint32_t preferredReadSize = 4 * 1024 * 1024;
/* Size of space allowed between files and still consider them consecutive.  */
static const uint32_t allowedHoleSize = 512 * 1024;
/* Maximum number of tasks to spawn to handle the reading. For platforms with non-concurrent storage media (hard
 * drive, crappy flash), two should be enough (basically one reading while other is handling processing) */
#if FB_ENABLE_HIGHLY_PARALLEL_LOADING == FB_TRUE
static const SizeType maxNumAsyncReadingTasks = 6;
#else
static const SizeType maxNumAsyncReadingTasks = 2;
#endif

static inline void simulateSlowDisk(BigSizeType bytesRead)
{
#if 0
	static const uint32_t readSpeedBytesPerSecond = 50 * 1024 * 1024;
	static const Time seekTime = Time::fromMilliseconds(20);
	/* Enable this to simulate slow disk. Could use some randomization too, or different modes, for better finding 
	 * race conditions and other timing related bugs */
	Time timeToSleep = seekTime + Time::fromSeconds(float(bytesRead) / readSpeedBytesPerSecond);
	Thread::sleep(uint32_t(timeToSleep.getMilliseconds()));
#endif
}

#define FB_VALIDATE_LOWLEVEL_FILEPATH(p_filepath_c_ptr) ::fb::file::validateFilePathIsRelativeToBase(p_filepath_c_ptr)

/* Note: Async stats don't really work with callback tasks, neither does idle time or hasAsyncOpensInProgress(). May 
 * also have other problems (like deadlocks) */
#define FB_FILE_MANAGER_SEPARATE_ASYNC_CALLBACK_TASKS FB_FALSE

void validateFilePathIsRelativeToBase(const StringRef &filePath)
{
	// please get rid of any "./data/foo" file names, etc. - just use "data/foo" instead.
	// using the . or .. paths will just eventually screw you over as far as file caching logic, etc. goes, so never use them!
	// (if you really must use them, then have you'll have to convert a lot of code to do the
	// "any path" -> "relative to base path" conversion.)

	fb_assertf(!isAbsolutePath(filePath), "Path %s is invalid. Absolute paths are not OK anymore. Use writeUserFile or deleteUserFile or appropriate flags if you need to write to %%appdata%%", filePath.getPointer());
	fb_assertf((filePath.isEmpty() || filePath[0] != '.'), "Path %s is invalid. You are not supposed to start the file path names with ./ or ../ directories.", filePath.getPointer());
	fb_assertf(!filePath.doesContain("\\"), "Path %s is invalid. You are not supposed to use the \\ characters in file path names.", filePath.getPointer());
	fb_assertf(!filePath.doesContain("./"), "Path %s is invalid. You are not supposed to use the ./ anywhere in file path names.", filePath.getPointer());
	fb_assertf(!filePath.doesContain("../"), "Path %s is invalid. You are not supposed to use the ../ anywhere in file path names.", filePath.getPointer());
};


FB_MEGATON_CLASS_IMPL(FileManager);
FB_STACK_SET_CLASS(FileManager);

class FileManager::Impl : public task::ISchedulerTask
{
public:
	struct AsyncQueueItem
	{
		DynamicString fileName;
		package::FileResolveData resolveData;
		CachePodVector<AsyncFileReceiver *, 2> receivers;
		AsyncOperationID id;
		uint32_t priority = 0;
		SizeType packageIndex = 0xFFFFFFFF;
		bool resolved = false;

		/* Goal is to sort the items so that the reading order is back to front */
		bool operator<(const AsyncQueueItem &other) const
		{
			if (priority != other.priority)
				return priority < other.priority;

			if (packageIndex != other.packageIndex)
				return packageIndex > other.packageIndex;

			if (resolveData.offset != other.resolveData.offset)
				return resolveData.offset > other.resolveData.offset;

			/* Sort by name as a final step isn't really important, but as Standard packages won't have offsets, 
			 * something is needed. Reading a folder at a time will at least have metadata caching benefit */
			return fileName < other.fileName;
		}
	};

	#if FB_FILE_MANAGER_SEPARATE_ASYNC_CALLBACK_TASKS == FB_TRUE
		class AsyncCallbackTask : public task::ISchedulerTask
		{
		public:
			const char *getStaticTaskNameString() const override
			{
				return "FileManagerAsyncRead";
			}

			void run(const task::SchedulerTaskData &data, task::Scheduler &scheduler) override
			{
				MutexGuard mg(mutex);
				while (!items.isEmpty())
				{
					fb_assert(items.getSize() == files.getSize() && "Out of sync");
					AsyncQueueItem item = items.getBack();
					items.popBack();
					File file = files.getBack();
					files.popBack();
					InvertedMutexGuard img(mg);
					for (AsyncFileReceiver *receiver : item.receivers)
					{
						fb_assert(receiver != nullptr && "W00t. This should never happen. Invalid AsyncQueueItem?");
						receiver->readFinished(item.fileName, file, item.id);
					}
				}
			}

			void addItems(const Vector<AsyncQueueItem> &newItems, Vector<File> &newFiles)
			{
				MutexGuard mg(mutex);
				items.insert(items.getBegin(), newItems.getBegin(), newItems.getEnd());
				files.insert(files.getBegin(), newFiles.getBegin(), newFiles.getEnd());
			}

			Mutex mutex;
			CacheVector<AsyncQueueItem, 8> items;
			CacheVector<File, 8> files;
		};
	#endif

	
	Impl()
	{
		initAsyncQueue();
	}


	~Impl()
	{
		for (SizeType i = 0; i < packages.getSize(); i++)
			delete packages[i].package;

		MutexGuard mg(queueMutex);
		asyncQueue.clear();
		filesInAsyncQueue.clear();
		if (asyncQueueDependencyId != task::Scheduler::getInvalidDependencyGroupId())
		{
			InvertedMutexGuard img(mg);
			getScheduler().waitForDependencies(task::Scheduler::TaskGroupBackground, asyncQueueDependencyId);
			fb_assert(asyncQueue.isEmpty() && filesInAsyncQueue.isEmpty() && "Someone is adding files to read while FileManager is being torn down");
			getScheduler().freeDependencyGroup(task::Scheduler::TaskGroupBackground, asyncQueueDependencyId);
		}
		#if FB_FILE_MANAGER_SEPARATE_ASYNC_CALLBACK_TASKS == FB_TRUE
			if (asyncCallbackDependencyId != task::Scheduler::getInvalidDependencyGroupId())
			{
				getScheduler().waitForDependencies(task::Scheduler::TaskGroupBackground, asyncCallbackDependencyId);
				getScheduler().freeDependencyGroup(task::Scheduler::TaskGroupBackground, asyncCallbackDependencyId);
			}
		#endif
	}


	static HeapString flagsToString(FileFlagMask flags)
	{
		HeapString str;
		flags.appendToString(str);
		return str;
	}


	static void checkFlags(FileFlagMask allowedFlags, FileFlagMask flagsToCheck)
	{
		fb_assertf((flagsToCheck & ~allowedFlags) == 0, "Unsupported flag(s) %s. Full mask is %s, allowed mask is %s",
			flagsToString(flagsToCheck & ~allowedFlags).getPointer(), flagsToString(flagsToCheck).getPointer(), flagsToString(allowedFlags).getPointer());
	}


	BigSizeType getFileSize(const DynamicString &fileName, FileFlagMask flags, bool onDisk)
	{
		FB_ZONE("FileManager::Impl::getFileSize");
		FB_VALIDATE_LOWLEVEL_FILEPATH(fileName);

		static FileFlagMask allowedFlags(FileFlagExcludeUser | FileFlagExcludeNormal);
		checkFlags(allowedFlags, flags);

		bool excludeUser = (flags & FileFlagExcludeUser) != 0;
		bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

		for (SizeType i = 0; i < packages.getSize(); i++)
		{
			Impl::PackageHolder &holder = packages[i];
			if (excludeUser && holder.userFilePackage)
				continue;

			if (excludeNormal && !holder.userFilePackage)
				continue;

			/* Note: This will actually fail, if we have zero sized file in higher priority package and non-zero sized
			 * file in lower priority package. The size of non-zero will be returned, but only file you can open will be
			 * the zero sized */
			BigSizeType s = onDisk ? holder.package->getFileSizeOnDisk(fileName) : holder.package->getFileSize(fileName);
			if (s > 0)
				return s;
		}
		return 0;
	}


	void initAsyncQueue()
	{
		asyncQueueDependencyId = getScheduler().createDependencyGroup(task::Scheduler::TaskGroupBackground);
		#if FB_FILE_MANAGER_SEPARATE_ASYNC_CALLBACK_TASKS == FB_TRUE
			asyncCallbackDependencyId = getScheduler().createDependencyGroup(task::Scheduler::TaskGroupBackground);
		#endif
		asyncQueue.reserve(1024);
		filesInAsyncQueue.reserve(1024);
	}


	/* This is used to stop all progress during Xbox One suspend. If we are about to suspend (FileManager::suspend has 
	 * been called), it will return only after resume is called */
	void checkSuspension()
	{
	}


	void run(const task::SchedulerTaskData &data, task::Scheduler &scheduler) override
	{
		FB_ZONE("FileManager::Impl::run - AsyncQueue");

		CacheVector<AsyncQueueItem, 8> itemsToRead;
		CacheVector<DynamicString, 8> fileNames;
		CacheVector<File, 8> files;
		MutexGuard mg(queueMutex);
		fb_assert(numAsyncReadingTasks <= maxNumAsyncReadingTasks);
		while (!asyncQueue.isEmpty())
		{
			FB_ZONE("while (!asyncQueue.isEmpty())");

			itemsToRead.pushBack(asyncQueue.getBack());
			asyncQueue.popBack();
			FB_UNUSED_NAMED_VAR(bool, found) = filesInAsyncQueue.eraseKey(itemsToRead.getBack().fileName);
			fb_assertf(found, "File not found from accelerator set. '%s'", itemsToRead.getBack().fileName.getPointer());
			uint64_t bytesToRead = itemsToRead.getFront().resolveData.rawSize;
			uint64_t bytesToReadWithHoles = itemsToRead.getFront().resolveData.rawSize;
			bool memoryResidentRun = itemsToRead.getFront().resolveData.memoryResident;
			while (!asyncQueue.isEmpty() && bytesToReadWithHoles < preferredReadSize)
			{
				AsyncQueueItem &candidate = asyncQueue.getBack();
				fb_assert(candidate.resolved);
				fb_assert(!candidate.receivers.isEmpty());
				const AsyncQueueItem &latestItem = itemsToRead.getBack();
				if (candidate.packageIndex != latestItem.packageIndex)
					break;

				/* Allow bundling memory resident files, and files that are pretty close */
				uint64_t holeSize = latestItem.resolveData.calculateHoleSize(candidate.resolveData);
				if (!memoryResidentRun && holeSize > allowedHoleSize)
					break;

				uint64_t bufferBytesNeeded = candidate.resolveData.rawSize + holeSize;
				if (!memoryResidentRun && candidate.resolveData.rawSize + bufferBytesNeeded > maxReadSize)
					break;

				/* We got another file to read */
				itemsToRead.pushBack(candidate);
				bytesToRead += candidate.resolveData.rawSize;
				bytesToReadWithHoles += bufferBytesNeeded;
				found = filesInAsyncQueue.eraseKey(itemsToRead.getBack().fileName);
				fb_assert(found && "File not found from accelerator set");
				asyncQueue.popBack();

				if (FB_ENABLE_HIGHLY_PARALLEL_LOADING == FB_TRUE)
				{
					if (itemsToRead.getSize() > 32)
						break;
				}
			}
			/* We have collected items that we want to read. No need to hold the mutex for a while */
			#if FB_FILE_MANAGER_STATS == FB_TRUE
				ScopedTimer processingTimer;
			#endif
			InvertedMutexGuard img(mg);
			for (const AsyncQueueItem &item : itemsToRead)
				fileNames.pushBack(item.fileName);

			#if FB_FILE_MANAGER_STATS == FB_TRUE
				ScopedTimer readTimer;
			#endif
			packages[itemsToRead.getFront().packageIndex].package->openFiles(fileNames, files);
			simulateSlowDisk(bytesToReadWithHoles);
			#if FB_FILE_MANAGER_STATS == FB_TRUE
				if (!memoryResidentRun)
					addPeriod(AsyncPeriodTypeRead, readTimer.getTime(), itemsToRead.getFront().fileName, bytesToRead, bytesToReadWithHoles, itemsToRead.getSize());
			#endif

			fb_assert(files.getSize() == itemsToRead.getSize());
			/* Callbacks */
			#if FB_FILE_MANAGER_SEPARATE_ASYNC_CALLBACK_TASKS == FB_TRUE
				/* Spawn million tasks */
				asyncCallbackTask.addItems(itemsToRead, files);
				for (SizeType i = 0, num = itemsToRead.getSize(); i < num; ++i)
					getScheduler().addTask(task::Scheduler::TaskGroupBackground, asyncCallbackDependencyId, task::Scheduler::TaskPriorityNormal, &asyncCallbackTask);
			#else
				for (SizeType i = 0, num = itemsToRead.getSize(); i < num; ++i)
				{
					AsyncQueueItem &item = itemsToRead[i];
					for (AsyncFileReceiver *receiver : item.receivers)
					{
						FB_ZONE_FMT("%d MB, %s", SizeType(files[i].getSize() / (1024 * 1024)), item.fileName.getPointer());

						fb_assert(receiver != nullptr && "W00t. This should never happen. Invalid AsyncQueueItem?");
						receiver->readFinished(item.fileName, files[i], item.id);
						lang::atomicIncRelaxed(asyncProgressIndicator);
					}
					/* Free as soon as possible */
					files[i].release();
				}
			#endif
			#if FB_FILE_MANAGER_STATS == FB_TRUE
				if (!memoryResidentRun)
					addPeriod(AsyncPeriodTypeProcess, processingTimer.getTime(), itemsToRead.getFront().fileName, bytesToRead, bytesToReadWithHoles, itemsToRead.getSize());
			#endif

			/* Get ready for the next round */
			itemsToRead.clear();
			fileNames.clear();
			files.clear();
			/* Checking suspension here and not on earlier phase is unoptimal from how-fast-can-we-suspend point of 
			 * view, but does allow any cancellations to proceed to an end. And no-one's waiting for actual async ops 
			 * to finish, right? */
			checkSuspension();
		}
		/* Queue is empty, time to stop */
		--numAsyncReadingTasks;
		lang::atomicIncRelaxed(asyncProgressIndicator);
		if (numAsyncReadingTasks == 0)
		{
			#if FB_FILE_MANAGER_STATS == FB_TRUE
				addPeriod(AsyncPeriodTypeBusy, asyncIdleTimer.getTime());
			#endif
			asyncIdleTimer.reset();
		}
	}

	const char *getStaticTaskNameString() const override
	{
		return "FileManagerAsyncRead";
	}

	AsyncOperationID getRunningAsyncOperationId()
	{
		/* Increase by two so we always return even values. This prevents hitting invalid index (0xFFFFFFFF) even if 
		 * we open very many files */
		return lang::atomicAddRelaxed(runningAsyncID, 2);
	}

	friend class FileManager;

	Mutex queueMutex;
	task::Scheduler::DependencyGroupId asyncQueueDependencyId = task::Scheduler::getInvalidDependencyGroupId();
	#if FB_FILE_MANAGER_SEPARATE_ASYNC_CALLBACK_TASKS == FB_TRUE
		task::Scheduler::DependencyGroupId asyncCallbackDependencyId = task::Scheduler::getInvalidDependencyGroupId();
		AsyncCallbackTask asyncCallbackTask;
	#endif

	lang::AtomicUInt32 runningAsyncID;
	SizeType numAsyncReadingTasks = 0;

	struct PackageHolder
	{
		PackageHolder()
		{
		}

		PackageHolder(package::IFilePackage *package, bool userFilePackage)
			: package(package)
			, userFilePackage(userFilePackage)
		{
		}

		package::IFilePackage *package = nullptr;
		bool userFilePackage = false;
	};

	PodVector<PackageHolder> packages;

	/* Holds items queued for read and information about who to notify when done reading */
	Vector<AsyncQueueItem> asyncQueue;
	/* Acceleration structure for common case of adding a not-yet-queued file to async queue */
	LinearHashSet<DynamicString> filesInAsyncQueue;
	/* Timer that is reset when last queued async task finishes */
	ScopedTimer asyncIdleTimer;
	lang::AtomicUInt32 asyncProgressIndicator;

	/* Stats tracking */
	#if FB_FILE_MANAGER_STATS == FB_TRUE
		Mutex periodTrackingMutex;
		ScopedTimer lifeTimeTimer;
		enum AsyncPeriodType
		{
			AsyncPeriodTypeInvalid,
			AsyncPeriodTypeIdle,
			AsyncPeriodTypeRead,
			AsyncPeriodTypeSyncedRead,
			AsyncPeriodTypeBusy,
			AsyncPeriodTypeProcess
		};
		struct AsyncPeriod
		{

			AsyncPeriod() { }
			AsyncPeriod(AsyncPeriodType type, Time timeStamp, Time duration, const DynamicString &fileName, uint64_t bytesRead, uint64_t bytesReadWithHoles, SizeType filesRead)
				: type(type)
				, fileName(fileName)
				, timeStamp(timeStamp)
				, duration(duration)
				, bytesRead(bytesRead)
				, bytesReadWithHoles(bytesReadWithHoles)
				, filesRead(filesRead)
			{
			}

			AsyncPeriodType type = AsyncPeriodTypeInvalid;
			DynamicString fileName;
			Time timeStamp;
			Time duration;
			BigSizeType bytesRead = 0;
			BigSizeType bytesReadWithHoles = 0;
			SizeType filesRead = 0;
		};
		Vector<AsyncPeriod> periods;

		void addPeriod(AsyncPeriodType type, Time duration, const DynamicString &fileName = DynamicString::empty, uint64_t bytesRead = 0, uint64_t bytesReadWithHoles = 0, SizeType filesRead = 0)
		{
			MutexGuard mg(periodTrackingMutex);
			periods.pushBack(AsyncPeriod(type, lifeTimeTimer.getTime(), duration, fileName, bytesRead, bytesReadWithHoles, filesRead));
		}
	#endif
};


FileManager::FileManager()
	: impl(new Impl())
{
}


FileManager::~FileManager()
{
}

File FileManager::openFile(const DynamicString &fileName, FileFlagMask flags)
{
	FB_ZONE("FileManager::openFile");
	FB_ZONE(fileName); // A separate zone for filename to have more information

	#if FB_FILE_MANAGER_STATS == FB_TRUE
		ScopedTimer timer;
	#endif

	// Not sure what's the real solution here. But trying to open "" happily goes through validation below and after prefixing with 
	// platform specific loading directory turns into valid location. Which is a directory.
	// Probably not what we want.
	/* Empty path is ok for listing directory, so it shouldn't not validate. Opening a directory as file is something 
	 * that shouldn't happen, but it's a problem of file packages, and catching "" here doesn't really solve that. So 
	 * fixed that separately (on PS4, hopefully doesn't happen on other platforms) and now just asserts on empty file 
	 * name. Will eventually return empty File */
	fb_assert(!fileName.isEmpty() && "Attempting to open file with no name");
	FB_VALIDATE_LOWLEVEL_FILEPATH(fileName);

	static FileFlagMask allowedFlags(FileFlagExcludeNonArchive | FileFlagExcludeUser | FileFlagExcludeNormal);
	Impl::checkFlags(allowedFlags, flags);

	bool excludeNonArchive = (flags & FileFlagExcludeNonArchive) != 0;
	bool excludeUser = (flags & FileFlagExcludeUser) != 0;
	bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

	for (SizeType i = 0; i < impl->packages.getSize(); i++)
	{
		Impl::PackageHolder &holder = impl->packages[i];
		if (excludeUser && holder.userFilePackage)
			continue;

		if (excludeNormal && !holder.userFilePackage)
			continue;

		File file = holder.package->openFile(fileName, excludeNonArchive);
		if (file)
		{
			#if FB_FILE_MANAGER_STATS == FB_TRUE
				if (!holder.package->isMemoryResident())
				{
					Time duration = timer.getTime();
					BigSizeType fileSize = getFileSize(fileName);
					impl->addPeriod(Impl::AsyncPeriodTypeSyncedRead, duration, fileName, fileSize, fileSize, 1);
				}
			#endif
			return file;
		}
	}
	return File();
}


AsyncOperationID FileManager::openFileAsync(AsyncFileReceiver *receiver, const DynamicString &fileName, uint32_t priority)
{
	StaticPodVector<AsyncOperationID, 1> idHolder;
	StaticVector<DynamicString, 1> fileNameHolder;
	fileNameHolder.pushBack(fileName);
	openFilesAsync(receiver, fileNameHolder, idHolder, priority);
	return idHolder.getFront();
}


void FileManager::openFilesAsync(AsyncFileReceiver *receiver, const FileNameVector &fileNames, OperationIDVector &idsOut, uint32_t priority)
{
	FB_ZONE("FileManager::openFilesAsync");

	fb_assert(receiver != nullptr && "Null AsyncFileReceiver is not allowed");
	if (receiver == nullptr)
		return;

	/* Resolve files */
	idsOut.clear();
	Vector<Impl::AsyncQueueItem> newItems;
	newItems.reserve(fileNames.getSize());
	for (const DynamicString &fileName : fileNames)
	{
		Impl::AsyncQueueItem &item = newItems.pushBack();
		item.fileName = fileName;
		for (SizeType i = 0, num = impl->packages.getSize(); i < num; i++)
		{
			Impl::PackageHolder &holder = impl->packages[i];
			if (holder.package->resolveFile(fileName, item.resolveData))
			{
				item.resolved = true;
				item.packageIndex = i;
				break;
			}
		}
		idsOut.pushBack(item.resolved ? impl->getRunningAsyncOperationId() : AsyncOperationID());
		item.id = idsOut.getBack();
		item.priority = priority;
		item.receivers.pushBack(receiver);
	}
	/* Add new files, update ones already in queue */
	MutexGuard mg(impl->queueMutex);
	for (SizeType i = 0, num = newItems.getSize(); i < num; ++i)
	{
		Impl::AsyncQueueItem &newItem = newItems[i];
		if (!newItem.resolved)
			continue;

		if (impl->filesInAsyncQueue.find(newItem.fileName) != impl->filesInAsyncQueue.getEnd())
		{
			/* We already have this file queued for reading. Just update the priority if necessary and add another 
			 * receiver */
			FB_UNUSED_NAMED_VAR(bool, found) = false;
			for (Impl::AsyncQueueItem &queuedItem : impl->asyncQueue)
			{
				if (queuedItem.fileName != newItem.fileName)
					continue;

				found = true;
				queuedItem.receivers.pushBack(receiver);
				queuedItem.priority = lang::max(queuedItem.priority, priority);
				idsOut[i] = queuedItem.id;
				break;
			}
			fb_assert(found && "Item not found though it is in cache set");
			continue;
		}
		impl->asyncQueue.pushBack(newItem);
		impl->filesInAsyncQueue.insert(newItem.fileName);
	}
	/* Finally, sort async queue and kick off tasks if necessary */
	algorithm::sort(impl->asyncQueue.getBegin(), impl->asyncQueue.getEnd());
	while (impl->numAsyncReadingTasks < maxNumAsyncReadingTasks)
	{
		if (impl->numAsyncReadingTasks == 0)
		{
			#if FB_FILE_MANAGER_STATS == FB_TRUE
				impl->addPeriod(Impl::AsyncPeriodTypeIdle, impl->asyncIdleTimer.getTime());
			#endif
			impl->asyncIdleTimer.reset();
		}
		getScheduler().addTask(task::Scheduler::TaskGroupBackground, impl->asyncQueueDependencyId, task::Scheduler::TaskPriorityNormal, impl.get());
		++impl->numAsyncReadingTasks;
	}
}


void FileManager::openFilesAsync(AsyncFileReceiver *receiver, const FileNameVector &fileNames, uint32_t priority)
{
	thread_local OperationIDVector oidv;
	openFilesAsync(receiver, fileNames, oidv, priority);
}


SizeType FileManager::cancelOpenFilesAsync(AsyncFileReceiver *receiver)
{
	SizeType numCancelled = 0;
	File emptyFile;
	MutexGuard mg(impl->queueMutex);
	/* First swap out, then sort. This is optimal for cases where removed operations make most of the queue or just 
	 * happen to be at the back. Also optimal for very random case and immune to really bad cases (e.g. removing 
	 * operations one at a time from the front, always copying the rest of the queue) */
	for (SizeType itemIndex = 0; itemIndex < impl->asyncQueue.getSize(); ++itemIndex)
	{
		Impl::AsyncQueueItem &item = impl->asyncQueue[itemIndex];
		/* Note: Same receiver may have been queued twice for an item */
		for (SizeType receiverIndex = 0; receiverIndex < item.receivers.getSize(); ++receiverIndex)
		{
			if (item.receivers[receiverIndex] == receiver)
			{
				++numCancelled;
				item.receivers[receiverIndex]->readFinished(item.fileName, emptyFile, item.id);
				if (item.receivers.getSize() == 1)
				{
					impl->filesInAsyncQueue.eraseKey(item.fileName);
					impl->asyncQueue.eraseIndex(itemIndex);
					--itemIndex;
				}
				else
				{
					item.receivers[receiverIndex] = item.receivers.getBack();
					item.receivers.popBack();
					--receiverIndex;
				}
			}
		}
	}
	algorithm::sort(impl->asyncQueue.getBegin(), impl->asyncQueue.getEnd());
	return numCancelled;
}


bool FileManager::cancelOpenFilesAsync(AsyncFileReceiver *receiver, AsyncOperationID id)
{
	File emptyFile;
	MutexGuard mg(impl->queueMutex);
	for (SizeType itemIndex = 0; itemIndex < impl->asyncQueue.getSize(); ++itemIndex)
	{
		Impl::AsyncQueueItem &item = impl->asyncQueue[itemIndex];
		if (item.id != id)
			continue;

		for (SizeType receiverIndex = 0, numReceivers = item.receivers.getSize(); receiverIndex < numReceivers; ++receiverIndex)
		{
			if (item.receivers[receiverIndex] == receiver)
			{
				item.receivers[receiverIndex]->readFinished(item.fileName, emptyFile, id);
				if (numReceivers == 1)
				{
					impl->filesInAsyncQueue.eraseKey(item.fileName);
					impl->asyncQueue.eraseIndex(itemIndex);
				}
				else
				{
					item.receivers[receiverIndex] = item.receivers.getBack();
					item.receivers.popBack();
				}
				return true;
			}
		}
		return false;
	}
	return false;
}


void FileManager::waitForAsyncProgress() const
{
	/* This would work better with some condition variable, but Trine 4 is coming up */
	const uint32_t originalProgress = lang::atomicLoadRelaxed(impl->asyncProgressIndicator);
	const uint32_t sleepStepNs = 1000;
	const uint32_t maxSleepTimeNs = 1000 * 1000;
	uint32_t timeToSleepNs = 0;
	/* Check for progress */
	while (originalProgress == lang::atomicLoadRelaxed(impl->asyncProgressIndicator))
	{
		/* Check to make sure there are some threads that can progress */
		{
			MutexGuard mg(impl->queueMutex);
			if (impl->numAsyncReadingTasks == 0)
				return;
		}

		/* Wait for something to happen */
		timeToSleepNs = lang::min(timeToSleepNs + sleepStepNs, maxSleepTimeNs);
		Thread::nanosleep(timeToSleepNs);
	}
}


bool FileManager::hasAsyncOpensInProgress() const
{
	MutexGuard mg(impl->queueMutex);
	return impl->numAsyncReadingTasks != 0 || !impl->asyncQueue.isEmpty();
}


Time FileManager::getAsyncIdleTime() const
{
	MutexGuard mg(impl->queueMutex);
	if (impl->numAsyncReadingTasks != 0 || !impl->asyncQueue.isEmpty())
		return Time::zero;

	return impl->asyncIdleTimer.getTime();
}


#if FB_FILE_MANAGER_STATS == FB_TRUE

	static SmallTempString getTimeStamp(const FileManager::Impl::AsyncPeriod &period)
	{
		static const StringRef spaces("      ");
		SmallTempString str(spaces);
		str << period.timeStamp.getMilliseconds();
		if (str.getLength() > spaces.getLength())
			str.trimLeft(str.getLength() - spaces.getLength());

		str << " ms: ";
		return str;
	}

	void FileManager::getAsyncStats(HeapString &resultOut, bool includeRawData) const
	{
		MutexGuard mg(impl->periodTrackingMutex);
		SizeType numIdlePeriods = 0;
		SizeType numBusyPeriods = 0;
		SizeType numProcessPeriods = 0;
		SizeType numReadPeriods = 0;
		SizeType numSyncedReadPeriods = 0;
		BigSizeType totalBytesRead = 0;
		BigSizeType totalBytesWithHolesRead = 0;
		BigSizeType totalSyncedBytesRead = 0;
		SizeType totalFilesRead = 0;
		Time totalTimeIdling;
		Time totalTimeBusy;
		Time totalTimeProcessing;
		Time totalTimeReading;
		Time totalTimeSyncedReading;
		TempString rawData;
		StringRef statsIndent("\t");
		/* Use extra spaces in timestamp as indent */
		StringRef rawDataIndent("\t");
		for (const Impl::AsyncPeriod &period : impl->periods)
		{
			switch (period.type)
			{
			case Impl::AsyncPeriodTypeIdle:
				++numIdlePeriods;
				totalTimeIdling += period.duration;
				if (includeRawData)
					rawData << rawDataIndent << getTimeStamp(period) << "Idle:       " << period.duration.getMilliseconds() << " ms" << FB_PLATFORM_LF;

				break;
			case Impl::AsyncPeriodTypeBusy:
				++numBusyPeriods;
				totalTimeBusy += period.duration;
				if (includeRawData)
					rawData << rawDataIndent << getTimeStamp(period) << "Busy:       " << period.duration.getMilliseconds() << " ms" << FB_PLATFORM_LF;

				break;
			case Impl::AsyncPeriodTypeRead:
				++numReadPeriods;
				totalTimeReading += period.duration;
				totalBytesRead += period.bytesRead;
				totalBytesWithHolesRead += period.bytesReadWithHoles;
				totalFilesRead += period.filesRead;
				if (includeRawData)
				{
					rawData << rawDataIndent << getTimeStamp(period) << "Reading:    " << period.duration.getMilliseconds() << " ms, " <<
						(period.bytesRead >> 10) << " kB, " << period.filesRead << " file(s) (";
					if (period.duration != Time::zero)
						rawData << ((period.bytesRead * 1000 * Time::getTicksInMillisecond() / period.duration.getTicks()) >> 10);
					else
						rawData << "infinite";

					rawData << " kB/s, with holes " << (period.bytesReadWithHoles >> 10) << " kB, ";
					if (period.duration != Time::zero)
						rawData << ((period.bytesReadWithHoles * 1000 * Time::getTicksInMillisecond() / period.duration.getTicks()) >> 10);
					else
						rawData << "infinite";

					rawData << " kB/s), " << period.fileName << "" << FB_PLATFORM_LF;
				}
				break;
			case Impl::AsyncPeriodTypeProcess:
				++numProcessPeriods;
				totalTimeProcessing += period.duration;
				if (includeRawData)
					rawData << rawDataIndent << getTimeStamp(period) << "Processing: " << period.duration.getMilliseconds() << " ms, " << period.filesRead << " file(s): " << period.fileName << "" << FB_PLATFORM_LF;

				break;
			case Impl::AsyncPeriodTypeSyncedRead :
				++numSyncedReadPeriods;
				totalTimeSyncedReading += period.duration;
				totalSyncedBytesRead += period.bytesRead;
				if (includeRawData)
				{
					rawData << rawDataIndent << getTimeStamp(period) << "SyncedRead: " << period.duration.getMilliseconds() << " ms, " <<
						(period.bytesRead >> 10) << " kB, " << period.filesRead << " file(s) (";
					if (period.duration != Time::zero)
						rawData << ((period.bytesRead * 1000 * Time::getTicksInMillisecond() / period.duration.getTicks()) >> 10);
					else
						rawData << "infinite";

					rawData << " kB/s), " << period.fileName << "" << FB_PLATFORM_LF;
				}
				break;
			default:
				fb_assert(0 && "Invalid type");
				break;
			}
		}
		/* Trim trailing line feed */
		if (!rawData.isEmpty())
			rawData.trimRight(1);

		Time startTime = !impl->periods.isEmpty() ? impl->periods.getFront().timeStamp : Time::zero;
		Time endTime = !impl->periods.isEmpty() ? impl->periods.getBack().timeStamp : Time::zero;
		Time deltaTime = endTime - startTime;

		resultOut << "Async stats:" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Measuring run: processed " << (totalBytesRead >> 10) << " kB in " <<
			deltaTime.getMilliseconds() << " ms, ";
		if (deltaTime != Time::zero)
			resultOut << ((totalBytesRead * 1000 * Time::getTicksInMillisecond() / deltaTime.getTicks()) >> 10);
		else
			resultOut << "infinite";

		resultOut << " kB/s (with holes " << (totalBytesWithHolesRead >> 10) << "kB, ";
		if (deltaTime != Time::zero)
			resultOut << ((totalBytesWithHolesRead * 1000 * Time::getTicksInMillisecond() / deltaTime.getTicks()) >> 10);
		else
			resultOut << "infinite";

		resultOut << " kB/s";

		resultOut << statsIndent << "Idle periods: " << numIdlePeriods << " (" << totalTimeIdling.getMilliseconds() << " ms)" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Busy periods: " << numBusyPeriods << " (" << totalTimeBusy.getMilliseconds() << " ms)" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Reading periods: " << numReadPeriods << " (" << totalTimeReading.getMilliseconds() << " ms, " << 
			(totalBytesRead >> 10) << " kB, with holes " << (totalBytesWithHolesRead >> 10) << " kB)" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Processing periods: " << numProcessPeriods << " (" << totalTimeProcessing.getMilliseconds() << " ms)" << FB_PLATFORM_LF;
		resultOut << statsIndent << "" << FB_PLATFORM_LF;
		float filesPerPeriod = numReadPeriods > 0 ? float(totalFilesRead) / float(numReadPeriods) : 0.0f;
		resultOut << statsIndent << "Total files read async: " << totalFilesRead << " (" << filesPerPeriod << " files per read)" << FB_PLATFORM_LF;
		uint64_t averageReadSize = numReadPeriods > 0 ? totalBytesRead / numReadPeriods : 0;
		uint64_t averageReadSizeWithHoles = numReadPeriods > 0 ? totalBytesWithHolesRead / numReadPeriods : 0;
		resultOut << statsIndent << "Average read size: " << (averageReadSize >> 10) << " kB (with holes " << (averageReadSizeWithHoles >> 10) << " kB)" << FB_PLATFORM_LF;
		uint64_t averageFileSize = totalFilesRead > 0 ? totalBytesRead / totalFilesRead : 0;
		resultOut << statsIndent << "Average file size: " << (averageFileSize >> 10) << " kB" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Raw reading speed: ";
		if (totalTimeReading != Time::zero)
			resultOut << ((totalBytesRead * 1000 * Time::getTicksInMillisecond() / totalTimeReading.getTicks()) >> 10);
		else
			resultOut << "infinite";
	
		resultOut << " kB/s" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Effective reading speed: ";
		if (totalTimeProcessing != Time::zero)
			resultOut << ((totalBytesRead * 1000 / totalTimeProcessing.getMilliseconds()) >> 10);
		else
			resultOut << "infinite";

		resultOut << " kB/s" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Synced read periods: " << numSyncedReadPeriods << " (" << totalTimeSyncedReading.getMilliseconds() << " ms)" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Synced read bytes: " << (totalSyncedBytesRead >> 10) << " kB (";
		if (totalTimeSyncedReading != Time::zero)
			resultOut << ((totalSyncedBytesRead * 1000 * Time::getTicksInMillisecond() / totalTimeSyncedReading.getTicks()) >> 10);
		else
			resultOut << "infinite";

		resultOut << " kB/s)" << FB_PLATFORM_LF;

		resultOut << statsIndent << "Parameters: maxNumAsyncReadingTasks: " << maxNumAsyncReadingTasks <<
			", maxReadSize: " << (maxReadSize >> 10) <<
			" kB, preferredReadSize: " << (preferredReadSize >> 10) <<
			" kB, allowedHoleSize: " << (allowedHoleSize >> 10) << " kB" << FB_PLATFORM_LF;
		resultOut << statsIndent << "" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Note: processing includes time spent reading" << FB_PLATFORM_LF;
		resultOut << statsIndent << "Note: reading doesn't include memory resident packages" << FB_PLATFORM_LF;
	
		if (includeRawData)
		{
			resultOut << statsIndent << "Note: each async read period should be followed by corresponding process period" << FB_PLATFORM_LF;
			resultOut << statsIndent << "Raw data:" << FB_PLATFORM_LF;
			resultOut << rawData;
		}
	}

	void FileManager::resetAsyncStats()
	{
		MutexGuard mg(impl->periodTrackingMutex);
		impl->periods.clear();
		impl->asyncIdleTimer.reset();
	}

#endif

	
bool FileManager::doesFileExist(const DynamicString &fileName, FileFlagMask flags)
{
	FB_ZONE("FileManager::doesFileExist");
	FB_VALIDATE_LOWLEVEL_FILEPATH(fileName);

	static FileFlagMask allowedFlags(FileFlagExcludeNonArchive | FileFlagExcludeUser | FileFlagExcludeNormal);
	Impl::checkFlags(allowedFlags, flags);

	bool excludeNonArchive = (flags & FileFlagExcludeNonArchive) != 0;
	bool excludeUser = (flags & FileFlagExcludeUser) != 0;
	bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

	for (Impl::PackageHolder &holder : impl->packages)
	{
		if (excludeUser && holder.userFilePackage)
			continue;

		if (excludeNormal && !holder.userFilePackage)
			continue;

		if (excludeNonArchive)
		{
			if (holder.package->doesFileExistInArchive(fileName))
				return true;
		}
		else
		{
			if (holder.package->doesFileExist(fileName))
				return true;
		}
	}
	return false;
}


file::TimeStamp64 FileManager::getFileTimeStamp(const DynamicString &fileName, FileFlagMask flags)
{
	FB_ZONE("FileManager::getFileTimeStamp");
	FB_VALIDATE_LOWLEVEL_FILEPATH(fileName);

	static FileFlagMask allowedFlags(FileFlagExcludeUser | FileFlagExcludeNormal);
	Impl::checkFlags(allowedFlags, flags);

	bool excludeUser = (flags & FileFlagExcludeUser) != 0;
	bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

	for (SizeType i = 0, num = impl->packages.getSize(); i < num; i++)
	{
		Impl::PackageHolder &holder = impl->packages[i];
		if (excludeUser && holder.userFilePackage)
			continue;

		if (excludeNormal && !holder.userFilePackage)
			continue;

		file::TimeStamp64 stamp = holder.package->getFileTimeStamp(fileName);
		if (stamp != -1)
			return stamp;
	}
	return file::TimeStamp64(-1);
}


BigSizeType FileManager::getFileSize(const DynamicString &fileName, FileFlagMask flags)
{
	return impl->getFileSize(fileName, flags, false);
}


BigSizeType FileManager::getFileSizeOnDisk(const DynamicString &fileName, FileFlagMask flags)
{
	return impl->getFileSize(fileName, flags, true);
}


bool FileManager::writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, FileFlagMask flags)
{
	FB_ZONE("FileManager::writeFile");
	FB_VALIDATE_LOWLEVEL_FILEPATH(fileName);

	static FileFlagMask allowedFlags(FileFlagCreatePathIfNecessary | FileFlagExcludeUser | FileFlagExcludeNormal);
	Impl::checkFlags(allowedFlags, flags);

	bool createPath = (flags & FileFlagCreatePathIfNecessary) != 0;
	bool excludeUser = (flags & FileFlagExcludeUser) != 0;
	bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

	for (SizeType i = 0; i < impl->packages.getSize(); i++)
	{
		Impl::PackageHolder &holder = impl->packages[i];
		if (excludeUser && holder.userFilePackage)
			continue;

		if (excludeNormal && !holder.userFilePackage)
			continue;

		if (impl->packages[i].package->writeFile(fileName, data, dataSize, createPath))
			return true;
	}
	return false;
}


bool FileManager::writeUserFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, FileFlagMask flags)
{
	flags |= FileFlagExcludeNormal;
	return writeFile(fileName, data, dataSize, flags);
}


bool FileManager::deleteFile(const DynamicString &fileName, FileFlagMask flags)
{
	FB_ZONE("FileManager::deleteFile");
	FB_VALIDATE_LOWLEVEL_FILEPATH(fileName);

	static FileFlagMask allowedFlags(FileFlagSuppressErrorMessages | FileFlagExcludeUser | FileFlagExcludeNormal);
	Impl::checkFlags(allowedFlags, flags);

	bool suppressErrorMessages = (flags & FileFlagSuppressErrorMessages) != 0;
	bool excludeUser = (flags & FileFlagExcludeUser) != 0;
	bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

	for (SizeType i = 0; i < impl->packages.getSize(); i++)
	{
		Impl::PackageHolder &holder = impl->packages[i];
		if (excludeUser && holder.userFilePackage)
			continue;

		if (excludeNormal && !holder.userFilePackage)
			continue;

		/* Don't try to delete non-existing files */
		if (!holder.package->doesFileExist(fileName))
			continue;

		if (holder.package->deleteFile(fileName, suppressErrorMessages))
			return true;
	}
	if (!suppressErrorMessages)
	{
		FB_LOG_ERROR(FB_MSG("Could not delete file ", fileName, ". File not found"));
	}
	return false;
}


bool FileManager::deleteUserFile(const DynamicString &fileName, FileFlagMask flags)
{
	flags |= FileFlagExcludeNormal;
	return deleteFile(fileName, flags);
}


void FileManager::listFiles(Vector<DynamicString> &results, const StringRef &dirWithMaybeSlash, FileFlagMask flags)
{
	FB_ZONE("FileManager::listFiles");
	TempString dir(dirWithMaybeSlash);
	/* Remove trailing slash */
	/* FIXME: This is actually pretty stupid, as some file packages will require the trailing slash and add it, if it 
	 * is missing. Would probably make more sense to require it for all packages */
	if (dir.getLength() >= 1 && dir[dir.getLength()-1] == '/')
		dir.trimRight(1);

	FB_VALIDATE_LOWLEVEL_FILEPATH(dir);

	static const FileFlagMask allowedFlags(FileFlagExcludeFiles | FileFlagExcludeDirs | FileFlagNoRecurse | FileFlagExcludeUser | FileFlagExcludeNormal);
	Impl::checkFlags(allowedFlags, flags);

	static const FileFlagMask packageFlagMask = FileFlagMask(FileFlagExcludeFiles | FileFlagExcludeDirs | FileFlagNoRecurse);
	FileFlagMask packageFlags = (flags & packageFlagMask);
	bool excludeUser = (flags & FileFlagExcludeUser) != 0;
	bool excludeNormal = (flags & FileFlagExcludeNormal) != 0;

	package::IFilePackage::FileList fileList;
	for (SizeType i = 0; i < impl->packages.getSize(); i++)
	{
		Impl::PackageHolder &holder = impl->packages[i];
		if (excludeUser && holder.userFilePackage)
			continue;

		if (excludeNormal && !holder.userFilePackage)
			continue;

		holder.package->listFiles(fileList, dir, packageFlags);
	}

	SizeType i = results.getSize();
	results.resize(results.getSize() + fileList.getElementCount());
	package::IFilePackage::FileList::ConstIterator it = fileList.getBegin();
	for (; it != fileList.getEnd(); it++)
		results[i++] = it.getKey();
}


void FileManager::addPackage(package::IFilePackage *package, bool addToHighestPriority)
{
	FB_STACK_METHOD();
	fb_assert(impl->asyncQueue.isEmpty() && "Too late to add packages");
	if (addToHighestPriority)
		impl->packages.insert(impl->packages.getBegin(), Impl::PackageHolder(package, false));
	else
		impl->packages.pushBack(Impl::PackageHolder(package, false));
}


void FileManager::addUserPackage(package::IFilePackage *package, bool addToHighestPriority)
{
	FB_STACK_METHOD();
	fb_assert(impl->asyncQueue.isEmpty() && "Too late to add packages");
	if (addToHighestPriority)
		impl->packages.insert(impl->packages.getBegin(), Impl::PackageHolder(package, true));
	else
		impl->packages.pushBack(Impl::PackageHolder(package, true));
}


void FileManager::sortFiles(FileSortInfo *pointer, SizeType files)
{
	// As first packages are most important for reading, sort out the files starting from the back!
	if (!impl->packages.isEmpty())
	{
		SizeType size = impl->packages.getSize();
		for (SizeType i = size - 1; i < size; --i)
			impl->packages[i].package->sortFiles(pointer, files);
	}
}


bool FileManager::resolveFile(const DynamicString &fileName, DynamicString &resultFileName, uint32_t &offset, uint32_t *size)
{
	for (SizeType i = 0; i < impl->packages.getSize(); ++i)
	{
		if (impl->packages[i].package->resolveFile(fileName, resultFileName, offset, size))
			return true;
	}

	return false;
}


void FileManager::waitForFileSystem()
{
	for (Impl::PackageHolder &holder : impl->packages)
		holder.package->waitForFileSystem();
}


void FileManager::dumpUnusedFiles()
{
#if FB_TRACK_UNUSED_FILES == FB_TRUE
	file::DebugBlockWriter writer(0);
	writer.open("unusedfiles");
	file::DebugBlockWriter writer2(0);
	writer2.open("loadedfiles");
	for (int i = 0; i < (int)impl->packages.getSize(); i++)
	{
		impl->packages[i]->dumpUnusedFiles(writer, writer2);
	}
	writer.flush();
#endif
}

FB_END_PACKAGE1()
