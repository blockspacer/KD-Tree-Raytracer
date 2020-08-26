#include "Precompiled.h"
#include "AssertHelp.h"

#include "fb/lang/DebugHelp.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/platform/LineFeed.h"
#include "fb/string/HeapString.h"

#include <stdarg.h> // For va_start, va_end
#include <stdio.h> // For vsnprintf
#include <vadefs.h> // For va_list

FB_PACKAGE0()

AssertHelp::Settings AssertHelp::settings;

bool AssertHelp::assertNoBreak(const char *predicate, const char *assertFile, uint32_t line)
{
	return DebugHelp::assertNoBreak(predicate, assertFile, line);
}

bool AssertHelp::assertFNoBreak(const char *predicate, const char *file, uint32_t line, const char* formatStr, ...)
{
	if (DebugHelp::shouldIgnoreAssert(file, line))
		return false;

	static const uint32_t bufferSize = 4096;
	char buffer[bufferSize];
	buffer[0] = { '\0' };
	va_list arguments;
	va_start(arguments, formatStr);
	int result = vsnprintf(buffer, bufferSize, formatStr, arguments);
	va_end(arguments);
	if (result < 0)
	{
		const char failMessage[] = "Printing formatted assert failed";
		lang::MemCopy::copy(buffer, failMessage, sizeof(failMessage));
	}

	LargeTempString predicateFaked;
#ifdef FB_PROGRAMMER_ASSERT_PRINTING
	predicateFaked << "Expr: (" << predicate << ")" FB_PLATFORM_LF << buffer;
#else
	predicateFaked << predicate << FB_PLATFORM_LF << "Expression printfed: " << buffer;
#endif

	return DebugHelp::assertNoBreak(predicateFaked.getPointer(), file, line);
}

FB_END_PACKAGE0()
