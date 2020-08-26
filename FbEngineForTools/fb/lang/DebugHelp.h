#pragma once

#include "fb/lang/stacktrace/StackTrace.h"

FB_DECLARE0(DynamicString)
FB_DECLARE0(HeapString)
FB_DECLARE0(StringRef)

FB_PACKAGE0()

/**
 * DebugHelp is a collection of helpers for low-level debugging like assert handling, exception catching, minidumps 
 * and so on. It also holds relevant runtime settings.
 */
class DebugHelp
{
public:
	static void initialize();

	typedef lang::CallStackCapture<50> StackTrace;
	typedef lang::CallStackCaptureBase StackTraceBase;
	/* Returns true if tracing succeeded. Returns stack trace in stackTrace variable. Prunes skipCount frames before
	* returing (note that getStackTrace itself introduces one frame) */
	static bool getStackTrace(StackTraceBase &stackTrace, SizeType skipCount);
	/* Resolves given stackTrace to errorMessage. Returns true, if at least somewhat successful */
	static bool resolveStackTrace(const StackTraceBase &stackTrace, HeapString &errorMessage);
	/* Combined getStackTrace(), resolveStackTrace call without extra variables. Introduces two frames to stack */
	static bool dumpStackTrace(HeapString &errorMessage, SizeType skipCount = 1);
	/* Appends filename from path to string, is tolerant to both '\\' and '/' directory separators */
	static void appendFileName(HeapString &str, const char (&filePath)[lang::StackFrame::MaxFilenameLength]);

	/* Note: assertNoBreak methods take const char*s instead of StringRef to minimize assert induced code bloat */
	/* Handles assert without breaking to debugger (unless user requests it) */
	FB_NOINLINE static bool assertNoBreak(const char *predicate, const char *file, uint32_t line);
	/* Handles assertf without breaking to debugger (unless user requests it) */

	#if FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC
		#define FB_DEBUGHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE __attribute__((format(printf, 4, 5)))
	#else
		#define FB_DEBUGHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE
	#endif
	FB_NOINLINE static bool assertFNoBreak(const char *predicate, const char *file, uint32_t line, const char* formatStr, ...) FB_DEBUGHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE;
	#undef FB_DEBUGHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE

	static bool shouldIgnoreAssert(const char *file, SizeType line);

	/* Prints given predicate and file info and depending on settings (printToLogInsteadOfAssert) possibly logs it */
	static void assertPrint(const char *predicateString, const char *file, uint32_t line);

	/* Check approximate stack size in current thread and prints the result if stack is larger than ever before */
	static void checkStackAllocation();

	typedef void AssertTriggeredFuncType(const StringRef &message, const StringRef &finalDumpFilename, bool &ignoreAll);
	/* These are called when assert is triggered */
	static void registerAssertTriggeredFunc(AssertTriggeredFuncType *assertTriggeredFunc);
	/* Checks if debugger is present */
	static bool isDebuggerPresent();
	/* Gets path to running exe. Use noErrors to e.g. avoid writing to log while dumping */
	static const DynamicString &getExePathAndName(bool noErrors = false);

	/* Disabled Windows error dialog and just lets the app die in case of uncaught error */
	static void setSilentErrorHandling(bool silentMode, bool evenWithDebugger = false);
	/* Shows message box with given message */
	static void showMessageBox(const StringRef &title, const StringRef &message);
	/* Copies previously written dump file to workspace along with log, exe and pdbs. Also writes additional log 
		* message, if one is specified */
	static HeapString copyDumpToWorkspace(const StringRef &finalDumpFilename, const StringRef &additionalLogMessage);
	/* We want to load specific version of DebugHelpDLL, so let's do it here instead of linking directly to OS's version
		* Returns NULL, if loading fails totally. Writes info to errorMessage, if there's any trouble (like not being able 
		* to load our version of DLL) */
	static void *loadDebugHelpDLL(HeapString &errorMessage);
	static void waitForPDBCopy();

	/* Helper struct. Splits path in filePath to drive, directory, filename and file extension parts */
	struct FilePathSplitter
	{
		/* Define these here instead of using macros from windows.h to avoid include */
		static const uint32_t maxPath = 260;
		static const uint32_t maxDrive = 3;
		static const uint32_t maxDir = 256;
		static const uint32_t maxFName = 256;
		static const uint32_t maxExt = 256;

		char filePath[maxPath] = { 0 };
		char drive[maxDrive] = { 0 };
		char dir[maxDir] = { 0 };
		char filename[maxFName] = { 0 };
		char extension[maxExt] = { 0 };

		void split();
	};

	/* Calls GetLastError() and appends the error code in human readable form ("ErrorCode: X, blah blah") to given
		* string. Also set last error to ERROR_SUCCESS. Returns true, if error code != 0 (ERROR_SUCCESS).
		* Use like: TempString msg("XXX failed. "); DebugHelp::addGetLastErrorCode(msg); FB_LOG_ERROR(msg); */
	static bool addGetLastErrorCode(HeapString &outMessage);

	#if FB_BUILD != FB_FINAL_RELEASE
		/* Finds out who is holding handles to given file and appends information to outMessage. If no handles are found,
		 * nothing will be appended. Return true, if everything goes nicely, false, if there was an error (not finding 
		 * handles is not considered an error) */
		static bool addWhoHoldsFileOpenInfo(HeapString &outMessage, const StringRef &fileName);
	#endif

	struct Settings
	{
		typedef void DumpCallbackFuncType(uint32_t exceptionCode, void *pointers);
		/* This is called when unexpected dump is taken */
		DumpCallbackFuncType *dumpCallbackFunc = nullptr;
		typedef void LuaUnwindFuncType(HeapString &outStack);
		/* This is called when unexpected dump is taken to get Lua stack. Note: function must not clear the string
		* passed, but append instead */
		LuaUnwindFuncType *luaUnwindFunc = nullptr;
		/* File to write asserts to if we don't break on them */
		const char *assertLogFilename = "log/assert.log";
		/* Can be set to true, to get more comprehensive memory dump */
		bool fullMemoryDump = false;
		/* Can be set to true, if no debugging is to be attempted on exception (just quit) */
		bool noDebugAtUnhandledException = false;
		/* Whether or not to write dump files to workspace */
		bool dumpToWorkspace = true;
		/* Can be set to true, if no spontaneous minidumps should be done */
		bool disableMinidump = false;
		/* Can be set to true, if asserts should not stop execution */
		bool printInsteadOfAssert = false;
		/* Can be set to true, if asserts should be printed to log when skipping them. Requires that 
		 * printInsteadOfAssert is true for any effect */
		bool printToLogInsteadOfAssert = false;
		/* Can be used to disable dumps from asserts */
		bool assertDumpsEnabled = true;
	};
	static Settings settings;
};

FB_END_PACKAGE0()
