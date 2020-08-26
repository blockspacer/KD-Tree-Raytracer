#include "Precompiled.h"
#include "Logger.h"

#include "fb/file/CreateDirectory.h"
#include "fb/file/DebugBlockWriter.h"
#include "fb/file/DoesFileExist.h"
#include "fb/file/MoveFile.h"
#include "fb/file/LineReader.h"
#include "fb/file/logger/FocusTestReportWriter.h"
#include "fb/file/OutputFile.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/time/SystemTime.h"
#include "fb/profiling/MutexProfiler.h"
#include "fb/string/HeapString.h"
#include "fb/task/Scheduler.h"

#include <assert.h>
#include <cstring>

/* All these mysterious defines should be taken a look at */
#define FB_WRITE_LOG_TO_DISK FB_FALSE
#define FB_USE_DBW_FOR_LOGGER FB_FALSE

// these asserts can't use fb_assert because it will trigger some kind of recursion?
#if (FB_ASSERT_ENABLED == FB_TRUE)
	#define fb_logger_assert(...) if (!DebugHelp::settings.printInsteadOfAssert) { assert(__VA_ARGS__); }
#else
	#define fb_logger_assert(...)
#endif

#define FB_WRITE_LOG_TO_FILE FB_TRUE

#if (FB_BUILD != FB_FINAL_RELEASE)
	#define FB_LOG_TO_STDOUT FB_TRUE
#else
	#define FB_LOG_TO_STDOUT FB_FALSE
#endif


FB_PACKAGE1(file)

static void rotateLogFiles(const StringRef &filename)
{
#if FB_EDITOR_ENABLED == FB_TRUE
	const SizeType numToRotate = 5;
	SmallTempString extension;
	extension.appendFileExtensionFromString(filename);
	SizeType totalExtensionLength = extension.isEmpty() ? 0 : (extension.getLength() + 1);
	TempString filenameWithoutExtension(filename.getPointer(), filename.getLength() - totalExtensionLength);
	for (SizeType i = numToRotate - 1; i < numToRotate; --i)
	{
		TempString fileToRename;
		if (i > 0)
			fileToRename << filenameWithoutExtension << "_old_" << i << "." << extension;
		else
			fileToRename << filename;

		if (!doesFileExist(fileToRename, true, true))
			continue;

		TempString fileToRenameTo;
		fileToRenameTo << filenameWithoutExtension << "_old_" << (i + 1) << "." << extension;

		if (!file::moveFile(fileToRename, fileToRenameTo, false))
		{
			FB_PRINTF("Could not successfully move log file %s to %s\n", fileToRename.getPointer(), fileToRenameTo.getPointer());
		}
	}
#endif
}


Logger::Logger(const StringRef &logfile, task::Scheduler &scheduler)
	: filename(logfile)
	, scheduler(scheduler)
{
	bool hasWritableFilesystem = true;

	profiling::MutexProfiler::setMutexName(&mutex, "Logger mutex");

	/* Pretty stupid two-define logic here. Originally the defines were in different files and debugBlockWriter was a 
	 * parameter, but that didn't make logic any more sensible */
#if FB_USE_DBW_FOR_LOGGER == FB_TRUE
	if (hasWritableFilesystem)
		debugBlockWriter.reset(new file::DebugBlockWriter(&scheduler, file::DebugBlockWriter::UseAutomagicalFlush));
#endif

#if FB_WRITE_LOG_TO_DISK == FB_TRUE
	if (debugBlockWriter.get() != nullptr)
	{
		if (!debugBlockWriter->open("log/fb_app_log"))
			this->debugBlockWriter.reset();
	}
#endif
}

static FILE *getFilePtr(void *ptr)
{
	return reinterpret_cast<FILE *>(ptr);
}

Logger::~Logger()
{
	/* This makes sure focus logger report background task gets properly closed. */
	createFocusTestReport(false);

#if (FB_WRITE_LOG_TO_FILE == FB_TRUE)
	if (file != nullptr)
	{
		fclose(getFilePtr(file));
		file = nullptr;
	}
#endif
}

void Logger::addTimestamp(HeapString &str)
{
	if (timestampsEnabled)
	{
		SystemTime::fromTimestamp(SystemTime::now().getAsTimestamp() + timestampDiff).addCustomTimeStampToString(str, false, false, false, true, true, true, true,
			StringRef("-"), StringRef(":"), StringRef("."), StringRef(" "));
		str << " ";
	}
}

