#include "Precompiled.h"

#include "fb/file/FileSystemListener.h"

#include "fb/container/Vector.h"
#include "fb/file/AbsolutePath.h"
#include "fb/file/DeleteFile.h"
#include "fb/file/DoesFileExist.h"
#include "fb/file/FileList.h"
#include "fb/file/OutputFile.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/thread/ConditionVariable.h"
#include "fb/lang/thread/IThreadEntry.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/HeapString.h"
#include "fb/string/util/UnicodeConverter.h"
#include "fb/sys/windows/FindProcess.h"

FB_PACKAGE1(file)

#define	FB_FILESYSTEM_LISTENER_MULTITHREADED FB_TRUE

struct FileSystemListener::Data: public IThreadEntry
{
	HeapString extraPath;
	PodVector<char> readBuffer;
	/* Windows stuff */
	HANDLE directoryHandle = INVALID_HANDLE_VALUE;
	HANDLE eventHandle = INVALID_HANDLE_VALUE;
	HANDLE closeHandle = INVALID_HANDLE_VALUE;
	/* Copies of closeHandle and eventHandle, so they can be multiple waited */
	HANDLE waitHandles[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	DWORD filter;

	Thread *bgThread;
	Mutex mutex;
	/* Stuff to implement waitForFileSystem() */
	ConditionVariable conditionVar;
	static const uint32_t invalidFileSystemWaitIndex = 0xFFFFFFFF;
	uint32_t expectedFileSystemWaitIndex = invalidFileSystemWaitIndex;
	static lang::AtomicUInt32 waitFileNameRunningIndex;

	/* Results are first collected to tmpResults to avoid hanging on to mutex */
	ResultList tmpResults;
	ResultList results;
	/* When new results are added, this is updated. Allows faster checking the common case of there not being any new results */
	lang::AtomicUInt32 resultsSinceSwap;

	/* TryAgainFileList is used for collecting files that can't be readily opened when they are noticed the first time */
	struct TryAgainFile
	{
		HeapString filename;
		string::SimpleUTF16String filenameWide;
	};
	typedef Vector<TryAgainFile> TryAgainFileList;


	static StringRef getFileSystemListenerWaitFileNameBody()
	{
		return StringRef("FB_FileSystemListener_wait_file_");
	}

	static void appendFileSystemWaitFileName(HeapString &strOut, uint32_t &indexOut)
	{
		indexOut = lang::atomicIncRelaxed(waitFileNameRunningIndex);
		/* Add process index so we don't bother other instances */
		indexOut |= (sys::windows::getProcessInstanceIndex() + 1) << 20;
		strOut << getFileSystemListenerWaitFileNameBody() << indexOut << ".tmp";
	}


	Data(const StringRef &path)
	{
		TempString pathStr;
		if (!file::isAbsolutePath(path))
		{
			char pathArray[255] = { 0 };
			GetCurrentDirectory(255, pathArray);
			pathStr << pathArray;
			pathStr << "\\";
		}
		pathStr << path;

		extraPath = path;
		if (extraPath.getLength() > 0 && extraPath[extraPath.getLength() - 1] != '/')
			extraPath += "/";

		// Handles for polling and quitting
		eventHandle = CreateEvent(0, FALSE, FALSE, 0);
		closeHandle = CreateEvent(0, FALSE, FALSE, 0);
		/* Closehandle must be first, so in case of a race we can still identify close handle */
		waitHandles[0] = closeHandle;
		waitHandles[1] = eventHandle;

		// File pointer
		string::SimpleUTF16String fileWide;
		fileWide.appendUTF8(pathStr);
		DWORD shareMode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
		DWORD semantics = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;
		directoryHandle = CreateFileW(fileWide.getPointer(), FILE_LIST_DIRECTORY | GENERIC_READ, shareMode, 0, OPEN_EXISTING, semantics, 0);
		if (directoryHandle == INVALID_HANDLE_VALUE)
		{
			TempString msg;
			msg << "CreateFileW failed with path " << pathStr << ". ";
			DebugHelp::addGetLastErrorCode(msg);
			fb_assertf(0, "%s", msg.getPointer());
		}

		// Read buffer

		readBuffer.resize(1024 * 128);

		// Read flags
		filter = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_CREATION;

		// Create the worker thread
#if (FB_FILESYSTEM_LISTENER_MULTITHREADED == FB_TRUE)
		if (directoryHandle != INVALID_HANDLE_VALUE)
		{
			FB_STATIC_CONST_STRING(threadName, "FileSystemListener");
			bgThread = Thread::createNewThread(this, threadName);
		}
#else
			bgThread = nullptr;
#endif
	}


	~Data()
	{
		// Send quit
		if (closeHandle)
			SetEvent(closeHandle);

		/* Make sure no-one's waiting on us */
		{
			MutexGuard mg(mutex);
			expectedFileSystemWaitIndex = invalidFileSystemWaitIndex;
		}
		conditionVar.broadcast();

		// Wait for exit
		if (bgThread)
			Thread::joinThread(bgThread);

		// Free resources
		if (closeHandle != INVALID_HANDLE_VALUE)
			CloseHandle(closeHandle);

		if (eventHandle != INVALID_HANDLE_VALUE)
			CloseHandle(eventHandle);

		if (directoryHandle != INVALID_HANDLE_VALUE)
			CloseHandle(directoryHandle);

		FB_LOG_INFO(FB_MSG("Stopped listening to", extraPath.isEmpty() ? HeapString::empty : extraPath));
	}


	void addFilesFromDir(const HeapString &path)
	{
		file::FileList lst(path);
		file::FileList::Entry *entry = nullptr;
		while ( (entry = lst.next()) != nullptr )
		{
			if (!entry->name.doesContain("/.svn")
				&& !entry->name.doesContain("/.git")
				&& !entry->name.doesContain("/dev/")
				&& !entry->name.doesContain("/temp/sym/")
				&& !entry->name.doesContain("/temp/pdb/"))
			{
				if (entry->directory)
				{
					// recurse
					addFilesFromDir( (path + "/" + entry->name) );
				}
				else
				{
					tmpResults.pushBack( (path + "/" + entry->name) );
				}
			}
		}
	}

	void handleNotifyInfo(FILE_NOTIFY_INFORMATION *fi, TryAgainFileList &tryAgainList)
	{
		static const SizeType fileArrayLength = 1024;
		char fileArray[fileArrayLength] = { 0 };
		memset(fileArray, 0, fileArrayLength);
		int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, fi->FileName, int(fi->FileNameLength / 2), fileArray, 255, 0, 0);
		if (len <= 0)
			return;

		fb_assert(len < fileArrayLength);

		for (int i = 0; i < len; ++i)
		{
			if (fileArray[i] == '\\')
				fileArray[i] = '/';
		}

		/* Ignore version control files, dev folders and livepp logs */
		if (strstr(fileArray, "/.svn") != nullptr || strstr(fileArray, "/.git") != nullptr && strstr(fileArray, "/dev/") != nullptr
			|| strstr(fileArray, "log_dev_space.txt") != nullptr || strstr(fileArray, "log_telemetry_space.txt") != nullptr)
			return;

		TempString filename = extraPath;
		filename += fileArray;

		/* Special check for DBG??.tmp files. They are created by calls to "SymInitialize" in StackTraceWindows.cpp. 
		 * Annoying, as they cause (harmless as such) errors on builders */
		if (filename.doesEndWith(".tmp"))
		{
			SmallTempString filePart;
			filePart.appendFileNameWithoutExtensionFromString(filename);
			/* Good enough. There should be only few digits even in worst cases */
			if (filePart.getLength() > 3 && filePart.getLength() < 9 && filePart.doesStartWith("DBG"))
			{
				FB_LOG_INFO(FB_MSG("Ignore file ", filename));
				return;
			}
			/* Check for wait file */
			if (filePart.doesStartWith(getFileSystemListenerWaitFileNameBody()))
			{
				SizeType bodyLength = getFileSystemListenerWaitFileNameBody().getLength();
				StringRef numberPart(filePart.getPointer() + bodyLength, filePart.getLength() - bodyLength);
				uint32_t parsedIndex = invalidFileSystemWaitIndex;
				if (numberPart.parse(parsedIndex))
				{
					MutexGuard mg(mutex);
					if (parsedIndex == expectedFileSystemWaitIndex)
					{
						expectedFileSystemWaitIndex = invalidFileSystemWaitIndex;
						conditionVar.broadcast();
					}
				}
				else
				{
					FB_LOG_ERROR(FB_MSG("Could not parse wait file index from name ", filename));
				}
				return;
			}
		}

		string::SimpleUTF16String filenameWide;
		filenameWide.appendUTF8(filename);

		DWORD attributes = GetFileAttributesW(filenameWide.getPointer());

		// a directory was added: scan all the files under it
		if ((attributes & FILE_ATTRIBUTE_DIRECTORY) && (fi->Action == FILE_ACTION_ADDED || fi->Action == FILE_ACTION_RENAMED_NEW_NAME))
			addFilesFromDir(filename);

		// file/dir deleted
		if ((attributes == -1) || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			// modified a file
			if (attributes != -1 && !(attributes & FILE_ATTRIBUTE_DIRECTORY) && (fi->Action == FILE_ACTION_MODIFIED || fi->Action == FILE_ACTION_ADDED))
			{
				/* Check if file can be opened or if it's locked. Try again later, if file is locked */
				HANDLE hFile = CreateFileW(filenameWide.getPointer(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, 0);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					tmpResults.pushBack(filename);
				}
				else
				{
					TryAgainFile &tryAgainFile = tryAgainList.pushBack();
					tryAgainFile.filename = filename;
					tryAgainFile.filenameWide = filenameWide;
				}
			}
			else
			{
				tmpResults.pushBack(filename);
			}
		}
	}

