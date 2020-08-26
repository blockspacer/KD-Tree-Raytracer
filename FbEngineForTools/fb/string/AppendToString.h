#pragma once

#include "fb/lang/Types.h"

FB_DECLARE0(HeapString)
FB_DECLARE0(StringRef)

FB_PACKAGE0()

// NOTE: Only use these with errors, assertions and other places that aren't included in FinalRelease.
HeapString &debugAppendToString(HeapString &result, bool val);
HeapString &debugAppendToString(HeapString &result, char val);
HeapString &debugAppendToString(HeapString &result, int32_t val);
HeapString &debugAppendToString(HeapString &result, uint32_t val);
HeapString &debugAppendToString(HeapString &result, int64_t val);
HeapString &debugAppendToString(HeapString &result, uint64_t val);
HeapString &debugAppendToString(HeapString &result, float val);
HeapString &debugAppendToString(HeapString &result, double val);

HeapString &debugAppendToString(HeapString &result, const StringRef &t);
HeapString &debugAppendToString(HeapString &result, const char *t);
HeapString &debugAppendToString(HeapString &result, const wchar_t *t);
HeapString &debugAppendToString(HeapString &result, const void *t);
HeapString &debugAppendToString(HeapString &result, nullptr_t);

inline HeapString &appendToString(HeapString &result)
{
	return result;
}
template <typename T>
inline HeapString &appendToString(HeapString &result, const T &t)
{
	debugAppendToString(result, t);
	return result;
}
template <typename T, typename... Targs>
inline HeapString &appendToString(HeapString &result, const T &t, Targs... args)
{
	debugAppendToString(result, t);
	return appendToString(result, args...);
}

FB_END_PACKAGE0()
