#pragma once

FB_DECLARE0(HeapString)

FB_PACKAGE2(string, detail)

/**
 * Replaces all instances of searchFor with replaceWith
 */
void replaceChar(HeapString &stringInOut, char searchFor, char replaceWith);

FB_END_PACKAGE2()
