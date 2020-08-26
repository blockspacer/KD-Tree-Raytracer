#pragma once

/* This tells various places that we are using engine as a base for some tool, not a game. Basically this avoids the 
 * need to create several small defines for things that never change in game projects */
#define FB_ENGINE_FOR_TOOLS FB_TRUE

#include "fb/lang/platform/WarningDisables.h"

#include "fb/lang/platform/FBConstants.h"
#include "fb/lang/Package.h"
#include "fb/lang/platform/Compiler.h"

#define FB_USE_SIMD  FB_TRUE

#include "fb/lang/platform/CompilerOptimizations.h"

#ifndef FB_BUILD
	#ifdef NDEBUG
		#define FB_BUILD FB_RELEASE
	#elif _DEBUG
		#define FB_BUILD FB_DEBUG
	#else
		#error Could not determine build type
	#endif
#endif

#if (FB_BUILD == FB_RELEASE)
	// hack to enable asserts
	#ifdef NDEBUG
		#undef NDEBUG
	#endif
#endif


#define FB_FATAL_ERROR(p_msg, p_return_value) \
	do { \
		std::cout << "FATAL ERROR. We will die: " << p_msg << std::endl; \
		exit(p_return_value); \
	} while (false);
