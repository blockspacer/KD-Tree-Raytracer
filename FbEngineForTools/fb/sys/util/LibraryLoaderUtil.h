#pragma once


#define FB_LOAD_DYNAMIC_FUNC(p_funcVar, p_libName, p_funcName, p_returnType, p_signature, p_errorMsg, ...) \
	typedef p_returnType (WINAPI *p_signature) (__VA_ARGS__); \
	static p_signature p_funcVar = nullptr; \
	if (p_funcVar == nullptr) \
	{ \
		HMODULE module = GetModuleHandle(p_libName); \
		if (module == nullptr) \
		{ \
			module = LoadLibrary(p_libName); \
		} \
		if (module != nullptr) \
		{ \
			/* Cast indirectly via void* to avoid compiler warning */ \
			void *function_ptr_temp = ::GetProcAddress(module, p_funcName); \
			p_funcVar = reinterpret_cast<p_signature>(function_ptr_temp); \
			if (p_funcVar == nullptr) \
			{ \
				FB_PRINTF("%sFailed to load %s\n", p_errorMsg, p_funcName); \
			} \
		} \
		else \
		{ \
			FB_PRINTF("%sFailed to load %s\n", p_errorMsg, p_libName); \
		} \
	} \
