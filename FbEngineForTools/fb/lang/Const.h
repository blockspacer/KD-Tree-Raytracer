#pragma once

FB_PACKAGE1(lang)

/* Declares static const variable in a way that is (more or less) immune to static initialization 
 * order issues and bindable to lua in a const way (as a getter method). Variable can be accessed 
 * with getVariableName() method and bind to lua with FB_BIND_STATIC_METHOD. */
#define FB_CONST(p_type, p_name, p_value) \
	static const p_type& get##p_name() \
	{ \
		static const p_type var##p_name(p_value); \
		return var##p_name; \
	} \

/* Same as above, but returns by value */
#define FB_CONST_POD(p_type, p_name, p_value) \
	static FB_FORCEINLINE p_type get##p_name() \
	{ \
		return p_value; \
	} \

 /* Vector version */
#define FB_CONST_VECTOR_IMPL(p_numValues, p_type, p_name, ...) \
	static const fb::StaticPodVector<const p_type, p_numValues>& get##p_name() \
	{ \
		static const p_type values[p_numValues] = { __VA_ARGS__ }; \
		static fb::StaticPodVector<const p_type, p_numValues> var##p_name(values); \
		return var##p_name; \
	} \


#define FB_CONST_VECTOR(p_type, p_name, ...) \
	FB_CONST_VECTOR_IMPL(FB_PP_NARG(__VA_ARGS__), p_type, p_name, __VA_ARGS__) \


/* Separate decl and impl versions of above macro, if one doesn't want to inline the method */
#define FB_DECL_CONST(p_type, p_name) \
	static const p_type& get##p_name(); \

#define FB_IMPL_CONST(p_class, p_type, p_name, p_value) \
	const p_type& p_class::get##p_name() \
	{ \
		static const p_type var##p_name(p_value); \
		return var##p_name; \
	} \


FB_END_PACKAGE1()
