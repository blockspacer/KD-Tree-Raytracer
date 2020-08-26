#include "Precompiled.h"
#include "FBAssert.h"

#include "fb/lang/DebugHelp.h"
#include "fb/lang/platform/LineFeed.h"
#include "fb/lang/time/SystemTime.h"
#include "fb/string/HeapString.h"

#include <cstdarg>
#include <stdio.h>

#undef NDEBUG

#ifdef _MSC_VER
/* Format string expected in argument 2 is not a string literal */
#pragma warning(disable:4774)
#endif


FB_PACKAGE0()

char *g_autoTestAssertionMessage = NULL;
bool fbAutoTestLoggingEnabled = false;


void assertLogF(const char *fmt, ...)
{
	FILE *f = fopen(DebugHelp::settings.assertLogFilename, "at");
	if (f != NULL)
	{
		va_list args;
		va_start(args, fmt);
		vfprintf(f, fmt, args);
		va_end(args);
		if (fbAutoTestLoggingEnabled)
		{
			fprintf(f, g_autoTestAssertionMessage);
			TempString timeStamp("ASSERTION TIMESTAMP: ");
			SystemTime::now().addTimeOnlyHumanReadableTimeStampToString(timeStamp);
			timeStamp << FB_PLATFORM_LF;
			fprintf(f, timeStamp.getPointer());
		}
		fclose(f);
	}
}

FB_END_PACKAGE0()
