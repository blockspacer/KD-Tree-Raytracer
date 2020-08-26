#include "Precompiled.h"
#include "StoredCallstack.h"

#include "fb/profiling/ZoneDefine.h"
#include "fb/string/HeapString.h"

#if FB_BUILD == FB_FINAL_RELEASE
	// Gathering callstack has performance impact so don't enable it just in case it happens to work
	#define FB_USING_STACKTRACE_INTERNALLY FB_FALSE
#else
	#define FB_USING_STACKTRACE_INTERNALLY FB_TRUE
#endif

#if FB_USING_STACKTRACE_INTERNALLY == FB_TRUE
	#include "fb/lang/DebugHelp.h"
	#include "fb/lang/ProgrammerAssertPrinting.h"
	#include "fb/lang/stacktrace/StackTrace.h"
#endif

#if FB_USE_ZONE_PROFILER == FB_TRUE
	#include "fb/profiling/ZoneProfilerCallstackDump.h"
	#define FB_USING_ZONECALLSTACK_INTERNALLY FB_TRUE
#else
	#define FB_USING_ZONECALLSTACK_INTERNALLY FB_FALSE
#endif

FB_PACKAGE1(profiling)
extern bool g_enableZoneProfiler;
FB_END_PACKAGE1()

FB_PACKAGE0()

struct StoredCallstack::Data
{
#if FB_USING_STACKTRACE_INTERNALLY == FB_TRUE
	lang::CallStackCapture<20> stackTrace;
#endif

#if FB_USING_ZONECALLSTACK_INTERNALLY == FB_TRUE
	CachePodVector<profiling::ZoneProfilerCallstackDump::ZoneCallstackFrame, 8> zoneCallstack;
#endif
};

#if FB_USING_ZONECALLSTACK_INTERNALLY == FB_TRUE
static profiling::ZoneProfilerCallstackDump::Args getArgs()
{
	profiling::ZoneProfilerCallstackDump::Args args;
	return args;
}
#endif

StoredCallstack::StoredCallstack(bool storeNow)
{
	if (storeNow)
	{
		store();
	}
}

StoredCallstack::StoredCallstack(StoredCallstack &&other)
	: outputOnSingleRow(other.outputOnSingleRow)
	, dontAppendErrors(other.dontAppendErrors)
{
	data = other.data;
	other.data = nullptr;
}

StoredCallstack::~StoredCallstack()
{
	if (data)
	{
		delete data;
		data = nullptr;
	}
}

bool StoredCallstack::store()
{
	if (FB_USING_STACKTRACE_INTERNALLY != FB_TRUE && FB_USING_ZONECALLSTACK_INTERNALLY != FB_TRUE)
		return false;

	if (!acceptNormalCallstacks && !acceptZoneCallstacks)
		return false;

	if (!data)
		data = new Data;

#if FB_USING_STACKTRACE_INTERNALLY == FB_TRUE
	if (acceptNormalCallstacks)
	{
		if (DebugHelp::getStackTrace(data->stackTrace, 2))
			return true;
	}
#endif

#if FB_USING_ZONECALLSTACK_INTERNALLY == FB_TRUE
	if (acceptZoneCallstacks)
	{
		profiling::ZoneProfilerCallstackDump::dumpProfileCallstack(getArgs(), data->zoneCallstack);
		if (!data->zoneCallstack.isEmpty())
			return true;
	}
#endif

	return false;
}

void StoredCallstack::appendToString(HeapString &result) const
{
	if (FB_USING_STACKTRACE_INTERNALLY != FB_TRUE && FB_USING_ZONECALLSTACK_INTERNALLY != FB_TRUE)
	{
		if (!dontAppendErrors)
			result << "No callstack implementation";

		return;
	}

	if (!acceptNormalCallstacks && !acceptZoneCallstacks)
	{
		if (!dontAppendErrors)
			result << "Not accepting either callstack type";

		return;
	}

	if (!data)
	{
		if (!dontAppendErrors)
			result << "No callstack stored";

		return;
	}

#if FB_USING_STACKTRACE_INTERNALLY == FB_TRUE
	if (acceptNormalCallstacks)
	{
		SizeType length = result.getLength();
		if (outputOnSingleRow)
			ProgrammerAssertPrinting::appendStackTraceSingleRow(result, data->stackTrace);
		else
			ProgrammerAssertPrinting::appendStackTraceCleaned(result, data->stackTrace);

		if (length < result.getLength())
			return;

		if (FB_USING_ZONECALLSTACK_INTERNALLY != FB_TRUE || !acceptZoneCallstacks)
		{
			if (!dontAppendErrors)
				result << "Empty callstack";

			return;
		}
	}
#endif

#if FB_USING_ZONECALLSTACK_INTERNALLY == FB_TRUE
	if (acceptZoneCallstacks)
	{
		if (!data->zoneCallstack.isEmpty())
		{
			for (SizeType i = data->zoneCallstack.getSize(); i-- > 0; )
			{
				if (outputOnSingleRow)
					result << data->zoneCallstack[i].name << "; ";
				else
					result << data->zoneCallstack[i].name << "\n";
			}
		}
		else
		{
			if (!dontAppendErrors)
			{
				if (!profiling::g_enableZoneProfiler)
					result << "Empty zone callstack (disabled)";
				else
					result << "Empty zone callstack (add more zones?)";
			}
		}
	}
#endif
}

HeapString &debugAppendToString(HeapString &result, const StoredCallstack *t)
{
	fb_assert(t);
	t->appendToString(result);
	return result;
}

FB_END_PACKAGE0()

#undef FB_USING_STACKTRACE_INTERNALLY
#undef FB_USING_ZONECALLSTACK_INTERNALLY
