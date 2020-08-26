#include "Precompiled.h"
#include "ProgrammerAssertPrinting.h"

#include "fb/container/LinearHashMap.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/logger/LoggingMacros.h"
#include "fb/lang/platform/LineFeed.h"
#include "fb/lang/stacktrace/StackTrace.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/util/CreateTemporaryHeapString.h"

FB_PACKAGE0()

static bool asciiOrColon(char c)
{
	if (c >= 'a' && c <= 'z')
		return true;

	return c == ':';
}

static bool resolveCallStackFrameUsingCache(lang::StackFrame &outFrame, const lang::CallStackFrameCapture &capturedFrame)
{
#if FB_BUILD == FB_FINAL_RELEASE
	// This is a debug feature. Don't waste space on cache in final release.
	#define FB_PAP_USE_CALLSTACK_CACHE FB_FALSE
#else
	// PC and non-final-release only
	#define FB_PAP_USE_CALLSTACK_CACHE FB_TRUE
#endif

#if FB_PAP_USE_CALLSTACK_CACHE == FB_TRUE

	struct HashFunctor
	{
		inline uint32_t operator() (lang::CallStackFrameCapture t) { return getNumberHashValue(uintptr_t(t)); }
	};

	typedef LinearHashMap<lang::CallStackFrameCapture, lang::StackFrame, HashFunctor> CacheMap;
	static CacheMap cache;

	CacheMap::ConstIterator it = cache.find(capturedFrame);
	if (it != cache.getEnd())
	{
		outFrame = it.getValue();
		return true;
	}

	FB_ZONE("lang::resolveCallStackFrame");
	// This function call takes about 1 ms on a PC, so using a cache is easily worthwhile.
	bool success = lang::resolveCallStackFrame(outFrame, capturedFrame);
	cache[capturedFrame] = outFrame;
	return success;

#else

	return lang::resolveCallStackFrame(outFrame, capturedFrame);
#endif

#undef FB_PAP_USE_CALLSTACK_CACHE
}

void ProgrammerAssertPrinting::appendStackTraceSingleRow(HeapString &errorMessage, const DebugHelp::StackTraceBase &stackTrace)
{
	FB_ZONE("ProgrammerAssertPrinting::appendStackTraceSingleRow");

	TempString prevFilename;

	bool skippingStopped = false;
	for (SizeType i = 0; i < stackTrace.numCapturedFrames; i++)
	{
		lang::StackFrame frame;

		FB_ZONE("Resolve");

		if (!resolveCallStackFrameUsingCache(frame, stackTrace.getCapturedFrames()[i]))
		{
			errorMessage << "?; ";
			continue;
		}

		FB_ZONE_EXIT();

		FB_ZONE_ENTER("Skip?");

		StringRef function = StringRef::make(frame.function);
		StringRef filename = StringRef::make(frame.file);

		if (!skippingStopped)
		{
			if (function.doesStartWith("fb::ProgrammerAssertPrinting"))
				continue;

			if (function.doesStartWith("fb::DebugHelp"))
				continue;

			if (filename.doesContainCaseInsensitive("AppendCallstack.cpp"))
				continue;

			if (filename.doesContainCaseInsensitive("StoredCallstack.cpp"))
				continue;

			if (filename.doesContainCaseInsensitive("StoreCallstack.cpp"))
				continue;

			if (filename.doesContainCaseInsensitive("StringBuilder."))
				continue;

			skippingStopped = true;
		}

		if (function.doesContainCaseInsensitive("WinMain"))
		{
			break;
		}

		if (function.doesContainCaseInsensitive("GameBaseApplication::updateTick"))
		{
			errorMessage << "main_loop";
			break;
		}

		if (function.doesContainCaseInsensitive("Group::ThreadEntry::entry"))
		{
			errorMessage << "worker_thread";
			break;
		}

		FB_ZONE_EXIT();

		FB_ZONE_ENTER("Concat string");

		if (function.doesStartWith("lua_") || function.doesStartWith("luaopen_"))
		{
			// Merge consecutive Lua frames
			if (prevFilename == "lua")
				continue;

			prevFilename = "lua";
			errorMessage << "lua; ";
			continue;
		}

		if (filename == prevFilename)
		{
			errorMessage << "=:";
		}
		else if (filename.doesContainCaseInsensitive("objectpropertysimple"))
		{
			errorMessage << "prop:";
		}
		else
		{
			SizeType fileStart = lang::min(filename.findRightEndOf("\\"), filename.findRightEndOf("/"));
			SizeType fileEnd = lang::min(filename.findRight("."), filename.getLength());

			if (fileEnd <= filename.getLength() && fileStart < fileEnd)
				errorMessage.append(filename.getPointer() + fileStart, fileEnd - fileStart);
			else
				errorMessage << filename;

			errorMessage << ":";
		}

		prevFilename = filename;

		SizeType endOfNamespaces = function.findRightEndOf(":");
		if (endOfNamespaces < function.getLength())
		{
			errorMessage.append(function.getPointer() + endOfNamespaces, function.getLength() - endOfNamespaces);
		}
		else
		{
			errorMessage << function;
		}

		errorMessage << ":" << frame.line;

		errorMessage << "; ";
	}
}

