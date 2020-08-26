#pragma once

/* Instead of many smaller defines, here's a big one for stuff that is different in FB Engine for Tools use */
#ifndef FB_ENGINE_FOR_TOOLS
	#define FB_ENGINE_FOR_TOOLS FB_FALSE
#endif

#include "WarningDisables.h"
#include "LocalConfig.h"
#include "fb/lang/preprocessor/CommonMacros.h"
#include "fb/lang/preprocessor/Increase.h"
#include "fb/lang/preprocessor/Decrease.h"
#include "fb/lang/preprocessor/Subtract.h"
#include "fb/lang/preprocessor/Narg.h"
#include "fb/lang/preprocessor/Unpack.h"
#include "fb/lang/preprocessor/StripCommas.h"
#include "fb/lang/preprocessor/Loop.h"
#include "fb/lang/Package.h"
#include "fb/lang/Declaration.h"
#include "fb/lang/platform/Platform.h"
#include "fb/lang/platform/FBConstants.h"

#if (FB_BUILD == FB_RELEASE)
	// hack to enable asserts
	#ifdef NDEBUG
		#undef NDEBUG
	#endif
#endif

#ifndef FB_BUILD
	#error "FB_BUILD undefined. You need to add that to the project configuration."
#endif

// This file will be force-included to all source files.

// Do NOT dump your defines here, rather add them to the module specific Config.h files.
// Keep this file clean, only add stuff here if you absolutely must do so!
// (Touching this file will cause a full rebuild.)

#if (FB_BUILD == FB_DEBUG)
	// hacky debug build defines come here


#elif (FB_BUILD == FB_RELEASE)
	// hacky release build defines come here


#elif (FB_BUILD == FB_FINAL_RELEASE)
	// hacky final release build defines come here

#else
  #error "Unknown build."
#endif

#include "PostGlobalConfigEditor.h"
#include "PostGlobalConfigRenderer.h"
#include "PostGlobalConfigInput.h"
#include "PostGlobalConfigAudio.h"
#include "PostGlobalConfigDemo.h"
#include "PostGlobalConfigMath.h"
#include "PostGlobalConfigGUI.h"
#include "PostGlobalConfigStore.h"
#include "PostGlobalConfigNet.h"
#include "PostGlobalConfigPhysics.h"
#include "PostGlobalConfigLang.h"

// editor only enabled for non-final Windows builds
#if FB_EDITOR_ENABLED == FB_TRUE
	#if (FB_BUILD == FB_FINAL_RELEASE)
		#undef FB_EDITOR_ENABLED
		#define FB_EDITOR_ENABLED FB_FALSE
	#endif
	#if FB_WINDOWS_USE_OPENGL == FB_TRUE
		// #undef FB_EDITOR_ENABLED
		// #define FB_EDITOR_ENABLED FB_FALSE
		#if defined(FB_FORCE_NO_EDITOR) && FB_FORCE_NO_EDITOR == FB_TRUE
			#define FB_OPENGL_PROCESS_RESOURCES FB_FALSE
		#else
			#define FB_OPENGL_PROCESS_RESOURCES FB_TRUE
		#endif
	#else
		#define FB_OPENGL_PROCESS_RESOURCES FB_FALSE
	#endif
#else
	#define FB_OPENGL_PROCESS_RESOURCES FB_FALSE
#endif

// NoEditor build config
#if FB_EDITOR_ENABLED == FB_TRUE
	#if defined(FB_FORCE_NO_EDITOR) && FB_FORCE_NO_EDITOR == FB_TRUE
			#undef FB_EDITOR_ENABLED
			#define FB_EDITOR_ENABLED FB_FALSE
	#endif
#endif

// Public editor release build
#ifndef FB_EDITOR_PUBLIC_ENABLED
	#define FB_EDITOR_PUBLIC_ENABLED FB_FALSE
#endif

// Appstore define backward compability (if it doesn't exist yet)
#ifndef FB_APPSTORE_ENABLED
	#define FB_APPSTORE_ENABLED FB_FALSE
#endif

// Appstore only enabled for OS X
#if FB_APPSTORE_ENABLED == FB_TRUE
	#if FB_PLATFORM != FB_OSX 
		#undef FB_APPSTORE_ENABLED
		#define FB_APPSTORE_ENABLED FB_FALSE
	#endif
#endif

//TAG: MAGICAL_THREE_PLAYER_LIMIT_CHANGED_TO_FOUR
#ifndef FB_MAX_REMOTE_CLIENTS
	#define FB_MAX_REMOTE_CLIENTS 3
#endif

#ifndef FB_LEGACY_SYNC_ENABLED
	#define FB_LEGACY_SYNC_ENABLED FB_TRUE
#endif

// dedicated server define
#ifndef FB_DEDICATED_SERVER
#define FB_DEDICATED_SERVER FB_FALSE
#endif

// default physics engine
#ifndef FB_PHYSICS_ENGINE

#define FB_PHYSICS_ENGINE FB_PHYSX

#endif
