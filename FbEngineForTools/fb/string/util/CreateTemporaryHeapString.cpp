#include "Precompiled.h"
#include "CreateTemporaryHeapString.h"

#include "fb/lang/Move.h"
#include <stdarg.h> // for va_arg

FB_PACKAGE1(string)

FB_NOINLINE HeapString createString(SizeType numArgs, ...)
{
	LargeTempString result;

	va_list args;
	va_start(args, numArgs);

	for (SizeType i = 0; i < numArgs; ++i)
	{
		const char* str = va_arg(args, const char*);
		if (str != NULL)
			result += str;
		else
			result += "(null)";
	}

	va_end(args);
	return lang::move(result);
}

FB_NOINLINE HeapString createStringAndDropNulls(SizeType numArgs, ...)
{
	LargeTempString result;

	va_list args;
	va_start(args, numArgs);

	for (SizeType i = 0; i < numArgs; ++i)
	{
		const char* str = va_arg(args, const char*);
		if (str != NULL)
			result += str;
	}

	va_end(args);
	return lang::move(result);
}


typedef const char *(*FilterFunction)(const char *) ;


FB_NOINLINE HeapString createStringWithFilter(FilterFunction filter, SizeType numArgs, ...)
{
	LargeTempString result;

	va_list args;
	va_start(args, numArgs);

	for (SizeType i = 0; i < numArgs; ++i)
	{
		const char* str = va_arg(args, const char*);
		result += filter(str);
	}

	va_end(args);
	return lang::move(result);
}

FB_END_PACKAGE1()
