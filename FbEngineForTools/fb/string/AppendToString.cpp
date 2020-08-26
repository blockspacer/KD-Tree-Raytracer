#include "Precompiled.h"
#include "AppendToString.h"

#include "fb/string/HeapString.h"
#include "fb/string/util/UnicodeConverter.h"


FB_PACKAGE0()

HeapString &debugAppendToString(HeapString &result, bool val)
{
	if (val)
		result += "true";
	else
		result += "false";

	return result;
}

HeapString &debugAppendToString(HeapString &result, char val)
{
	if (val != '\0')
		result += val;

	return result;
}

HeapString &debugAppendToString(HeapString &result, int32_t val)
{
	return result += val;
}

HeapString &debugAppendToString(HeapString &result, uint32_t val)
{
	return result += val;
}

HeapString &debugAppendToString(HeapString &result, int64_t val)
{
	return result += val;
}

HeapString &debugAppendToString(HeapString &result, uint64_t val)
{
	return result += val;
}

HeapString &debugAppendToString(HeapString &result, float val)
{
	return result += val;
}

HeapString &debugAppendToString(HeapString &result, double val)
{
	return result += val;
}

HeapString &debugAppendToString(HeapString &result, std::nullptr_t)
{
	return result += "nullptr_t";
}

HeapString &debugAppendToString(HeapString &result, const void *ptr)
{
	if (ptr == NULL)
		return result += "null";

	CacheHeapString<20> temp;
	temp.appendHexNumber((SizeType)(uintptr_t)ptr);

	SizeType padding = sizeof(uintptr_t) * 2 - temp.getLength();
	if (padding > 16)
		padding = 16;

	temp.insert(0, "0x0000000000000000", padding + 2); // Add "0x" and leading zeros
	result += temp;
	return result;
}

HeapString &debugAppendToString(HeapString &result, const StringRef &t)
{
	return result += t;
}

HeapString &debugAppendToString(HeapString &result, const char *t)
{
	return result += t;
}

HeapString &debugAppendToString(HeapString &result, const wchar_t *t)
{
	string::UnicodeConverter::addUTF16StrToUTF8String(t, result, string::UnicodeConverter::UseReplacementChar);
	return result;
}

FB_END_PACKAGE0()

#include "fb/string/util/CreateTemporaryHeapString.h"

FB_PACKAGE0()

void testAppendToString()
{
	HeapString asdf;
	const char *thing = "asdfdfs";
	debugAppendToString(asdf, thing);
	TempString tempstr(FB_MSG("asdf", 1, nullptr));
	TempString tempStr(FB_MSG(tempstr, "asdf", 1, nullptr));
}

FB_END_PACKAGE0()