void Logger::writeToLog(const StringRef &msg)
{
#if (FB_WRITE_LOG_TO_FILE == FB_TRUE)
	if (file == nullptr)
	{
		file::createPathIfMissing(filename);
		rotateLogFiles(filename);
		file = fopen(filename.getPointer(), "wb");
		if (file != nullptr)
		{
			SmallTempString str;
			SystemTime::now().addHumanReadableTimeStampToString(str, false);
			str << FB_LOGGER_LINEFEED;
			fprintf(getFilePtr(file), "%s", str.getPointer());
		}
	}
	if (file != nullptr)
	{
		fprintf(getFilePtr(file), FB_LOGGER_LINEFEEDED_S, msg.getPointer());
#if (FB_LOG_TO_STDOUT == FB_TRUE)
		FB_PRINTF(FB_LOGGER_LINEFEEDED_S, msg.getPointer());
#endif
		fflush(getFilePtr(file));
	}
	else
	{
		FB_PRINTF(FB_LOGGER_LINEFEEDED_S, msg.getPointer());
	}
#else

#if FB_WRITE_LOG_TO_DISK == FB_TRUE
	/* Print to SD card */
	if (debugBlockWriter != nullptr)
	{
		SizeType length = msg.getLength();
		if (length > 0)
			debugBlockWriter->write(msg);

		/* Not very pretty way to handle linefeed, but should work (Logger is thread-safe) */
		debugBlockWriter->write(StringRef(FB_LOGGER_LINEFEED, (SizeType)strlen(FB_LOGGER_LINEFEED)));
	}
#endif

	FB_PRINTF(FB_LOGGER_LINEFEEDED_S, msg.getPointer());

#endif

#if FB_DEDICATED_SERVER == FB_TRUE
	FB_PRINTF(FB_LOGGER_LINEFEEDED_S, msg.getPointer());
#endif
}

void Logger::setLogLevel(Logger::Level level)
{
	logLevel = level;
}

void Logger::setListenerLogLevel(Logger::Level level)
{
	listenerLogLevel = level;
}

Logger::Level Logger::getListenerLogLevel()
{
	return listenerLogLevel;
}

void Logger::setListener(lang::ILoggerListener *newListener)
{
	fb_assert(this->listener == nullptr || newListener == nullptr);

	this->listener = newListener;

	if (listener)
	{
		for (SizeType i = 0; i < cachedMessages.getSize(); ++i)
		{
			const Message &data = cachedMessages[i];

			if (logLevel >= data.first)
				listener->logMessage(data.second.getPointer(), data.first);
		}
	}

	cachedMessages.clear();
}

void Logger::syncListener()
{
	MutexGuard guard(mutex);

	flushingMessagesToListener = true;

	for (SizeType i = 0; i < messagesToListener.getSize(); ++i)
	{
		const Message &data = messagesToListener[i];
		listener->logMessage(data.second.getPointer(), data.first);
		logMessageSignal(data.second, data.first);
	}

	messagesToListener.clear();

	flushingMessagesToListener = false;
}

void Logger::debug(const StringRef &callerFunc, const StringRef &message)
{
	if (logLevel < Logger::LevelDebug)
		return;

	StringRef callerFunction(callerFunc);
	if (callerFunction.doesStartWith("fb::"))
		callerFunction = StringRef(callerFunction.getPointer() + 4, callerFunction.getLength() - 4);

	StringType msg;
	msg.reserve(message.getLength() + callerFunction.getLength() + 30);
	addTimestamp(msg);
	msg << "DEBUG: " << callerFunction << " - " << message;

	MutexGuard guard(mutex);
	writeToLog(msg);
	if (listener != nullptr)
	{
		if (listenerLogLevel >= Logger::LevelDebug)
		{
			if (!flushingMessagesToListener)
			{
				messagesToListener.pushBack(Message(Logger::LevelDebug, lang::move(msg)));
			}
			else
			{
				static bool no_more_assert = false;
				if (!no_more_assert)
				{
					no_more_assert = true;
					fb_logger_assert(0 && "Logger encountered a listener making a recursive call. (a log message causing the logger listener to log more messages - possible infinite recursion)");
				}
			}
		}
	}
	else
	{
		cachedMessages.pushBack(Message(Logger::LevelDebug, StringType(msg)));
	}
}

