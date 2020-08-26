#pragma once

#include "fb/string/HeapString.h"
#include "fb/string/StringBuilder.h"

FB_DECLARE0(DynamicString);

// toString macro for errors, assertions and such
#define FB_DEBUG_TO_STR_STR(w) (fb::debug_to_string_macro_functions::debug_to_str(w))
#define FB_DEBUG_TO_STR(w) (FB_DEBUG_TO_STR_STR(w).getPointer())

#define FB_FMT(...) (fb::TempString().doSprintf(__VA_ARGS__))
#define FB_MSG(...) fb::string::ErrorStringBuilder(__VA_ARGS__).getString()
#define FB_MSG_DROP_NULLS(...) (fb::string::createStringAndDropNulls(FB_PP_NARG(__VA_ARGS__), FB_PP_FOREACH(FB_DEBUG_TO_STR, __VA_ARGS__)).getPointer())

FB_PACKAGE1(string)

FB_NOINLINE HeapString createString(SizeType numArgs, ...);
FB_NOINLINE HeapString createStringAndDropNulls(SizeType numArgs, ...);

typedef const char *(*FilterFunction)(const char *);
FB_NOINLINE HeapString createStringWithFilter(FilterFunction filter, SizeType numArgs, ...);

FB_END_PACKAGE1()

FB_PACKAGE1(debug_to_string_macro_functions)

template<typename T>
inline HeapString debug_to_str(const T &val)
{
	HeapString result;
	debugAppendToString(result, val);
	return result;
}
inline const DynamicString& debug_to_str(const DynamicString& val)
{
	return val;
}
struct JustAString
{
	JustAString(const char *ptr) : ptr(ptr) { }
	const char *getPointer() const { return ptr; }
	const char *ptr;
};
inline JustAString debug_to_str(const char *val) { return JustAString(val); }
inline StringRef debug_to_str(StringRef val) { return val; }

FB_END_PACKAGE1()
