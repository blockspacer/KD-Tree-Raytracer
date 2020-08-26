#pragma once

#include "fb/lang/platform/FBConstants.h"

// Fastmath versions
#define FB_USE_FASTMATH FB_FALSE
#if FB_USE_FASTMATH == FB_TRUE
	#define FB_FASIN(r) f_asinf(r)
	#define FB_FACOS(r) f_acosf(r)
	#define FB_FSIN(r) f_sinf(r)
	#define FB_FCOS(r) f_cosf(r)
	#define FB_FATAN(r) f_atanf(r)
	#define FB_FATAN2(a,b) f_atan2f(a,b)
	#define FB_FTAN(r) f_tanf(r)
	#define FB_FFLOOR(r) f_floorf(r)
	#define FB_FCEIL(r) f_ceilf(r)
	#define FB_FMOD(a,b) f_fmodf(a,b)
	#define FB_FSQRT(r) f_sqrtf(r)
	#define FB_FPOW(a,b) f_powf(a,b)
#else
	#include <math.h>
	#define FB_FASIN(r) ::asinf(r)
	#define FB_FACOS(r) ::acosf(r)
	#define FB_FSIN(r) ::sinf(r)
	#define FB_FCOS(r) ::cosf(r)
	#define FB_FATAN(r) ::atanf(r)
	#define FB_FATAN2(a,b) ::atan2f(a,b)
	#define FB_FTAN(r) ::tanf(r)
	#define FB_FFLOOR(r) ::floorf(r)
	#define FB_FCEIL(r) ::ceilf(r)
	#define FB_FMOD(a,b) ::fmodf(a,b)
	#define FB_FSQRT(r) ::sqrtf(r)
	#define FB_FPOW(a,b) ::powf(a,b)
#endif
