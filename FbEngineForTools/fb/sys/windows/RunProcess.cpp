#include "Precompiled.h"

#include "RunProcess.h"

#include "fb/lang/CallStack.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/ScopedArray.h"
#include "fb/lang/thread/Thread.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/string/HeapString.h"

FB_PACKAGE2(sys, windows)

class ProcessRunner::Impl
{
public:
	Impl()
	{
	}

	Impl(const StringRef &binary, const StringRef &parameters, bool inheritHandles)
		: binary(binary)
		, parameters(parameters)
		, inheritHandles(inheritHandles)
	{
	}

	HeapString binary;
	HeapString parameters;

	PROCESS_INFORMATION processInfo;
	HANDLE stderrReadHandle = nullptr;
	HANDLE stdoutReadHandle = nullptr;
	HANDLE stdinReadHandle = nullptr;
	HANDLE stderrWriteHandle = nullptr;
	HANDLE stdoutWriteHandle = nullptr;
	HANDLE stdinWriteHandle = nullptr;

	uint32_t bufferSize = 128 * 1024;

	Vector<HeapString> output;
	HeapString tmpOutLog;
	HeapString tmpErrLog;
	int64_t returnValue = 0;

	bool running = false;
	bool inheritHandles = false;
};

ProcessRunner::ProcessRunner()
	: impl(new Impl)
{
	
}


ProcessRunner::ProcessRunner(const StringRef &binary, const StringRef &parameters, bool inheritHandles)
	: impl(new Impl(binary, parameters, inheritHandles))
{
}


ProcessRunner::~ProcessRunner()
{
	CloseHandle(impl->stderrReadHandle);
	CloseHandle(impl->stdoutReadHandle);
	CloseHandle(impl->stdinReadHandle);
	CloseHandle(impl->stderrWriteHandle);
	CloseHandle(impl->stdoutWriteHandle);
	CloseHandle(impl->stdinWriteHandle);
	CloseHandle(impl->processInfo.hProcess);
	CloseHandle(impl->processInfo.hThread);
}


bool ProcessRunner::start(const StringRef &binaryParam, const StringRef &params, bool inheritHandles)
{
	impl->binary = binaryParam;
	impl->parameters = params;
	impl->inheritHandles = inheritHandles;
	return start();
}


#define FB_PR_PRINT_TO_OUTPUT(...) \
	FB_PRINTF(__VA_ARGS__); \
	impl->output.pushBack(); \
	impl->output.getBack().doSprintf(__VA_ARGS__);

#define FB_PR_HANDLE_GETLASTERROR(p_msg) \
	{ \
		DWORD errorCode = GetLastError(); \
		TempString msg(p_msg); \
		msg << int64_t(errorCode); \
		FB_PR_PRINT_TO_OUTPUT(msg.getPointer()); \
	}

bool ProcessRunner::start()
{
	if (impl->running)
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - alredy running");
		return false;
	}

	/* See https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx */

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&impl->stdoutReadHandle, &impl->stdoutWriteHandle, &sa, impl->bufferSize))
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - Failed to create output pipe");
		return false;
	}
	if (!SetHandleInformation(impl->stdoutReadHandle, HANDLE_FLAG_INHERIT, 0))
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - Failed to set output pipe inherit flag");
		return false;
	}

	if (!CreatePipe(&impl->stderrReadHandle, &impl->stderrWriteHandle, &sa, impl->bufferSize))
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - Failed to create error pipe");
		return false;
	}

	if (!SetHandleInformation(impl->stderrReadHandle, HANDLE_FLAG_INHERIT, 0))
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - Failed to set error pipe inherit flag");
		return false;
	}

	if (!CreatePipe(&impl->stdinReadHandle, &impl->stdinWriteHandle, &sa, impl->bufferSize))
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - Failed to create input pipe");
		return false;
	}

	if (!SetHandleInformation(impl->stdinWriteHandle, HANDLE_FLAG_INHERIT, 0))
	{
		FB_PR_PRINT_TO_OUTPUT("ProcessRunner::start() - Failed to set input pipe inherit flag");
		return false;
	}

	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_HIDE;

	if (impl->inheritHandles)
	{
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		startupInfo.hStdOutput = impl->stdoutWriteHandle;
		startupInfo.hStdError = impl->stderrWriteHandle;
		startupInfo.hStdInput = impl->stdinReadHandle;
	}

	ZeroMemory(&impl->processInfo, sizeof(PROCESS_INFORMATION));

	TempString cmdLine;
	cmdLine << "\"" << impl->binary << "\" " << impl->parameters;
	/* CreateProcessA's parameters aren't const, so we'll create a temp buffer */
	ScopedArray<char> cmdLineBuffer(new char[cmdLine.getLength() + 1]);
	strncpy(&cmdLineBuffer[0], cmdLine.getPointer(), cmdLine.getLength() + 1);
	if (CreateProcessA(nullptr, cmdLineBuffer.get(), nullptr, nullptr, impl->inheritHandles, 0, nullptr, nullptr, &startupInfo, &impl->processInfo) == 0)
	{
		TempString msg("Creating process failed. ");
		DebugHelp::addGetLastErrorCode(msg);
		FB_PR_PRINT_TO_OUTPUT("%s", msg.getPointer());
		return false;
	}

	impl->running = true;
	return true;
}


