#pragma once

#include "Config.h"
#include "fb/lang/platform/Platform.h"

// This defines the FB_PLATFORM_LF to either "\n" or "\r\n"

// (You are supposed to wrap that case-by-case to different macros, such as LOGGER_LF or similar for 
// log file usage - as not all components use the platform specific linefeeds)
#define FB_PLATFORM_LF "\r\n"
#define FB_PLATFORM_LF_WIDE L"\r\n"
#define FB_PLATFORM_LF_LEN 2
