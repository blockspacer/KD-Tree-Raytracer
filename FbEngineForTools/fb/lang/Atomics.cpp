#include "Precompiled.h"
#include "Atomics.h"

#include "fb/lang/platform/CompilerOptimizations.h"
#include "fb/lang/platform/Platform.h"

FB_PACKAGE1(lang)

void atomicThreadPause()
{
	// For slightly less disastrous spin loops
	_mm_pause();
}

FB_END_PACKAGE1()
