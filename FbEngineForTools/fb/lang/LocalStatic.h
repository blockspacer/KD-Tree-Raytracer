#pragma once

#include "fb/lang/platform/Likely.h"

#define FB_LOCAL_STATIC(p_type, p_name, ...)                \
	thread_local p_type *staticValuePtr_##p_name = nullptr; \
	if (FB_LIKELY(staticValuePtr_##p_name != nullptr))      \
	{                                                       \
	}                                                       \
	else                                                    \
	{                                                       \
		static p_type staticValue_##p_name __VA_ARGS__;     \
		staticValuePtr_##p_name = &staticValue_##p_name;    \
	}                                                       \
	p_type &p_name = *staticValuePtr_##p_name;

/*
// Instead of this:

static State &getState()
{
	static State state;
	return state;
}

// do this:

static State &getState()
{
	FB_LOCAL_STATIC(State, state);
	return state;
}

//
// They do the same thing in an almost identical manner but the former is buggy at least in VS 2015:
//  * If the getState() function gets inlined in several places the local static variable may be instantiated more than once
//
// As a small bonus FB_LOCAL_STATIC may be slightly more efficient on ARM CPUs as it doesn't require an expensive memory fence unlike the default implementation on ARM Clang.
//
*/
