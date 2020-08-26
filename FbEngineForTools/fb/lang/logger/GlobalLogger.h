#pragma once

#include "ILogger.h"
#include "fb/lang/Pimpl.h"

/**
 * GlobalLogger is a low-level logger usable pretty much everywhere. Its main purpose is to wrap some real logger that 
 * actually gets stuff written to disk. Real logger is set via addLogger method. Messages logged before real logger 
 * has been set are held in memory and delivered once real logger is available.
 *
 * Note: GlobalLogger is not thread safe. No calls to addLogger and removeLogger should be made in multi-threaded 
 * context. In practice, addLogger calls should be made during or soon after static initialization and removeLogger 
 * calls after all threads that may use GlobalLogger has stopped executing.

 * It is presumed real loggers added via addLogger are thread-safe though, so their methods (as defined in 
 * ILogger interface) will be called without mutexing.
 *
 * Note: For micro-optimization, and as there's currently no need to support more than one real logger, addLogger only 
 * works once, or if removeLogger has been called after previous addLogger call. If multiple loggers are needed, this 
 * should be trivial to change.
 */

FB_PACKAGE0()

class GlobalLogger : public lang::ILogger
{
	GlobalLogger();
public:
	virtual ~GlobalLogger();
	/* Adds logger that GlobalLogger forwards messages to */
	void addLogger(ILogger *logger);
	void removeLogger(ILogger *logger);

	/* Returns GlobalLogger singleton */
	static GlobalLogger &getInstance();

	/* ILogger interface */

	virtual void debug(const StringRef &callerFunc, const StringRef &message) override;
	virtual void info(const StringRef &callerFunc, const StringRef &message) override;
	virtual void warning(const StringRef &callerFunc, const StringRef &message) override;
	virtual void error(const StringRef &callerFunc, const StringRef &message) override;

	virtual void swapLogFile(const char *newLogFileName) override;
	virtual const DynamicString &getLogFile() const override;

	virtual void focusTestEvent(const DynamicString& eventType, float posX, float posY, float posZ, const StringRef &msg) override;
	virtual void setFocusTestLoggingDir(const DynamicString& directory) override;

	// sets the level above which messages are ignore overrided
	virtual void setLogLevel(ILogger::Level level) override;

	// sets the level above which messages are sent to listener
	// general log level affects this also, setting this above that level
	// will have no effect. (messages that are ignored won't be sent to
	// listener either even if the listener log level would be greater).
	virtual void setListenerLogLevel(ILogger::Level level) override;

	virtual void setListener(lang::ILoggerListener *listener) override;

	// call to actually pass the messages to the listener
	// (required due to safe multithreading)
	virtual void syncListener() override;

	virtual LogMessageSignal& getLogMessageSignal() override;

private:
	class Impl;
	Impl &impl;
};

FB_END_PACKAGE0()
