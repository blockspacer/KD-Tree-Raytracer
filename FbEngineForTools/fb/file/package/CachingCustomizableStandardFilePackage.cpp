#include "Precompiled.h"

#include "CachingCustomizableStandardFilePackage.h"

#include "fb/container/LinearMap.h"
#include "fb/container/PodVector.h"
#include "fb/file/CreateDirectory.h"
#include "fb/file/DeleteFile.h"
#include "fb/file/DoesFileExist.h"
#include "fb/file/FileAllocator.h"
#include "fb/file/FileList.h"
#include "fb/file/FileSystemListener.h"
#include "fb/file/InputFile.h"
#include "fb/file/OutputFile.h"
#include "fb/file/RootPath.h"
#include "fb/file/TimeStamp.h"
#include "fb/lang/CallStack.h"
#include "fb/lang/Const.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/profiling/ScopedProfiler.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/task/Scheduler.h"
#include "fb/string/StaticString.h"

#define FB_USE_MEMORY_MAPPED_FILE_SUPPORT FB_MEMORY_MAPPED_FILE_SUPPORT


FB_PACKAGE2(file, package)

FB_STACK_SET_CLASS(CachingCustomizableStandardFilePackage);

class CachingCustomizableStandardFilePackage::Impl
{
public:
	/* Check from cache if file exists before trying to open it */
	static const bool optimizeOpenWithExistenceCache = true;

	/* Basic stuff */
	typedef TempString TempFileString;
	StaticString basePath;
	CustomizableStandardFilePackage::FilePackageFilter filter;

	/* Cache stuff */
	enum FB_ENUM_UNSIGNED
	{
		FileInfoCacheStartingSize = 65536,
		DirInfoCacheStartingSize = 8192,
		FileScanInitialReserve = 128,
	};
	struct FileInfo
	{
		FileInfo() : lastModified(0), size(0) { }

		file::TimeStamp64 lastModified;
		BigSizeType size;
	};

	typedef LinearHashMap<DynamicString, FileInfo> FileInfoCache;
	FileInfoCache fileInfoCache;

	/* Most directories have only few files and fewer subdirectories. Average is less than 20 files per dir under 
	 * binary, even with few outliers. However, the outliers (shader dirs in builds), with tens of thousands of files 
	 * may be a problem in some cases. */
	struct DirectoryInfo
	{
		Vector<DynamicString> files;
		Vector<DynamicString> subDirs;
	};

	typedef LinearHashMap<DynamicString, DirectoryInfo> DirectoryInfoCache;
	DirectoryInfoCache dirInfoCache;

	typedef Vector<TempFileString> AbsolutePathVector;
	struct InitTask : public task::ISchedulerTask
	{
		struct Params
		{
			Params() { }

			AbsolutePathVector* absolutePaths = nullptr;

			FB_SCHEDULER_TASK_PARAM_TYPE();
		};
		InitTask() :impl(nullptr) { }

		const char *getStaticTaskNameString() const
		{
			return "CachingCustomizableStandardFilePackage init task";
		}

		virtual void run(const task::SchedulerTaskData &data, task::Scheduler &scheduler)
		{
			//profiling::ScopedProfiler sp(FB_FUNCTION);
			Params p;
			data.getParam(p);
			impl->listFilesForInit(*p.absolutePaths);
			delete p.absolutePaths;
		}

		Impl* impl = nullptr;
	};

	Mutex mutex;
	task::Scheduler* scheduler = nullptr;
	task::Scheduler::TaskGroup initTaskGroup = task::Scheduler::TaskGroupForeground;
	task::Scheduler::DependencyGroupId initDependencyId;
	InitTask initTask;
	TempFileString absoluteBasePath;
	bool initialized = false;

	file::FileSystemListener fileListener;
	CustomizableStandardFilePackage standardPackage;

	static StaticString getFileListenerPath(const StringRef &basePath)
	{
		StringRef path = basePath != "./" ? basePath : StringRef::empty;
		/* If directory doesn't exist yet, create it */
		if (!path.isEmpty())
			file::createPathIfMissing(path);

		return StaticString::createFromAnyString(path);
	}


