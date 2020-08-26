#pragma once

/**
 * Use like this:
 *
 * 
	SomeClass::someFunc()
	{
		uint32_t foo = 32;
		string::TempString bar("BBABAAAAR");
		FB_DEFER(uint32_t, foo, const string::TempString&, bar,
			if (foo > 31)
				FB_PRINTF("RESULT::::: %s: %d\n", bar.getPointer(), foo);
		);
		// Rest of code
		// ...
		// Provided code will be executed when printer drops out of scope
	}
 * 
 */


#include "fb/lang/ScopedDeferrer.h"

#define FB_SCOPED_DEFER_CALL_ARGS_IMPL(p_param_type, p_param_name) p_param_name
#define FB_SCOPED_DEFER_CALL_ARGS_4(p_args) FB_SCOPED_DEFER_CALL_ARGS_IMPL p_args
#define FB_SCOPED_DEFER_CALL_ARGS_3(p_args) FB_SCOPED_DEFER_CALL_ARGS_4(p_args)
#define FB_SCOPED_DEFER_CALL_ARGS_2(p_args) FB_SCOPED_DEFER_CALL_ARGS_3(p_args)
#define FB_SCOPED_DEFER_CALL_ARGS_1(p_args) FB_SCOPED_DEFER_CALL_ARGS_2(p_args)
#define FB_SCOPED_DEFER_CALL_ARGS(...) FB_SCOPED_DEFER_CALL_ARGS_1((__VA_ARGS__))

#define FB_DEFER(...) \
		FB_SCOPED_DEFERRER(FB_PP_CONCAT(DeferStruct, __LINE__), __VA_ARGS__) \
		FB_PP_CONCAT(deferInstance, __LINE__){FB_PP_FOREACH_PAIR_IGNORE_LAST(FB_SCOPED_DEFER_CALL_ARGS, __VA_ARGS__) }

#define FB_DEFER0(p_body) \
		FB_SCOPED_DEFERRER0(FB_PP_CONCAT(DeferStruct, __LINE__), p_body) \
		FB_PP_CONCAT(deferInstance, __LINE__)
