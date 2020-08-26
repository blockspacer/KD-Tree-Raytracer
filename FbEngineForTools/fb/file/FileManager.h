#pragma once

#include "AsyncFileReceiver.h"
#include "File.h"
#include "FileFlag.h"
#include "fb/container/Vector.h"
#include "fb/lang/Pimpl.h"
#include "fb/lang/ScopedPointer.h"
#include "fb/lang/Megaton.h"
#include "fb/string/DynamicString.h"
#include "fb/file/TimeStamp.h"

FB_DECLARE(file, package, IFilePackage)
FB_DECLARE_STRUCT(file, FileSortInfo)

FB_PACKAGE1(file)

#define FB_FILE_MANAGER_STATS FB_FALSE

enum AsyncFileReadPriority
{
	AsyncFileReadPriorityIdle = 1 << 0,
	AsyncFileReadPriorityLow = 1 << 8,
	AsyncFileReadPriorityNormal = 1 << 16,
	AsyncFileReadPriorityHigh = 1 << 24,
};

class FileManager
{
	FB_MEGATON_CLASS_DECL();
public:
	FileManager();
	~FileManager();

	/**
	 * Loads and returns file (empty if loading fails). 
	 * Flag ExcludeNonArchive can be specified to instruct only try to load the file from archive package, not from 
	 * file system.
	 * Flag ExcludeUser can be specified to instruct not loading from user file packages.
	 * Flag ExcludeNormal can be specified to instruct not loading from normal file packages (user packages only).
	 */
	File openFile(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);

	/* Async file reading operations. Returned OperationIDs can be used to identify a particular readFinished() 
	 * callback, but there's no particular reason to use them. If file reading fails for some reason, the callback 
	 * will deliver invalid File (so that should be checked). If failure happens early enough (e.g. file does not 
	 * exist in any package), AsyncFileReceiver::invalidOperationID is returned. Requests with higher priority are 
	 * usually served first (normal, synchronous openFiles will have a sort of highest priority though), but this 
	 * should not be depended on */
	typedef Vector<DynamicString> FileNameVector;
	typedef PodVector<AsyncOperationID> OperationIDVector;
	AsyncOperationID openFileAsync(AsyncFileReceiver *receiver, const DynamicString &fileName, uint32_t priority = AsyncFileReadPriorityNormal);
	void openFilesAsync(AsyncFileReceiver *receiver, const FileNameVector &fileNames, OperationIDVector &idsOut, uint32_t priority = AsyncFileReadPriorityNormal);
	void openFilesAsync(AsyncFileReceiver *receiver, const FileNameVector &fileNames, uint32_t priority = AsyncFileReadPriorityNormal);
	/* Cancel all async file opens queued by given receover. Each cancelled call will trigger readFinished() with an 
	 * empty file. Some calls may already be in progress and will trigger readFinished() (in time) normally. It is 
	 * important that the caller makes sure all requests really finish before e.g. destroying itself. Returns number 
	 * of operations cancelled */
	SizeType cancelOpenFilesAsync(AsyncFileReceiver *receiver);
	/* Same as above, but only attempts to cancel a single operation. Returns true, if successful, false otherwise */
	bool cancelOpenFilesAsync(AsyncFileReceiver *receiver, AsyncOperationID id);
	/* Waits until some (unspecified) async progress is made. If you need great performance with zero additional wait 
	 * time, don't use this */
	void waitForAsyncProgress() const;
	/* Returns true, if there are async loads still going on */
	bool hasAsyncOpensInProgress() const;
	/* Returns time since last queued async file operation finished */
	Time getAsyncIdleTime() const;
	#if FB_FILE_MANAGER_STATS == FB_TRUE
		/* Appends stats about async operations after last reset (or creation of FileManager, if never reset) to given string */
		void getAsyncStats(HeapString &resultOut, bool includeRawData) const;
		/* Resets async stats */
		void resetAsyncStats();
	#endif

	/**
	 * Returns true if given file can be opened.
	 * Flag ExcludeNonArchive can be specified to instruct only check the file from archive package, not from
	 * file system.
	 * Flag ExcludeUser can be specified to instruct not checking from user file packages.
	 * Flag ExcludeNormal can be specified to instruct not checking from normal file packages (user packages only).
	 */
	bool doesFileExist(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);

