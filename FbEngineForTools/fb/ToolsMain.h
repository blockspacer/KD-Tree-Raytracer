#pragma once

#include "ConsoleOutputReceiver.h"
#include "fb/string/StaticString.h"

FB_DECLARE(file, Logger)
FB_DECLARE(task, Scheduler)

FB_PACKAGE1(toolsengine)

/**
 * ToolsMain contains some useful basic stuff like initializing DebugHelp, Scheduler and Logger. No initialization is 
 * done automatically, so you can totally ignore this and do your own thing.
 */

class ToolsMain
{
	ToolsMain(const StringRef & appId, int argc, const char * args[], uint32_t schedulerOverrideProcessorCount);
	~ToolsMain();

public:
	/* Log will be written based on appId to %appdata%/appId/appId.log */
	static ToolsMain &initialize(const StringRef &appId, int argc = 0, const char *args[] = { nullptr }, uint32_t schedulerOverrideProcessorCount = 0);
	/* Must call initialize once before getInstance() can work properly */
	static ToolsMain &getInstance();

	/* Returns the command line arguments given in initialization phase */
	const Vector<StaticString> &getCommandLine() const;

private:
	StaticString appId;
	ScopedPointer<file::Logger> logger;
	Vector<StaticString> commandLine;
	ScopedPointer <task::Scheduler> scheduler;
	ConsoleOutputReceiver consoleOutput;
};

FB_END_PACKAGE1()
