#pragma once

#include "fb/lang/ScopedPointer.h"
#include "fb/lang/Pimpl.h"
#include "fb/string/HeapString.h"
#include "fb/file/TimeStamp.h"

FB_PACKAGE1(file)

/**
 * Lists files from a directory
 */
class FileList
{
public:
	enum Flags
	{
		FlagExcludeFiles = (1<<0),
		FlagExcludeDirs = (1<<1),
		FlagIncludeSpecialDirs = (1<<2)
	};

	struct Entry
	{
		CacheHeapString<260> name;
		bool directory;
		BigSizeType size;
		TimeStamp64 lastModified;
	};

public:
	FileList(const StringRef &dir, int32_t flags = 0);
	~FileList();

	/**
	 * Returns next entry in list or nullptr if no more left
	 */
	Entry *next();

	FB_PIMPL;
};

FB_END_PACKAGE1()
