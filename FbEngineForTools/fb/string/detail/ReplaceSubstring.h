#pragma once

#include "fb/lang/Types.h"

FB_DECLARE0(HeapString)
FB_DECLARE0(StringRef)

FB_PACKAGE2(string, detail)

/**
 * Replaces all instances of t1 with t2
 */
void replaceSubstring(HeapString &stringInOut, const StringRef &searchFor, const StringRef &replaceWith);

FB_END_PACKAGE2()