	Impl(const DynamicString &basePath, const CustomizableStandardFilePackage::FilePackageFilter &filter, task::Scheduler* scheduler)
		: basePath(basePath != "./" ? basePath : StaticString::empty)
		, filter(filter)
		, scheduler(scheduler)
		, initDependencyId(task::Scheduler::getInvalidDependencyGroupId())
		, fileListener(getFileListenerPath(basePath))
		, standardPackage(basePath, filter)
	{
		init();
	}

	TempFileString getAbsoluteFilename(const DynamicString fileName)
	{
		TempFileString str;
		str += file::RootPath::get();
		str += basePath;
		str += fileName;
		return str;
	}


	void updateFileInfo(const DynamicString& fileName)
	{
		/* Early out: if file doesn't exist anymore, there's no use to cache it for a while (happens a lot during 
		 * processing) */
		if (!file::doesFileExist(getAbsoluteFilename(fileName), true))
			return;

		updateFileInfo(fileName, fileInfoCache[fileName]);
		/* Presume we must update directory cache too */
		StaticString dirname = getDirectoryNameFromFilename(fileName);
		{
			DirectoryInfo& info = dirInfoCache[dirname];
			Vector<DynamicString>& files = info.files;
			for (SizeType i = 0, num = files.getSize(); i < num; ++i)
			{
				if (files[i] == fileName)
					return;
			}
			files.pushBack(fileName);
		}
		if (dirname.isEmpty())
			return;

		/* Make sure higher level dirs exist too */
		TempFileString upperDirNameConstructionHelper(dirname.getPointer(), dirname.getLength() - 1);
		StaticString upperDirname = getDirectoryNameFromFilename(upperDirNameConstructionHelper);
		while (!upperDirname.isEmpty())
		{
			DirectoryInfo& info = dirInfoCache[upperDirname];
			for (SizeType i = 0, num = info.subDirs.getSize(); i < num; ++i)
			{
				if (info.subDirs[i] == dirname)
				{
					/* Dir was found, all done */
					return;
				}
			}
			/* Dir was not found. Add and try with upper again. */
			info.subDirs.pushBack(dirname);
			upperDirNameConstructionHelper.clear();
			upperDirNameConstructionHelper.append(upperDirname.getPointer(), upperDirname.getLength() - 1);
			dirname = upperDirname;
			upperDirname = getDirectoryNameFromFilename(upperDirNameConstructionHelper);
		}
	}

	void updateFileInfo(const DynamicString& fileName, FileInfo& info, const file::FileList::Entry& entry)
	{
		fb_expensive_assert(!fileName.isEmpty());
		fb_expensive_assert(fileName[fileName.getLength() - 1] != '/');
		info.lastModified = entry.lastModified;
		info.size = entry.size;
	}

	void updateFileInfo(const DynamicString& fileName, FileInfo& info)
	{
		fb_expensive_assert(!fileName.isEmpty());
		fb_expensive_assert(fileName[fileName.getLength() - 1] != '/');
		fileName.convertToStatic();
		TempFileString absoluteName = getAbsoluteFilename(fileName);
		info.lastModified = file::getFileTimestamp64(absoluteName);
		file::InputFile file;
		if (file.open(absoluteName))
			info.size = file.getSize();
	}

	void removeFile(const DynamicString& fileName)
	{
		{
			FileInfoCache::Iterator iter = fileInfoCache.find(fileName);
			if (iter != fileInfoCache.getEnd())
				fileInfoCache.erase(iter);
		}
		/* FileSystemListener doesn't support directories, so we don't either */
		fb_assert(!isDirectoryName(fileName) && "Removing directories not supported");
		StaticString dirname = getDirectoryNameFromFilename(fileName);
		DirectoryInfo& info = dirInfoCache[dirname];
		Vector<DynamicString>& files = info.files;
		for (SizeType i = 0, num = files.getSize(); i < num; ++i)
		{
			if (files[i] == fileName)
			{
				files[i] = files.getBack();
				files.popBack();
				break;
			}
		}
	}

	template<typename StringType>
	static StaticString getDirectoryNameFromFilename(const StringType& fileName)
	{
		fb_assert(!isDirectoryName(fileName));
		for (SizeType i = fileName.getLength() - 1; i < fileName.getLength(); --i)
		{
			if (fileName[i] == '/')
				return StaticString(fileName.getPointer(), i + 1);
		}
		return StaticString::empty;
	}

