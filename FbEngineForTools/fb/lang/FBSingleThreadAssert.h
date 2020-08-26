#pragma once

#include "FBAssert.h"
#include "Config.h"

#if (FB_SINGLE_THREAD_ASSERT_ENABLED == FB_TRUE)
	extern bool g_single_thread_phase;

	extern bool g_single_thread_static_init_complete; // the purpose of this is to allow the fb_static_thread_assert in static init phase.

	#define fb_single_thread_assert() if (!g_single_thread_phase && g_single_thread_static_init_complete) { fb_assert(0 && "fb_single_thread_assert triggered."); }
	void fb_main_thread_assert();

	#define FB_SINGLE_THREAD_ASSERT_BEGIN_SINGLE_THREAD_PHASE() fb_impl_single_thread_assert_enter_single_thread_phase()
	#define FB_SINGLE_THREAD_ASSERT_END_SINGLE_THREAD_PHASE() fb_impl_single_thread_assert_exit_single_thread_phase()
	#define FB_SINGLE_THREAD_ASSERT_INIT() fb_impl_single_thread_assert_init()
	#define FB_SINGLE_THREAD_ASSERT_UNINIT() fb_impl_single_thread_assert_uninit()
	#define FB_SINGLE_THREAD_ASSERT_INC_RUNNING_THREAD_COUNT() fb_impl_single_thread_assert_inc_running_thread_count()
	#define FB_SINGLE_THREAD_ASSERT_DEC_RUNNING_THREAD_COUNT() fb_impl_single_thread_assert_dec_running_thread_count()

	// use the provided macros, never use these directly.
	void fb_impl_single_thread_assert_enter_single_thread_phase();
	void fb_impl_single_thread_assert_exit_single_thread_phase();
	void fb_impl_single_thread_assert_init();
	void fb_impl_single_thread_assert_uninit();
	void fb_impl_single_thread_assert_inc_running_thread_count();
	void fb_impl_single_thread_assert_dec_running_thread_count();
#else
	#define fb_single_thread_assert() 
	#define fb_main_thread_assert() 

	#define FB_SINGLE_THREAD_ASSERT_BEGIN_SINGLE_THREAD_PHASE() 
	#define FB_SINGLE_THREAD_ASSERT_END_SINGLE_THREAD_PHASE() 
	#define FB_SINGLE_THREAD_ASSERT_INIT() 
	#define FB_SINGLE_THREAD_ASSERT_UNINIT() 
	#define FB_SINGLE_THREAD_ASSERT_INC_RUNNING_THREAD_COUNT() 
	#define FB_SINGLE_THREAD_ASSERT_DEC_RUNNING_THREAD_COUNT() 
#endif
