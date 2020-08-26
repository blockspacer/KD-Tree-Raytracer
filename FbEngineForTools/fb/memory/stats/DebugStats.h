#pragma once

#include "Config.h"

#include "DebugStatsVariable.h"
#include "DebugStatsScope.h"
#include "DebugStatsScopeIterator.h"
#include "IDebugStatsBreakpointListener.h"

#include "fb/lang/platform/Decl.h"
#include "fb/string/StaticString.h"

FB_DECLARE0(StringRef);

// The macros of that are likely of some interest: 
// -----------------------------------------------

// FB_DSTAT_ALLOC(p_scope, p_varname)
// FB_DSTAT_ALLOC_INC(p_varname)
// FB_DSTAT_ALLOC_DEC(p_varname)

// And add the "fb/memory/stats/DebugStatsInitForLibarary.cpp" to sources of any library
// And add the "fb/memory/stats/DebugStatsInitForMain.cpp" to sources of the main application

// There are some other alternatives as well (for non-allocations) such as the following:
// FB_DSTAT(p_scope, p_varname)
// FB_DEBUG_STATS_VAR_WITH_LIMIT(p_scope, p_varname, p_min, p_max)
// FB_DSTAT_SET(p_varname, p_value)

// Notice, the parameters are NOT strings

// And the scope parameter is expected to be a namespace, whereas the varname is expected to be a classname.
// (the varname must contain alphanumeric characters only (and possibly underscores)

// Assuming you had a class MyClass inside fb::memory namespace, then you might use these as follows:
// --------------------------------------------------------------------------------------------------

// inside the fb::memory package, you might add:
	// FB_DEBUG_STATS_ALLOC_VAR(fb::memory, MyClass)

// then, inside the MyClass constructor:
	// FB_DEBUG_STATS_ALLOC_INC(MyClass)

// and inside the destructor:
	// FB_DEBUG_STATS_ALLOC_DEC(MyClass)


FB_DECLARE(memory, stats, DebugStatsVariableIterator)

#if (FB_MEMORY_DEBUG_STATS_ENABLED == FB_TRUE)	
	static bool noMoreDebugStatsAssertPersistent = false;
	static bool noMoreDebugStatsAssertTemp = false;

	// notice, this is a non-const function. a hacky way to implement some assert once behaviour
	// it is specifically hacky because it relies on the assert if clause boolean evaluation order!
	inline int debug_stats_assert_impl(int assertValue)
	{
		// since assert (fb_assert particularly) cannot really handle non-const functions, doing some magic
		if (noMoreDebugStatsAssertPersistent)
		{
			return 1;
		}
		else
		{
			if (assertValue == 0)
				noMoreDebugStatsAssertTemp = true;

			return assertValue;
		}
	}
#endif

FB_PACKAGE2(memory, stats)

// ------------------------------------------------------------------------------
// The most common debug stats macros have now been shortened to sensible lengths:

#define FB_DSTAT(p_scope, p_varname) FB_DEBUG_STATS_VAR_NO_LIMIT(p_scope, p_varname)
#define FB_DSTAT_ALLOC(p_scope, p_varname) FB_DEBUG_STATS_ALLOC_VAR_NO_LIMIT(p_scope, p_varname)

#define FB_DSTAT_INC(p_varname) FB_DEBUG_STATS_INC(p_varname)
#define FB_DSTAT_DEC(p_varname) FB_DEBUG_STATS_DEC(p_varname)

#define FB_DSTAT_ADD(p_varname, p_value) FB_DEBUG_STATS_ADD(p_varname, p_value)
#define FB_DSTAT_SUB(p_varname, p_value) FB_DEBUG_STATS_SUB(p_varname, p_value)

#define FB_DSTAT_GET(p_varname) FB_DEBUG_STATS_GET(p_varname)
#define FB_DSTAT_SET(p_varname, p_value) FB_DEBUG_STATS_SET(p_varname, p_value)

#define FB_DSTAT_ALLOC_INC(p_varname) FB_DEBUG_STATS_ALLOC_INC(p_varname)
#define FB_DSTAT_ALLOC_DEC(p_varname) FB_DEBUG_STATS_ALLOC_DEC(p_varname)