void ProgrammerAssertPrinting::appendStackTraceCleaned(HeapString &errorMessage, const DebugHelp::StackTraceBase &stackTrace)
{
	bool skippingStopped = false;
	for (SizeType i = 0; i < stackTrace.numCapturedFrames; i++)
	{
		lang::StackFrame frame;
		if (resolveCallStackFrameUsingCache(frame, stackTrace.getCapturedFrames()[i]))
		{
			StringRef function = StringRef::make(frame.function);
			if (!skippingStopped && function.doesStartWith("fb::ProgrammerAssertPrinting"))
				continue;

			if (!skippingStopped && function.doesStartWith("fb::DebugHelp"))
				continue;

			if (!skippingStopped && StringRef::make(frame.file).doesContainCaseInsensitive("AppendCallstack.cpp"))
				continue;

			TempString stackRow;
			stackRow << "+" << function;
			for (SizeType emergencyBreak = 0; emergencyBreak < 16 && stackRow.doesContain(":ResourcePointer"); ++emergencyBreak)
			{
				// Resource pointer lines are long enough to often be truncated from the beginning
				// Do custom handling to eat partially truncated namespaces
				SizeType index = stackRow.find(":ResourcePointer");
				SizeType startOfNamespaces = index;
				while (startOfNamespaces > 0 && startOfNamespaces < stackRow.getLength() && asciiOrColon(stackRow[startOfNamespaces - 1U]))
				{
					--startOfNamespaces;
				}
				if (index > startOfNamespaces)
					stackRow.erase(startOfNamespaces, index - startOfNamespaces + 1);
			}

			stackRow.replace("fb::engine::base::resourcebase::", "");
			stackRow.replace("fb::engine::base::handle::", "");
			stackRow.replace("fb::engine::base::", "");
			stackRow.replace("fb::engine::resource::", "");
			stackRow.replace("fb::engine::instance::", "");
			stackRow.replace("fb::engine::type::", "");
			stackRow.replace("fb::container::", "");
			stackRow.replace("fb::string::util::", "");
			stackRow.replace("fb::string::", "");
			stackRow.replace("fb::", "");
			stackRow << " (line " << frame.line << " in ";
			DebugHelp::appendFileName(stackRow, frame.file);
			stackRow << ")" FB_PLATFORM_LF;
			errorMessage << stackRow;
		}
		else
		{
			errorMessage << "+(unknown)" FB_PLATFORM_LF;
		}
		skippingStopped = true;
	}
}

bool ProgrammerAssertPrinting::dumpStackTraceCleaned(HeapString &errorMessage)
{
	FB_ZONE("ProgrammerAssertPrinting::dumpStackTraceCleaned");

	DebugHelp::StackTrace stackTrace;
	if (!DebugHelp::getStackTrace(stackTrace, 0))
		return false;

	appendStackTraceCleaned(errorMessage, stackTrace);
	return true;
}

bool ProgrammerAssertPrinting::dumpStackTraceSingleRow(HeapString &errorMessage)
{
	FB_ZONE("ProgrammerAssertPrinting::dumpStackTraceCleaned");

	DebugHelp::StackTrace stackTrace;
	if (!DebugHelp::getStackTrace(stackTrace, 0))
		return false;

	appendStackTraceSingleRow(errorMessage, stackTrace);
	return true;
}

void ProgrammerAssertPrinting::print(const StringRef &assertMessage)
{
#if FB_PRINTF_ENABLED == FB_TRUE
#define FB_PROGRAMMER_PRINTF(...) FB_PRINTF(__VA_ARGS__)
#else
#define FB_PROGRAMMER_PRINTF(...) FB_FINAL_LOG_DEBUG(FB_FMT(__VA_ARGS__))
#endif

	StringRef lflf(FB_PLATFORM_LF FB_PLATFORM_LF);

	SizeType endOfPreamble = assertMessage.findEndOf(lflf);
	SizeType nextIndex = assertMessage.findEndOf(lflf, endOfPreamble);
	if (nextIndex != string::NoPosition && nextIndex < 600)
		endOfPreamble = nextIndex;

	if (endOfPreamble < assertMessage.getLength())
		FB_PROGRAMMER_PRINTF("ASSERT CALLSTACK:\n%.*s...\n\n", 600, assertMessage.getPointer() + endOfPreamble);

	if (assertMessage.doesStartWith("Copy & paste"))
	{
		// Non-shortened messages

		SizeType exprStart = assertMessage.findEndOf(FB_PLATFORM_LF FB_PLATFORM_LF "Expression: ");
		FB_PROGRAMMER_PRINTF("ASSERT:\n%.*s\n\n", assertMessage.find(FB_PLATFORM_LF, exprStart) - exprStart, assertMessage.getPointer() + exprStart);
	}
	else
	{
		// Shortened messages

		bool containsExprLine = true;
		SizeType exprStart = assertMessage.findEndOf("Expr: ");
		if (exprStart > 50 || exprStart >= assertMessage.getLength())
		{
			// End up here when coming from fb_assert (instead of fb_assertf)

			containsExprLine = false;
			exprStart = 0;
		}

		SizeType exprEnd = assertMessage.find(FB_PLATFORM_LF, exprStart);
		if (exprEnd < assertMessage.getLength())
		{
			FB_PROGRAMMER_PRINTF("ASSERTION: %.*s\n", exprEnd - exprStart, assertMessage.getPointer() + exprStart);

			if (containsExprLine)
			{
				// The message is on the next row
				SizeType msgStart = exprEnd + FB_PLATFORM_LF_LEN;
				SizeType msgEnd = assertMessage.find(FB_PLATFORM_LF, msgStart);
				if (msgStart < msgEnd && msgEnd < assertMessage.getLength())
				{
					FB_PROGRAMMER_PRINTF("MSG: %.*s\n\n", msgEnd - msgStart, assertMessage.getPointer() + msgStart);
				}
			}
		}
	}
#undef FB_PROGRAMMER_PRINTF
}

FB_END_PACKAGE0()
