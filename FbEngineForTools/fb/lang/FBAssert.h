#pragma once

#include "fb/lang/Config.h"

/* Note: Most includes at the end of file */

FB_PACKAGE0()

// Modify these in assert_wrapper.cpp to toggle them on/off on the fly
extern bool fbAutoTestLoggingEnabled; 
extern char *g_autoTestAssertionMessage;

/* This is still used in few places. It's unclear if it's any use */
void assertLogF(const char *fmt, ...);


FB_END_PACKAGE0()

#define FB_ASSERT_DEBUG_BREAK() __debugbreak();

/* Use this if you want to track stack usage */
//#define FB_STACK_ALLOCATION_CHECK() fb::AssertHelp::checkStackAllocation()
#define FB_STACK_ALLOCATION_CHECK()

#ifndef FB_FINAL_RELEASE_OBFUSCATION_ENABLED
#error "FB_FINAL_RELEASE_OBFUSCATION_ENABLED not defined"
#endif

#if FB_BUILD == FB_FINAL_RELEASE && FB_FINAL_RELEASE_OBFUSCATION_ENABLED == FB_TRUE
	#define FB_ASSERT_FILENAME ""
	#define FB_ASSERT_LINENUMBER __LINE__
#else
	#define FB_ASSERT_FILENAME __FILE__
	#define FB_ASSERT_LINENUMBER __LINE__
#endif

