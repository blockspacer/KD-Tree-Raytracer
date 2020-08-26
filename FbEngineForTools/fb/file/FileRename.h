#pragma once

#include "fb/container/Vector.h"
#include "fb/string/HeapString.h"
#include "fb/container/LinearHashMap.h"

FB_PACKAGE1(file)

/**
 * Moves files/directories while maintaining version control
 */
class FileRename
{
public:
	bool renameDirectory(const StringRef &oldFile, const StringRef &newFile, HeapString &errorString);
	bool renameFile(const StringRef &oldFile, const StringRef &newFile, HeapString &errorString);

	bool finish(bool showCommitDialog = true);

protected:
	typedef LinearHashMap<HeapString, HeapString> StringMap;
	StringMap fileMapping;
	StringMap directoryMapping;
	void moveFilesFromDir(const StringRef &oldDirectory, const StringRef &newDirectory);
};


FB_END_PACKAGE1()