	static bool isDirectoryName(const StringRef& fileName)
	{
		return !fileName.isEmpty() && fileName[fileName.getLength() - 1] == '/';
	}

	void listFilesForInit(AbsolutePathVector& absolutePathsToExplore)
	{
		PodVector<FileInfo> infoResultVector;
		DirectoryInfo dirInfo;
		infoResultVector.reserve(FileScanInitialReserve);
		/* Note that dirInfo gets copied while inserting to actual cache,  */
		dirInfo.files.reserve(FileScanInitialReserve);
		dirInfo.subDirs.reserve(FileScanInitialReserve);
		/* Quick start. Create new tasks as long as we are generally speaking finding more work to do */
		SizeType spawnLimit = lang::max<SizeType>(2, absolutePathsToExplore.getSize());
		while (!absolutePathsToExplore.isEmpty())
		{
			listFilesForInit(absolutePathsToExplore, dirInfo, infoResultVector);
			if (absolutePathsToExplore.getSize() > spawnLimit)
			{
				spawnLimit = absolutePathsToExplore.getSize();
				SizeType numToSeparate = spawnLimit / 2;
				InitTask::Params p;
				AbsolutePathVector* startPoint = new AbsolutePathVector();
				startPoint->reserve(FileScanInitialReserve > numToSeparate ? FileScanInitialReserve : numToSeparate);
				startPoint->insert(startPoint->getBegin(), absolutePathsToExplore.getEnd() - numToSeparate, absolutePathsToExplore.getEnd());
				absolutePathsToExplore.erase(absolutePathsToExplore.getEnd() - numToSeparate, absolutePathsToExplore.getEnd());
				/* Swap vectors to continue with the latest directories. They are most likely to be in cache. */
				absolutePathsToExplore.swap(*startPoint);
				p.absolutePaths = startPoint;
				scheduler->addTask(initTaskGroup, initDependencyId, task::Scheduler::TaskPriorityHigh, &initTask, task::SchedulerTaskData(p));
			}
		}
	}

	void listFilesForInit(AbsolutePathVector& absolutePathsToExplore, DirectoryInfo& dirInfo, PodVector<FileInfo>& infoResultVector)
	{
		TempFileString absolutePath = absolutePathsToExplore.getBack();
		absolutePathsToExplore.popBack();
		file::FileList list(absolutePath, file::FileList::FlagIncludeSpecialDirs);
		file::FileList::Entry *entry = nullptr;
		while ((entry = list.next()) != nullptr)
		{
			TempFileString fullEntryName(absolutePath);
			fullEntryName += entry->name;
			StaticString fileNameInPackage(fullEntryName.getPointer() + absoluteBasePath.getLength(), fullEntryName.getLength() - absoluteBasePath.getLength());
			if (entry->directory)
			{
				if (!filter.isPathExcluded(fileNameInPackage))
				{
					fullEntryName += "/";
					StaticString directoryFilenameInPackage(fullEntryName.getPointer() + absoluteBasePath.getLength(), fullEntryName.getLength() - absoluteBasePath.getLength());
					dirInfo.subDirs.pushBack(directoryFilenameInPackage);
					absolutePathsToExplore.pushBack(fullEntryName);
				}
				else
				{
					continue;
				}
			}
			else
			{
				dirInfo.files.pushBack(fileNameInPackage);
				infoResultVector.pushBack(FileInfo());
				updateFileInfo(fileNameInPackage, infoResultVector.getBack(), *entry);
			}
		}
		/* One directory handled */
		StaticString thisDirFilenameInPackage(absolutePath.getPointer() + absoluteBasePath.getLength(), absolutePath.getLength() - absoluteBasePath.getLength());
		{
			/* Sync */
			MutexGuard guard(mutex);
			if (fileInfoCache.getCapacity() < FileInfoCacheStartingSize)
			{
				fb_assert(dirInfoCache.getCapacity() < DirInfoCacheStartingSize);
				fileInfoCache.reserve(FileInfoCacheStartingSize);
				dirInfoCache.reserve(DirInfoCacheStartingSize);
			}

			for (SizeType i = 0, num = dirInfo.files.getSize(); i < num; ++i)
				fileInfoCache.insertOrAssign(dirInfo.files[i], infoResultVector[i]);

			for (SizeType i = 0, num = dirInfo.subDirs.getSize(); i < num; ++i)
				fileInfoCache.insertOrAssign(dirInfo.subDirs[i], FileInfo());

			dirInfoCache.insertOrAssign(thisDirFilenameInPackage, dirInfo);
		}
		/* Clear vectors for next round */
		dirInfo.files.clear();
		dirInfo.subDirs.clear();
		infoResultVector.clear();
	}