void Logger::info(const StringRef &callerFunc, const StringRef &message)
{
	if (logLevel < Logger::LevelInfo)
		return;

	StringRef callerFunction(callerFunc);
	if (callerFunction.doesStartWith("fb::"))
		callerFunction = StringRef(callerFunction.getPointer() + 4, callerFunction.getLength() - 4);

	StringType msg;
	msg.reserve(message.getLength() + callerFunction.getLength() + 30);
	addTimestamp(msg);
	msg << "INFO: " << callerFunction << " - " << message;

	MutexGuard guard(mutex);
	writeToLog(msg);
	if (listener != nullptr)
	{
		if (listenerLogLevel >= Logger::LevelInfo)
		{
			if (!flushingMessagesToListener)
			{
				messagesToListener.pushBack(Message(Logger::LevelInfo, lang::move(msg)));
			}
			else
			{
				static bool no_more_assert = false;
				if (!no_more_assert)
				{
					no_more_assert = true;
					fb_logger_assert(0 && "Logger encountered a listener making a recursive call. (a log message causing the logger listener to log more messages - possible infinite recursion)");
				}
			}
		}
	}
	else
	{
		cachedMessages.pushBack(Message(Logger::LevelInfo, StringType(msg)));
	}
}

void Logger::warning(const StringRef &callerFunc, const StringRef &message)
{
	if (logLevel < Logger::LevelWarning)
		return;

	StringRef callerFunction(callerFunc);
	if (callerFunction.doesStartWith("fb::"))
		callerFunction = StringRef(callerFunction.getPointer() + 4, callerFunction.getLength() - 4);

	StringType msg;
	msg.reserve(message.getLength() + callerFunction.getLength() + 30);
	addTimestamp(msg);
	msg << "WARNING: " << callerFunction << " - " << message;

	MutexGuard guard(mutex);
	writeToLog(msg);
	if (listener != nullptr)
	{
		if (listenerLogLevel >= Logger::LevelWarning)
		{
			if (!flushingMessagesToListener)
			{
				messagesToListener.pushBack(Message(Logger::LevelWarning, lang::move(msg)));
			}
			else
			{
				static bool no_more_assert = false;
				if (!no_more_assert)
				{
					no_more_assert = true;
					fb_logger_assert(0 && "Logger encountered a listener making a recursive call. (a log message causing the logger listener to log more messages - possible infinite recursion)");
				}
			}
		}
	}
	else
	{
		cachedMessages.pushBack(Message(Logger::LevelWarning, StringType(msg)));
	}
}

void Logger::error(const StringRef &callerFunc, const StringRef &message)
{
	if (logLevel < Logger::LevelError)
		return;

	StringRef callerFunction(callerFunc);
	if (callerFunction.doesStartWith("fb::"))
		callerFunction = StringRef(callerFunction.getPointer() + 4, callerFunction.getLength() - 4);

	StringType msg;
	msg.reserve(message.getLength() + callerFunction.getLength() + 30);
	addTimestamp(msg);
	msg << "ERROR: " << callerFunction << " - " << message;

	MutexGuard guard(mutex);
	writeToLog(msg);

	if (listener != nullptr)
	{
		if (listenerLogLevel >= Logger::LevelError)
		{
			if (!flushingMessagesToListener)
			{
				messagesToListener.pushBack(Message(Logger::LevelError, lang::move(msg)));
			}
			else
			{
				static bool no_more_assert = false;
				if (!no_more_assert)
				{
					no_more_assert = true;
					fb_logger_assert(0 && "Logger encountered a listener making a recursive call. (a log message causing the logger listener to log more messages - possible infinite recursion)");
				}
			}
		}
	}
	else
	{
		cachedMessages.pushBack(Message(Logger::LevelError, StringType(msg)));
	}
}


static void writePositionToString(HeapString& str, float posX, float posY, float posZ)
{
	str += "VC3(";
	str.appendFloat(posX, 2);
	str += ", ";
	str.appendFloat(posY, 2);
	str += ", ";
	str.appendFloat(posZ, 2);
	str += ")";
}


#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE

namespace
{
	class FocusLoggerReportWriterTask;
	static ScopedPointer<FocusLoggerReportWriterTask> focusLoggerReportWriterTaskInstance;

