#include "Precompiled.h"

#include <cstring>

FB_PACKAGE2(util, lz4)

#pragma warning(push)
#pragma warning(disable: 4365)
/* is not defined as a preprocessor macro, replacing with ... */
#pragma warning( disable: 4668 )
#include "lz4.c"
#pragma warning(pop)

FB_END_PACKAGE2()