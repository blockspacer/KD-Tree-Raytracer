#pragma once

#include "fb/lang/platform/Platform.h"
#include "fb/lang/platform/Compiler.h"

// Some SIMD / SSE stuff

#ifndef FB_USE_SIMD
	#error "FB_USE_SIMD should be defined in PostGlobalConfigMath.h"
#endif

#if FB_USE_SIMD == FB_FALSE
	#define FB_SIMD_SSE FB_FALSE
	#define FB_SIMD_NEON FB_FALSE
#else
	#define FB_SIMD_SSE FB_TRUE
	#define FB_SIMD_NEON FB_FALSE
#endif

#if FB_SIMD_NEON == FB_TRUE
#include <arm_neon.h>
#endif

#if FB_SIMD_SSE == FB_TRUE

	#if FB_COMPILER == FB_CLANG
		#include <x86intrin.h>
	#else
		#pragma warning(push)
		/* 4548: Expression before comma has no effect; expected expression with side-effect */
		#pragma warning(disable: 4548)
		#include <intrin.h>
		#include <xmmintrin.h>
		#pragma warning(pop)
	#endif
	#if FB_COMPILER == FB_GNUC
		#include <x86intrin.h>
	#endif

	#if FB_COMPILER == FB_CLANG
		// clang generates some unnecessary instructions with _mm_set_ss(), this union seems to be the only thing that works
		union SSEUnion
		{
		  __m128 vec;
		  float v[4];
		};
		static inline __m128 toVec(float scalar)
		{
			SSEUnion u;
			u.v[0] = scalar;
			return u.vec;
		}
	#else
		#define toVec(scalar) _mm_set_ss(scalar)
	#endif

#endif
