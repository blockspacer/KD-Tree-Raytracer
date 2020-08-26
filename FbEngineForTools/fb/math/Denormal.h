#pragma once

#include "fb/lang/FBAssert.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/NumericLimits.h"

FB_PACKAGE1(math);

class Denormal
{
public:
	template <typename F>
	static bool isDenormal(F f)
	{
		return f != 0.0f && lang::abs(f) < lang::NumericLimits<F>::getMin();
	}

	/* Specialization for floats, since we should have denormals flushed to zero */
	static bool isDenormal(float f)
	{
		fb_expensive_assert(!debugTestForDenormalSupport<float>() && "Seems that denormals aren't flushed to zero");
		return false;
	}

	/* Returns true if denormals are supported for given type */
	template <typename F>
	static bool debugTestForDenormalSupport()
	{
		/* If denormal support is disabled compile time, we just presume no one has turned it back on. The test below 
		 * wouldn't work, as denorm_min() just becomes normal min() if denormals are not supported */
		if (!std::numeric_limits<float>::has_denorm)
			return false;

		/* Note: Denormal values can still be present (for example, the below returns denormal on NX) */
		volatile F denormalMin = std::numeric_limits<F>::denorm_min();
		return denormalMin > F(0);
	}

	static bool initFlushingDenormalsToZero();
	/* If initialization was successful, this returns true. If initialization wasn't done, of failed, this returns false */
	static inline bool isInited() { return flushingDenormalsToZeroInitialized; }

private:
	static const bool flushingDenormalsToZeroInitialized;
};

FB_END_PACKAGE1();
