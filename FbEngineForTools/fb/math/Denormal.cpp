#include "Precompiled.h"
#include "Denormal.h"

#include "fb/lang/FBAssert.h"
#include "fb/lang/FBStaticAssert.h"

#include <float.h>

FB_PACKAGE1(math);

bool Denormal::initFlushingDenormalsToZero()
{
	unsigned int current_word = 0;
	int result = _controlfp_s(&current_word, _DN_FLUSH, _MCW_DN);
	fb_assert(result == 0 && "Something went wrong with floating point denormal magic");
	return true;
}

const bool Denormal::flushingDenormalsToZeroInitialized = initFlushingDenormalsToZero();

FB_END_PACKAGE1();
