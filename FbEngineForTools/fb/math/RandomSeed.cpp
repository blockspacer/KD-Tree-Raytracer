#include "Precompiled.h"

#include "RandomSeed.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/lang/thread/MutexGuard.h"

#pragma warning(push)
/* 'argument': conversion from 'XXX' to 'YYY', signed/unsigned mismatch */
#pragma warning(disable: 4365)
/* 4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc */
#pragma warning(disable: 4530)
/* C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught */
#pragma warning(disable: 4571)
#if FB_VS2017_IN_USE == FB_TRUE
	/* C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal */
#pragma warning(disable: 4774)
#endif

#if FB_COMPILER == FB_MSC && FB_BUILD == FB_DEBUG
/* 4548: Expression before comma has no effect; expected expression with side-effect */
#pragma warning(disable: 4548)
#endif

#include <random>
#pragma warning(pop)

FB_PACKAGE1(math)

struct RandomDevice
{
	Mutex mutex;
	std::random_device randomDevice;
};

static RandomDevice &getRandomDevice()
{
	static RandomDevice randomDevice;
	return randomDevice;
}

uint32_t generateRandomSeed32()
{
	RandomDevice &device = getRandomDevice();
	MutexGuard mg(device.mutex);
	return device.randomDevice();
}

uint64_t generateRandomSeed64()
{
	RandomDevice &device = getRandomDevice();
	MutexGuard mg(device.mutex);
	uint64_t result = device.randomDevice();
	result <<= 32;
	result |= device.randomDevice();
	return result;
}

FB_END_PACKAGE1()