bool ProcessRunner::update()
{
	return updateImpl(false);
}


bool ProcessRunner::updateImpl(bool waitMode)
{
	if (!impl->running)
		return false;

	ScopedTimer timer;

	DWORD exitCode = 0;
	if (!GetExitCodeProcess(impl->processInfo.hProcess, &exitCode))
	{
		FB_PR_HANDLE_GETLASTERROR("ProcessRunner::update() - Checking process exit code failed. Error was: ");
		exitCode = 0xFFFF;
	}

	if (impl->inheritHandles)
	{
		DWORD numOutputBytes = 0;
		DWORD numErrorBytes = 0;
		if (!PeekNamedPipe(impl->stdoutReadHandle, nullptr, 0, nullptr, &numOutputBytes, nullptr))
		{
			FB_PR_HANDLE_GETLASTERROR("ProcessRunner::update() - Peeking output pipe failed. Error was: ");
		}
		if (!PeekNamedPipe(impl->stderrReadHandle, nullptr, 0, nullptr, &numErrorBytes, nullptr))
		{
			FB_PR_HANDLE_GETLASTERROR("ProcessRunner::update() - Peeking error pipe failed. Error was: ");
		}

		while (numOutputBytes > 0)
		{
			char buffer[4096] = { 0 };
			DWORD numBytesToRead = lang::min(numOutputBytes, DWORD(4095));
			numOutputBytes -= numBytesToRead;
			DWORD numBytesRead = 0;
			if (!ReadFile(impl->stdoutReadHandle, buffer, 4095, &numBytesRead, nullptr))
			{
				FB_PR_HANDLE_GETLASTERROR("ProcessRunner::update() - Reading output pipe failed. Error was: ");
			}
			impl->tmpOutLog.append(&buffer[0], numBytesRead);
		}
		while (numErrorBytes > 0)
		{
			char buffer[4096] = { 0 };
			DWORD numBytesToRead = lang::min(numErrorBytes, DWORD(4095));
			numErrorBytes -= numBytesToRead;
			DWORD numBytesRead = 0;
			if (!ReadFile(impl->stderrReadHandle, buffer, numBytesToRead, &numBytesRead, nullptr))
			{
				FB_PR_HANDLE_GETLASTERROR("ProcessRunner::update() - Reading error pipe failed. Error was: ");
			}
			impl->tmpErrLog.append(&buffer[0], numBytesRead);
		}

		while (!impl->tmpErrLog.isEmpty())
		{
			const char *start = impl->tmpErrLog.getPointer();
			uint32_t numToSkip = 2;
			const char *ending = strstr(start, "\r\n");
			if (!ending)
			{
				numToSkip = 1;
				ending = strstr(start, "\n");
			}

			if (!ending)
				break;

			impl->output.pushBack(HeapString(start, SizeType(ending - start)));
			impl->tmpErrLog.erase(0, (SizeType)(ending - start) + numToSkip);
		}

		while (!impl->tmpOutLog.isEmpty())
		{
			const char *start = impl->tmpOutLog.getPointer();
			uint32_t numToSkip = 2;
			const char *ending = strstr(start, "\r\n");
			if (!ending)
			{
				numToSkip = 1;
				ending = strstr(start, "\n");
			}

			if (!ending)
				break;

			impl->output.pushBack(HeapString(start, SizeType(ending - start)));
			impl->tmpOutLog.erase(0, (SizeType)(ending - start) + numToSkip);
		}
	}

	/* Check the exit code we got earlier */
	if (exitCode != STILL_ACTIVE)
	{
		impl->returnValue = exitCode;
		impl->running = false;
	}

	if (waitMode && timer.getMilliseconds() < 1)
	{
		/* This isn't pretty or efficient, but better ways require using asynchronous input, which is pretty heavy for 
		 * current use case (MapBuilder). See ReadFileEx, if interested. */
		Thread::sleep(100);
	}

	return impl->running;
}


bool ProcessRunner::wait(Time timeout)
{
	ScopedTimer timer;
	while (impl->running)
	{
		updateImpl(true);
		if (timeout > Time::zero)
		{
			if (timer.getMilliseconds() > uint64_t(timeout.getMilliseconds()))
				break;
		}
	}
	return !impl->running;
}


bool ProcessRunner::isRunning() const
{
	return impl->running;
}


bool ProcessRunner::kill()
{
	if (!impl->running)
		return true;

	if (TerminateProcess(impl->processInfo.hProcess, 0xFFFF))
	{
		wait(Time::fromSeconds(1));
		return !impl->running;
	}
	TempString msg("ProcessRunner failed to kill a process. ");
	DebugHelp::addGetLastErrorCode(msg);
	FB_PR_PRINT_TO_OUTPUT("%s", msg.getPointer());
	return false;
}


