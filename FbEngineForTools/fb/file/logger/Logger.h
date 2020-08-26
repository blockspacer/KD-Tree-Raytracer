#pragma once

#include "fb/lang/platform/Config.h"

#include "fb/container/Pair.h"
#include "fb/container/Vector.h"
#include "fb/lang/Declaration.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/Signal.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"

FB_DECLARE(file, DebugBlockWriter)
FB_DECLARE(file, LineReader)
FB_DECLARE(file, FocusTestReportWriter)
FB_DECLARE(task, Scheduler)

FB_PACKAGE1(file)

class Logger : public lang::ILogger
{
public:
	Logger(const StringRef &logfile, task::Scheduler &scheduler);
	~Logger();

	// sets the level above which messages are ignored
	void setLogLevel(Logger::Level level) override;

	// sets the level above which messages are sent to listener
	// general log level affects this also, setting this above that level
	// will have no effect. (messages that are ignored won't be sent to
	// listener either even if the listener log level would be greater).
	void setListenerLogLevel(Logger::Level level) override;

	Level getListenerLogLevel();

	// different methods for different level of log messages
	void debug(const StringRef &callerFunc, const StringRef &message) override;
	void info(const StringRef &callerFunc, const StringRef &message) override;
	void warning(const StringRef &callerFunc, const StringRef &message) override;
	void error(const StringRef &callerFunc, const StringRef &message) override;
	void swapLogFile(const char *filename) override;
	const DynamicString &getLogFile() const override;

	void setFocusTestReportWriter(FocusTestReportWriter *focusTestReportWriter);
	void focusTestEvent(const DynamicString& eventType, float posX, float posY, float posZ, const StringRef &message) override;
	void setFocusTestLoggingDir(const DynamicString& directory) override;
	/* Called when game ended (with intermediate false) or every now and then so as to not lose data */
	void createFocusTestReport(bool intermediate);
	/* Called when game started */ 
	void createFocusTestDirectory();


	// sets a listener object
	// (if you want to forward the logged messages to some other object too)
	// the message forwarded may or may not contain level info before the
	// actual message. (probably will change over time)
	void setListener(lang::ILoggerListener *listener) override;

	// call to actually pass the messages to the listener
	// (required due to safe multithreading)
	void syncListener() override;

	virtual LogMessageSignal& getLogMessageSignal() override
	{
		return logMessageSignal;
	}

	void setTimestampsEnabled(bool enabled) { timestampsEnabled = enabled; }
	void setTimestampDiff(int64_t diff) { timestampDiff = diff; }

private:
	void addTimestamp(HeapString &str);
	void writeToLog(const StringRef &msg);

	Mutex mutex;
	StaticString filename;
	task::Scheduler &scheduler;
	Level logLevel = LevelNone;
	Level listenerLogLevel = LevelNone;
	lang::ILoggerListener *listener = nullptr;
	void *file = nullptr;

	ScopedPointer<file::DebugBlockWriter> debugBlockWriter;

#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	ScopedPointer<file::DebugBlockWriter> focusTestWriter;
	ScopedPointer<file::DebugBlockWriter> focusTestHighBandwidthWriter;
	StaticString focusTestDirectory;
	FocusTestReportWriter *focusTestReportWriter = nullptr;
#endif

	bool flushingMessagesToListener = false;
	LogMessageSignal logMessageSignal;

	typedef HeapString StringType;
	typedef Pair<Level, StringType> Message;
	typedef Vector<Message> MessageContainer;

	MessageContainer cachedMessages;
	MessageContainer messagesToListener;

	bool timestampsEnabled = false;
	int64_t timestampDiff = 0;
};

FB_END_PACKAGE1()
