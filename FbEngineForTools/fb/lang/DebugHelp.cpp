#include "Precompiled.h"
#include "DebugHelp.h"

#include "fb/container/Vector.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/ProgrammerAssertPrinting.h"
#include "fb/lang/ProjectInfo.h"
#include "fb/lang/platform/LineFeed.h"
#include "fb/lang/resource.h"
#include "fb/lang/thread/DataGuard.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/time/SystemTime.h"
#include "fb/string/HeapString.h"
#include "fb/string/util/CreateTemporaryHeapString.h"
#include "fb/string/util/UnicodeConverter.h"

#include <cstdio>

#pragma warning(push)
/* 'typedef ': ignored on left of '' when no variable is declared */
#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(pop)

#include <TlHelp32.h>

#include <process.h>
#include <Psapi.h>

#include "fbtools/fb_symstore/Cabinet.h"

#ifdef NO_ATL_SUPPORT
	/* Inhibit custom assert dialog usage (required if no ATL support on vs express or 2013 without older versions) */
	#define FB_NO_CUSTOM_ASSERT_DIALOG FB_TRUE
#else
	#define FB_NO_CUSTOM_ASSERT_DIALOG FB_FALSE
#endif
#if FB_BUILD != FB_FINAL_RELEASE
	/* For checking open file handles on file */
	#include <RestartManager.h>
	#pragma comment(lib, "Rstrtmgr.lib")
#endif

/* If FB_PRINTF is not defined, just printf asserts and hope that's good enough */
#if FB_PRINTF_ENABLED == FB_TRUE
	#define FB_ASSERT_PRINTF(...) FB_PRINTF(__VA_ARGS__)
#elif FB_PRINTF_ENABLED == FB_FALSE
	#define FB_ASSERT_PRINTF(...) printf(__VA_ARGS__)
#else
	#error FB_PRINTF_ENABLED not defined
#endif

FB_PACKAGE0()

typedef CacheHeapString<MAX_PATH + 1> PathString;

void DebugHelp::FilePathSplitter::split()
{
	fb_static_assert(MAX_PATH <= maxPath);
	fb_static_assert(_MAX_DRIVE <= maxDrive);
	fb_static_assert(_MAX_DIR <= maxDir);
	fb_static_assert(_MAX_FNAME <= maxFName);
	fb_static_assert(_MAX_EXT <= maxExt);

	_splitpath_s(filePath, drive, _MAX_DRIVE, dir, _MAX_DIR, filename, _MAX_FNAME, extension, _MAX_EXT);
}

struct DebugHelpGuard
{
	DebugHelpGuard()
	{
		oldValue = lang::atomicIncAcquire(getDebugHelpBusy());
	}
	~DebugHelpGuard()
	{
		lang::atomicDecRelease(getDebugHelpBusy());
	}

	bool isBusy() const
	{
		return oldValue != 0;
	}

	static lang::AtomicInt32 &getDebugHelpBusy()
	{
		static lang::AtomicInt32 busy;
		return busy;
	}
	int oldValue;
};


