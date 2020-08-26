#pragma once

#include "fb/lang/stacktrace/StackTrace.h"
#include "fb/container/Vector.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"

FB_DECLARE_STRUCT(lang, StackFrame)

FB_PACKAGE0()

struct DumpCallstackEvent
{
	enum
	{
		MaxCallstackDepth = 32
	};


	lang::CallStackCapture<MaxCallstackDepth> stack;
	StaticString name;
	HeapString message;
	double time;
	uint32_t thread;
};

class CallstackStorage
{
public:
	static void storeCallstack(const StringRef &name, const StringRef &message);
	static void registerCallstackInfo(const StringRef &name, SizeType maxCount, SizeType offset);
	static bool resolveFrame(const StringRef &name, SizeType index, SizeType depth, lang::StackFrame &stackFrame, const char *&outMessage);
	static bool resolveFrame(const StringRef &name, SizeType index, SizeType depth, HeapString &outFile, HeapString &outFunction, SizeType &line, const char *&outMessage);
	static SizeType getFrameCount(const StringRef &name);
	static SizeType getFrameDepth(const StringRef &name, SizeType index);
	static void getCallStackNames(Vector<StaticString> &outResults);
	static void printCallstacks();

	static void appendSingleRowCallstack(HeapString &outputStr, const StringRef &name, SizeType index);

	static void dumpCallstacks(uint64_t nowTimestamp, Vector<DumpCallstackEvent> &dumpResult);
};

FB_END_PACKAGE0()
