#pragma once

#include "LoggingMacros.h"
#include "fb/lang/Signal.h"

FB_DECLARE0(DynamicString);
FB_DECLARE0(StringRef)
FB_DECLARE0(HeapString)

FB_PACKAGE1(lang)

// an interface for a class that logger will forward it's messages to
class ILoggerListener
{
public:
	virtual ~ILoggerListener() {}
	virtual void logMessage(const char *msg, int level) = 0;
};



/**
 * The generic logger interface.
 */
class ILogger
{
public:
	virtual ~ILogger() {}

	enum Level
	{
		LevelNone = 0,
		LevelError = 1,
		LevelWarning = 2,
		LevelInfo = 3,
		LevelDebug = 4,

		NumLevels
	};

	/* Methods for different level of log messages */
	virtual void debug(const StringRef &callerFunc, const StringRef &message) = 0;
	virtual void info(const StringRef &callerFunc, const StringRef &message) = 0;
	virtual void warning(const StringRef &callerFunc, const StringRef &message) = 0;
	virtual void error(const StringRef &callerFunc, const StringRef &message) = 0;

	virtual void swapLogFile(const char *newLogFileName) = 0;
	/* Gets current log file path */
	virtual const DynamicString &getLogFile() const = 0;
	/* Special method for focus test event logging */
	virtual void focusTestEvent(const DynamicString& eventType, float posX, float posY, float posZ, const StringRef &msg) = 0;
	virtual void setFocusTestLoggingDir(const DynamicString& directory) = 0;

	// This is used by the macros to disable the logger calls that to a dummy that does nothing. (and hopefully gets optimized away)
	static inline void dummy() { }

	// sets the level above which messages are ignored
	virtual void setLogLevel(ILogger::Level level) = 0;

	// sets the level above which messages are sent to listener
	// general log level affects this also, setting this above that level
	// will have no effect. (messages that are ignored won't be sent to
	// listener either even if the listener log level would be greater).
	virtual void setListenerLogLevel(ILogger::Level level) = 0;

	virtual void setListener(ILoggerListener *listener) = 0;

	/* Call to actually pass the messages to the listener. Also signals LogMessageSignal
	 * (Required due to safe multi-threading) */
	virtual void syncListener() = 0;

	typedef Signal<void(const HeapString&, Level)> LogMessageSignal;
	virtual LogMessageSignal& getLogMessageSignal() = 0;
};


FB_END_PACKAGE1()