	void init()
	{
		fb_assert(!isPathExcluded(basePath));
		this->absoluteBasePath = getAbsoluteFilename(DynamicString::empty);
		InitTask::Params p;
		AbsolutePathVector* startPoint = new AbsolutePathVector();
		startPoint->reserve(1024);
		startPoint->pushBack(TempFileString(getAbsoluteFilename(DynamicString::empty)));
		p.absolutePaths = startPoint;
		initTask.impl = this;
		initDependencyId = scheduler->createDependencyGroup(initTaskGroup);
		scheduler->addTask(initTaskGroup, initDependencyId, task::Scheduler::TaskPriorityHigh, &initTask, task::SchedulerTaskData(p));
	}

	void update()
	{
		if (initialized)
		{
			file::FileSystemListener::ResultList results;
			fileListener.swapResults(file::FileSystemListener::FlagAll, results);
			if (!results.isEmpty())
			{
				/* Handle changed files */
				// We get a lot of duplicates from the listener, so track their state in here.
				// Boolean value is true IFF file needs to be inserted infoCache
				typedef LinearHashMap<StaticString, bool> ModifiedFileState;
				ModifiedFileState modifiedFileStates;
				modifiedFileStates.reserve(results.getSize());

				for (uint32_t i = 0, num = results.getSize(); i < num; ++i)
				{
					const HeapString &result = results[i];
					if (isPathExcluded(result))
						continue;

					StaticString fileName(result.getPointer() + basePath.getLength(), result.getLength() - basePath.getLength());

					// Do not update same file multiple times
					{
						ModifiedFileState::Iterator it = modifiedFileStates.find(fileName);
						if (it != modifiedFileStates.getEnd())
							continue;
					}

					FileInfoCache::Iterator cacheIterator = fileInfoCache.find(fileName);
					bool doesFileExistOnDisk = file::doesFileExist(result);
					bool doesFileExistInCache = cacheIterator != fileInfoCache.getEnd();

					// Only do updates here which keep current cache sorted

					bool insertFileToCache = false;
					if (doesFileExistInCache && !doesFileExistOnDisk)
					{
						removeFile(fileName);
					}
					else if (doesFileExistInCache && doesFileExistOnDisk)
					{
						// Just update time stamp
						updateFileInfo(fileName, cacheIterator.getValue());
					}
					else if (!doesFileExistInCache && doesFileExistOnDisk)
					{
						// Delay file add
						insertFileToCache = true;
					}

					modifiedFileStates.insertOrAssign(fileName, insertFileToCache);
				}

				for (ModifiedFileState::Iterator it = modifiedFileStates.getBegin(), itEnd = modifiedFileStates.getEnd(); it != itEnd; ++it)
				{
					if (it.getValue() == false)
						continue;

					const StaticString& fileName = it.getKey();
					FileInfo info;
					updateFileInfo(fileName);
				}
			}
		}
		else
		{
			profiling::LoggingScopedProfiler sp(FB_FUNCTION" Non-initialized");
			mutex.leave();
			scheduler->waitForDependencies(initTaskGroup, initDependencyId);
			mutex.enter();
			this->initialized = true;
			update();
		}
	}

	/**
	 * This will return true if the filepath should be excluded. The given path may specify a folder or a file name.
	 */
	bool isPathExcluded(const StringRef &path)
	{
		return filter.isPathExcluded(path);
	}
};


CachingCustomizableStandardFilePackage::CachingCustomizableStandardFilePackage(const DynamicString &basePath, const CustomizableStandardFilePackage::FilePackageFilter &filter, task::Scheduler* scheduler)
{
	/* FileSystemListener can't function without a path. */
	if (!basePath.isEmpty())
		file::createPathIfMissing(basePath);

	impl.reset(new Impl(basePath, filter, scheduler));
}


CachingCustomizableStandardFilePackage::~CachingCustomizableStandardFilePackage()
{
	// nop
}


