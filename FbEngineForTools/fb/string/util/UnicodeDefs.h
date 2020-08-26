#pragma once

FB_PACKAGE1(string)

typedef char UTF8Unit;
/* Note: this may be either 16 or 32 bits depending on platform. That shouldn't matter. */
typedef wchar_t UTF16Unit;
typedef uint32_t UTF32Unit;
static UTF8Unit getUTF8NulChar() { return '\0'; }
static UTF16Unit getUTF16NulChar() { return 0; }

FB_END_PACKAGE1()
