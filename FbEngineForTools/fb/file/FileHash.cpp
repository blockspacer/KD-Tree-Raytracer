#include "Precompiled.h"
#include "FileHash.h"

#include "InputFile.h"
#include "fb/container/PodVector.h"
#include "fb/lang/hash/Hash.h"

FB_PACKAGE1(file)

bool getHash64(const StringRef &filepath, uint64_t &hashOut)
{
	file::InputFile file;
	if (!file.open(filepath))
		return false;

	if (file.getSize() == 0)
	{
		hashOut = 0;
		return true;
	}
	PodVector<char> buffer;
	buffer.resize((SizeType)file.getSize());

	file.readData(&buffer[0], file.getSize());

	hashOut = getHashValue64(&buffer[0], SizeType(file.getSize()));

	return true;
}

bool getHash(const StringRef &filepath, uint32_t &hashOut)
{
	file::InputFile file;
	if (!file.open(filepath))
		return false;

	if (file.getSize() == 0)
	{
		hashOut = 0;
		return true;
	}
	PodVector<char> buffer;
	buffer.resize((SizeType)file.getSize());

	file.readData(&buffer[0], file.getSize());

	hashOut = getHashValue(&buffer[0], SizeType(file.getSize()));

	return true;
}

FB_END_PACKAGE1()