class DebugHelpImpl
{
public:
	static bool getModuleName(void *address, HeapString &outModuleName, intptr_t &outBaseAddr)
	{
		/* Get the base address */
		MEMORY_BASIC_INFORMATION info;
		memset(&info, 0, sizeof(MEMORY_BASIC_INFORMATION));
		if (VirtualQuery(address, &info, sizeof(MEMORY_BASIC_INFORMATION)) >= sizeof(MEMORY_BASIC_INFORMATION))
		{
			outBaseAddr = (intptr_t)info.BaseAddress;

			/* Get module handle from base address */
			HMODULE module = 0;
			if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)info.BaseAddress, &module) != 0)
			{
				DebugHelp::FilePathSplitter filePathSplitter;
				DWORD filenameLength = GetModuleFileName(module, filePathSplitter.filePath, MAX_PATH);
				filePathSplitter.split();
				outModuleName << filePathSplitter.filename;
				outModuleName << filePathSplitter.extension;
				return true;
			}
		}
		return false;
	}

	struct CreateDumpThreadProcParams
	{
		CreateDumpThreadProcParams(PEXCEPTION_POINTERS exceptionPointers = nullptr, HeapString *errorMessage = nullptr, HeapString *finalDumpFilename = nullptr)
			: exceptionPointers(exceptionPointers)
			, errorMessage(errorMessage)
			, finalDumpFilename(finalDumpFilename)
		{
		}

		PEXCEPTION_POINTERS exceptionPointers = nullptr;
		HeapString *errorMessage = nullptr;
		HeapString *finalDumpFilename = nullptr;
		bool success = false;
	};

	/* Entry point for minidump creation thread */
	static unsigned int __stdcall createDumpThreadProc(void *param)
	{
		/* Turn asserts off to avoid unnecessary complications on really bad crashes */
		bool originalPrintAsserts = DebugHelp::settings.printInsteadOfAssert;
		bool originalPrintToLogAsserts = DebugHelp::settings.printToLogInsteadOfAssert;
		DebugHelp::settings.printInsteadOfAssert = true;
		DebugHelp::settings.printToLogInsteadOfAssert = true;

		CreateDumpThreadProcParams *params = reinterpret_cast<CreateDumpThreadProcParams*>(param);
		PEXCEPTION_POINTERS exceptionPointers = params->exceptionPointers;
		CacheHeapString<16 * 1024> errorMessageLocal;
		HeapString &errorMessage = params->errorMessage != nullptr ? *params->errorMessage : errorMessageLocal;
		PathString finalDumpFilenameLocal;
		HeapString &finalDumpFilename = params->finalDumpFilename != nullptr ? *params->finalDumpFilename : finalDumpFilenameLocal;

		errorMessage << lang::ProjectInfo::getFullBuildString() << FB_PLATFORM_LF;
		errorMessage.doSprintf("Exception code: 0x%x%s", uint32_t(exceptionPointers->ExceptionRecord->ExceptionCode), FB_PLATFORM_LF);
		errorMessage.doSprintf("Address: 0x%p", exceptionPointers->ExceptionRecord->ExceptionAddress);
		{
			PathString moduleName;
			intptr_t moduleAddress = 0;
			if (getModuleName(exceptionPointers->ExceptionRecord->ExceptionAddress, moduleName, moduleAddress))
			{
				errorMessage << " in ";
				errorMessage << moduleName;
				errorMessage << ":";
				errorMessage.doSprintf("%p", (const void*)moduleAddress);
			}
		}
		errorMessage << "\n";

		{
			StaticVector<PathString, DebugHelp::StackTrace::maxDepth> modules;
			for (SizeType i = 0; i < getMinidumpStackTrace().numCapturedFrames; i++)
			{
				PathString moduleName;
				intptr_t moduleAddress = 0;
				if (getModuleName(getMinidumpStackTrace().getCapturedFrames()[i], moduleName, moduleAddress))
				{
					bool alreadyListed = false;
					for (SizeType j = 0, numModules = modules.getSize(); j < numModules; j++)
					{
						if (moduleName == modules[j])
						{
							alreadyListed = true;
							break;
						}
					}
					if (!alreadyListed)
						modules.pushBack(moduleName);
				}
			}
			if (!modules.isEmpty())
			{
				errorMessage << "Related modules:";
				for (const PathString &module : modules)
				{
					errorMessage << " ";
					errorMessage << module;
				}
				errorMessage << FB_PLATFORM_LF;
			}
		}


		if (DebugHelp::settings.luaUnwindFunc != nullptr)
		{
			LargeTempString stacks;
			(*DebugHelp::settings.luaUnwindFunc)(stacks);
			errorMessage << stacks;
		}

		params->success = writeDumpImpl(exceptionPointers, errorMessage, finalDumpFilename);

#if FB_BUILD != FB_FINAL_RELEASE || FB_FINAL_RELEASE_ASSERTS_ENABLED == FB_TRUE
		DebugHelp::resolveStackTrace(getMinidumpStackTrace(), errorMessage);
#endif

		PathString dumpPath = DebugHelp::copyDumpToWorkspace(finalDumpFilename, errorMessage);
		if (!dumpPath.isEmpty())
			errorMessage << FB_PLATFORM_LF << "Dump copied to " << dumpPath;

		errorMessage << FB_PLATFORM_LF << "Press CTRL+C to copy this message";
		showExceptionMessage(errorMessage);

		if (DebugHelp::settings.noDebugAtUnhandledException)
		{
			/* Exit code of 0xEC hex == 236 dec */
			ExitProcess(0xEC);
		}
		DebugHelp::settings.printInsteadOfAssert = originalPrintAsserts;
		DebugHelp::settings.printToLogInsteadOfAssert = originalPrintToLogAsserts;
		return 0;
	}

	static LONG CALLBACK exceptionHandler(PEXCEPTION_POINTERS exceptionPointers)
	{
		/* HACK: Minidumping on assert will crash, but still manages to write the dump. So we ignore that crash and continue like nothing happened */
		if (getDoingAssertDump())
			return EXCEPTION_CONTINUE_SEARCH;

		if (DebugHelp::settings.disableMinidump)
			return EXCEPTION_CONTINUE_SEARCH;

		if (exceptionPointers && exceptionPointers->ExceptionRecord)
		{
			/* Filter out uninteresting exceptions */
			switch (exceptionPointers->ExceptionRecord->ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION:
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			case EXCEPTION_DATATYPE_MISALIGNMENT:
			case EXCEPTION_ILLEGAL_INSTRUCTION:
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
			case EXCEPTION_STACK_OVERFLOW:
				break;
			default:
				return EXCEPTION_CONTINUE_SEARCH;
			};

			// ignore exceptions from non-FB threads (ESET Internet Security / ebehmoni.dll freeze fix)
			if (!Thread::isFBThread())
				return EXCEPTION_CONTINUE_SEARCH;

			static bool dumped = false;
			if (!dumped)
			{
				dumped = true;
				if (DebugHelp::settings.dumpCallbackFunc != nullptr)
					(*DebugHelp::settings.dumpCallbackFunc)(exceptionPointers->ExceptionRecord->ExceptionCode, exceptionPointers);

				CreateDumpThreadProcParams params(exceptionPointers);
				DebugHelpImpl::createDump(&params);
			}
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	static void createDump(CreateDumpThreadProcParams *params)
	{
		FB_CAPTURE_STACK_TRACE(getMinidumpStackTrace());
		DWORD threadId = 0;
		HANDLE thread = (HANDLE)_beginthreadex(nullptr, 0, DebugHelpImpl::createDumpThreadProc, params, 0, 0);
		WaitForSingleObject(thread, INFINITE);
	}

	static void showExceptionMessage(const StringRef &message)
	{
		if (!DebugHelp::settings.noDebugAtUnhandledException)
		{
			DebugHelp::showMessageBox(StringRef("Unhandled exception"), message);
		}
		else
		{
			FILE *f = fopen(DebugHelp::settings.assertLogFilename, "at");
			if (f != nullptr)
			{
				fputs(message.getPointer(), f);
				fclose(f);
			}
		}
	}

	static bool launchWriteDump(struct _EXCEPTION_POINTERS *ep, HeapString &errorMessage, HeapString &finalDumpFilename)
	{
		FB_CAPTURE_STACK_TRACE(DebugHelpImpl::getMinidumpStackTrace());
		DebugHelpImpl::CreateDumpThreadProcParams params;
		params.exceptionPointers = ep;
		params.errorMessage = &errorMessage;
		params.finalDumpFilename = &finalDumpFilename;
		HANDLE thread = (HANDLE)_beginthreadex(nullptr, 0, DebugHelpImpl::createDumpThreadProc, &params, 0, 0);
		WaitForSingleObject(thread, INFINITE);
		return params.success;

	}

	static bool createMinidumpImpl(struct _EXCEPTION_POINTERS *ep, HeapString &errorMessage, HeapString &finalDumpFilename)
	{
		CreateDumpThreadProcParams params(ep, &errorMessage, &finalDumpFilename);
		createDump(&params);
		return params.success;
	}

	static bool createMinidumpForAssertImpl(struct _EXCEPTION_POINTERS *ep, HeapString &errorMessage, HeapString &finalDumpFilename)
	{
		bool result = writeDumpImpl(ep, errorMessage, finalDumpFilename);
		return result;
	}

	static void createMinidumpForAssert(HeapString &errorMessage, HeapString &finalDumpFilename)
	{
		/* HACK: Minidumping on assert will crash, but still manages to write the dump. We set doingAssertDump so that 
		 * exception handler knows to ignore that crash and continue like nothing happened. Happily, assertNoBreak has 
		 * a mutex to prevent concurrent access via multiple threads, so we can ignore such things here */
		getDoingAssertDump() = true;
		/* no dump when debugger is attached */
		if (DebugHelp::isDebuggerPresent())
			return;

		__try
		{
			throw -1;
		}
		__except (createMinidumpForAssertImpl(GetExceptionInformation(), errorMessage, finalDumpFilename))
		{
		}
		getDoingAssertDump() = false;
	}

	static HWND getMainHWND()
	{
		static int numHWNDs = 0;
		static const int maxHWNDs = 256;
		static HWND hwnds[maxHWNDs] = { 0 };
		struct Impl
		{
			static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
			{
				DWORD procId = 0;
				GetWindowThreadProcessId(hwnd, &procId);
				if (procId == GetCurrentProcessId())
				{
					if (numHWNDs < maxHWNDs)
						hwnds[numHWNDs++] = hwnd;
				}
				return TRUE;
			}
		};
		numHWNDs = 0;
		EnumWindows(Impl::enumWindowsProc, 0);

		for (int i = 0; i < numHWNDs; i++)
		{
			/* Only care about main windows */
			HWND hwnd = hwnds[i];
			bool mainWindow = true;
			HWND parent = GetParent(hwnd);
			for (int j = 0; j < numHWNDs; j++)
			{
				if (i != j && hwnds[j] == parent)
				{
					mainWindow = false;
					break;
				}
			}
			if (!mainWindow)
				continue;

			/* Ignore hidden windows */
			if (!IsWindowVisible(hwnd))
				continue;

			/* ignore tool windows */
			WINDOWINFO info = { 0 };
			GetWindowInfo(hwnd, &info);
			if (info.dwExStyle & WS_EX_TOOLWINDOW)
				continue;

			/* Ignore windows with no title */
			char text[128] = { 0 };
			SendMessageTimeoutA(hwnd, WM_GETTEXT, 128, (LPARAM)text, SMTO_NORMAL, 100, 0);
			if (strlen(text) == 0)
				continue;

			return hwnd;
		}
		return 0;
	}

	static LRESULT APIENTRY buttProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_GETDLGCODE)
			return DLGC_WANTALLKEYS;

		if (uMsg == WM_SYSKEYDOWN)
		{
			if ((lParam & (1 << 29)) != 0)
			{
				/* ALT+I == ignore, ALT+D == debug, ALT+E == exit */
				if (wParam == 'I')
				{
					PostMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)IDC_IGNORE, (LPARAM)0);
					return 1;
				}
				if (wParam == 'D')
				{
					PostMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)IDC_DEBUG, (LPARAM)0);
					return 1;
				}
				if (wParam == 'E')
				{
					PostMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)IDC_EXIT, (LPARAM)0);
					return 1;
				}
			}
		}

		/* Ignore other keys */
		if (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP || uMsg == WM_CHAR || uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)
			return 1;

		return CallWindowProc(getWpOrigButtonProc(), hwnd, uMsg, wParam, lParam);
	}

	static INT_PTR CALLBACK assertDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_INITDIALOG:
		{
			SetDlgItemTextW(hwnd, IDC_EDIT1, getAssertDialogText().getPointer());
			HWND hwndOwner = GetDesktopWindow();
			RECT rc, rcDlg, rcOwner;
			GetWindowRect(hwndOwner, &rcOwner);
			GetWindowRect(hwnd, &rcDlg);
			CopyRect(&rc, &rcOwner);
			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);
			SetWindowPos(hwnd, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE);

			#pragma warning (suppress: 4302)
			HICON hIcon = ::LoadIcon(NULL, MAKEINTRESOURCE(IDI_ERROR));
			SendDlgItemMessageW(hwnd, IDC_STATIC1, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

			EnableWindow(GetDlgItem(hwnd, IDC_IGNOREALL), TRUE);

			/* I don't know what this is */
			#define GWL_WNDPROC (-4)
			/* Haxor button proc */
			getWpOrigButtonProc() = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwnd, IDC_IGNORE), GWL_WNDPROC, (LONG_PTR)buttProc);
			getWpOrigButtonProc() = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwnd, IDC_IGNOREALL), GWL_WNDPROC, (LONG_PTR)buttProc);
			getWpOrigButtonProc() = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwnd, IDC_EXIT), GWL_WNDPROC, (LONG_PTR)buttProc);
			getWpOrigButtonProc() = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwnd, IDC_DEBUG), GWL_WNDPROC, (LONG_PTR)buttProc);
			#undef GWL_WNDPROC
		}
		break;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_IGNOREALL:
			case IDC_EXIT:
			case IDC_IGNORE:
			case IDC_DEBUG:
			{
				EndDialog(hwnd, INT_PTR(wParam));
			}
			return TRUE;
			}
		}
		break;
		}
		return FALSE;
	}

	typedef BOOL(WINAPI *MiniDumpWriteDumpFn_t)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
		CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	static bool writeDumpImpl(struct _EXCEPTION_POINTERS *exceptionPointers, HeapString &errorMessage, HeapString &finalDumpFilename)
	{
		DebugHelpGuard guard;
		if (guard.isBusy())
		{
			errorMessage << "Dump not generated (DebugHelp is busy)." << FB_PLATFORM_LF;
			return false;
		}

		HMODULE module = (HMODULE)DebugHelp::loadDebugHelpDLL(errorMessage);
		if (module == NULL)
			return false;

		/* Get function from DLL */
		/* Should be safe to suppress 4191: 'type cast': unsafe conversion from 'FARPROC' to 'fb::DebugHelpImpl::MiniDumpWriteDumpFn_t' */
		#pragma warning(suppress: 4191)
		MiniDumpWriteDumpFn_t miniDumpWriteDump = (MiniDumpWriteDumpFn_t)::GetProcAddress(module, "MiniDumpWriteDump");
		if (miniDumpWriteDump == NULL)
		{
			errorMessage << "Cannot create minidump: Failed to load MiniDumpWriteDump" << FB_PLATFORM_LF;
			return false;
		}

		/* Create file */
		DebugHelp::FilePathSplitter filePathSplitter;
		DWORD filenameLength = GetModuleFileName(NULL, filePathSplitter.filePath, MAX_PATH);
		filePathSplitter.split();

		TempString tickCountStr;
		tickCountStr.doSprintf("%lx", GetTickCount());

		PathString filename;
		filename << filePathSplitter.drive << filePathSplitter.dir << "log\\" << filePathSplitter.filename << filePathSplitter.extension << "_" << tickCountStr << ".dmp";;
		HANDLE dumpFile = CreateFile(filename.getPointer(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
		if (dumpFile == INVALID_HANDLE_VALUE)
		{
			/* Failed: try directly to same path */
			filename.clear();
			filename << filePathSplitter.drive << filePathSplitter.dir << filePathSplitter.filename << filePathSplitter.extension << "_" << tickCountStr << ".dmp";
			dumpFile = CreateFile(filename.getPointer(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
			if (dumpFile == INVALID_HANDLE_VALUE)
			{
				errorMessage << "Cannot create minidump: Failed open file" << FB_PLATFORM_LF << filename << FB_PLATFORM_LF;
				return false;
			}
		}

		/* Dump */
		MINIDUMP_EXCEPTION_INFORMATION expParam;
		memset(&expParam, 0, sizeof(MINIDUMP_EXCEPTION_INFORMATION));
		expParam.ThreadId = GetCurrentThreadId();
		expParam.ExceptionPointers = exceptionPointers;
		/* ClientPointers should be FALSE when dumping within same process */
		expParam.ClientPointers = FALSE;
		MINIDUMP_USER_STREAM_INFORMATION usrStrmInfo;
		memset(&usrStrmInfo, 0, sizeof(MINIDUMP_USER_STREAM_INFORMATION));
		MINIDUMP_USER_STREAM usrStrm;
		memset(&usrStrm, 0, sizeof(MINIDUMP_USER_STREAM));
		usrStrm.Type = CommentStreamA;
		usrStrm.BufferSize = (ULONG)(errorMessage.getLength() + 1);
		usrStrm.Buffer = (void*)errorMessage.getPointer();
		usrStrmInfo.UserStreamCount = 1;
		usrStrmInfo.UserStreamArray = &usrStrm;
		finalDumpFilename << filename;
		auto process = GetCurrentProcess();
		auto processId = GetCurrentProcessId();
		int result = miniDumpWriteDump(process, processId, dumpFile, DebugHelp::settings.fullMemoryDump ? MiniDumpWithFullMemory : MiniDumpWithDataSegs, &expParam, &usrStrmInfo, NULL);
		if (result)
		{
			errorMessage << "Minidump: " << filename << FB_PLATFORM_LF;
		}
		else
		{
			errorMessage.doSprintf("Cannot create minidump: MinidumpWriteDump failed with code 0x%x%s", result, FB_PLATFORM_LF);
			CloseHandle(dumpFile);
			return false;
		}

		CloseHandle(dumpFile);
		return true;
	}

	static WNDPROC &getWpOrigButtonProc()
	{
		static WNDPROC proc = nullptr;
		return proc;
	}

	static DebugHelp::StackTrace &getMinidumpStackTrace()
	{
		static DebugHelp::StackTrace stackTrace;
		return stackTrace;
	}

	static bool &getDoingAssertDump()
	{
		static bool doingAssertDump = false;
		return doingAssertDump;
	}

	static bool getPDBInfo(HeapString &pdbName, HeapString &pdbVersion)
	{
		MODULEINFO moduleInfo = { 0 };
		if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(moduleInfo)))
			return false;

		const uint8_t *base = (const uint8_t *)moduleInfo.lpBaseOfDll;
		PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)base;
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(base + dosHeader->e_lfanew);
		PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)(base + dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
		if (!ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress)
			return false;

		if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size < sizeof(IMAGE_DEBUG_DIRECTORY))
			return false;

		PIMAGE_DEBUG_DIRECTORY debugDirectory = (PIMAGE_DEBUG_DIRECTORY)(base + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);
		if (debugDirectory->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
			return false;

		struct Data
		{
			DWORD Signature;
			GUID PdbSig70;
			DWORD PdbAge;
			char PdbFileName[1];
		};
		if (debugDirectory->SizeOfData < sizeof(Data))
			return false;

		const Data &data = *(const Data *)(base + debugDirectory->AddressOfRawData);
		const char *pdbFileName = strrchr(data.PdbFileName, '\\');
		if (!pdbFileName)
			pdbFileName = strrchr(data.PdbFileName, '/');
		if (!pdbFileName)
			return false;

		pdbName = HeapString(pdbFileName + 1);
		pdbVersion.doSprintf("%08lX", data.PdbSig70.Data1);
		pdbVersion.doSprintf("%04X", data.PdbSig70.Data2);
		pdbVersion.doSprintf("%04X", data.PdbSig70.Data3);
		for (int j = 0; j < 8; j++)
			pdbVersion.doSprintf("%02X", data.PdbSig70.Data4[j]);
		pdbVersion.doSprintf("%x", data.PdbAge);
		return true;
	}

	static Mutex &getPDBCopyMutex()
	{
		static Mutex mutex;
		return mutex;
	}

	static void extractPDBArchive()
	{
		MutexGuard mg(getPDBCopyMutex());
		static bool extracted = false;
		if (extracted)
			return;
		extracted = true;

		HeapString pdbName, pdbVersion;
		if (!getPDBInfo(pdbName, pdbVersion))
			return;

		char currentDir[MAX_PATH] = { 0 };
		GetCurrentDirectoryA(sizeof(currentDir), currentDir);

		TempString destinationPath;
		destinationPath << currentDir << "\\builds\\temp\\pdb\\" << pdbName << "\\" << pdbVersion;

		TempString destinationFilename;
		destinationFilename << destinationPath << "\\" << pdbName;
		if (GetFileAttributesA(destinationFilename.getPointer()) != INVALID_FILE_ATTRIBUTES)
		{
			// already extracted
			return;
		}

		// .pdb -> .pd_
		TempString archiveFilename(destinationFilename);
		archiveFilename.trimRight(1);
		archiveFilename += "_";

		Vector<HeapString> log;
		if (!cabinet::extractCabinet(archiveFilename, destinationPath, log))
		{
			DeleteFileA(destinationFilename.getPointer());
		}
	}

	static unsigned int __stdcall copyPDBProc(void *param)
	{
		MutexGuard mg(getPDBCopyMutex());

		HeapString pdbName, pdbVersion;
		if (!getPDBInfo(pdbName, pdbVersion))
			return 0;

		TempString source;
		source << "\\\\workspace\\workspace\\pdb\\" << pdbName << "\\" << pdbVersion << "\\" << pdbName;
		// .pdb -> .pd_
		source.trimRight(1);
		source += "_";

		WIN32_FILE_ATTRIBUTE_DATA sourceAttributes = { 0 };
		if (!GetFileAttributesExA(source.getPointer(), GetFileExInfoStandard, &sourceAttributes))
			return 0;

		char currentDir[MAX_PATH] = { 0 };
		GetCurrentDirectoryA(sizeof(currentDir), currentDir);
		TempString destination;
		destination << currentDir << "\\builds\\temp\\pdb\\" << pdbName << "\\" << pdbVersion << "\\" << pdbName;
		// .pdb -> .pd_
		destination.trimRight(1);
		destination += "_";

		WIN32_FILE_ATTRIBUTE_DATA destinationAttributes = { 0 };
		if (GetFileAttributesExA(destination.getPointer(), GetFileExInfoStandard, &destinationAttributes))
		{
			if (destinationAttributes.ftLastWriteTime.dwHighDateTime == sourceAttributes.ftLastWriteTime.dwHighDateTime
				&& destinationAttributes.ftLastWriteTime.dwLowDateTime == sourceAttributes.ftLastWriteTime.dwLowDateTime)
			{
				// timestamp already matches, no need to copy
				return 0;
			}
		}

		// create path
		for (SizeType i = StringRef(currentDir).getLength() + 1; destination[i]; i++)
		{
			if (destination[i] == '\\')
			{
				TempString path(destination.getPointer(), i);
				CreateDirectoryA(path.getPointer(), nullptr);
			}
		}

		// copy the file
		if (!CopyFileA(source.getPointer(), destination.getPointer(), FALSE))
			return 0;

		// create pingme.txt so the path recognized as a symbol store
		{
			TempString filename;
			filename << currentDir << "\\builds\\temp\\pdb\\pingme.txt";
			HANDLE handle = CreateFileA(filename.getPointer(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, 0, nullptr);
			if (handle == INVALID_HANDLE_VALUE)
				return 0;
			CloseHandle(handle);
		}

		// set timestamp
		{
			HANDLE handle = CreateFileA(destination.getPointer(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
			if (handle != INVALID_HANDLE_VALUE)
			{
				SetFileTime(handle, nullptr, &sourceAttributes.ftLastWriteTime, &sourceAttributes.ftLastWriteTime);
				CloseHandle(handle);
			}
		}
		return 0;
	}

	static void startCopyPDBThread()
	{
#if FB_EDITOR_ENABLED == FB_TRUE || FB_FINAL_RELEASE_ASSERTS_ENABLED == FB_TRUE
		_beginthreadex(nullptr, 0, DebugHelpImpl::copyPDBProc, nullptr, 0, 0);
#endif
	}

	static Mutex &getMutex()
	{
		static Mutex mutex;
		return mutex;
	}

	static string::SimpleUTF16String &getAssertDialogText()
	{
		static string::SimpleUTF16String text;
		return text;
	}

	static PodVector<DebugHelp::AssertTriggeredFuncType *> &getAssertTriggeredFuncs()
	{
		static PodVector<DebugHelp::AssertTriggeredFuncType *> funcs;
		return funcs;
	}

};


DebugHelp::Settings DebugHelp::settings;


void DebugHelp::initialize()
{
	if (!settings.disableMinidump)
	{
		AddVectoredExceptionHandler(0, DebugHelpImpl::exceptionHandler);
		DebugHelpImpl::startCopyPDBThread();
	}
}

bool DebugHelp::getStackTrace(StackTraceBase &stackTrace, SizeType skipCount)
{
#ifdef FB_CAPTURE_STACK_TRACE_WITH_SKIPPING
	{
		FB_CAPTURE_STACK_TRACE_WITH_SKIPPING(stackTrace, skipCount);
		return stackTrace.numCapturedFrames > 0;
	}
#else
	{
		FB_CAPTURE_STACK_TRACE(stackTrace);
		bool success = stackTrace.numCapturedFrames > 0;
		if (skipCount > 0)
		{
			for (SizeType i = skipCount; i < stackTrace.numCapturedFrames; ++i)
				stackTrace.getCapturedFrames()[i - skipCount] = stackTrace.getCapturedFrames()[i];

			if (stackTrace.numCapturedFrames <= skipCount)
				stackTrace.numCapturedFrames = 0;
			else
				stackTrace.numCapturedFrames -= skipCount;
		}
		return success;
	}
#endif
}

bool DebugHelp::resolveStackTrace(const StackTraceBase &stackTrace, HeapString &errorMessage)
{
	for (SizeType i = 0; i < stackTrace.numCapturedFrames; i++)
	{
		lang::StackFrame frame;
		if (lang::resolveCallStackFrame(frame, stackTrace.getCapturedFrames()[i]))
		{
			errorMessage << "+" << frame.function << " (line " << frame.line;
			if (StringRef(&frame.function[0]).doesContain("anonymous"))
			{
				errorMessage << " in ";
				DebugHelp::appendFileName(errorMessage, frame.file);
			}
			errorMessage << ")" << FB_PLATFORM_LF;
		}
		else
			errorMessage << "+(unknown)" << FB_PLATFORM_LF;
	}
	/* Yes, yes, we always succeed */
	return true;
}

bool DebugHelp::dumpStackTrace(HeapString &errorMessage, SizeType skipCount)
{
	StackTrace stackTrace;
	bool success = getStackTrace(stackTrace, skipCount);
	success = success && resolveStackTrace(stackTrace, errorMessage);
	return success;
}

void DebugHelp::appendFileName(HeapString &str, const char (&filePath)[lang::StackFrame::MaxFilenameLength])
{
	StringRef filename(&filePath[0]);
	SizeType index = filename.findRightEndOf("\\");
	if (index >= filename.getLength())
		index = filename.findRightEndOf("/");

	if (index < filename.getLength())
		str.append(filename.getPointer() + index, filename.getLength() - index);
	else
		str << filename;
}

static bool useShortInfo()
{
#ifdef FB_PROGRAMMER_ASSERT_PRINTING
	// More concise assert messages for easier skimming while debugging
	return true;
#else
	// Maximum verbosity for logging
	return false;
#endif
}


struct AssertInfo
{
	AssertInfo(const char *assertFile, uint32_t line)
		: assertFile(StringRef(assertFile))
		, line(line)
	{

	}
	const StringRef assertFile;
	const uint32_t line = 0;
	uint32_t dumpCount = 0;
	bool ignore = false;
};

static ScopedRef<Vector<AssertInfo>> getAssertInfoVector()
{
	static DataGuard<Vector<AssertInfo>> infos;
	return infos;
}

static void fetchAssertInfo(AssertInfo &infoOut)
{
	ScopedRef<Vector<AssertInfo>> assertInfoVectorRef = getAssertInfoVector();
	for (const AssertInfo &info : assertInfoVectorRef.getRawRef())
	{
		if (info.line == infoOut.line && info.assertFile == infoOut.assertFile)
		{
			infoOut.dumpCount = info.dumpCount;
			infoOut.ignore = info.ignore;
			return;
		}
	}
}

static void setAssertInfo(const AssertInfo &infoIn)
{
	ScopedRef<Vector<AssertInfo> > assertInfoVectorRef = getAssertInfoVector();
	for (AssertInfo &info : assertInfoVectorRef.getRawRef())
	{
		if (info.line == infoIn.line && info.assertFile == infoIn.assertFile)
		{
			info.dumpCount = infoIn.dumpCount;
			info.ignore = infoIn.ignore;
			return;
		}
	}
	assertInfoVectorRef->pushBack(infoIn);
}

bool DebugHelp::shouldIgnoreAssert(const char *file, SizeType line)
{
	AssertInfo info(file, line);
	fetchAssertInfo(info);
	return info.ignore;
}

bool DebugHelp::assertNoBreak(const char *predicate, const char *assertFile, uint32_t line)
{
	AssertInfo info(assertFile, line);
	fetchAssertInfo(info);
	if (info.ignore)
		return false;

	MutexGuard guard(DebugHelpImpl::getMutex());

	if (settings.printInsteadOfAssert)
	{
		assertPrint(predicate, assertFile, line);

		/* Dump at most twice if printInsteadOfAssert is active */
		PathString finalDumpFilename;
		if (settings.assertDumpsEnabled && !isDebuggerPresent() && info.dumpCount < 2)
		{
			++info.dumpCount;
			setAssertInfo(info);
			CacheHeapString<16 * 1024> dumpMessage;
			DebugHelpImpl::createMinidumpForAssert(dumpMessage, finalDumpFilename);
		}
		if (!DebugHelpImpl::getAssertTriggeredFuncs().isEmpty())
		{
			TempString message;
			message.doSprintf("Assertion failed: %s (%s, %d)%s", predicate, assertFile, line, FB_PLATFORM_LF);
			if (!finalDumpFilename.isEmpty())
				message << "Minidump: " << finalDumpFilename << FB_PLATFORM_LF;

			message << FB_PLATFORM_LF;
			DebugHelp::dumpStackTrace(message);

			for (AssertTriggeredFuncType *func : DebugHelpImpl::getAssertTriggeredFuncs())
				(*func)(message, finalDumpFilename, info.ignore);
		}

		return false;
	}

	CacheHeapString<16 * 1024> assertDialogTextUTF8;

	if (useShortInfo())
	{
		assertDialogTextUTF8 << predicate << FB_PLATFORM_LF;
		assertDialogTextUTF8 << assertFile << ":" << line << FB_PLATFORM_LF << FB_PLATFORM_LF;
	}
	else
	{
		assertDialogTextUTF8 << "Copy & paste this to a programmer." << FB_PLATFORM_LF << FB_PLATFORM_LF;
		assertDialogTextUTF8 << "Expression: " << predicate << FB_PLATFORM_LF;
		assertDialogTextUTF8 << "File: " << assertFile << FB_PLATFORM_LF << "Line: " << line << FB_PLATFORM_LF;
	}


	PathString finalDumpFilename;
	if (!isDebuggerPresent())
	{
		/* Dump error message directly to the message string. It will all get appended */
		DebugHelpImpl::createMinidumpForAssert(assertDialogTextUTF8, finalDumpFilename);
	}

	if (useShortInfo())
	{
		ProgrammerAssertPrinting::dumpStackTraceCleaned(assertDialogTextUTF8);
	}
	else
	{
		assertDialogTextUTF8 << FB_PLATFORM_LF << "Stack trace:" << FB_PLATFORM_LF;
		DebugHelp::dumpStackTrace(assertDialogTextUTF8);
	}

	if (DebugHelp::settings.luaUnwindFunc != nullptr)
	{
		assertDialogTextUTF8 << FB_PLATFORM_LF;
		(*fb::DebugHelp::settings.luaUnwindFunc)(assertDialogTextUTF8);
	}

	for (AssertTriggeredFuncType *func : DebugHelpImpl::getAssertTriggeredFuncs())
		(*func)(assertDialogTextUTF8, finalDumpFilename, info.ignore);

	PathString dumpPath = fb::DebugHelp::copyDumpToWorkspace(finalDumpFilename, assertDialogTextUTF8);
	if (!dumpPath.isEmpty())
		assertDialogTextUTF8 << FB_PLATFORM_LF << "Dump copied to " << dumpPath;

	if (useShortInfo())
		ProgrammerAssertPrinting::print(assertDialogTextUTF8);

	/* SimpleUTF16String will hold 512 bytes. After that an allocation is made, which usually isn't a problem. If it
	* some day turns out to be, just preallocate during static initialization */
	DebugHelpImpl::getAssertDialogText().clear();
	string::UnicodeConverter::addUTF8StrToUTF16String(assertDialogTextUTF8.getPointer(), DebugHelpImpl::getAssertDialogText(), string::UnicodeConverter::UseReplacementChar);

	/* Make sure cursor is visible */
	ShowCursor(TRUE);

#if (FB_NO_CUSTOM_ASSERT_DIALOG == FB_TRUE)
	INT_PTR code = -1;
	ignoreAll = true;
#else
	INT_PTR code = DialogBoxW((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCEW(IDD_ASSERT_DIALOG), DebugHelpImpl::getMainHWND(), DebugHelpImpl::assertDialogProc);
#endif

	if (code <= 0)
	{
		/* Failsafe if dialog failed to open */
		switch (MessageBoxW(0, DebugHelpImpl::getAssertDialogText().getPointer(), L"Assertion failed.", MB_ABORTRETRYIGNORE | MB_ICONERROR))
		{
		case IDRETRY:
			return true;

		case IDABORT:
			TerminateProcess((HANDLE)-1, 0);
		}
	}
	else
	{
		if (code == IDC_IGNOREALL)
			info.ignore = true;
		else if (code == IDC_DEBUG)
			return true;
		else if (code == IDC_EXIT)
			TerminateProcess(GetCurrentProcess(), 0);
	}

	if (info.ignore)
		setAssertInfo(info);

	ShowCursor(FALSE);

	return false;
}

bool DebugHelp::assertFNoBreak(const char *predicate, const char *file, uint32_t line, const char* formatStr, ...)
{
	AssertInfo info(file, line);
	fetchAssertInfo(info);
	if (info.ignore)
		return false;

	static const uint32_t bufferSize = 4096;
	char buffer[bufferSize];
	buffer[0] = { '\0' };
	va_list arguments;
	va_start(arguments, formatStr);
	int result = vsnprintf(buffer, bufferSize, formatStr, arguments);
	va_end(arguments);
	if (result < 0)
	{
		const char failMessage[] = "Printing formatted assert failed";
		lang::MemCopy::copy(buffer, failMessage, sizeof(failMessage));
	}

	LargeTempString predicateFaked;
	if (useShortInfo())
		predicateFaked << "Expr: (" << predicate << ")" FB_PLATFORM_LF << buffer;
	else
		predicateFaked << predicate << FB_PLATFORM_LF << "Expression printfed: " << buffer;

	return assertNoBreak(predicateFaked.getPointer(), file, line);
}

void DebugHelp::assertPrint(const char *predicateString, const char *file, uint32_t line)
{
	FB_PRINTF("Assertion failed: %s (%s, %d)%s", predicateString, file, line, FB_PLATFORM_LF);
	if (DebugHelp::settings.printToLogInsteadOfAssert)
	{
		CacheHeapString<4 * 1024> errorMessage;
		errorMessage.doSprintf("Assertion failed: %s (%s, %d)%s", predicateString, file, line, FB_PLATFORM_LF);
		DebugHelp::dumpStackTrace(errorMessage);
		FB_ASSERT_PRINTF("%s", errorMessage.getPointer());
		/* Actually log to file too */
		assertLogF("%s", errorMessage.getPointer());
	}
}

void DebugHelp::checkStackAllocation()
{
#if FB_BUILD != FB_FINAL_RELEASE
	thread_local bool printing = false;
	if (printing)
		return;

	char thingOnStack = '\0';
	thread_local char *firstSeenPointer = &thingOnStack;
	thread_local intptr_t stackSize = 0;
	/* Presume stack grows from top to bottom, as it does on Windows */
	intptr_t newSize = (firstSeenPointer - &thingOnStack) >> 10;
	if (newSize <= stackSize)
		return;

	/* This happens during static initialization */
	if (!Thread::hasCurrentThread())
		return;

	printing = true;
	stackSize = newSize;
	FB_PRINTF("Largest stack size for thread %s is %d kB\n", Thread::getCurrentThread().getName().getPointer(), uint32_t(newSize));
	printing = false;
#endif
}

void DebugHelp::registerAssertTriggeredFunc(AssertTriggeredFuncType *assertTriggeredFunc)
{
	MutexGuard guard(DebugHelpImpl::getMutex());
	DebugHelpImpl::getAssertTriggeredFuncs().pushBack(assertTriggeredFunc);
}

bool DebugHelp::isDebuggerPresent()
{
	BOOL remoteDebuggerPresent = FALSE;
	CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDebuggerPresent);
	return remoteDebuggerPresent || IsDebuggerPresent();
}

const DynamicString &DebugHelp::getExePathAndName(bool noErrors)
{
	static StaticString path;
	if (!path.isEmpty())
		return path;

	wchar_t fileNameBuffer[MAX_PATH] = { '\0' };
	DWORD filenameLength = GetModuleFileNameW(nullptr, fileNameBuffer, MAX_PATH);
	if (filenameLength == 0)
	{
		if (!noErrors)
		{
			TempString msg("Failed to get executable name: ");
			addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
		}
	}
	else if (filenameLength == MAX_PATH)
	{
		fb_assert(GetLastError() == ERROR_INSUFFICIENT_BUFFER && "GetModuleFileNameW() API seems to have changed. Probably worth checking it out");
		/* We ran out of space */
		wchar_t *dynamicFileNameBuffer = new wchar_t[65500];
		filenameLength = GetModuleFileNameW(nullptr, dynamicFileNameBuffer, 65500);
		if (filenameLength > 0)
		{
			/* It must be ok */
			TempString tmpPath;
			string::UnicodeConverter::addUTF16StrToUTF8String(dynamicFileNameBuffer, tmpPath);
			path = StaticString(tmpPath);
		}
		else
		{
			if (!noErrors)
			{
				TempString msg("Failed to get executable name: ");
				addGetLastErrorCode(msg);
				FB_LOG_ERROR(msg);
			}
		}
	}
	else
	{
		/* All well */
		TempString tmpPath;
		string::UnicodeConverter::addUTF16StrToUTF8String(fileNameBuffer, tmpPath);
		/* Convert to standard notation, so file name helper work with it too */
		tmpPath.replace("\\", "/");
		path = StaticString(tmpPath);
	}
	return path;
}

/* Writes minidump. Returns true on success. Related message (error message, if false was returned) and dump file
 * name are returned in respective variables.
 * NOTE: This may not work. See createMinidumpForAssert for an example of difficulties to minidump */
static bool writeDump(struct _EXCEPTION_POINTERS *exceptionPointers, HeapString &errorMessage, HeapString &finalDumpFilename)
{
	bool success = false;
	__try
	{
		throw - 1;
	}
	__except (success = DebugHelpImpl::launchWriteDump(GetExceptionInformation(), errorMessage, finalDumpFilename))
	{
	}
	return success;
}

void DebugHelp::setSilentErrorHandling(bool silentMode, bool evenWithDebugger)
{
	if (silentMode && !evenWithDebugger && isDebuggerPresent())
		return;

	if (silentMode)
		SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	else
		SetErrorMode(0);
}

void DebugHelp::showMessageBox(const StringRef &title, const StringRef &message)
{
	MessageBoxA(0, message.getPointer(), title.getPointer(), MB_OK | MB_ICONERROR);
}

static void launchCopy(const StringRef &source, const StringRef &dest)
{
	// make sure source file exists
	if (GetFileAttributes(source.getPointer()) == INVALID_FILE_ATTRIBUTES)
		return;

	TempString parameters = "tools\\dump_copy.exe ";
	parameters << "\"" << source << "\" \"" << dest << "\"";
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION process = { 0 };
	if (!CreateProcessA(NULL, (LPSTR)parameters.getPointer(), 0, 0, FALSE, 0, 0, 0, &startupInfo, &process))
	{
		// copy manually if dump_copy.exe was not found
		CopyFile(source.getPointer(), dest.getPointer(), false);
	}
}

HeapString DebugHelp::copyDumpToWorkspace(const StringRef &finalDumpFilename, const StringRef &additionalLogMessage)
{
	/* Yes, do not try to create dumps to workspace in public editor!
	 * Though in development environment this would be actually very useful, but for users not that much. */
#if FB_EDITOR_PUBLIC_ENABLED == FB_FALSE
	if (!DebugHelp::settings.dumpToWorkspace)
		return HeapString();

	if (finalDumpFilename.isEmpty())
		return HeapString();

	/* No copying when debugger is attached */
	if (isDebuggerPresent())
		return HeapString();

	char filename[MAX_PATH] = { 0 };
	char drive[_MAX_DRIVE] = { 0 };
	char dir[_MAX_DIR] = { 0 };
	char fname[_MAX_FNAME] = { 0 };
	char ext[_MAX_EXT] = { 0 };
	DWORD filenameLength = GetModuleFileName(nullptr, filename, MAX_PATH);
	_splitpath_s(filename, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	/* Try to dump to Workspace too */
	PathString sharePath("\\\\fileshare\\dumps");
	if (GetFileAttributes(sharePath.getPointer()) == INVALID_FILE_ATTRIBUTES)
		return HeapString();

	/* Create project folder */
	sharePath << "\\" << fname;
	BOOL dirCreateSuccess = CreateDirectory(sharePath.getPointer(), nullptr);

	/* Make sure we have unique path, use id */
	static int dumpID = 1;

	SystemTime timeStamp = SystemTime::now();
	TempString dateString;
	TempString timeString;
	timeStamp.addDateOnlyFileTimeStampToString(dateString);
	timeStamp.addTimeOnlyFileTimeStampToString(timeString);
	timeString << "__" << dumpID;

	/* Increment for next time */
	dumpID++;

	/* Create date folder */
	sharePath << "\\" << dateString;
	dirCreateSuccess = CreateDirectory(sharePath.getPointer(), nullptr);

	/* Create datestring__timestamp__id__computername folder */
	sharePath << "\\" << dateString << "__" << timeString << "__" << getenv("COMPUTERNAME");
	PathString dumpPath = sharePath;
	dirCreateSuccess = CreateDirectory(dumpPath.getPointer(), nullptr);

	/* Create dump file */
	{
		PathString dest;
		dest << sharePath << "\\" << fname << ".dmp";
		launchCopy(finalDumpFilename, dest);
	}

	/* Copy log file */
	{
		PathString dest;
		dest << sharePath << "\\";
		dest.appendFileNameFromString(GlobalLogger::getInstance().getLogFile());
		launchCopy(GlobalLogger::getInstance().getLogFile(), dest);
	}

	/* Copy exe and pdb files */
	/* Just copy the 64 bit ones since 99.99 % of the time we are using the 64 bit editor */
	{
		PathString src;
		PathString dest;
		src << fname << ".exe";
		dest << sharePath << "\\" << fname << ".exe";
		launchCopy(src, dest);

		// copying PDBs should no longer be necessary
		/*
		src.clear();
		dest.clear();
		src << fname << ".pdb";
		dest << sharePath << "\\" << fname << ".pdb";
		launchCopy(src, dest);

		src.clear();
		dest.clear();
		src << "dev\\pdb\\win64_release\\" << fname << ".pdb";
		dest << sharePath << "\\" << fname << ".pdb";
		launchCopy(src, dest);

		src.clear();
		dest.clear();
		src << "dev\\pdb\\win64_release\\csharpuiwrapper64.pdb";
		dest << sharePath << "\\csharpuiwrapper64.pdb";
		launchCopy(src, dest);

		src.clear();
		dest.clear();
		src << "dev\\pdb\\win64_release\\csharpui.pdb";
		dest << sharePath << "\\csharpui.pdb";
		launchCopy(src, dest);
		*/
	}

	/* Write additional log message */
	if (!additionalLogMessage.isEmpty())
	{
		PathString src;
		src << "log/dump_" << dateString << "__" << timeString << ".log";
		FILE *f = fopen(src.getPointer(), "wt");
		if (f)
		{
			fputs(additionalLogMessage.getPointer(), f);
			fclose(f);

			PathString dest;
			dest << sharePath << "\\dump.log";
			launchCopy(src, dest);
		}
	}

	return dumpPath;
#else
	return HeapString();
#endif
}

void *DebugHelp::loadDebugHelpDLL(HeapString &errorMessage)
{
	static Mutex mutex;
	MutexGuard guard(mutex);
	static HMODULE module = nullptr;
	if (module != nullptr)
		return module;

	#if FB_PLATFORM_BITS == 64 && FB_BUILD != FB_FINAL_RELEASE
		/* Prefer our local copy in SVN */
		#if FB_VS2017_IN_USE == FB_TRUE
			const StringRef debugHelpDLLPath("tools\\dbghelp_x64\\2017\\dbghelp.dll");
		#else
			const StringRef debugHelpDLLPath("tools\\dbghelp_x64\\dbghelp.dll");
		#endif
		module = LoadLibraryA(debugHelpDLLPath.getPointer());
		if (module == nullptr)
			errorMessage << "Failed to load dbghelp.dll from " << debugHelpDLLPath << ". Will try with plain name next" << FB_PLATFORM_LF;
		else
			return module;
	#endif
	const StringRef debugHelpDLLPlainName("DbgHelp.dll");
	module = LoadLibraryA(debugHelpDLLPlainName.getPointer());
	#if FB_PLATFORM_BITS == 64 && FB_BUILD != FB_FINAL_RELEASE
		if (module != nullptr)
			errorMessage << "Success with " << debugHelpDLLPlainName << FB_PLATFORM_LF;
	#endif
	if (module == nullptr)
		errorMessage << "Failed to load " << debugHelpDLLPlainName << FB_PLATFORM_LF;

	return module;
}

void DebugHelp::waitForPDBCopy()
{
	DebugHelpImpl::getPDBCopyMutex().enter();
	DebugHelpImpl::getPDBCopyMutex().leave();

	// let's extract manually because dbghelp sucks at doing this
	DebugHelpImpl::extractPDBArchive();
}

bool DebugHelp::addGetLastErrorCode(HeapString & outMessage)
{
	DWORD errorCode = GetLastError();
	SetLastError(ERROR_SUCCESS);
	if (errorCode == ERROR_SUCCESS)
		return false;

	const DWORD bufferSize = 512;
	wchar_t messageBuffer[bufferSize + 1] = { 0 };
	DWORD size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &(messageBuffer[0]), bufferSize, nullptr);

	outMessage << "ErrorCode: " << uint32_t(errorCode) << ", ";
	string::UnicodeConverter::addUTF16StrToUTF8String(messageBuffer, outMessage);
	/* Some errors seem to have linefeed at the end. Some have that and . */
	while (outMessage.doesEndWith("\n") || outMessage.doesEndWith("\r") || outMessage.doesEndWith("."))
		outMessage.trimRight(1);

	return true;
}

#if FB_BUILD != FB_FINAL_RELEASE

bool DebugHelp::addWhoHoldsFileOpenInfo(HeapString &outMessage, const StringRef &fileName)
{
	/* Base code from [1] https://blogs.msdn.microsoft.com/oldnewthing/20120217-00/?p=8283/ */
	DWORD session = 0;
	/* This buffer must be filled with zeroes, as string returned is not null terminated (according to [1]) */
	WCHAR sessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
	DWORD errorCode = RmStartSession(&session, 0, sessionKey);
	if (errorCode != ERROR_SUCCESS)
	{
		FB_LOG_ERROR(FB_MSG("RmStartSession returned ", uint64_t(errorCode), ". See https://docs.microsoft.com/en-us/windows/desktop/api/RestartManager/nf-restartmanager-rmstartsession"));
		return false;
	}
	string::SimpleUTF16String fileNameWide;
	string::UnicodeConverter::addUTF8StrToUTF16String(fileName, fileNameWide);
	PCWSTR fileList = fileNameWide.getPointer();
	errorCode = RmRegisterResources(session, 1, &fileList, 0, NULL, 0, NULL);
	if (errorCode != ERROR_SUCCESS)
	{
		FB_LOG_ERROR(FB_MSG("RmRegisterResources returned ", uint64_t(errorCode), " for file ", fileName, ". See https://docs.microsoft.com/en-us/windows/desktop/api/restartmanager/nf-restartmanager-rmregisterresources"));
		return false;
	}

	DWORD reason = 0;
	UINT i = 0;
	UINT procInfoNeeded = 0;
	const UINT maxNumProcInfo = 10;
	UINT numProcInfo = maxNumProcInfo;
	RM_PROCESS_INFO rgpi[maxNumProcInfo];
	errorCode = RmGetList(session, &procInfoNeeded, &numProcInfo, rgpi, &reason);
	if (errorCode != ERROR_SUCCESS && errorCode != ERROR_MORE_DATA)
	{
		FB_LOG_ERROR(FB_MSG("RmGetList returned ", uint64_t(errorCode), " for file ", fileName, ". See https://docs.microsoft.com/en-us/windows/desktop/api/restartmanager/nf-restartmanager-rmgetlist"));
		return false;
	}
	for (i = 0; i < numProcInfo; i++)
	{
		string::UnicodeConverter::addUTF16StrToUTF8String(rgpi[i].strAppName, outMessage);
		outMessage << " (";
		HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, rgpi[i].Process.dwProcessId);
		if (processHandle)
		{
			/* Compare process creation time in addition to pid, since pid can be recycled */
			FILETIME createTime;
			FILETIME exitTime;
			FILETIME kernelTime;
			FILETIME userTime;
			if (GetProcessTimes(processHandle, &createTime, &exitTime, &kernelTime, &userTime) && CompareFileTime(&rgpi[i].Process.ProcessStartTime, &createTime) == 0)
			{
				WCHAR sz[MAX_PATH];
				DWORD cch = MAX_PATH;
				if (QueryFullProcessImageNameW(processHandle, 0, sz, &cch) && cch <= MAX_PATH)
				{
					string::UnicodeConverter::addUTF16StrToUTF8String(sz, outMessage);
					DWORD exitCode = 0xFACE;
					if (GetExitCodeProcess(processHandle, &exitCode))
					{
						if (exitCode == STILL_ACTIVE)
							outMessage << " (still running)";
						else
							outMessage << " (exited with code " << int64_t(exitCode) << ")";
					}
					else
					{
						FB_LOG_ERROR("Could not get exit code for a process");
					}
					outMessage << ", ";
				}
			}
			CloseHandle(processHandle);
		}
		outMessage << "pid: " << uint64_t(rgpi[i].Process.dwProcessId) << "), ";
	}
	if (errorCode == ERROR_MORE_DATA)
		outMessage << (procInfoNeeded - maxNumProcInfo) << " more not listed";
	else if (procInfoNeeded > 0)
		outMessage.trimRight(2);
	else
	{
		outMessage << "No process seems to be holding on to file " << fileName;
	}

	RmEndSession(session);
	return true;
}


#endif

FB_END_PACKAGE0()