File CachingCustomizableStandardFilePackage::openFile(const DynamicString &fileName, bool archiveOnly)
{
	FB_ZONE("CachingCustomizableStandardFilePackage::openFile");
	FB_STACK_METHOD();

	/* Archive not supported */
	/* If configured so, only try to open files in cache */
	if (archiveOnly || (Impl::optimizeOpenWithExistenceCache && !doesFileExist(fileName)))
		return File(getEmptyBuffer(), 0);

	#if FB_USE_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		MemoryMappedFile file;
	#else
		InputFile file;
	#endif
	Impl::TempFileString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;
	#if FB_USE_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		if (file.open(str, true))
		{
			BigSizeType fileSize = file.getSize();
			FileBuffer *fileBuffer = new FileBuffer(lang::move(file));
			FileBufferPointer buf(fileBuffer);
			return File(buf, fileSize);
		}
	#else
		if (file.open(str))
		{
			BigSizeType bufferSize = file.getSize();
			if (bufferSize > 0)
			{
				file::FileBufferPointer buf = getFileBuffer(bufferSize, fileName.getPointer());
				file.readData(buf.getMutable(), bufferSize);
				return File(buf, bufferSize);
			}
		}
	#endif
	return File(getEmptyBuffer(), 0);
}

bool CachingCustomizableStandardFilePackage::openFiles(const FileNameVector &names, FileVector &outFiles)
{
	FB_ZONE("CachingCustomizableStandardFilePackage::openFiles");

	bool success = true;
	for (const DynamicString &fileName : names)
	{
		outFiles.pushBack(openFile(fileName, false));
		success = success && outFiles.getBack();
	}

	return success;
}


bool CachingCustomizableStandardFilePackage::doesFileExist(const DynamicString &fileName)
{
	MutexGuard guard(impl->mutex);
	if (!impl->initialized)
	{
		/* Still initializing. Let's do this old school. */
		return impl->standardPackage.doesFileExist(fileName);
	}
	impl->update();

	Impl::FileInfoCache::Iterator iter = impl->fileInfoCache.find(fileName);
	return iter != impl->fileInfoCache.getEnd();
}


file::TimeStamp64 CachingCustomizableStandardFilePackage::getFileTimeStamp(const DynamicString &fileName)
{
	MutexGuard guard(impl->mutex);
	impl->update();

	Impl::FileInfoCache::Iterator iter = impl->fileInfoCache.find(fileName);
	if (iter != impl->fileInfoCache.getEnd())
		return iter.getValue().lastModified;

	return file::TimeStamp64(-1);
}


BigSizeType CachingCustomizableStandardFilePackage::getFileSize(const DynamicString &fileName)
{
	MutexGuard guard(impl->mutex);
	impl->update();

	Impl::FileInfoCache::Iterator iter = impl->fileInfoCache.find(fileName);
	if (iter != impl->fileInfoCache.getEnd())
		return iter.getValue().size;

	return 0;
}


BigSizeType CachingCustomizableStandardFilePackage::getFileSizeOnDisk(const DynamicString &fileName)
{
	return getFileSize(fileName);
}


bool CachingCustomizableStandardFilePackage::writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath)
{
	FB_STACK_METHOD();

	bool isExcluded = impl->isPathExcluded(fileName);
	fb_assert(!isExcluded && "Notice, writing a file that will be excluded by read operations (due to path exclusion filter).");

	file::OutputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;
	if (createPath)
		file::createPathIfMissing(str);

	MutexGuard guard(impl->mutex);
	if (!file.open(str))
		return false;

	file.writeData(data, dataSize);
	file.close();
	if (!isExcluded)
	{
		impl->update();
		fileName.convertToStatic();
		impl->updateFileInfo(fileName);
	}
	return true;
}


bool CachingCustomizableStandardFilePackage::deleteFile(const DynamicString &fileName, bool suppressErrorMessages)
{
	FB_STACK_METHOD();

	if (!impl->isPathExcluded(fileName))
	{
		fileName.convertToStatic();
		MutexGuard guard(impl->mutex);
		impl->update();

		Impl::FileInfoCache::Iterator iter = impl->fileInfoCache.find(fileName);
		if (iter != impl->fileInfoCache.getEnd())
			impl->removeFile(fileName);
	}
	else
	{
		fb_assert(0 && "Notice, deleting a file that would have been excluded by read operations (due to path exclusion filter).");
	}

	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;
	return file::deleteFile(str, suppressErrorMessages);
}


