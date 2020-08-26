#pragma once

#include "fb/lang/platform/Compiler.h"

/* Don't disable warnings here, if you just have some 3rd party code that isn't up to our standard. Just use
 * #pragma warning(push) 
 * [ Insert comment here like below ]
 * #pragma warning(disable: 1234)
 * #include "CrappyLib.h"
 * #pragma warning(pop)*/

#ifdef _MSC_VER
	/* 4061 could be enabled, but could make some people unhappy */
	#pragma warning( disable: 4061 ) // enumerator in switch of enum is not explicitly handled by a case label
	#pragma warning( disable: 4068 ) // Unrecognized pragma
	/* 4100 could be enabled, if anyone bothered to */
	#pragma warning( disable: 4100 ) // '...' : unreferenced formal parameter
	#pragma warning( disable: 4127 ) // conditional expression is constant
	/* 4189 can be enabled in VS 2017. See https://docs.microsoft.com/fi-fi/cpp/cpp/attributes2 */
	#pragma warning( disable: 4189 ) // local variable is initialized but not referenced
	#pragma warning( disable: 4201 ) // nonstandard extension used : nameless struct/union
	#pragma warning( disable: 4371 ) // layout of class may have changed from a previous version of the compiler due to better packing of member
	#pragma warning( disable: 4472 ) // X is a native enum: add an access specifier (private/public) to declare a WinRT enum
	#pragma warning( disable: 4505 ) // unreferenced local function has been removed
	#pragma warning( disable: 4512 ) // assignment operator could not be generated
	#pragma warning( disable: 4514 ) // unreferenced inline function has been removed
	#pragma warning( disable: 4623 ) // 'X': default constructor was implicitly defined as deleted
	#pragma warning( disable: 4625 ) // copy constructor could not be generated because a base class copy constructor is inaccessible
	#pragma warning( disable: 4626 ) // assignment operator could not be generated because a base class assignment operator is inaccessible
	#pragma warning( disable: 4710 ) // function not inlined
	#pragma warning( disable: 4711 ) // function selected for automatic inline expansion
	#pragma warning( disable: 4714 ) // variable marked as __forceinline not inlined
	#pragma warning( disable: 4738 ) // storing 32-bit float result in memory, possible loss of performance
	#pragma warning( disable: 4820 ) // X bytes padding added after data member
	#pragma warning( disable: 4987 ) // nonstandard extension used
	#pragma warning( disable: 4996 ) // 'GetVersionExA': was declared deprecated
	#pragma warning( disable: 5026 ) // move constructor was implicitly defined as deleted
	#pragma warning( disable: 5027 ) // move assignment operator was implicitly defined as deleted


	#if FB_VS2017_IN_USE == FB_TRUE
		#pragma warning( disable: 4577 ) // 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed. Specify /EHsc
		#pragma warning( disable: 5045 ) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
	#endif

#endif
