#include "Precompiled.h"
#include "FindString.h"

#include "fb/string/HeapString.h"
#include "fb/string/detail/CompareString.h"
#include "fb/string/detail/CaseConversion.h"

#include <cstring>

FB_PACKAGE2(string, detail)

SizeType findString(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType startingAtPosition)
{
	if (startingAtPosition >= searchInsideString.getLength())
		return string::NoPosition;

	const char *resultPtr = strstr(searchInsideString.getPointer() + startingAtPosition, stringToLookFor.getPointer());
	return resultPtr != nullptr ? SizeType(resultPtr - searchInsideString.getPointer()) : string::NoPosition;
}


SizeType findStringCaseInsensitive(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType startingAtPosition)
{
	/* This is not super optimal, but if you really care about that, you should find a way to avoid case-insensitive 
	 * comparison */
	TempString lowerCaseCopyOfSearchInsideString(searchInsideString);
	lowerCaseCopyOfSearchInsideString.toLower();
	TempString lowerCaseCopyOfStringToLookFor(stringToLookFor);
	lowerCaseCopyOfStringToLookFor.toLower();
	return findString(lowerCaseCopyOfSearchInsideString, lowerCaseCopyOfStringToLookFor, startingAtPosition);
}


SizeType findStringRight(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType start)
{
	if (stringToLookFor.getLength() > searchInsideString.getLength())
		return string::NoPosition;

	if (start > searchInsideString.getLength())
		start = searchInsideString.getLength();
	else if (start < stringToLookFor.getLength())
		return string::NoPosition;

	start -= stringToLookFor.getLength();

	for (SizeType i = start + 1; i-- > 0; )
	{
		if (strncmp(searchInsideString.getPointer() + i, stringToLookFor.getPointer(), stringToLookFor.getLength()) != 0)
			continue;

		return i;
	}

	return string::NoPosition;
}


SizeType findStringCountOf(const StringRef &searchInsideString, const StringRef &stringToLookFor, SizeType startingAtPosition)
{
	if (startingAtPosition >= searchInsideString.getLength())
		return 0;

	SizeType occurrences = 0;
	while (startingAtPosition < searchInsideString.getLength())
	{
		SizeType result = findString(searchInsideString, stringToLookFor, startingAtPosition);
		if (result == string::NoPosition)
			break;

		occurrences += 1;
		startingAtPosition = result + 1;
	}

	return occurrences;
}


FB_END_PACKAGE2()

