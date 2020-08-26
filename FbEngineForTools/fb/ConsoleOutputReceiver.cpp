#include "Precompiled.h"
#include "ConsoleOutputReceiver.h"

#pragma warning(push)
/* 'argument': conversion from 'x' to 'y', signed/unsigned mismatch */
#pragma warning(disable: 4365)
/* C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc */
#pragma warning(disable: 4530)
/* 'sprintf_s' : format string expected in argument 3 is not a string literal */
#pragma warning(disable: 4774)
#include <iostream>
#pragma warning(pop)

FB_PACKAGE1(toolsengine)

void ConsoleOutputReceiver::write(const StringRef &str)
{
	std::cout << str.getPointer();
}

FB_END_PACKAGE1()