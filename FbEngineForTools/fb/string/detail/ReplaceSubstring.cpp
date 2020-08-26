#include "Precompiled.h"
#include "ReplaceSubstring.h"

#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/MemTools.h"
#include "fb/string/util/CreateTemporaryHeapString.h"

FB_PACKAGE2(string, detail)

/**
 * Replaces all instances of t1 with t2
 */
void replaceSubstring(HeapString &stringInOut, const StringRef &searchFor, const StringRef &replaceWith)
{
	if (searchFor.isEmpty())
	{
		FB_LOG_ERROR(FB_MSG("Empty search string is not allowed. Original: ", stringInOut, ", replaceWith: ", replaceWith));
		return;
	}

	SizeType searchForLen = searchFor.getLength();
	SizeType replaceWithLen = replaceWith.getLength();

	SizeType i = stringInOut.find(searchFor);
	while (i < stringInOut.getLength())
	{
		if (searchForLen < replaceWithLen)
		{
			/* Overwrite with end of replaceWith */
			lang::MemCopy::copy(&stringInOut[i], replaceWith.getPointer() + (replaceWithLen - searchForLen), searchForLen);
			/* Insert beginning of replaceWith */
			stringInOut.insert(i, replaceWith.getPointer(), replaceWithLen - searchForLen);
		}
		else
		{
			/* Erase extra chars and overwrite (erasing 0 chars is ok) */
			stringInOut.erase(i, searchForLen - replaceWithLen);
			lang::MemCopy::copy(&stringInOut[i], replaceWith.getPointer(), replaceWithLen);
		}
		i = stringInOut.find(searchFor, i + replaceWithLen);
	}
}

FB_END_PACKAGE2()
