#include "Precompiled.h"
#include "GlobalLogger.h"

#include "fb/container/Vector.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"
#include "fb/string/util/CreateTemporaryHeapString.h"
#include "fb/lang/FBSingleThreadAssert.h"

FB_PACKAGE0()

class GlobalLogger::Impl
{
public:
	class MemoryLogger : public ILogger
	{
	public:

		void debug(const StringRef &callerFunc, const StringRef &message) override
		{
			MutexGuard guard(mutex);
			messages.pushBack(Message(LevelDebug, callerFunc, message));
			limitMessages();
			FB_PRINTF("DEBUG: %s - %s\n", callerFunc.getPointer(), messages.getBack().message.getPointer());
		}

		virtual void info(const StringRef &callerFunc, const StringRef &message) override
		{
			MutexGuard guard(mutex);
			messages.pushBack(Message(LevelInfo, callerFunc, message));
			limitMessages();
			FB_PRINTF("INFO: %s - %s\n", callerFunc.getPointer(), messages.getBack().message.getPointer());
		}

		void warning(const StringRef &callerFunc, const StringRef &message) override
		{
			MutexGuard guard(mutex);
			messages.pushBack(Message(LevelWarning, callerFunc, message));
			limitMessages();
			FB_PRINTF("WARNING: %s - %s\n", callerFunc.getPointer(), messages.getBack().message.getPointer());
		}

		void error(const StringRef &callerFunc, const StringRef &message) override
		{
			MutexGuard guard(mutex);
			messages.pushBack(Message(LevelError, callerFunc, message));
			limitMessages();
			FB_PRINTF("ERROR: %s - %s\n", callerFunc.getPointer(), messages.getBack().message.getPointer());
		}

		virtual void swapLogFile(const char *newLogFileName) override { }
		const DynamicString &getLogFile() const override { return DynamicString::empty; }

		virtual void focusTestEvent(const DynamicString& eventType, float posX, float posY, float posZ, const StringRef &msg) override
		{
			MutexGuard guard(mutex);
			messages.pushBack(Message(eventType, posX, posY, posZ, msg));
			limitMessages();
		}

		virtual void setFocusTestLoggingDir(const DynamicString& directory) override { }

		virtual void setLogLevel(ILogger::Level level) override { }

		virtual void setListenerLogLevel(ILogger::Level level) override { }
		virtual void setListener(lang::ILoggerListener *listener) override { }

		/* Nop. MemoryDebugger will log everything real logger when such is set up, so syncing or signaling would just mess things up */
		virtual void syncListener() override { }

		virtual LogMessageSignal& getLogMessageSignal() override { return dummyLogMessageSignalNeverCallMe; }

		struct Message
		{
			Message() { }
			Message(Level level, const StringRef &callerFunc, const StringRef &msg)
				: level(level)
				, callerFunc(callerFunc)
			{
				message << msg;
			}

			Message(const DynamicString &focusTestEvent, float posX, float posY, float posZ, const StringRef &msg)
				: message(msg)
				, focusTestEvent(focusTestEvent)
				, posX(posX)
				, posY(posY)
				, posZ(posZ)
			{

			}

			Level level = LevelNone;
			TempString callerFunc;
			TempString message;
			DynamicString focusTestEvent;
			float posX;
			float posY;
			float posZ;
		};

		void limitMessages()
		{
			// we don't want to run out of memory here
			if (messages.getSize() > 10000)
				messages.eraseIndex(0);
		}

		/* MemoryLogger shouldn't really be in use during threaded phase, but who knows */
		Mutex mutex;
		/* Cache first 8 messages to memory (if they are short enough), so we avoid doing any allocations */
		CacheVector<Message, 8> messages;

		LogMessageSignal dummyLogMessageSignalNeverCallMe;
	};

	void refreshBackingLoggerParameters()
	{
		fb_assert(backingLogger != nullptr);
		if (!logFileName.isEmpty())
			backingLogger->swapLogFile(logFileName.getPointer());

		backingLogger->setLogLevel(loggingLevel);
		backingLogger->setListenerLogLevel(listenerLevel);
		backingLogger->setListener(listener);
		logMessageSignalBind = bindDelegate(backingLogger->getLogMessageSignal(), &Impl::logMessageSignalListener, this);
	}

	void logMessageSignalListener(const HeapString &message, Level level)
	{
		logMessageSignal(message, level);
	}

	static Impl &getInstance()
	{
		static Impl instance;
		return instance;
	}

#if FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC
	static const TempString &sanitizeFuncName(const StringRef &funcName)
	{
		/* Function signature on Clang (and presumably on GCC) is like following: static bool fb::engine::base::errortag::ErrorTaggerManager::logReplacedErrorTag(fb::engine::base::errortag::TagType, const fb::engine::base::Blobject *, const fb::StringRef &, const fb::StringRef &, const fb::StringRef &, const fb::StringRef &) */
		thread_local TempString str;
		str.clear();
		static const StringRef virtualStr("virtual ");
		static const StringRef staticStr("static ");
		SizeType charsToSkip = 0;
		if (funcName.doesStartWith(virtualStr))
			charsToSkip = virtualStr.getLength();
		else if (funcName.doesStartWith(staticStr))
			charsToSkip = staticStr.getLength();

		/* Remove return parameter */
		SizeType charsToInclude = 0;
		for (SizeType i = charsToSkip, len = funcName.getLength(); i < len; ++i)
		{
			if (funcName[i] == ' ')
			{
				charsToSkip = i + 1;
				break;
			}
			if (funcName[i] == '(')
			{
				/* Whoops, this is probably constructor */
				charsToInclude = i;
				break;
			}
		}

		/* Found opening bracket (unless already found while looking for end of return parameter) */
		if (charsToInclude == 0)
		{
			for (SizeType i = charsToSkip, len = funcName.getLength(); i + 1 < len; ++i)
			{
				if (funcName[i] == '(')
					break;

				++charsToInclude;
			}
		}
		str.append(funcName.getPointer() + charsToSkip, charsToInclude);
		return str;
	}
#else
	static const StringRef &sanitizeFuncName(const StringRef &funcName)
	{
		return funcName;
	}
#endif

