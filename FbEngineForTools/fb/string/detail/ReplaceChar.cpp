#include "Precompiled.h"
#include "ReplaceChar.h"

#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/logger/LoggingMacros.h"
#include "fb/string/util/CreateTemporaryHeapString.h"

FB_PACKAGE2(string, detail)

/**
 * Replaces all instances of searchFor with replaceWith
 */
void replaceChar(HeapString &stringInOut, char searchFor, char replaceWith)
{
	fb_assertf(searchFor != 0 && replaceWith != 0, "0-char is not allowed. Original: %s", stringInOut.getPointer());

	for (SizeType i = 0, end = stringInOut.getLength(); i < end; ++i)
	{
		if (stringInOut[i] != searchFor)
			continue;

		stringInOut[i] = replaceWith;
	}
}

FB_END_PACKAGE2()