	bool update()
	{
#if FB_EDITOR_ENABLED == FB_FALSE
		/* In FinalRelease, simply returning here leads to busy waiting. */
		Thread::sleep(0xFB);
		return false;
#else
		fb_assert(tmpResults.isEmpty());
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(OVERLAPPED));
		overlapped.hEvent = eventHandle;

		FB_ZONE("FileSystemListener::Data::update");

		// Start async waiting

		memset(&readBuffer[0], 0, readBuffer.getSize());
		fb_assert(readBuffer.getSize() > sizeof(FILE_NOTIFY_INFORMATION) * 2);

		BOOL result = ReadDirectoryChangesW(directoryHandle, &readBuffer[0], (DWORD)readBuffer.getSize(), TRUE, filter, nullptr, &overlapped, 0);
		if (result == 0)
		{
			TempString msg("ReadDirectoryChangeW failed. Error is ");
			msg << int64_t(GetLastError());
			FB_LOG_ERROR(msg);
			return false;
		}

		/* Wait for something to happen */
		DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
		/* First handle is the closeHandle. If we get result from anything other than the second handle (eventHandle), we quit */
		if (waitResult - WAIT_OBJECT_0 != 1)
			return false;

		TryAgainFileList tryAgainList;

		/* Handle new files */
		for (DWORD offset = 0; ; )
		{
			fb_assert(offset + sizeof(FILE_NOTIFY_INFORMATION) <= readBuffer.getSize() && "Out of bounds");
			FILE_NOTIFY_INFORMATION *fi = (FILE_NOTIFY_INFORMATION *)&readBuffer[offset];
			/* If all of the stuff is zero, then something is not right here. We've been woken up early */
			if (fi->Action == 0 && offset == 0 && fi->NextEntryOffset == 0)
				break;

			handleNotifyInfo(fi, tryAgainList);

			if (fi->NextEntryOffset == 0)
				break;

			offset += fi->NextEntryOffset;
		}

