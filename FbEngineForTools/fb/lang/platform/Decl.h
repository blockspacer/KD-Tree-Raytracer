#pragma once

// ------------------------------------------------------------------------------
// platfor specific declares

#if (FB_COMPILER == FB_MSC)

	#define FB_RESTRICT __restrict
	#define FB_FUNCTION_RESTRICT __declspec(restrict) 
	#define FB_ALIGN_4(p_vardef) __declspec(align(4)) p_vardef
	#define FB_ALIGN_8(p_vardef) __declspec(align(8)) p_vardef
	#define FB_ALIGN_16(p_vardef) __declspec(align(16)) p_vardef
	#define FB_ALIGN_32(p_vardef) __declspec(align(32)) p_vardef
	#define FB_THREAD_LOCAL_STORAGE(p_vardef) __declspec(thread) p_vardef
	#define FB_FUNCTION_DEPRECATED __declspec(deprecated)
	#define FB_UNUSED_VAR(p_type, ...) FB_UNUSED_NAMED_VAR(p_type, unusedVariableXYZ_##__VA_ARGS__)
	#define FB_UNUSED_NAMED_VAR(p_type, p_name) p_type p_name
	#define FB_ENUM_UNSIGNED : unsigned
	
#elif (FB_COMPILER == FB_ICC)

	#define FB_RESTRICT __restrict
	#define FB_FUNCTION_RESTRICT __declspec(restrict) 
	#define FB_ALIGN_4(p_vardef) __declspec(align(4)) p_vardef
	#define FB_ALIGN_8(p_vardef) __declspec(align(8)) p_vardef
	#define FB_ALIGN_16(p_vardef) __declspec(align(16)) p_vardef
	#define FB_ALIGN_32(p_vardef) __declspec(align(32)) p_vardef
	#define FB_THREAD_LOCAL_STORAGE(p_vardef) __declspec(thread) p_vardef
	#define FB_FUNCTION_DEPRECATED __declspec(deprecated)
	#define FB_UNUSED_VAR(p_type, ...) FB_UNUSED_NAMED_VAR(p_type, unusedVariableXYZ_##__VA_ARGS__)
	#define FB_UNUSED_NAMED_VAR(p_type, p_name) p_type p_name
	#define FB_ENUM_UNSIGNED : unsigned

#elif (FB_COMPILER == FB_CLANG)

#ifndef __INTELLISENSE__
	#define FB_RESTRICT __restrict__
#else
	#define FB_RESTRICT
#endif

	#define FB_FUNCTION_RESTRICT
	#define FB_ALIGN_4(p_vardef) p_vardef __attribute__ ((aligned(4)))
	#define FB_ALIGN_8(p_vardef) p_vardef __attribute__ ((aligned(8)))
	#define FB_ALIGN_16(p_vardef) p_vardef __attribute__ ((aligned(16)))
	#define FB_ALIGN_32(p_vardef) p_vardef __attribute__ ((aligned(32)))
	#define FB_THREAD_LOCAL_STORAGE(p_vardef) __thread p_vardef
	#define FB_FUNCTION_DEPRECATED __attribute__((__deprecated__))
	#define FB_UNUSED_VAR(p_type, ...) FB_UNUSED_NAMED_VAR(p_type, unusedVariableXYZ_##__VA_ARGS__)
	#define FB_UNUSED_NAMED_VAR(p_type, p_name) p_type p_name __attribute__((unused))
	#define FB_ENUM_UNSIGNED : unsigned

#elif (FB_COMPILER == FB_GNUC)

	#define FB_RESTRICT __restrict__
	#define FB_FUNCTION_RESTRICT
	#define FB_ALIGN_4(p_vardef) p_vardef __attribute__ ((aligned(4)))
	#define FB_ALIGN_8(p_vardef) p_vardef __attribute__ ((aligned(8)))
	#define FB_ALIGN_16(p_vardef) p_vardef __attribute__ ((aligned(16)))
	#define FB_ALIGN_32(p_vardef) p_vardef __attribute__ ((aligned(32)))
	#define FB_THREAD_LOCAL_STORAGE(p_vardef) __thread p_vardef
	#define FB_FUNCTION_DEPRECATED __attribute__((__deprecated__))
	#define FB_UNUSED_VAR(p_type, ...) FB_UNUSED_NAMED_VAR(p_type, unusedVariableXYZ_##__VA_ARGS__)
	#define FB_UNUSED_NAMED_VAR(p_type, p_name) p_type p_name __attribute__((unused))
	#define FB_ENUM_UNSIGNED : unsigned

#else

	#error "Unsupported compiler."

#endif


// ------------------------------------------------------------------------------
