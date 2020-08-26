#include "Precompiled.h"
#include "SplitString.h"

#include <cstring>

FB_PACKAGE1(string)

SplitString::SplitString()
{
}


SplitString::SplitString(const StringRef &stringToSplit, const StringRef &separator, SizeType maxPieces)
{
	reset(stringToSplit, separator, maxPieces);
}


SplitString::SplitString(const char *stringDataToSplit, SizeType dataSize, const StringRef &separator, SizeType maxPieces)
{
	reset(stringDataToSplit, dataSize, separator, maxPieces);
}


void SplitString::reset(const StringRef &stringToSplit, const StringRef &separator, SizeType maxPieces)
{
	reset(stringToSplit.getPointer(), stringToSplit.getLength(), separator, maxPieces);
}


void SplitString::reset(const char *stringDataToSplit, SizeType dataSize, const StringRef &separator, SizeType maxPieces)
{
	if (maxPieces == 0)
		maxPieces = 0xFFFFFFFF;

	results.clear();
	backingString.clear();
	fb_assert(!separator.isEmpty() && "Cannot split string by empty separator");
	if (separator.isEmpty())
		return;

	backingString.insert(backingString.getBegin(), stringDataToSplit, dataSize);
	backingString.pushBack('\0');

	/* Note: len includes NUL terminator on purpose (handles separator at the end of string case) */
	for (SizeType searchIndex = 0, len = backingString.getSize(); searchIndex < len; /* Nop */)
	{
		char *searchPtr = &(backingString[searchIndex]);
		char *nextSplitPtr = (results.getSize() != maxPieces - 1) ? strstr(searchPtr, separator.getPointer()) : nullptr;
		if (nextSplitPtr != nullptr)
		{
			/* StringRef should yell loudly, if math here is wrong */
			SizeType pieceLength = SizeType(nextSplitPtr - searchPtr);
			nextSplitPtr[0] = '\0';
			results.pushBack(StringRef(searchPtr, pieceLength));
			searchIndex += pieceLength + separator.getLength();
		}
		else
		{
			/* Push rest of the string to results */
			results.pushBack(StringRef(searchPtr, backingString.getSize() - searchIndex - 1));
			break;
		}
	}
}


SizeType SplitString::getNumPieces() const
{
	return results.getSize();
}


const StringRef &SplitString::operator[](SizeType index) const
{
	return results[index];
}


const SplitString::Pieces &SplitString::getPieces() const
{
	return results;
}

FB_END_PACKAGE1()
