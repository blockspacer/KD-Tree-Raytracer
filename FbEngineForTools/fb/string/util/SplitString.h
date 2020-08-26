#pragma once

#include "fb/container/PodVector.h"
#include "fb/container/Vector.h"
#include "fb/string/StringRef.h"

FB_PACKAGE1(string)

/**
 * Splits the string by the given separator. Number of resulting pieces and each piece can be queried after the split 
 * is done. Number of splits to do can be limited with maxPieces parameter.
 * 
 * Notes on some (not really so) special cases:
 *   1) Separator at the beginning of the string will cause first piece to be an empty string.
 *   2) Separator at the end of the string will cause last piece to be an empty string.
 *   3) If separator equals the string, result will be two pieces, each an empty string.
 *   4) If maxPieces is 1, the string will be returned as is.
 * 
 */
class SplitString
{
public:
	SplitString();
	SplitString(const StringRef &stringToSplit, const StringRef &separator, SizeType maxPieces = 0);
	SplitString(const char *stringDataToSplit, SizeType dataSize, const StringRef &separator, SizeType maxPieces = 0);

	/* Clears previous data and replaces it with new */
	void reset(const StringRef &stringToSplit, const StringRef &separator, SizeType maxPieces = 0);
	void reset(const char *stringDataToSplit, SizeType dataSize, const StringRef &separator, SizeType maxPieces = 0);

	/* Returns number of strings after splitting */
	SizeType getNumPieces() const;
	/* Returns requested split */
	const StringRef &operator[](SizeType index) const;
	
	typedef Vector<StringRef> Pieces;
	/* Returns Vector containing all splits */
	const Pieces &getPieces() const;

private:
	PodVector<char> backingString;
	Pieces results;
};

FB_END_PACKAGE1()
