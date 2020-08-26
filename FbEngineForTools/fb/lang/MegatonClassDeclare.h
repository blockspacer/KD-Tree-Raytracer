#pragma once

#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE0()

//
// Macros for making a class Megaton compatible
// 
// Usage:
// 
// ### MyClass.h ###
// class MyClass
// {
//     FB_MEGATON_CLASS_DECL();
// }
//
// ### MyClass.cpp ###
// FB_MEGATON_CLASS_IMPL(MyClass)
// MyClass::MyClass()
// {
//     getMegatonReference().set(this);
// }
// MyClass::~MyClass()
// {
//     getMegatonReference().unset(this);
// }
// 

#define FB_MEGATON_CLASS_DECL() \
	public: \
		static fb::SizeType megatonIndex; \
		static const char *getDebugMegatonName(); \
	private: \

#define FB_MEGATON_CLASS_IMPL(p_class) \
	static FB_UNUSED_NAMED_VAR(fb::CheckBaseClassMegaton::Impl< p_class >, check##p_class##Megaton); \
	fb::SizeType p_class::megatonIndex = fb::Megaton::getUndefinedIndex(); \
	const char *p_class::getDebugMegatonName() { return #p_class; }; \
	extern p_class *getDebugMegaton_##p_class() { return (p_class *)fb::lang::MegatonDebugger::getMegatonPointer(p_class::megatonIndex); };

//
// Macros for creating fb::getMyClass() global function
// This only works for objects within GlobalMegaton, so mostly modules
//
// For an usage example see PlatformModule and ctrl+f "megaton"
// 
// !!! NOTE !!!
// Must be declared outside of the FB_PACKAGE#() macros in global scope
// !!! NOTE !!!
//
#define FB_MEGATON_GLOBAL_GETTER(p_namespace, p_classname) \
	FB_PACKAGE0() \
		static FB_FORCEINLINE fb::p_namespace::p_classname &get##p_classname() \
		{ \
			return getGlobalMegaton().getQuickly<fb::p_namespace::p_classname>(); \
		} \
	FB_END_PACKAGE0()

struct CheckBaseClassMegaton
{
	template <typename T, typename = int>
	struct HasMegatonIndex { static const bool value = false; };

	template <typename T>
	struct HasMegatonIndex <T, decltype((void)T::megatonIndex, 0)> { static const bool value = true; };

	template<typename T>
	struct void_ { typedef void type; };

	template<typename T, typename = void>
	struct Impl {};

	template<typename T>
	struct Impl <T, typename void_<typename T::BaseClass>::type>
	{
		fb_static_assertf(!HasMegatonIndex<typename T::BaseClass>::value, "Base class already using FB_MEGATON_CLASS_DECL()");
	};
};

FB_END_PACKAGE0()
