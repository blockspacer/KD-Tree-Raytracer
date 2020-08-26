#include "Precompiled.h"

#include "StackTrace.h"

#include "fb/lang/DebugHelp.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/MemoryOperatorsConfig.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/string/HeapString.h"

#pragma warning(push)
/* 'typedef ': ignored on left of '' when no variable is declared */
#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(pop)

FB_PACKAGE1(lang)

#if FB_ASSERT_ENABLED == FB_TRUE
	#define fb_lowlevel_assertf(x, fmt, ...) \
		do {	\
			if (!(x)) \
			{ \
				HeapString str("Assertion " #x "  failed at " FB_ASSERT_FILENAME " line "); \
				str << FB_ASSERT_LINENUMBER ; \
				str.doSprintf(fmt, __VA_ARGS__); \
				FB_PRINTF("%s", str.getPointer()); \
				if (IsDebuggerPresent()) \
					__debugbreak(); \
			} \
		} while (false)

#else
	#define fb_lowlevel_assertf(x, fmt, ...) 
#endif

class SymbolAddressResolver
{
public:
	SymbolAddressResolver()
		: process(GetCurrentProcess())
	{
	}


	static void leftClipStrcpy(char *dst, size_t dstSize, const char *src)
	{
		if (!src)
		{
			dst[0] = '\0';
			return;
		}

		size_t srcSize = strlen(src) + 1;
		if (srcSize > dstSize)
		{
			const char *pos = src;
			pos += srcSize - dstSize;
			memcpy(dst, pos, dstSize);
		}
		else
		{
			memcpy(dst, src, srcSize);
		}
	}


	bool resolveAddress(StackFrame &result, DWORD64 address)
	{
#if FB_BUILD != FB_FINAL_RELEASE || FB_FINAL_RELEASE_ASSERTS_ENABLED == FB_TRUE
		static Mutex mutex;
		static bool symbolHandlerInitialized = false;
		MutexGuard guard(mutex);
		if (!symbolHandlerInitialized)
		{
			static uint32_t tryAgainCounter = 10;
			if (tryAgainCounter == 0)
				return false;

			--tryAgainCounter;
			/* Load dbghelp.dll and required functions */
			TempString errorMessage;
			HMODULE module = (HMODULE)DebugHelp::loadDebugHelpDLL(errorMessage);
			if (module == nullptr)
			{
				fb_lowlevel_assertf(false, "DebugHelp::loadDebugHelpDLL failed. Error was: %s", errorMessage.getPointer());
				return false;
			}
			if (!errorMessage.isEmpty())
			{
				FB_PRINTF("Some extra action in DebugHelp::loadDebugHelpDLL: %s\n", errorMessage.getPointer());
			}
			typedef BOOL(WINAPI *SymInitializeFuncType)(__in HANDLE hProcess, __in_opt PCTSTR UserSearchPath, __in BOOL fInvadeProcess);
			typedef PCHAR(WINAPI *SymSetHomeDirectoryFuncType)(HANDLE hProcess, PCSTR dir);
			/* Should be safe to suppress 4191: 'type cast': unsafe conversion from 'FARPROC' to 'fb::DebugHelpImpl::MiniDumpWriteDumpFn_t' */
			#pragma warning(suppress: 4191)
			SymInitializeFuncType symInitialize = (SymInitializeFuncType)::GetProcAddress(module, "SymInitialize");
			fb_lowlevel_assertf(symInitialize != nullptr, "Failed to load SymInitialize");
			#pragma warning(suppress: 4191)
			SymSetHomeDirectoryFuncType symSetHomeDirectory = (SymSetHomeDirectoryFuncType)::GetProcAddress(module, "SymSetHomeDirectory");
			fb_lowlevel_assertf(symSetHomeDirectory != nullptr, "Failed to load SymSetHomeDirectory");
			#pragma warning(suppress: 4191)
			symGetLineFromAddr64 = (SymGetLineFromAddr64FuncType)::GetProcAddress(module, "SymGetLineFromAddr64");
			fb_lowlevel_assertf(symGetLineFromAddr64 != nullptr, "Failed to load SymGetLineFromAddr64");
			#pragma warning(suppress: 4191)
			symFromAddr = (SymFromAddrFuncType)::GetProcAddress(module, "SymFromAddr");
			fb_lowlevel_assertf(symFromAddr != nullptr, "Failed to load SymFromAddr");
			if (symInitialize == nullptr || symGetLineFromAddr64 == nullptr || symFromAddr == nullptr)
				return false;

			/* Wait for PDB copy to finish */
			#if FB_ENABLE_ALLOCATION_TRACKER == FB_FALSE
				DebugHelp::waitForPDBCopy();
			#endif

			char currentDir[MAX_PATH] = { 0 };
			GetCurrentDirectoryA(MAX_PATH, currentDir);
			{
				/* Set home directory*/
				CacheHeapString<MAX_PATH> homeDirectory;
				homeDirectory << currentDir << "\\builds\\temp";
				symSetHomeDirectory(process, homeDirectory.getPointer());
			}

			/* Set paths for symInitialize */
			DebugHelp::FilePathSplitter filePathSplitter;
			DWORD filenameLength = GetModuleFileName(NULL, filePathSplitter.filePath, MAX_PATH);
			filePathSplitter.split();
			CacheHeapString<MAX_PATH * 3> pdbPaths;
			pdbPaths << filePathSplitter.drive << filePathSplitter.dir;
			pdbPaths.trimRight(1);
			pdbPaths << ";";
			pdbPaths << currentDir;
			pdbPaths << ";";
			pdbPaths << currentDir << "\\builds\\temp\\pdb";
			if (!symInitialize(process, pdbPaths.getPointer(), TRUE))
			{
				DWORD lastError = GetLastError();
				LPVOID msgBuffer = nullptr;
				DWORD numChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuffer, 0, NULL);
				fb_lowlevel_assertf(false, "SymInitialize returned with error %d (%s) from SymFromAddr", int32_t(lastError), numChars > 0 ? reinterpret_cast<const char*>(msgBuffer) : "Failed to parse");
				return false;
			}

			symbolHandlerInitialized = true;
		}
		// get symbol
		DWORD64 displacement = 0;
		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {0};
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;
		if (!symFromAddr(process, address, &displacement, pSymbol))
		{
			/* This happens. Not all code can be resolved. Left here for debugging */
			//DWORD lastError = GetLastError();
			//LPVOID msgBuffer = nullptr;
			//DWORD numChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			//	NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuffer, 0, NULL);
			//fb_lowlevel_assertf(false, "SymFromAddr returned with error %d (%s) from SymFromAddr", lastError, numChars > 0 ? msgBuffer : "Failed to parse");
			return false;
		}

		// get filename + line
		IMAGEHLP_LINE64 line;
		memset(&line, 0, sizeof(IMAGEHLP_LINE64));
		DWORD displacement2 = 0;
		if (!symGetLineFromAddr64(process, address, &displacement2, &line))
		{
			/* This happens. Not all code can be resolved. Left here for debugging */
			//DWORD lastError = GetLastError();
			//LPVOID msgBuffer = nullptr;
			//DWORD numChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			//	NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuffer, 0, NULL);
			//fb_lowlevel_assertf(false, "SymGetLineFromAddr64 returned with error %d (%s) from SymGetLineFromAddr64", lastError, numChars > 0 ? msgBuffer : "Failed to parse");
		}

		leftClipStrcpy(result.function, sizeof(result.function), pSymbol->Name);
		leftClipStrcpy(result.file, sizeof(result.file), line.FileName);
		result.line = line.LineNumber;
		result.displacement = displacement;

		return true;
#else
		return false;
#endif
	}
	typedef BOOL(WINAPI *SymFromAddrFuncType)(__in HANDLE hProcess, __in DWORD64 Address, __out_opt PDWORD64 Displacement, __inout PSYMBOL_INFO Symbol);
	typedef BOOL(WINAPI *SymGetLineFromAddr64FuncType)(__in HANDLE hProcess, __in DWORD64 dwAddr, __out PDWORD pdwDisplacement, __out PIMAGEHLP_LINE64 Line);

	HANDLE process;
	SymFromAddrFuncType symFromAddr;
	SymGetLineFromAddr64FuncType symGetLineFromAddr64;
};

bool resolveCallStackFrame(StackFrame& frame, const CallStackFrameCapture &capturedFrame)
{
	static Mutex resolverMutex;
	MutexGuard mg(resolverMutex);
	static SymbolAddressResolver *resolver = new SymbolAddressResolver();

	return resolver->resolveAddress(frame, (DWORD64)(uintptr_t)capturedFrame);
}

uint32_t captureCallStackImpl(CallStackFrameCapture *frameStore, uint32_t maxDepth, uint32_t skipCount)
{
	return uint32_t(CaptureStackBackTrace(skipCount, (ULONG)(maxDepth), frameStore, NULL));
}

FB_END_PACKAGE1()
