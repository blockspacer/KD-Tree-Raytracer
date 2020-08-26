#pragma once

#include <stdio.h>

#include "fb/lang/Types.h"

FB_PACKAGE1(file)

class InputFile
{
public:
	InputFile();
	~InputFile();

	bool open(const StringRef &filename);
	void close();

	BigSizeType readData(void *buffer, BigSizeType size);

	// ReadDataWithOffset guarantees, that multiple reads from different offsets at the same time is safe without mutex/such protection.
	// This guarantee only applies to readWithOffset method, don't call any other methods at the same time!
	BigSizeType readDataWithOffset(BigSizeType offset, void *buffer, BigSizeType size) const;

	BigSizeType getSize() const;

	enum SeekMode
	{
		SeekModeSet,
		SeekModeCur,
		SeekModeEnd
	};
	void seek(BigSizeType pos, SeekMode mode);
	BigSizeType getPosition() const;
	
private:
	FILE *file = nullptr;
#if FB_BUILD != FB_FINAL_RELEASE
	DynamicString fileName;
#endif
};

FB_END_PACKAGE1()
