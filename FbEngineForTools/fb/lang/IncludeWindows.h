#pragma once

#pragma warning(push)
// is not defined as a preprocessor macro, replacing with ...
#pragma warning( disable: 4668 )

#ifndef FB_PLATFORM
#error FB_PLATFORM not defined
#endif

#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
/* winsock2.h needs to be included before windows.h */
/* For VS2015, should actually fix these warnings */
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#undef DrawState


#pragma warning(pop)
