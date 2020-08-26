#pragma once

#include "fb/container/Vector.h"
#include "fb/lang/ScopedPointer.h"
#include "fb/string/HeapString.h"

FB_PACKAGE1(file)

/// Asynchoronously listens file system changes
class FileSystemListener
{
public:
	typedef Vector<HeapString> ResultList;
	/* Directories aren't supported */
	enum ListeningType
	{
		FlagFile = 1,
		FlagAll = 1
	};

	// Only file implemented at the moment
	FileSystemListener(const StringRef &path);
	~FileSystemListener();

	/* Get all current results to outResults and clear results */
	void swapResults(ListeningType type, ResultList &outResults);

	/* Adds a temporary file and waits for it to show up, thus making sure all previous files are also done */
	void waitForFileSystem();

private:
	FileSystemListener(const FileSystemListener &) = delete;
	void operator = (const FileSystemListener &) = delete;

	struct Data;
	ScopedPointer<Data> data;
};

FB_END_PACKAGE1()
