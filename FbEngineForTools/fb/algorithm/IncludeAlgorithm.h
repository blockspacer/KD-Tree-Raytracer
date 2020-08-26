#pragma once

#pragma warning(push)

#if FB_COMPILER == FB_MSC && FB_BUILD == FB_DEBUG
/* 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch */
#pragma warning(disable: 4365)
// algorithm includes malloc on debug builds via million indirections, and it does so in a way that somehow causes a compiler warning
// "c:\program files (x86)\windows kits\10\include\10.0.10240.0\ucrt\malloc.h(160): warning C4548: expression before comma has no effect; expected expression with side-effect"
#pragma warning(disable : 4548)
#endif

#include <algorithm>

#pragma warning(pop)
