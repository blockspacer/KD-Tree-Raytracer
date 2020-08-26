#pragma once

#include "fb/lang/platform/IncludeIntrin.h"
#include "fb/lang/platform/Sel.h"

#if FB_SIMD_SSE == FB_TRUE

	#define FB_FABS(x) _mm_cvtss_f32(_mm_andnot_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), toVec(x)))
	#define FB_FRESE(x) _mm_cvtss_f32(_mm_rcp_ss(toVec(x)))
	#define FB_FRESSQRTE(x) _mm_cvtss_f32(_mm_rsqrt_ss(toVec(x)))
	#define FB_FSQRTE(x) _mm_cvtss_f32(_mm_rcp_ss(_mm_rsqrt_ss(toVec(x))))
	#define FB_FMAX(x, y) _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(x), toVec(y)))
	#define FB_FMIN(x, y) _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(x), toVec(y)))

#elif FB_SIMD_NEON == FB_TRUE

	#define FB_FABS(x) __builtin_fabsf(x)
	#define FB_FRESE(x) (vrecpes_f32(x))
	#define FB_FRESSQRTE(x) (vrsqrtes_f32(x))
	#define FB_FSQRTE(f) (vrecpes_f32(vrsqrtes_f32(f)))
	#define FB_FMAX(a, b) (__builtin_fmaxf(a, b))
	#define FB_FMIN(a, b) (__builtin_fminf(a, b))

#else

	#define FB_FRESE(x) (1.0f/(x))
	#define FB_FRESSQRTE(x) (1.0f/::sqrtf(x))
	#define FB_FSQRTE(f) (::sqrtf(f))
	#define FB_FMAX(a, b) FB_FSEL((a)-(b), a, b)
	#define FB_FMIN(a, b) FB_FSEL((a)-(b), b, a)

#endif

// And some "nice to use" stuff implemented with above

#ifndef FB_FABS
#define FB_FABS(r) FB_FSEL(r, r, -(r))
#endif

#define FB_FCLAMP(v, minv, maxv) FB_FMIN(maxv, FB_FMAX(minv, v))

#define FB_DABS(r) FB_DSEL(r, r, -(r))
#define FB_DMAX(a, b) FB_DSEL((a)-(b), a, b)
#define FB_DMIN(a, b) FB_DSEL((a)-(b), b, a)
#define FB_DCLAMP(v, minv, maxv) FB_DMIN(maxv, FB_DMAX(minv, v))
