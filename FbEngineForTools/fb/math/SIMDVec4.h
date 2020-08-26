#pragma once

#include "fb/lang/platform/IncludeIntrin.h"
#include "fb/lang/FBStaticAssert.h"

#define FB_SIMDVEC4_REFERENCE_TEST_ENABLED FB_FALSE

#if FB_SIMDVEC4_REFERENCE_TEST_ENABLED == FB_TRUE
	#include "SIMDVec4Tester.h"
	#include "fb/math/SIMDVec4Ref.h"
#endif

#if FB_SIMD_SSE == FB_TRUE

	#define FB_SIMD_VEC4_AVAILABLE FB_TRUE
	#define FB_SIMD_VEC4_NEON_IN_USE FB_FALSE

	// SSE implementation
	#include "SIMDVec4SSE.h"

	FB_PACKAGE1(math)
		#if FB_SIMDVEC4_REFERENCE_TEST_ENABLED == FB_TRUE
			#define FB_SIMD_VEC4_SSE_IN_USE FB_FALSE
			/* allowedErrorOnLength == 10, allowedErrorOnNormalize == 11 */
			typedef SIMDVec4Tester<SIMDVec4SSE, SIMDVec4Ref, 10, 11> SIMDVec4;
		#else
			#define FB_SIMD_VEC4_SSE_IN_USE FB_TRUE
			typedef SIMDVec4SSE SIMDVec4;
		#endif
	FB_END_PACKAGE1()

#elif FB_SIMD_NEON == FB_TRUE

	#define FB_SIMD_VEC4_AVAILABLE FB_TRUE
	#define FB_SIMD_VEC4_SSE_IN_USE FB_FALSE

	// NEON implementation
	#include "SIMDVec4Neon.h"

	FB_PACKAGE1(math)
		#if FB_SIMDVEC4_REFERENCE_TEST_ENABLED == FB_TRUE
			#define FB_SIMD_VEC4_NEON_IN_USE FB_TRUE
			#if FB_NEON_ACCURATE_MODE_ENABLED == FB_TRUE
				/* allowedErrorOnLength == 0, allowedErrorOnNormalize == 0 */
				typedef SIMDVec4Tester<SIMDVec4Neon, SIMDVec4Ref, 0, 0> SIMDVec4;
			#else
				/* allowedErrorOnLength == 7, allowedErrorOnNormalize == 8 */
				typedef SIMDVec4Tester<SIMDVec4Neon, SIMDVec4Ref, 7, 8> SIMDVec4;
			#endif
		#else
			#define FB_SIMD_VEC4_NEON_IN_USE FB_FALSE
			typedef SIMDVec4Neon SIMDVec4;
		#endif
	FB_END_PACKAGE1()

#else

	#define FB_SIMD_VEC4_AVAILABLE FB_FALSE
	#define FB_SIMD_VEC4_NEON_IN_USE FB_FALSE
	#define FB_SIMD_VEC4_SSE_IN_USE FB_FALSE

	/* Reference implementation */
	#include "fb/math/SIMDVec4Ref.h"

	FB_PACKAGE1(math)
		typedef SIMDVec4Ref SIMDVec4;
	FB_END_PACKAGE1()

#endif

FB_PACKAGE1(math)
	static const uint32_t simdAlignment = SIMDVec4::alignment;
	static const uint32_t simdAlignmentMask = SIMDVec4::simdAlignmentMask;
	fb_static_assert(alignof(SIMDVec4) <= simdAlignment);
FB_END_PACKAGE1()


#if FB_SIMD_VEC4_AVAILABLE != FB_TRUE && FB_SIMD_VEC4_AVAILABLE != FB_FALSE
	#error FB_SIMD_VEC4_AVAILABLE not defined or invalid
#endif

#if FB_SIMD_VEC4_NEON_IN_USE != FB_TRUE && FB_SIMD_VEC4_NEON_IN_USE != FB_FALSE
	#error FB_SIMD_VEC4_NEON_IN_USE not defined or invalid
#endif

#if FB_SIMD_VEC4_SSE_IN_USE != FB_TRUE && FB_SIMD_VEC4_SSE_IN_USE != FB_FALSE
	#error FB_SIMD_VEC4_SSE_IN_USE not defined or invalid
#endif
