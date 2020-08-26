#pragma once

FB_DECLARE0(StringRef)
FB_DECLARE0(HeapString)
FB_DECLARE_STRUCT(lang, CallStackCaptureBase)

FB_PACKAGE0()

struct ProgrammerAssertPrinting
{
	static bool dumpStackTraceCleaned(HeapString &errorMessage);
	static bool dumpStackTraceSingleRow(HeapString &errorMessage);
	static void appendStackTraceSingleRow(HeapString &errorMessage, const lang::CallStackCaptureBase &stackTrace);
	static void appendStackTraceCleaned(HeapString &errorMessage, const lang::CallStackCaptureBase &stackTrace);
	static void print(const StringRef &assertMessage);
};

FB_END_PACKAGE0()
