#pragma once

#include "fb/string/StaticString.h"

FB_PACKAGE1(file)

class OutputFile
{
public:
	OutputFile();
	~OutputFile();

	enum Flags
	{
		FlagAppend = 1<<0,
	};

	bool open(const StringRef &fileName, int32_t flags = 0);
	void flush();
	void close();

	void writeData(const void *buffer, BigSizeType size);
	BigSizeType tryToWriteData(const void *buffer, BigSizeType size);


private:
	enum : intptr_t { InvalidHandleValue = (intptr_t)-1 }; // Same as INVALID_HANDLE_VALUE in Windows headers

	void *file;
#if FB_BUILD != FB_FINAL_RELEASE
	DynamicString fileName;
#endif
};

FB_END_PACKAGE1()