void CachingCustomizableStandardFilePackage::listFiles(FileList &results, const StringRef &dir, FileFlagMask flags)
{
	FB_ZONE("CachingCustomizableStandardFilePackage::listFiles");

	bool includeFiles = !(flags & FileFlagExcludeFiles);
	bool includeDirs = !(flags & FileFlagExcludeDirs);
	bool recurse = !(flags & FileFlagNoRecurse);

	Impl::TempFileString modifiedDir;
	modifiedDir += file::RootPath::get();
	modifiedDir += impl->basePath;
	if (modifiedDir.getLength() > 0 && (modifiedDir[modifiedDir.getLength() - 1] != '/' && modifiedDir[modifiedDir.getLength() - 1] != '\\') && !dir.isEmpty())
		modifiedDir += "/";

	modifiedDir += dir;

	if (impl->isPathExcluded(modifiedDir))
	{
		// this seems like an unintended scenario - someone filtering out an entire dir that someone is specifically
		// trying to list files in?
		fb_expensive_assert_once("The entire listFiles call will be ignored due to path exclusion filter.");
		return;
	}

	MutexGuard guard(impl->mutex);
	impl->update();

	Impl::TempFileString directoryTmp(dir);
	if (!directoryTmp.isEmpty() && directoryTmp[directoryTmp.getLength() - 1] != '/')
		directoryTmp += "/";

	Vector<DynamicString> dirsToCheck;
	dirsToCheck.reserve(recurse ? 128U : 1U);
	dirsToCheck.pushBack(DynamicString(directoryTmp.getPointer(), directoryTmp.getLength()));

	while (!dirsToCheck.isEmpty())
	{
		if (dirsToCheck.getBack().doesContain("/dev/"))
		{
			dirsToCheck.popBack();
			continue;
		}

		Impl::DirectoryInfo& info = impl->dirInfoCache[dirsToCheck.getBack()];
		dirsToCheck.popBack();
		if (includeFiles)
		{
			for (SizeType i = 0, num = info.files.getSize(); i < num; ++i)
				results.insertOrAssign(info.files[i], this);
		}
		if (includeDirs)
		{
			for (SizeType i = 0, num = info.subDirs.getSize(); i < num; ++i)
			{
				const DynamicString& dirName = info.subDirs[i];
				/* Nice (legacy) feature. File list doesn't list files in dev-folders. */
				if (dirName.doesContain("/dev/"))
					continue;

				results.insertOrAssign(dirName, this);
			}
		}
		if (recurse)
			dirsToCheck.insert(dirsToCheck.getEnd(), info.subDirs.getBegin(), info.subDirs.getEnd());
	}
}


void CachingCustomizableStandardFilePackage::sortFiles(FileSortInfo *pointer, SizeType files)
{
}


bool CachingCustomizableStandardFilePackage::resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const
{
	file::InputFile file;
	MutexGuard guard(impl->mutex);
	impl->update();

	Impl::FileInfoCache::Iterator iter = impl->fileInfoCache.find(fileName);
	if (iter == impl->fileInfoCache.getEnd())
		return false;

	resolveDataOut.offset = 0;
	resolveDataOut.rawSize = file.getSize();
	resolveDataOut.size = resolveDataOut.rawSize;
	return true;
}

bool CachingCustomizableStandardFilePackage::resolveFile(const DynamicString &fileName, DynamicString &resultFilename, uint32_t &offset, uint32_t *size)
{
	file::InputFile file;

	Impl::TempFileString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;

	MutexGuard guard(impl->mutex);
	impl->update();

	Impl::FileInfoCache::Iterator iter = impl->fileInfoCache.find(fileName);
	if (iter == impl->fileInfoCache.getEnd())
		return false;

	resultFilename = DynamicString(str);
	offset = 0;
	if (size)
		*size = uint32_t(iter.getValue().size);

	return true;
}


bool CachingCustomizableStandardFilePackage::resolvePackage(const DynamicString &fileName, ResolveData &resolveData)
{
	// We don't care
	return false;
}


void CachingCustomizableStandardFilePackage::waitForFileSystem()
{
	impl->fileListener.waitForFileSystem();
}

FB_END_PACKAGE2()