		/* Make results available */
		addTmpResultsToResults();
		fb_assert(tmpResults.isEmpty());
		/* Try again the files that previously failed. Try again time used to be 200 ms per file. Now it's 500 ms for the whole operation */
		const uint32_t tryAgainTimeMs = 500;
		ScopedTimer timer;
		while (!tryAgainList.isEmpty() && timer.getMilliseconds() < tryAgainTimeMs)
		{
			ScopedTimer loopTimer;
			for (SizeType i = 0; i < tryAgainList.getSize(); ++i)
			{
				const TryAgainFile &tryAgainFile = tryAgainList[i];
				/* Check if file can be opened or if it's locked. Try again later, if file is locked */
				HANDLE hFile = CreateFileW(tryAgainFile.filenameWide.getPointer(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, 0);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					tmpResults.pushBack(tryAgainFile.filename);
					tryAgainList[i] = tryAgainList.getBack();
					tryAgainList.popBack();
					--i;
				}
				else
				{
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						/* Don't waste time trying to open files already deleted */
						tryAgainList[i] = tryAgainList.getBack();
						tryAgainList.popBack();
						--i;
					}
				}
			}
			/* Make try-again results available */
			addTmpResultsToResults();
			uint32_t loopTime = uint32_t(loopTimer.getMilliseconds());
			if (loopTime < 10)
				Thread::sleep(10 - loopTime);
		}
		if (!tryAgainList.isEmpty())
		{
			TempString msg("Failed to open some files from tryAgainList: ");
			for (SizeType i = 0; i < tryAgainList.getSize(); ++i)
			{
				msg << tryAgainList[i].filename.getPointer() << ", ";
			}
			msg.trimRight(2);
			FB_LOG_WARNING(msg);
		}
		return true;