#define FB_DSTAT_PEAK(p_peakvar, p_currentvar) FB_DSTAT_SET(p_peakvar, FB_DSTAT_GET(p_currentvar) > FB_DSTAT_GET(p_peakvar) ? FB_DSTAT_GET(p_currentvar) : FB_DSTAT_GET(p_peakvar))



// ------------------------------------------------------------------------------

/* Note: Unless compiler is total n00b, it should be able to optimize based on const bool has_limit 
 * variable. Even if not (e.g. unoptimized debug build), branch prediction should be easy. */

#if (FB_MEMORY_DEBUG_STATS_ENABLED == FB_TRUE)
	#define FB_DSTAT_UPDATE_BADNESS_LIMITS(p_scope, p_varname, p_low_limit, p_high_limit) \
		fb::memory::stats::DebugStats::getInstance()->setBadnessLimits(DynamicString(#p_scope), DynamicString(#p_varname), p_low_limit, p_high_limit)

	#define FB_DSTAT_SET_BADNESS_LIMITS(p_scope, p_varname, p_low_limit, p_high_limit) \
		static const bool debug_stats_var_##p_varname##_badness_limits_set = FB_DSTAT_UPDATE_BADNESS_LIMITS(p_scope, p_varname, p_low_limit, p_high_limit)

	#define FB_DEBUG_STATS_DEFAULT_MAX_ALLOC_LIMIT 10000

	#define FB_DEBUG_STATS_ALLOC_VAR_WITH_LIMIT(p_scope, p_varname, p_min_limit, p_max_limit, p_has_limit) \
		static const bool debug_stats_var_##p_varname##_has_limit = p_has_limit; \
		static bool debug_stats_var_##p_varname##_has_breakpoint = false; \
		static fb::memory::stats::DebugStatsVariableImplementingType &p_varname##AllocStaticInit() \
		{ \
			FB_STATIC_CONST_STRING(scope, #p_scope); \
			FB_STATIC_CONST_STRING(varName, #p_varname); \
			FB_STATIC_CONST_STRING(fullName, #p_scope "::" #p_varname); \
			fb::memory::stats::DebugStats *statsInst = fb::memory::stats::DebugStats::getInstance(); \
			fb::memory::stats::DebugStatsVariableImplementingType &var = statsInst->allocateVariable(fullName); \
			statsInst->defineScope(scope); \
			statsInst->defineAllocationVariable(varName, var, scope, &debug_stats_var_##p_varname##_has_breakpoint); \
			return var; \
		} \
		static fb::memory::stats::DebugStatsVariableImplementingType &get##DebugStatsVar_##p_varname() \
		{ \
			static fb::memory::stats::DebugStatsVariableImplementingType& var = p_varname##AllocStaticInit(); \
			return var; \
		} \
		static const fb::memory::stats::DebugStatsVariableImplementingType& get##DebugStatsVar_##p_varname##MinLimit() \
		{ \
			static fb::memory::stats::DebugStatsVariableImplementingType var(p_min_limit); \
			return var; \
		} \
		static const fb::memory::stats::DebugStatsVariableImplementingType& get##DebugStatsVar_##p_varname##MaxLimit() \
		{ \
			static fb::memory::stats::DebugStatsVariableImplementingType var(p_max_limit); \
			return var; \
		} \
		/* This is just to make initialization happen on startup, instead of first access. Initialization can't fail */ \
		static const FB_UNUSED_NAMED_VAR(bool, debug_stats_var_##p_varname##_init_complete) = get##DebugStatsVar_##p_varname() != 39359604689;

	#define FB_DEBUG_STATS_ARRAY_VAR_NO_LIMIT(p_scope, p_varname, p_arrayelements) \
		static fb::memory::stats::DebugStatsVariableImplementingType* p_varname##ArrayStaticInit() \
		{ \
			FB_STATIC_CONST_STRING(fullName, #p_scope "::" #p_varname); \
			FB_STATIC_CONST_STRING(scope, #p_scope); \
			FB_STATIC_CONST_STRING(varName, #p_varname); \
			fb::memory::stats::DebugStats *statsInst = fb::memory::stats::DebugStats::getInstance(); \
			fb::memory::stats::DebugStatsVariableImplementingType* var = statsInst->allocateVariableArray(fullName, p_arrayelements); \
			statsInst->defineScope(scope); \
			statsInst->defineArrayVariable(varName, p_arrayelements, var, scope); \
			return var; \
		} \
		static fb::memory::stats::DebugStatsArrayVariableImplementingType &get##DebugStatsVar_##p_varname() \
		{ \
			static fb::memory::stats::DebugStatsArrayVariableImplementingType var(p_varname##ArrayStaticInit(), p_arrayelements); \
			return var; \
		} \
		/* This is just to make initialization happen on startup, instead of first access. Initialization can't fail */ \
		static const FB_UNUSED_NAMED_VAR(bool, debug_stats_var_##p_varname##_init_complete) = get##DebugStatsVar_##p_varname().getSize() != 3935904689;

	#define FB_DEBUG_STATS_ALLOC_VAR(p_scope, p_varname) \
		FB_DEBUG_STATS_ALLOC_VAR_WITH_LIMIT(p_scope, p_varname, 0, 10000, true)

	#define FB_DEBUG_STATS_ALLOC_VAR_NO_LIMIT(p_scope, p_varname) \
		FB_DEBUG_STATS_ALLOC_VAR_WITH_LIMIT(p_scope, p_varname, 0, -1, false)

	#define FB_DEBUG_STATS_VAR_WITH_LIMIT(p_scope, p_varname, p_min_limit, p_max_limit, p_has_limit) \
		static const FB_UNUSED_NAMED_VAR(bool, debug_stats_var_##p_varname##_has_limit) = p_has_limit; \
		static FB_UNUSED_NAMED_VAR(bool, debug_stats_var_##p_varname##_has_breakpoint) = false; \
		static fb::memory::stats::DebugStatsVariableImplementingType& p_varname##StaticInit() \
		{ \
			FB_STATIC_CONST_STRING(scope, #p_scope); \
			FB_STATIC_CONST_STRING(varName, #p_varname); \
			FB_STATIC_CONST_STRING(fullName, #p_scope "::" #p_varname); \
			fb::memory::stats::DebugStats *statsInst = fb::memory::stats::DebugStats::getInstance(); \
			static fb::memory::stats::DebugStatsVariableImplementingType& var = statsInst->allocateVariable(fullName); \
			statsInst->defineScope(scope); \
			statsInst->defineVariable(varName, var, scope, &debug_stats_var_##p_varname##_has_breakpoint); \
			return var; \
		} \
		static fb::memory::stats::DebugStatsVariableImplementingType &get##DebugStatsVar_##p_varname() \
		{ \
			static fb::memory::stats::DebugStatsVariableImplementingType& var = p_varname##StaticInit(); \
			return var; \
		} \
		static const fb::memory::stats::DebugStatsVariableImplementingType &get##DebugStatsVar_##p_varname##MinLimit() \
		{ \
			static fb::memory::stats::DebugStatsVariableImplementingType var(p_min_limit); \
			return var; \
		} \
		static const fb::memory::stats::DebugStatsVariableImplementingType &get##DebugStatsVar_##p_varname##MaxLimit() \
		{ \
			static fb::memory::stats::DebugStatsVariableImplementingType var(p_max_limit); \
			return var; \
		} \
		/* This is just to make initialization happen on startup, instead of first access. Initialization can't fail */ \
		static const FB_UNUSED_NAMED_VAR(bool, debug_stats_var_##p_varname##_init_complete) = get##DebugStatsVar_##p_varname() != 39359604689;

	#define FB_DEBUG_STATS_VAR(p_scope, p_varname) \
		FB_DEBUG_STATS_VAR_WITH_LIMIT(p_scope, p_varname, 0, -1, true)

	#define FB_DEBUG_STATS_VAR_NO_LIMIT(p_scope, p_varname) \
		FB_DEBUG_STATS_VAR_WITH_LIMIT(p_scope, p_varname, 0, -1, false)

	#define FB_DEBUG_STATS_ALLOC_INC(p_varname) \
		{ \
			++get##DebugStatsVar_##p_varname(); \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been increased to maximum value. If this is a valid case, define the variable with a higher max value. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() <= get##DebugStatsVar_##p_varname##MaxLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_ALLOC_DEC(p_varname) \
		{ \
			--get##DebugStatsVar_##p_varname(); \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been decreased to minimum value. FB_DEBUG_STATS_ALLOC_INC / FB_DEBUG_STATS_ALLOC_DEC calls out of sync. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() >= get##DebugStatsVar_##p_varname##MinLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_INC(p_varname) \
		{ \
			++get##DebugStatsVar_##p_varname(); \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been increased to maximum value. If this is a valid case, define the variable with a higher max value or no limits. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() <= get##DebugStatsVar_##p_varname##MaxLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_DEC(p_varname) \
		{ \
			--get##DebugStatsVar_##p_varname(); \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been decreased to minimum value. If this is a valid case, define the variable with a lower min value or no limits. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() >= get##DebugStatsVar_##p_varname##MinLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_ADD(p_varname, p_value) \
		{ \
			get##DebugStatsVar_##p_varname() += p_value; \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been increased to maximum value. If this is a valid case, define the variable with a higher max value or no limits. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() <= get##DebugStatsVar_##p_varname##MaxLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_SUB(p_varname, p_value) \
		{ \
			get##DebugStatsVar_##p_varname() -= p_value; \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been decreased to minimum value. If this is a valid case, define the variable with a lower min value or no limits. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() >= get##DebugStatsVar_##p_varname##MinLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_SET(p_varname, p_value) \
		{ \
			get##DebugStatsVar_##p_varname() = p_value; \
			if (debug_stats_var_##p_varname##_has_limit) \
			{ \
				fb_assert(#p_varname " has been set above maximum value. If this is a valid case, define the variable with a higher max value or no limits. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() <= get##DebugStatsVar_##p_varname##MaxLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				fb_assert(#p_varname " has been set below minimum value. If this is a valid case, define the variable with a lower min value or no limits. Further asserts about this will be surpressed." && debug_stats_assert_impl(get##DebugStatsVar_##p_varname() >= get##DebugStatsVar_##p_varname##MinLimit() || get##DebugStatsVar_##p_varname##MaxLimit() < get##DebugStatsVar_##p_varname##MinLimit())); \
				noMoreDebugStatsAssertPersistent = noMoreDebugStatsAssertTemp; \
			} \
		}

	#define FB_DEBUG_STATS_GET(p_varname) \
		(get##DebugStatsVar_##p_varname())

	#define FB_DEBUG_STATS_ARRAY_INC(p_varname, p_elementnum) \
		++get##DebugStatsVar_##p_varname()[p_elementnum];

	#define FB_DEBUG_STATS_ARRAY_DEC(p_varname, p_elementnum) \
		--get##DebugStatsVar_##p_varname()[p_elementnum];

	#define FB_DEBUG_STATS_ARRAY_SET(p_varname, p_elementnum, p_value) \
		get##DebugStatsVar_##p_varname()[p_elementnum] = p_value;

	#define FB_DEBUG_STATS_ARRAY_GET(p_varname, p_elementnum) \
		get##DebugStatsVar_##p_varname()[p_elementnum]

	#define FB_DEBUG_STATS_CHECK_SCOPE_FOR_ALLOC_EMPTINESS(p_scope) \
		fb::memory::stats::DebugStats::getInstance()->checkDebugScopeForEmptiness(fb::StaticString(#p_scope));

	#define FB_DEBUG_STATS_LIBRARY() \
		extern fb::memory::stats::DebugStats *debugStatsMainInstance; \
		extern int initDebugStatsMain(); \
		extern int debug_stats_main_initialized; \
		static int initDebugStats() \
		{ \
			if (debug_stats_main_initialized == 0) \
			{ \
				debug_stats_main_initialized = initDebugStatsMain(); \
			} \
			fb::memory::stats::DebugStats::setInstanceForLibrary(debugStatsMainInstance); \
			return 1; \
		} \
		static int debug_stats_initialized = initDebugStats(); \
		;

	#define FB_DEBUG_STATS_MAIN() \
		fb::memory::stats::DebugStats *debugStatsMainInstance = NULL; \
		extern int debug_stats_main_initialized; \
		int initDebugStatsMain() \
		{ \
			if (debug_stats_main_initialized) \
			{ \
				return 1; \
			} \
			fb::memory::stats::DebugStats::createInstance(); \
			debugStatsMainInstance = fb::memory::stats::DebugStats::getInstance(); \
			debug_stats_main_initialized = 1; \
			return 1; \
		} \
		int debug_stats_main_initialized = initDebugStatsMain(); \
		;

#elif (FB_MEMORY_DEBUG_STATS_ENABLED == FB_FALSE)
	#define FB_DSTAT_UPDATE_BADNESS_LIMITS(p_scope, p_varname, p_low_limit, p_high_limit)
	#define FB_DSTAT_SET_BADNESS_LIMITS(p_scope, p_varname, p_low_limit, p_high_limit)
	#define FB_DEBUG_STATS_ALLOC_VAR_WITH_LIMIT(p_scope, p_varname, p_min_limit, p_max_limit, p_has_limit)
	#define FB_DEBUG_STATS_ALLOC_VAR_NO_LIMIT(p_scope, p_varname)
	#define FB_DEBUG_STATS_ALLOC_VAR(p_scope, p_varname)
	#define FB_DEBUG_STATS_ALLOC_INC(p_varname)
	#define FB_DEBUG_STATS_ALLOC_DEC(p_varname)
	#define FB_DEBUG_STATS_ARRAY_VAR_NO_LIMIT(p_scope, p_varname, p_arrayelements)
	#define FB_DEBUG_STATS_VAR_WITH_LIMIT(p_scope, p_varname, p_min_limit, p_max_limit)
	#define FB_DEBUG_STATS_VAR_NO_LIMIT(p_scope, p_varname)
	#define FB_DEBUG_STATS_VAR(p_scope, p_varname)
	#define FB_DEBUG_STATS_ARRAY_VAR(p_scope, p_varname, p_arrayelements)
	#define FB_DEBUG_STATS_INC(p_varname)
	#define FB_DEBUG_STATS_DEC(p_varname)
	#define FB_DEBUG_STATS_ADD(p_varname, p_value)
	#define FB_DEBUG_STATS_SUB(p_varname, p_value)
	#define FB_DEBUG_STATS_SET(p_varname, p_value)
	#define FB_DEBUG_STATS_GET(p_varname) 0
	#define FB_DEBUG_STATS_ARRAY_INC(p_varname, p_elementnum)
	#define FB_DEBUG_STATS_ARRAY_DEC(p_varname, p_elementnum)
	#define FB_DEBUG_STATS_ARRAY_SET(p_varname, p_elementnum, p_value)
	#define FB_DEBUG_STATS_ARRAY_GET(p_varname, p_elementnum)
	#define FB_DEBUG_STATS_CHECK_SCOPE_FOR_ALLOC_EMPTINESS(p_scope)
	#define FB_DEBUG_STATS_LIBRARY()
	#define FB_DEBUG_STATS_MAIN()
#else
	#error "FB_MEMORY_DEBUG_STATS_ENABLED value invalid."
#endif


class DebugStatsImpl;
class DebugStatsVariable;

/**
 * This debug stats class is mostly intended for use of object allocation counting.
 * It can be used to get the amount of object allocations for debug printing or checked for leaks.
 * It can also be used for other debug variable purposes than allocations.
 *
 * NOTE: Generally, you want to use this with the macros, not by using the class directly.
 */
class DebugStats
{
private:
	DebugStats();

public:
	~DebugStats();
	/**
	 * Returns the singleton instance of the debug stats.
	 *
	 * This call should return you the static instance inside the actual main application, or it will assert if you 
	 * are trying to use it inside a library for which the DebugStats has not been properly initialized.
	 * Use the initializeInstance inside the library to do it.
	 */
	static DebugStats *getInstance();

	/**
	 * Creates a new singleton instance, this should be called by the mani application, before use of the debug stats.
	 * Also, after calling this, you wan't to call the setInstanceForLibrary for any library with the instance
	 * created in the main program. (this allows use of the same static instance for all libraries)
	 */
	static void createInstance();

	/**
	 * Clean up the singleton. Make sure you set the library instances to null after this one.
	 */
	static void cleanInstance();

	/**
	 * Should be called before static uninitialization.
	 */
	static void stopped();

	/**
	 * This should be called by any library, with the proper debug stats instance received from the main application.
	 */
	static void setInstanceForLibrary(DebugStats *instance);

	/**
	 * Required to be called once per tick/frame/... if breakpoint functionality is to be fully supported.
	 */
	void update();

	void defineScope(const StaticString &scopeName);
	DebugStatsVariable *defineVariable(const StaticString &name, DebugStatsVariableImplementingType &variable, const StaticString &scopeName, bool *breakpointFlag);
	DebugStatsVariable *defineArrayVariable(const StaticString &name, SizeType arrayElements, DebugStatsVariableImplementingType *variable, const StaticString &scopeName);
	DebugStatsVariable *defineAllocationVariable(const StaticString &name, DebugStatsVariableImplementingType &variable, const StaticString &scopeName, bool *breakpointFlag);

	// NOTE: use the macros instead of using these directly!
	// NOTE: these are not thread safe.
	// Note, instead of using these, now the macros now use the variables directly unless
	// a variable has a breakpoint (to avoid some static uninitialization asserts) 
	void increaseVariable(DebugStatsVariableImplementingType &variable, DebugStatsVariable *varInDebugStatsScope);
	void decreaseVariable(DebugStatsVariableImplementingType &variable, DebugStatsVariable *varInDebugStatsScope);
	void setVariable(DebugStatsVariableImplementingType &variable, int64_t value, DebugStatsVariable *varInDebugStatsScope);
	/* Allow setting from unsigned too */
	void setVariable(DebugStatsVariableImplementingType &variable, uint64_t value, DebugStatsVariable *varInDebugStatsScope);
	int64_t getVariable(DebugStatsVariableImplementingType &variable, DebugStatsVariable *varInDebugStatsScope);

	/**
	 * Checks the scope with given name for emptiness. (and any other sub-scope that starts with "<given scopeName here>::..."
	 * All the allocation variables within the scope are asserted to be zero.
	 */
	void checkDebugScopeForEmptiness(const DynamicString &scopeName);

	/**
	 * Notice, the returned iterator/iterator values may be validated by any non-const debug stats calls...
	 */
	DebugStatsScopeIterator getDebugScopeIterator();

	/**
	 * Notice, the returned iterator/iterator values may be validated by any non-const debug stats calls...
	 */
	DebugStatsVariableIterator getDebugStatsVariableIterator(const DynamicString &scopeName);

	/**
	 * @return scope with given name or NULL if no such scope exists.
	 */
	DebugStatsScope *findScopeByName(const DynamicString &scopeName);

	/**
	 * Returns a pointer to the "internal" representation of a variable, for the given
	 * scope/variable name. NULL may be returned if the variable does not exist.
	 */
	DebugStatsVariable *findDebugStatsVariableByName(const DynamicString &scopeName, const DynamicString &variableName);

	DebugStatsVariableImplementingType &allocateVariable(const StaticString &name);
	DebugStatsVariableImplementingType *allocateVariableArray(const StaticString &name, SizeType arraySize);
	bool setBadnessLimits(const DynamicString &scopeName, const DynamicString &variableName, DebugStatsVariableImplementingType lowLimit, DebugStatsVariableImplementingType highLimit);

private:
	DebugStatsImpl *impl;

	friend class DebugStatsScopeIterator;
};

FB_END_PACKAGE2()