#define FB_ASSERT_IMP(predicate, expensive) \
	do \
	{ \
		FB_ASSERT_COUNTER(predicate); \
		if (predicate) \
		{ \
		} \
		else if ((expensive && fb::AssertHelp::settings.expensiveAssertEnabled) || (!expensive && fb::AssertHelp::settings.normalAssertEnabled)) \
		{ \
			if (fb::AssertHelp::assertNoBreak(#predicate, FB_ASSERT_FILENAME, unsigned(FB_ASSERT_LINENUMBER))) \
				FB_ASSERT_DEBUG_BREAK(); \
		} \
		FB_STACK_ALLOCATION_CHECK(); \
	} while (false)

//All platforms now support this, and if some platform doesn't, make it support this otherwise expected behaviour will be missed!
#define FB_ASSERT_IMP_VALIST(predicate, expensive, fmt, ...) \
	do \
	{\
		FB_ASSERT_COUNTER(predicate); \
		if (predicate)\
		{ \
		} \
		else if ((expensive && fb::AssertHelp::settings.expensiveAssertEnabled) || (!expensive && fb::AssertHelp::settings.normalAssertEnabled)) \
		{\
			static_assert(sizeof( #__VA_ARGS__ ) > 1, "fb_assertf requires at least one parameter"); \
			if (fb::AssertHelp::assertFNoBreak(#predicate, FB_ASSERT_FILENAME, unsigned(FB_ASSERT_LINENUMBER), fmt, __VA_ARGS__)) \
				FB_ASSERT_DEBUG_BREAK(); \
		}\
		FB_STACK_ALLOCATION_CHECK(); \
	} while (false)

// The assert once version (works only as a statement within a function scope, don't try to use as a condition clause or such)
#define FB_ASSERT_ONCE_IMP(predicate, expensive) \
	do \
	{ \
		if (predicate) \
		{ \
		} \
		else if ((expensive && fb::AssertHelp::settings.expensiveAssertEnabled) || (!expensive && fb::AssertHelp::settings.normalAssertEnabled)) \
		{\
			/* Use somewhat cumbersome variable names that are unlikely to clash with anything in surrounding code */ \
			static bool assertOnceWasTriggered = false; \
			if (!assertOnceWasTriggered) \
			{ \
				assertOnceWasTriggered = true; \
				if (fb::AssertHelp::assertNoBreak(#predicate " - Notice, Any further messages about this will be suppressed.", FB_ASSERT_FILENAME, unsigned(FB_ASSERT_LINENUMBER))) \
					FB_ASSERT_DEBUG_BREAK(); \
			}\
		} \
	} \
	while (false)


#if (FB_ASSERT_ENABLED == FB_TRUE)
	#define FB_ASSERT_DONT_USE_DIRECTLY(predicate) FB_ASSERT_IMP(predicate, false)
	#define FB_ASSERT_ONCE_DONT_USE_DIRECTLY(predicate) FB_ASSERT_ONCE_IMP(predicate, false)
	#define FB_ASSERT_DONT_USE_DIRECTLY_VALIST(predicate, fmt, ...) FB_ASSERT_IMP_VALIST(predicate, false, fmt, ## __VA_ARGS__)
	#define fb_assert_logf(fmt, ...) fb::assertLogF(fmt, __VA_ARGS__)
#elif (FB_ASSERT_ENABLED == FB_FALSE)
	#define FB_ASSERT_DONT_USE_DIRECTLY(predicate) do { } while (false)
	#define FB_ASSERT_ONCE_DONT_USE_DIRECTLY(predicate) do { } while (false)
	#define FB_ASSERT_DONT_USE_DIRECTLY_VALIST(predicate, fmt, ...) do { } while (false)
	#define fb_assert_logf(fmt, ...)
#else
	#error "FB_ASSERT_ENABLED define missing or invalid."
#endif

#if (FB_EXPENSIVE_ASSERT_ENABLED == FB_TRUE)
	#define FB_EXPENSIVE_ASSERT_DONT_USE_DIRECTLY(predicate) FB_ASSERT_IMP(predicate, true)
	#define FB_EXPENSIVE_ASSERT_ONCE_DONT_USE_DIRECTLY(predicate) FB_ASSERT_ONCE_IMP(predicate, true)
	#define FB_EXPENSIVE_ASSERT_DONT_USE_DIRECTLY_VALIST(predicate, fmt, ...) FB_ASSERT_IMP_VALIST(predicate, true, fmt, ## __VA_ARGS__)
#elif (FB_EXPENSIVE_ASSERT_ENABLED == FB_FALSE)
	#define FB_EXPENSIVE_ASSERT_DONT_USE_DIRECTLY(predicate) do { } while (false)
	#define FB_EXPENSIVE_ASSERT_ONCE_DONT_USE_DIRECTLY(predicate) do { } while (false)
	#define FB_EXPENSIVE_ASSERT_DONT_USE_DIRECTLY_VALIST(predicate, fmt, ...) do { } while (false)
#else
#error "FB_EXPENSIVE_ASSERT_ENABLED define missing or invalid."
#endif

#define fb_assert(x) FB_ASSERT_DONT_USE_DIRECTLY(x)
#define fb_assertf(x,fmt,...) FB_ASSERT_DONT_USE_DIRECTLY_VALIST(x, fmt, ## __VA_ARGS__)
#define fb_expensive_assert(x) FB_EXPENSIVE_ASSERT_DONT_USE_DIRECTLY(x)
#define fb_expensive_assertf(x,fmt,...) FB_EXPENSIVE_ASSERT_DONT_USE_DIRECTLY_VALIST(x, fmt, ## __VA_ARGS__)
#define fb_assert_once(x) FB_ASSERT_ONCE_DONT_USE_DIRECTLY(x)
#define fb_expensive_assert_once(x) FB_EXPENSIVE_ASSERT_ONCE_DONT_USE_DIRECTLY(x)

/* Define the macros first, include stuff only then */

#include "fb/lang/AssertHelp.h"


/* This has about 10 % performance penalty for Release build if enabled (tested with big level loading in Space) */
#define FB_ASSERT_COUNTER_ENABLED FB_FALSE

#if FB_ASSERT_COUNTER_ENABLED == FB_TRUE
	#include "fb/lang/AssertStats.h"
#else
	#define FB_ASSERT_COUNTER(predicate)
#endif

static_assert(sizeof(unsigned) == 4, "Trusting that unsigned is the same as uint32_t. That trust has been betrayed.");
