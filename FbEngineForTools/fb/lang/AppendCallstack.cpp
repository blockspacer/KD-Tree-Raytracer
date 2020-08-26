#include "Precompiled.h"
#include "AppendCallstack.h"

#include "fb/lang/StoredCallstack.h"
#include "fb/profiling/ZoneDefine.h" // For FB_USE_ZONE_PROFILER
#include "fb/string/HeapString.h"

#if FB_USE_ZONE_PROFILER == FB_TRUE
#include "fb/profiling/ZoneProfilerCallstackDump.h"

FB_PACKAGE1(profiling)
extern bool g_enableZoneProfiler;
FB_END_PACKAGE1()
#endif


FB_PACKAGE0()

HeapString &debugAppendToString(HeapString &result, const AppendCallstack &)
{
	StoredCallstack callstack(true);
	callstack.appendToString(result);
	return result;
}

HeapString &debugAppendToString(HeapString &result, const AppendSingleRowCallstack &)
{
	SizeType originalLength = result.getLength();

	StoredCallstack callstack(true);
	callstack.outputOnSingleRow = true;
	callstack.appendToString(result);

	for (SizeType i = originalLength; i < result.getLength(); ++i)
	{
		char c = result[i];
		if (c != '?' && c != ';' && c != ' ')
			return result;
	}

	// If output contains only "?; " the symbols are not available or something
	// Fallback to Zone callstack
	result.resizeAndFill(originalLength, ' ');
	return debugAppendToString(result, AppendSingleRowZoneCallstack());
}

HeapString &debugAppendToString(HeapString &result, const AppendSingleRowZoneCallstack &)
{
	StoredCallstack callstack;
	callstack.acceptNormalCallstacks = false;
	callstack.outputOnSingleRow = true;
	callstack.store();
	callstack.appendToString(result);
	return result;
}

FB_END_PACKAGE0()