	/**
	 * Returns TimeStamp for file, if available (returns -1 TimeStamp if not)
	 * Flag ExcludeNonArchive can be specified to instruct only check the file from archive package, not from
	 * file system.
	 * Flag ExcludeUser can be specified to instruct not checking from user file packages.
	 * Flag ExcludeNormal can be specified to instruct not checking from normal file packages (user packages only).
	 */
	file::TimeStamp64 getFileTimeStamp(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);

	/**
	 * Returns the size of the given file. 
	 * Will return zero for non-existing files - but will also return zero for existing, zero sized files!
	 * Flag ExcludeUser can be specified to instruct not checking from user file packages.
	 * Flag ExcludeNormal can be specified to instruct not checking from normal file packages (user packages only).
	 */
	BigSizeType getFileSize(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);
	/* Same as above, but returns size on disk (for compressed archives) */
	BigSizeType getFileSizeOnDisk(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);

	/**
	 * Creates a new file or overwrites an existing one, returns true on success.
	 * Flag CreatePathIfNecessary can be specified to instruct FileManager to creating missing directories on write.
	 * Flag ExcludeUser can be specified to instruct not writing to user file packages.
	 * Flag ExcludeNormal can be specified to instruct not writing to normal file packages (user packages only).
	 *
	 * Note: Although normal packages usually have higher priority than user packages, it's not enforced by 
	 * FileManager. So if someone decides to be clever, it may happen that plain write goes to user package.
	 */
	bool writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, FileFlagMask flags = FileFlagNone);
	/* Adds ExcludeNormal flag automatically */
	bool writeUserFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, FileFlagMask flags = FileFlagNone);

	/** 
	 * Deletes existing file, returns true on success. Parameter suppressErrorMessages can be used to prevent errors 
	 * being written to log. It does not affect return value
	 * Flag SuppressErrorMesssages can be specified to prevent writing errors to log. It does not affect return value.
	 * Flag ExcludeUser can be specified to instruct not accessing user file packages.
	 * Flag ExcludeNormal can be specified to instruct not accessing normal file packages (user packages only).
	 *
	 * Note: Although normal packages usually have higher priority than user packages, it's not enforced by
	 * FileManager. So if someone decides to be clever, it may happen that plain delete goes to user package.
	 */
	bool deleteFile(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);
	/* Adds ExcludeNormal flag automatically */
	bool deleteUserFile(const DynamicString &fileName, FileFlagMask flags = FileFlagNone);

	/**
	 * Lists files and/or directories.
	 * Flag ExcludeFiles can be specified to not list files (only dirs).
	 * Flag ExcludeDirs can be specified to not list directories (only files).
	 * Flag NoRecurse can be specified to not recurse to subdirectories.
	 * Flag ExcludeUser can be specified to instruct not accessing user file packages.
	 * Flag ExcludeNormal can be specified to instruct not accessing normal file packages (user packages only).
	 */
	void listFiles(Vector<DynamicString> &results, const StringRef &dir, FileFlagMask flags);

	/* Takes ownership of package and adds it as the lowest or highest priority package */
	void addPackage(package::IFilePackage *package, bool addToHighestPriority);
	void addUserPackage(package::IFilePackage *package, bool addToHighestPriority);

	/* Sort list of files for optimal reading performance */
	void sortFiles(FileSortInfo *pointer, SizeType files);

	/* Resolve given file name to matching raw file name/offset pair (as it might be located uncompressed inside a custom archive) */
	bool resolveFile(const DynamicString &fileName, DynamicString &resultFileName, uint32_t &offset, uint32_t *size = 0);

	/* Waits for changes in OS file system to propagate to the file packages. This should be used when there's a race 
	 * between outside process modifying the file system and FileManager noticing the modification in time. Note that 
	 * this is a relatively slow call */
	void waitForFileSystem();

	void dumpUnusedFiles();

	FB_PIMPL;
};

FB_END_PACKAGE1()

FB_MEGATON_GLOBAL_GETTER(file, FileManager);