#endif
	}


	void addTmpResultsToResults()
	{
		if (tmpResults.isEmpty())
			return;

		MutexGuard guard(mutex);
		lang::atomicAddRelaxed(resultsSinceSwap, tmpResults.getSize());
		if (results.isEmpty())
		{
			tmpResults.swap(results);
		}
		else
		{
			results.insert(results.getEnd(), tmpResults.getBegin(), tmpResults.getEnd());
			tmpResults.clear();
		}
	}


	void entry()
	{
		FB_ZONE("FileSystemListener::Data::entry");

		while (update())
		{
			/* Just run */
		}
	}

};

lang::AtomicUInt32 FileSystemListener::Data::waitFileNameRunningIndex;


FileSystemListener::FileSystemListener(const StringRef &path)
	: data(new Data(path))
{
}


FileSystemListener::~FileSystemListener()
{
}


void FileSystemListener::swapResults(ListeningType type, ResultList &outResults)
{
	outResults.clear();
	/* Optimization: if there's no new results, don't take the mutex (this should be the case most of the time) */
	if (lang::atomicLoadRelaxed(data->resultsSinceSwap) > 0)
	{
		MutexGuard guard(data->mutex);
		lang::atomicStoreRelaxed(data->resultsSinceSwap, 0);
		outResults.swap(data->results);
	}
}


void FileSystemListener::waitForFileSystem()
{
#if FB_EDITOR_ENABLED == FB_TRUE
	MutexGuard mg(data->mutex);
	while (data->expectedFileSystemWaitIndex != Data::invalidFileSystemWaitIndex)
	{
		/* Wait until it is our turn */
		data->conditionVar.wait(mg, Time::fromSeconds(1));
	}
	fb_assert(data->expectedFileSystemWaitIndex == Data::invalidFileSystemWaitIndex && "Someone else still waiting?");
	TempString testFileName = data->extraPath;
	Data::appendFileSystemWaitFileName(testFileName, data->expectedFileSystemWaitIndex);
	/* Create test file using low-level functions, as we don't want FileManager to notice */
	if (file::doesFileExist(testFileName))
	{
		FB_LOG_ERROR(FB_MSG("Test file ", testFileName, " already exists. Deleting before continuing"));
		if (!file::deleteFile(testFileName))
			FB_LOG_ERROR(FB_MSG("Could not delete pre-existing test file ", testFileName));
	}
	OutputFile testFile;
	if (!testFile.open(testFileName))
	{
		FB_LOG_ERROR(FB_MSG("Could not create test file ", testFileName));
		data->expectedFileSystemWaitIndex = Data::invalidFileSystemWaitIndex;
		return;
	}
	if (!testFile.tryToWriteData("Nope", 4))
	{
		FB_LOG_ERROR(FB_MSG("Could not write to test file ", testFileName));
		data->expectedFileSystemWaitIndex = Data::invalidFileSystemWaitIndex;
		return;
	}
	testFile.close();
	while (data->expectedFileSystemWaitIndex != Data::invalidFileSystemWaitIndex)
	{
		/* Wait until our file is detected */
		/* For some mysterious reason (can't see how that could happen) we sometimes get stuck here even though 
		 * expectedFileSystemWaitIndex is invalid. Could be a problem with ConditionVariable */
		data->conditionVar.wait(mg, Time::fromSeconds(1));
	}
	if (!file::deleteFile(testFileName))
		FB_LOG_ERROR(FB_MSG("Could not delete test file ", testFileName));
#endif
}


FB_END_PACKAGE1()