	MemoryLogger memoryLogger;
	ILogger::LogMessageSignal logMessageSignal;
	SignalBind logMessageSignalBind;
	lang::ILogger *backingLogger = nullptr;
	lang::ILoggerListener *listener = nullptr;

	StaticString logFileName;
	StaticString focusTestDirName;
	ILogger::Level loggingLevel = ILogger::LevelDebug;
	ILogger::Level listenerLevel = ILogger::LevelDebug;
};


GlobalLogger::GlobalLogger()
	: impl(Impl::getInstance())
{
	impl.backingLogger = &impl.memoryLogger;
}

GlobalLogger::~GlobalLogger()
{
}

void GlobalLogger::addLogger(ILogger *logger)
{
	fb_single_thread_assert();
	fb_main_thread_assert();
	fb_assert(impl.backingLogger == &impl.memoryLogger);
	impl.backingLogger = logger;
	impl.refreshBackingLoggerParameters();
	impl.backingLogger->info(FB_FUNCTION, "Changed from MemoryLogger to real one");
	if (impl.memoryLogger.messages.isEmpty())
		return;

	/* Log messages from memory logger to real logger */
	impl.backingLogger->info(FB_FUNCTION, "Logging messages from MemoryLogger");
	for (const Impl::MemoryLogger::Message &message : impl.memoryLogger.messages)
	{
		if (message.focusTestEvent.isEmpty())
		{
			switch (message.level)
			{
			case LevelError :
				impl.backingLogger->error(message.callerFunc, message.message);
				break;
			case LevelWarning :
				impl.backingLogger->warning(message.callerFunc, message.message);
				break;
			case LevelInfo :
				impl.backingLogger->info(message.callerFunc, message.message);
				break;
			case LevelDebug :
				impl.backingLogger->debug(message.callerFunc, message.message);
				break;
			default :
				fb_assert(0 && "Unexpected ILogger::Level");
			}
		}
		else
		{
			impl.backingLogger->focusTestEvent(message.focusTestEvent, message.posX, message.posY, message.posZ, message.message);
		}
	}
	impl.memoryLogger.messages.clear();
}

void GlobalLogger::removeLogger(ILogger *logger)
{
	fb_single_thread_assert();
	fb_main_thread_assert();
	if (impl.backingLogger == logger)
	{
		impl.backingLogger->info(FB_FUNCTION, "Changing from real logger to MemoryLogger");
		impl.backingLogger = &impl.memoryLogger;
		impl.refreshBackingLoggerParameters();
	}
}

GlobalLogger &GlobalLogger::getInstance()
{
	static GlobalLogger logger;
	return logger;
}

void GlobalLogger::debug(const StringRef &callerFunc, const StringRef &msg)
{
	impl.backingLogger->debug(Impl::sanitizeFuncName(callerFunc), msg);
}

void GlobalLogger::info(const StringRef &callerFunc, const StringRef &msg)
{
	impl.backingLogger->info(Impl::sanitizeFuncName(callerFunc), msg);
}

void GlobalLogger::warning(const StringRef &callerFunc, const StringRef &msg)
{
	impl.backingLogger->warning(Impl::sanitizeFuncName(callerFunc), msg);
}

void GlobalLogger::error(const StringRef &callerFunc, const StringRef &msg)
{
	impl.backingLogger->error(Impl::sanitizeFuncName(callerFunc), msg);
}

void GlobalLogger::swapLogFile(const char *newLogFileName)
{
	impl.logFileName = StaticString::createFromConstChar(newLogFileName);
	impl.backingLogger->swapLogFile(newLogFileName);
}

const DynamicString &GlobalLogger::getLogFile() const
{
	return impl.backingLogger->getLogFile();
}

void GlobalLogger::focusTestEvent(const DynamicString &eventType, float posX, float posY, float posZ, const StringRef &msg)
{
	impl.backingLogger->focusTestEvent(eventType, posX, posY, posZ, msg);
}

void GlobalLogger::setFocusTestLoggingDir(const DynamicString &directory)
{
	impl.focusTestDirName = directory;
	impl.backingLogger->setFocusTestLoggingDir(directory);
}

void GlobalLogger::setLogLevel(ILogger::Level level)
{
	impl.loggingLevel = level;
	impl.backingLogger->setLogLevel(level);
}

void GlobalLogger::setListenerLogLevel(ILogger::Level level)
{
	impl.listenerLevel = level;
	impl.backingLogger->setListenerLogLevel(level);
}

void GlobalLogger::setListener(lang::ILoggerListener *listener)
{
	impl.listener = listener;
	impl.backingLogger->setListener(listener);
}

void GlobalLogger::syncListener()
{
	impl.backingLogger->syncListener();
}

lang::ILogger::LogMessageSignal& GlobalLogger::getLogMessageSignal()
{
	return impl.logMessageSignal;
}

FB_END_PACKAGE0()
