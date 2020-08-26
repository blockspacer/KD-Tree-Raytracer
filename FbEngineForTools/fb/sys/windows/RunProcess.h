#pragma once

#include "fb/lang/time/Time.h"
#include "fb/lang/Pimpl.h"

FB_DECLARE0(HeapString)
FB_DECLARE_TEMPLATED_STRUCT0(Vector)

FB_PACKAGE2(sys, windows)

class ProcessRunner
{
public:
	ProcessRunner();
	/* Inheriting handles is usually a bad idea. The new process will get e.g. copies of open file handles, preventing 
	 * deleting the files. However, it is (probably) necessary for redirecting input and output. 
	 * FIXME: Fix the mess and never inherit handles. */
	ProcessRunner(const StringRef &binary, const StringRef &parameters, bool inheritHandles);
	~ProcessRunner();

	bool start(const StringRef &binary, const StringRef &parameters, bool inheritHandles);
	bool start();
	/* Reads stuff from pipes (to prevent processes from getting stuck with full buffers) and checks whether process is 
	 * still running. Returns true if still running, false if finished (or not yet started). */
	bool update();
	/* Wait for process to finish. Returns true, unless something mysterious went wrong with, given the timeout 
	 * parameter larger than zero, will wait at least that time and then return false, if not ready */
	bool wait(Time timeout = Time::zero);
	/* Returns true if process is still running */
	bool isRunning() const;
	/* Kills process, if it is still running. Return true, if successful, false if killing failed */
	bool kill();
	int64_t getReturnValue() const;
	const Vector<HeapString>& getOutput() const;

private:
	bool updateImpl(bool waitMode);

	FB_PIMPL
};

/// Run the given process. Doesn't wait for it to end
bool runProcess(const char *binary, const char *parameters, bool inheritHandles);

/// Run the given process. Wait for the process to end.
/* Implementation is somewhat broken. Won't wait windowed processes. This also always inherits handles, as the 
 * convoluted error checking doesn't otherwise work. 
 * FIXME: Use ProcessRunner (and saner error checking rules) instead. */
bool runProcessWait(const char *binary, const char *parameters);

FB_END_PACKAGE2()
