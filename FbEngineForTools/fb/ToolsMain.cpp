#include "Precompiled.h"
#include "ToolsMain.h"

#include "fb/file/logger/Logger.h"
#include "fb/file/SpecialDirectory.h"
#include "fb/lang/ProjectInfo.h"
#include "fb/memory/stats/DebugStats.h"
#include "fb/task/Scheduler.h"

FB_PACKAGE1(toolsengine)

ToolsMain::ToolsMain(const StringRef &appId, int argc, const char *args[], uint32_t schedulerOverrideProcessorCount)
	: appId(appId)
{
	DebugHelp::settings.printInsteadOfAssert = !DebugHelp::isDebuggerPresent();
	DebugHelp::settings.dumpToWorkspace = false;
	DebugHelp::initialize();

	lang::PrintfHandler::getPrintfHandler().addCharacterOutputReceiver(&consoleOutput);

	/* ProjectInfo initialization */
	uint32_t majorVersion = 1;
	uint32_t minorVersion = 0;
	uint32_t hotfixVersion = 0;
	uint32_t baseNetVersion = 1;
	uint32_t netGameID = FB_FOURCC('T','o','o','l');
	uint32_t buildNumber = 1;
	uint32_t sourceRevisionNumber = 1;
	/* Some suggestions */
	//SmallTempString versionStringPostfix("dev");
	//SmallTempString versionStringPostfix("alpha");
	//SmallTempString versionStringPostfix("beta");
	SmallTempString versionStringPostfix;

	/* Should be false for all new projects */
	bool useLegacyNumericalversionGeneration = false;

	lang::ProjectInfo::initializeVersionInfo(majorVersion, minorVersion, hotfixVersion, baseNetVersion, netGameID, buildNumber,
		sourceRevisionNumber, versionStringPostfix, useLegacyNumericalversionGeneration);

	const StringRef applicationName("Unnamed Tool");
	const StringRef applicationFileName("unnamedtool");
	const StringRef applicationFileNameUpperCase("UnnamedTool");
	const StringRef gameSourceDateString("N/A");
	lang::ProjectInfo::initializeNameInfo(applicationName, applicationFileName, applicationFileNameUpperCase, gameSourceDateString);

	const StringRef compatibleVersionsString("");
	lang::ProjectInfo::initializeCompatibleVersionsList(compatibleVersionsString);
	/* Uncomment to check info */
	//lang::ProjectInfo::debugPrintAll();

	/* Misc stuff */
	TempString logFileName(file::SpecialDirectory::getApplicationDataDirectory());
	logFileName << appId << "/" << appId << ".log";
	scheduler.reset(new task::Scheduler(task::Scheduler::InitFlagUseAllLogicalProcessors, schedulerOverrideProcessorCount));
	getGlobalMegaton().set(scheduler.get());
	logger.reset(new file::Logger(logFileName, *scheduler));
	GlobalLogger::getInstance().addLogger(logger.get());
	for (int i = 0; i < argc; ++i)
	{
		fb_assert(args[i] != nullptr && "Null arguments WTF?");
		commandLine.pushBack(StaticString::createFromConstChar(args[i]));
	}
}

ToolsMain::~ToolsMain()
{
	GlobalLogger::getInstance().removeLogger(logger.get());
	logger.reset();
	getGlobalMegaton().unset(scheduler.get());
	scheduler.reset();
	lang::PrintfHandler::getPrintfHandler().removeCharacterOutputReceiver(&consoleOutput);
	memory::stats::DebugStats::cleanInstance();
}

ToolsMain *&getInstancePtr()
{
	static ToolsMain *ptr = nullptr;
	return ptr;
}

ToolsMain &ToolsMain::initialize(const StringRef &appId, int argc, const char *args[], uint32_t schedulerOverrideProcessorCount)
{
	static ToolsMain toolsMain(appId, argc, args, schedulerOverrideProcessorCount);
	getInstancePtr() = &toolsMain;
	FB_LOG_INFO("ToolsMain initialization complete");
	return getInstance();
}

ToolsMain &ToolsMain::getInstance()
{
	fb_assert(getInstancePtr() != nullptr && "Instance not yet initialized. Must call initialize before calling getInstance()");
	return *getInstancePtr();
}


const Vector<StaticString> &ToolsMain::getCommandLine() const
{
	return commandLine;
}

FB_END_PACKAGE1()
