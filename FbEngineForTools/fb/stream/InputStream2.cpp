#include "Precompiled.h"
#include "InputStream2.h"

FB_PACKAGE0()

bool streamReadFailed(const char *file, int line)
{
	fb_assertf(0, "Stream read failed at %s:%i", file, line);
	return false;
}

FB_END_PACKAGE0()