	class FocusLoggerReportWriterTask : public task::ISchedulerTask
	{
		FocusLoggerReportWriterTask(task::Scheduler &scheduler, file::DebugBlockWriter &focusTestWriter, file::DebugBlockWriter &focusTestHighBandwidthWriter, FocusTestReportWriter &focusTestReportWriter)
			: scheduler(scheduler)
			, focusTestWriter(focusTestWriter)
			, focusTestHighBandwidthWriter(focusTestHighBandwidthWriter)
			, focusTestReportWriter(focusTestReportWriter)
			, taskId(task::Scheduler::getInvalidRepeatableTaskID())
			, dependencyGroupId(task::Scheduler::getInvalidDependencyGroupId())
			, writingReport(false)
		{
		}


	public:
		~FocusLoggerReportWriterTask()
		{
			fb_assert(taskId == task::Scheduler::getInvalidRepeatableTaskID());
			fb_assert(dependencyGroupId == task::Scheduler::getInvalidDependencyGroupId());
		}


		const char *getStaticTaskNameString() const
		{
			return "FocusLoggerReportWriterTask";
		}


		bool isRunning() const
		{
			return this->taskId != task::Scheduler::getInvalidRepeatableTaskID();
		}


		bool hasDependencyGroup() const
		{
			return this->dependencyGroupId != task::Scheduler::getInvalidDependencyGroupId();
		}

		bool start()
		{
			fb_assert(!isRunning());
			if (!hasDependencyGroup())
				dependencyGroupId = scheduler.createDependencyGroup(getTaskGroup());

			taskId = scheduler.addRepeatableTask(getTaskGroup(), this, task::Scheduler::TaskGranularityMillisecondBased, 1000);
			return true;
		}


		void end()
		{
			fb_assert(isRunning());
			scheduler.removeRepeatableTask(taskId);
			taskId = task::Scheduler::getInvalidRepeatableTaskID();
			removeDependencyGroup();
		}


		void removeDependencyGroup()
		{
			fb_assert(!isRunning());
			scheduler.waitForDependencies(getTaskGroup(), dependencyGroupId);
			scheduler.freeDependencyGroup(getTaskGroup(), dependencyGroupId);
			dependencyGroupId = task::Scheduler::getInvalidDependencyGroupId();
		}


		void writeReport()
		{
			{
				MutexGuard guard(mutex);
				if (this->writingReport)
					return;

				this->writingReport = true;
			}

			DynamicString filename = focusTestWriter.getOpenFilename();
			DynamicString highBandwidthFilename = focusTestHighBandwidthWriter.getOpenFilename();
			TempString outputFilename;
			outputFilename.appendPathFromString(filename);
			outputFilename += "report.txt";

			file::LineReader reader;

			if (reader.open(filename))
			{
				file::LineReader highBandwidthReader;
				highBandwidthReader.open(highBandwidthFilename);

				focusTestReportWriter.init(&reader, &highBandwidthReader);
				HeapString reportText = focusTestReportWriter.getReport();
				focusTestReportWriter.unInit();
				reader.close();
				highBandwidthReader.close();

				//reportText. \n -> \r\n
				reportText.replace("\n", "\r\n");

				file::OutputFile outputFile;
				outputFile.open(outputFilename);
				outputFile.writeData(reportText.getPointer(), reportText.getLength());
				outputFile.close();
			}
			MutexGuard guard(mutex);
			this->writingReport = false;
		}


		void run(const task::SchedulerTaskData &data, task::Scheduler &)
		{
			fb_assert(taskId != task::Scheduler::getInvalidRepeatableTaskID());
			fb_assert(dependencyGroupId != task::Scheduler::getInvalidDependencyGroupId());

			writeReport();
		}

		static FocusLoggerReportWriterTask& getInstance(task::Scheduler &scheduler, file::DebugBlockWriter &focusTestWriter, file::DebugBlockWriter &focusTestHighBandwidthWriter, FocusTestReportWriter &focusTestReportWriter)
		{
			if (focusLoggerReportWriterTaskInstance.get() != nullptr)
				return *focusLoggerReportWriterTaskInstance;

			focusLoggerReportWriterTaskInstance.reset(new FocusLoggerReportWriterTask(scheduler, focusTestWriter, focusTestHighBandwidthWriter, focusTestReportWriter));
			focusLoggerReportWriterTaskInstance->start();
			return *focusLoggerReportWriterTaskInstance;
		}


		static void reset()
		{
			if (focusLoggerReportWriterTaskInstance.get() != nullptr && focusLoggerReportWriterTaskInstance->isRunning())
				focusLoggerReportWriterTaskInstance->end();

			focusLoggerReportWriterTaskInstance.reset();
		}

