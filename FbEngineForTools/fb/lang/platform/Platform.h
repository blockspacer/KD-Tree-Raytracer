#pragma once

// autodetection of the "major platform"...
// (sets the appropriate macros that should be used elsewhere in the code)

#include "FBConstants.h"

// ------------------------------------------------------------
// WINDOWS

// NOTE: WIN32 and WIN64 should be now defined in VC Win32.props and x64.props files, so this manual check shouldn't be needed
#if (defined(WIN32) || defined(WIN64))
	#define FB_PLATFORM_TYPE FB_PC
	#define FB_PLATFORM FB_WINDOWS
	#ifdef _M_IX86
		#define FB_PLATFORM_BITS 32
	#elif defined(_M_X64)
		#define FB_PLATFORM_BITS 64
		#ifndef WIN64
			#define WIN64
		#endif
	#else
		#error "Unknown amount of bits for the Windows platform."
	#endif
#endif


// ------------------------------------------------------------

// most other things can be considered as "minor platforms"
// such as the use of specific library, such as STLPort.
// they are not exclusive - several of such platforms might
// co-exist simultaneously and they may or may not also
// exist with several of the major platforms.
// (event the PLATFORM_BITS_... defines as well as the endianess
// could be considered "minor" platform in the sense that
// they may or may not be used on different major platforms.
