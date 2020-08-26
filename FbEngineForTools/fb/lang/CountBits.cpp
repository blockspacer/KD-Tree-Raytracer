#include "Precompiled.h"
#include "CountBits.h"

FB_PACKAGE1(lang)

#if FB_POPCNT_ENABLED == FB_TRUE
bool checkPopcntSupport()
{
	int info[4] = {0};
	__cpuid(info, 0x00000001);
	bool supported = (info[2] & (1<<23)) != 0;
	return supported;
}
#endif

FB_END_PACKAGE1()

