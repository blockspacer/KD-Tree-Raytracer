#pragma once

#include "fb/lang/platform/ForceInline.h"
#include "fb/lang/IntTypes.h"

FB_PACKAGE0()

/**
 * AssertHelp is a minimalist header for FBAssert.h to include instead of DebugHelp to avoid include bloat
 */
class AssertHelp
{
public:
	/* Note: assertNoBreak methods take const char*s instead of StringRef to minimize assert induced code bloat */
	/* Handles assert without breaking to debugger (unless user requests it) */
	FB_NOINLINE static bool assertNoBreak(const char *predicate, const char *file, uint32_t line);

	/* Handles assertf without breaking to debugger (unless user requests it) */
	#if FB_COMPILER == FB_CLANG || FB_COMPILER == FB_GNUC
		#define FB_ASSERTHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE __attribute__((format(printf, 4, 5)))
	#else
		#define FB_ASSERTHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE
	#endif
	FB_NOINLINE static bool assertFNoBreak(const char *predicate, const char *file, uint32_t line, const char* formatStr, ...) FB_ASSERTHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE;
	#undef FB_ASSERTHELP_ASSERTF_ARGUMENT_CHECK_ATTRIBUTE

	struct Settings
	{
		/* This can be used to toggle expensive asserts on runtime, if build supports them */
		bool expensiveAssertEnabled = true;
		/* This can be used to toggle normal asserts on runtime, if build supports them */
		bool normalAssertEnabled = true;
	};
	static Settings settings;
};

FB_END_PACKAGE0()
