#include "Precompiled.h"
#include "FBSingleThreadAssert.h"

#include "fb/lang/thread/Thread.h"

/* FIXME: All this should be made part of Thread */

bool g_single_thread_phase = false;
bool g_single_thread_static_init_complete = false;
bool g_single_thread_assert_inited = false;
bool g_single_thread_assert_running_thread_count = 0;

void fb_impl_single_thread_assert_enter_single_thread_phase()
{
	fb_assert(g_single_thread_assert_inited);
	fb_assert(g_single_thread_assert_running_thread_count == 1);
	fb_assert(!g_single_thread_phase); 
	g_single_thread_phase = true;
}

void fb_impl_single_thread_assert_exit_single_thread_phase()
{
	fb_assert(g_single_thread_assert_inited);
	fb_assert(g_single_thread_assert_running_thread_count == 1);
	fb_assert(g_single_thread_phase); 
	g_single_thread_phase = false;
}

void fb_impl_single_thread_assert_init()
{
	fb_assert(!g_single_thread_assert_inited);
	g_single_thread_assert_inited = true;
	g_single_thread_static_init_complete = true;
	g_single_thread_phase = true;
	g_single_thread_assert_running_thread_count = 1;
}

#if (FB_SINGLE_THREAD_ASSERT_ENABLED == FB_TRUE)
void fb_main_thread_assert()
{
	fb_assert(fb::Thread::getCurrentThreadId() == fb::Thread::getMainThreadId());
}
#endif

void fb_impl_single_thread_assert_uninit()
{
	fb_assert(g_single_thread_assert_inited);
	fb_assert(g_single_thread_assert_running_thread_count == 1);
	g_single_thread_assert_inited = false;
	// HACK: removed this to prevent atexit single_thread asserts.
	//g_single_thread_phase = false;
}

	
	
