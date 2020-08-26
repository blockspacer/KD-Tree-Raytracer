#ifndef FB_LANG_CALLSTACK_H
#define FB_LANG_CALLSTACK_H

#include "fb/lang/platform/Config.h"
#include "fb/lang/platform/FBConstants.h"
#include "fb/lang/platform/Platform.h"
#include "fb/lang/Package.h"

#if FB_EDITOR_ENABLED == FB_TRUE
	#define FB_LANG_CALLSTACK FB_TRUE
#else
	#define FB_LANG_CALLSTACK FB_FALSE
#endif

FB_PACKAGE1(lang)

#if (FB_LANG_CALLSTACK == FB_TRUE)
	namespace detail {
		// Do not use this imp directly, always use the macros!
		void pushIdentifier(const char *id);
		void pushIdentifier(const char *className, const char *id);
		void popIdentifier(const char *className, const char *id);
	}

	struct CallStackImplementationClass
	{
		const char *className;
		const char *idName;
		
		CallStackImplementationClass(const char *id)
		:	className(0),
			idName(id)
		{
			detail::pushIdentifier(id);
		}

		CallStackImplementationClass(const char *className_, const char *id)
		:	className(className_),
			idName(id)
		{
			detail::pushIdentifier(className, id);
		}

		~CallStackImplementationClass()
		{
			release();
		}

		void release()
		{
			if (idName)
			{
				detail::popIdentifier(className, idName);
				idName = 0;
				className = 0;
			}
		}
	};

	// All parameters are assumed to be compile time constants!
	// Also, we are intentionally using constant variable name for scope guard, as you need to bracket your scope
	// usage anyway.

#ifdef FB_UNITY_BUILD
	#define FB_STACK_MERGE2(a,b) a##b
	#define FB_STACK_MERGE(a, b) FB_STACK_MERGE2(a,b)
	/// Set the current class to file scope
	#define FB_STACK_SET_CLASS(className) static const char *FB_STACK_MERGE(FB_LANG_STACK_ACTIVE_CLASS_,FB_PROP_IMPL_USE_CLASSNAME) = #className
	/// Push current method to stack -- requires FB_STACK_SET_CLASS on file scope
	#define FB_STACK_METHOD() fb::lang::CallStackImplementationClass callStackImplementationClass(FB_STACK_MERGE(FB_LANG_STACK_ACTIVE_CLASS_,FB_PROP_IMPL_USE_CLASSNAME), __FUNCTION__)
#else
	/// Set the current class to file scope
	#define FB_STACK_SET_CLASS(className) static const char *FB_LANG_STACK_ACTIVE_CLASS = #className
	/// Push current method to stack -- requires FB_STACK_SET_CLASS on file scope
	#define FB_STACK_METHOD() fb::lang::CallStackImplementationClass callStackImplementationClass(FB_LANG_STACK_ACTIVE_CLASS, __FUNCTION__)
#endif
	/// Push current function to stack
	#define FB_STACK_FUNC() fb::lang::CallStackImplementationClass callStackImplementationClass(__FUNCTION__)
	/// Push custom id to the stack
	#define FB_TOKENPASTE(x, y) x ## y
	#define FB_TOKENPASTE2(x, y) FB_TOKENPASTE(x, y)
	#define FB_STACK_CUSTOM(id) fb::lang::CallStackImplementationClass FB_TOKENPASTE2(callStackImplementationClassCustom, __LINE__) (id)
	/// Push custom class and method to the stack
	#define FB_STACK_CUSTOM_METHOD(clid) fb::lang::CallStackImplementationClass callStackImplementationClass(clid, __FUNCTION__)
#else
	#define FB_STACK_SET_CLASS(className)
	#define FB_STACK_METHOD()
	#define FB_STACK_FUNC()
	#define FB_STACK_CUSTOM(id) 
	#define FB_STACK_CUSTOM_METHOD(clid)
#endif

FB_END_PACKAGE1()

#endif