int64_t ProcessRunner::getReturnValue() const
{
	return impl->returnValue;
}


const Vector<HeapString>& ProcessRunner::getOutput() const
{
	return impl->output;
}


namespace {

	bool runProcessImp(const char *binary, const char *parameters)
	{
		FB_STACK_FUNC();

		SECURITY_ATTRIBUTES sa;
		memset(&sa, 0, sizeof(sa));
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		HANDLE hOutputReadTmp, hOutputWrite;
		if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 32*1024))
		{
			FB_PRINTF("runProcessImp - Failed to create output pipe");
			return false;
		}

		HANDLE hErrorWrite;
		if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
		{
			FB_PRINTF("runProcessImp - Failed to duplicate error write handle");
			return false;
		}

		HANDLE hOutputRead;
		if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			FB_PRINTF("runProcessImp - Failed to duplicate output read handle");
			return false;
		}

		HANDLE hInputRead, hInputWriteTmp;
		if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096))
		{
			FB_PRINTF("runProcessImp - Failed to create input pipe");
			return false;
		}

		HANDLE hInputWrite;
		if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			FB_PRINTF("runProcessImp - Failed to duplicate input write handle");
			return false;
		}

		CloseHandle(hOutputReadTmp);
		CloseHandle(hInputWriteTmp);

		STARTUPINFO startupInfo = { 0 };
		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		startupInfo.wShowWindow = SW_HIDE;
		startupInfo.hStdOutput = hOutputWrite;
		startupInfo.hStdError = hErrorWrite;
		startupInfo.hStdInput = hInputRead;
		PROCESS_INFORMATION process = { 0 };
	
		char cmdLine[64];
		strcpy(cmdLine, "cmd /q");
		CreateProcessA(nullptr, cmdLine, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startupInfo, &process);

		char command[4096];
		DWORD bytesWritten = 0;

		sprintf(command, "\n@PROMPT $ \necho ******** BEGIN\n");
		WriteFile(hInputWrite, command, (DWORD)strlen(command), &bytesWritten, nullptr);
		
		sprintf(command, "\"%s\" %s\n", binary, parameters);
		WriteFile(hInputWrite, command, (DWORD)strlen(command), &bytesWritten, nullptr);

		sprintf(command, "\necho ******** END %%ERRORLEVEL%%\nexit\n");
		WriteFile(hInputWrite, command, (DWORD)strlen(command), &bytesWritten, nullptr);

		CloseHandle(hOutputWrite);
		CloseHandle(hInputRead);
		CloseHandle(hErrorWrite);
				
		DWORD tid = GetCurrentThreadId();
		bool buildStarted = false;
		bool buildSucceeded = false;
		bool buildComplete = false;
		bool buildHadErrors = false;

		TempString buildLog;
		while (!buildComplete)
		{
			char buffer[4096] = {0};
			DWORD nBytesRead = 0;
			if (!ReadFile(hOutputRead, buffer, 4095, &nBytesRead,nullptr) || !nBytesRead)
				break;
			buildLog << buffer;

			// find complete lines from log
			Vector<HeapString> lines;
			while (!buildLog.isEmpty())
			{
				const char *start = buildLog.getPointer();
				const char *ending = strstr(start, "\r\n");
				if (!ending)
					break;
				lines.pushBack(HeapString(start, SizeType(ending - start)));
				buildLog.erase(0, (SizeType)(ending-start) + 2);
			}

			for (SizeType i = 0; i < lines.getSize(); i++)
			{
				if (strstr(lines[i].getPointer(), "******** END ") == lines[i].getPointer()) // build complete msg
				{
					buildComplete = true;
					// exit code 0 indicates success
					buildSucceeded = (lines[i] == "******** END 0");
					if (!buildSucceeded || buildHadErrors)
					{
						FB_PRINTF("%i> Exited with code %s from command '%s %s'\n", tid, lines[i].getPointer() + strlen("******** END "), binary, parameters);
					}
					break;
				}
				else if (lines[i] == "******** BEGIN") // build started msg
				{
					buildStarted = true;
				}
				else if (buildStarted && !lines[i].isEmpty())
				{
					// let's just consider any kind of message as an error
					buildHadErrors = true;
					FB_PRINTF("%i> %s\n", tid, lines[i].getPointer());
				}
			}
		}

		CloseHandle(hOutputRead);
		CloseHandle(hInputWrite);
		
		WaitForSingleObject(process.hThread, INFINITE);
		
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);

		return buildSucceeded;
	}

} // unnamed


bool runProcess(const char *binary, const char *parameters, bool inheritHandles)
{
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION process = { 0 };
	
	BOOL result = CreateProcessA(binary, (char *) parameters, 0, 0, inheritHandles, CREATE_NO_WINDOW, 0, 0, &startupInfo, &process);
	if (!result)
		return false;

	CloseHandle(process.hProcess);
	CloseHandle(process.hThread);
	return true;
}


bool runProcessWait(const char *binary, const char *parameters)
{
	return runProcessImp(binary, parameters);
}

FB_END_PACKAGE2()
