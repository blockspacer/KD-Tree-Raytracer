#include "Precompiled.h"
#include "Cstrlen.h"

#include "fb/string/Common.h"

#pragma warning(push)
#if FB_COMPILER == FB_MSC
/* C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch */
#pragma warning(disable: 4365)
/* 4548: Expression before comma has no effect; expected expression with side-effect */
#pragma warning(disable: 4548)
#endif
/* C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc */
#pragma warning(disable: 4530)
/* Format string expected in argument 1 is not a string literal */
#pragma warning(disable: 4774)
// Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable: 4571)
#include <string>
#pragma warning(pop)

FB_PACKAGE0()

SizeType fbStrLen(const char *str)
{
	fb_expensive_assert(str && "Null given to fbStrLen");
	size_t length = std::strlen(str);
	fb_expensive_assert(length < string::NoPosition);
	return SizeType(length);
}

FB_END_PACKAGE0()
