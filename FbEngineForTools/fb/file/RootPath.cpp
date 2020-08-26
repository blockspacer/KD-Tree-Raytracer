#include "Precompiled.h"
#include "RootPath.h"

#include <cstring>

FB_PACKAGE1(file)

	static char emptyRootPath[] = "";
	static char *rootPath = emptyRootPath;

	const char *RootPath::get()
	{
		return rootPath;
	}

	void RootPath::set(const char *str)
	{
		if (rootPath != emptyRootPath)
			delete[] rootPath;
		size_t len = strlen(str);
		rootPath = new char[len+1];
		strcpy(rootPath, str);
	}

FB_END_PACKAGE1()