	private:
		FB_CONST_POD(task::Scheduler::TaskGroup, TaskGroup, task::Scheduler::TaskGroupBackground);

		Mutex mutex;
		task::Scheduler &scheduler;
		file::DebugBlockWriter &focusTestWriter;
		file::DebugBlockWriter &focusTestHighBandwidthWriter;
		FocusTestReportWriter &focusTestReportWriter;
		task::Scheduler::RepeatableTaskId taskId;
		task::Scheduler::DependencyGroupId dependencyGroupId;
		bool writingReport;
	};
	
}
#endif


void Logger::setFocusTestReportWriter(file::FocusTestReportWriter *reportWriter)
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	focusTestReportWriter = reportWriter;
	focusTestWriter.reset(new file::DebugBlockWriter(&scheduler, file::DebugBlockWriter::UseAutomagicalFlush));
	focusTestHighBandwidthWriter.reset(new file::DebugBlockWriter(&scheduler, file::DebugBlockWriter::UseAutomagicalFlush));
#endif
}


void Logger::focusTestEvent(const DynamicString& eventType, float posX, float posY, float posZ, const StringRef &msg)
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	FB_STATIC_CONST_STRING(playerPositionEventString, "PlayerPosition");
	if (focusTestDirectory.isEmpty() || focusTestWriter == nullptr || focusTestHighBandwidthWriter == nullptr)
		return;

	HeapString str;
	SystemTime::now().addHumanReadableTimeStampToString(str, false);
	str << "\t" << eventType << "\t";
	writePositionToString(str, posX, posY, posZ);
	str << "\t" << msg << "\n";
	if (eventType == playerPositionEventString)
		focusTestHighBandwidthWriter->write(str);
	else
		focusTestWriter->write(str);
#endif
}


void Logger::setFocusTestLoggingDir(const DynamicString& directory)
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	if (focusTestWriter == nullptr || focusTestHighBandwidthWriter == nullptr)
		return;

	this->focusTestDirectory = directory;
	focusTestWriter->close();
	focusTestHighBandwidthWriter->close();
#endif
}

void Logger::createFocusTestDirectory()
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	if (focusTestWriter == nullptr || focusTestHighBandwidthWriter == nullptr)
		return;

	if (!focusTestDirectory.isEmpty())
	{
		HeapString dateStr;
		SystemTime::now().addHumanReadableTimeStampToString(dateStr, false);

		TempString fullPathToFiles(focusTestDirectory);
		if (!fullPathToFiles.doesEndWith("/"))
			fullPathToFiles += "/";

		fullPathToFiles += dateStr;
		fullPathToFiles += "/";
		TempString focusFile(fullPathToFiles);
		focusFile += "focusEventLog.txt";
		focusTestWriter->openFile(focusFile);
		focusFile = fullPathToFiles;
		focusFile += "focusEventHighBandwidthLog.txt";
		focusTestHighBandwidthWriter->openFile(focusFile);
	}
#endif
}

void Logger::createFocusTestReport(bool intermediate)
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	if (focusTestWriter.get() == nullptr || focusTestHighBandwidthWriter.get() == nullptr)
		return;

	if (!focusTestDirectory.isEmpty() && focusTestReportWriter != nullptr)
	{
		FocusLoggerReportWriterTask& focusReportWriterTask = FocusLoggerReportWriterTask::getInstance(scheduler, *focusTestWriter, *focusTestHighBandwidthWriter, *focusTestReportWriter);
		if (!intermediate)
		{
			focusReportWriterTask.end();
			focusReportWriterTask.writeReport();
			FocusLoggerReportWriterTask::reset();
		}
	}

	if (!intermediate)
	{
		focusTestWriter->close();
		focusTestHighBandwidthWriter->close();
	}
#endif
}

void Logger::swapLogFile(const char *newFilename)
{
	rotateLogFiles(StringRef(newFilename));
	FILE * pFile = fopen(newFilename, "wb");
	if ( pFile != nullptr)
	{
		fclose(getFilePtr(file));
	}
	else
	{
		fb_assertf(0, "%s could not swap log files!", FB_FUNCTION);
		return;
	}

	file = pFile;
	this->filename = DynamicString::createFromConstChar(newFilename);
}

const DynamicString &Logger::getLogFile() const
{
	return filename;
}


FB_END_PACKAGE1()
