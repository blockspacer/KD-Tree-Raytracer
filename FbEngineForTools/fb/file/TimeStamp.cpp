#include "Precompiled.h"
#include "fb/file/TimeStamp.h"

FB_PACKAGE1(file)

TimeStamp getNewerTimeStamp(TimeStamp a, TimeStamp b)
{
	return a > b ? a : b;
}

bool isFileNewerThanFile(TimeStamp stamp, TimeStamp thanStamp)
{
	if (stamp > thanStamp)
		return true;

	return false;
}

TimeStamp64 getNewerTimeStamp(TimeStamp64 a, TimeStamp64 b)
{
	return a > b ? a : b;
}

bool isFileNewerThanFile(TimeStamp64 stamp, TimeStamp64 thanStamp)
{
	if (stamp > thanStamp)
		return true;

	return false;
}

FB_END_PACKAGE1()
