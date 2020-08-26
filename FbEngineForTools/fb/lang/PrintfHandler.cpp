#include "Precompiled.h"
#include "PrintfHandler.h"

#include "fb/string/StringRef.h"

#include <stdio.h>
#include <stdarg.h>

#include "fb/lang/IncludeWindows.h"

#define FB_FORCE_DBW_FOR_PRINTF FB_FALSE

#if FB_FORCE_DBW_FOR_PRINTF == FB_TRUE
#define FB_WRITE_PRINTF_TO_DBW FB_TRUE
#else
#define FB_WRITE_PRINTF_TO_DBW FB_FALSE
#endif

FB_PACKAGE1(lang)

PrintfHandler::PrintfHandler()
	: numReceivers(0)
{
}


PrintfHandler::~PrintfHandler()
{
}


bool PrintfHandler::addCharacterOutputReceiver(ICharacterOutputReceiver* receiver)
{
	if (numReceivers < maxReceivers)
	{
		receivers[numReceivers] = receiver;
		++numReceivers;
		return true;
	}
	return false;
}


bool PrintfHandler::removeCharacterOutputReceiver(ICharacterOutputReceiver* receiver)
{
	for (SizeType i = 0; i < numReceivers; ++i)
	{
		if (receivers[i] == receiver)
		{
			receivers[i] = receivers[numReceivers - 1];
			--numReceivers;
			return true;
		}
	}
	return false;
}


PrintfHandler& PrintfHandler::getPrintfHandler()
{
	static PrintfHandler handler;
	return handler;
}

void PrintfHandler::doPrintf(const char *fmt, ...)
{
#if (FB_COMPILER == FB_MSC)
	{
		static const size_t bufferSize = 64 * 1024;
		va_list args;
		va_start(args, fmt);

		size_t neededSpace = size_t(_vscprintf(fmt, args) + 1);

		if (neededSpace >= bufferSize)
		{
			char *tmp = new char[neededSpace];
			tmp[0] = 0;
			tmp[neededSpace - 1] = 0;

			vsprintf_s(tmp, neededSpace, fmt, args);
			va_end(args);
			OutputDebugStringA(tmp);
#if FB_DEDICATED_SERVER == FB_TRUE
			fwrite(tmp, 1, strlen(tmp), stdout);
#endif

			delete[] tmp;
		}
		else
		{
			char buffer[bufferSize];
			buffer[0] = 0;
			buffer[bufferSize - 1] = 0;

			vsprintf_s(buffer, bufferSize - 1, fmt, args);
			va_end(args);
			OutputDebugStringA(buffer);
#if FB_DEDICATED_SERVER == FB_TRUE
			fwrite(buffer, 1, strlen(buffer), stdout);
#endif
		}
	}
#else
	if (numReceivers == 0)
	{
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
#endif

	if (numReceivers > 0)
	{
		size_t length = 0;

		va_list args;
		va_start(args, fmt);

		length = size_t(vsnprintf(NULL, 0, fmt, args));

		// Kept here just in case, if code doesn't compile with vsnprintf, add required #ifs to make it work
		/*
		length = size_t(vprintf(fmt, args));
		fflush(stdout);
		*/

		/* Only allocate from heap if result string is very long */
		static const size_t stackBufferSize = 2048;
		char stackBuffer[stackBufferSize];
		char* buffer = stackBuffer;

		if (length >= stackBufferSize)
			buffer = new char[length + 1];

		SizeType dataSize = SizeType(vsprintf(buffer, fmt, args));
		va_end(args);

		/* Probably not the best idea to assert here */
		if (dataSize != length)
			printf("Epic fail. Different sizes");

		for (SizeType i = 0; i < numReceivers; ++i)
			receivers[i]->write(StringRef(buffer, dataSize));

		if (length >= stackBufferSize)
			delete[] buffer;
	}
}


void BasicCharacterOutputReceiver::write(const StringRef &str)
{
	if (!str.isEmpty())
		printf("%s", str.getPointer());
}

FB_END_PACKAGE1()
