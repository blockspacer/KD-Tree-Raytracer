#pragma once

#include "fb/math/Denormal.h"

FB_PACKAGE2(math, util)

/* TODO: Should optimize, if this is ever used to anything that requires performance (debugging doesn't count). */
template <typename F> 
static FB_FORCEINLINE bool isDenormal(F f)
{
	return fb::math::Denormal::isDenormal(f);
}

FB_END_PACKAGE2()
