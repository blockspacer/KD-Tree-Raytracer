#pragma once

#define FB_DECLARE_CUSTOM_OPER_DONE(p_count, p_oper, p_vars, p_input, p_begin, p_end, p_namespaces) \
	FB_PP_STRIP_COMMAS(FB_PP_UNPACK(FB_PP_POP_FIRST(p_begin))) \
	p_oper(FB_PP_FIRST(p_input), FB_PP_UNPACK(p_vars)) \
	FB_PP_STRIP_COMMAS(FB_PP_UNPACK(FB_PP_POP_FIRST(p_end)))

#define FB_DECLARE_CUSTOM_OPER_LOOP(p_count, p_oper, p_vars, p_input, p_begin, p_end, p_namespaces) \
	p_count, p_oper, p_vars, \
	FB_PP_POP_FIRST(p_input), \
	(FB_PP_UNPACK(p_begin), namespace FB_PP_FIRST(p_input) { ), \
	(FB_PP_UNPACK(p_end), }), \
	(FB_PP_UNPACK(p_namespaces), FB_PP_FIRST(p_input))

#define FB_DECLARE_CUSTOM_OPER(p_count, p_oper, p_vars, p_input, p_begin, p_end, p_namespaces) \
	FB_PP_CONCAT(FB_DECLARE_CUSTOM_OPER_, FB_PP_LOOP_STATE(p_count)) (FB_PP_DECREASE(p_count), p_oper, p_vars, p_input, p_begin, p_end, p_namespaces)

// Wrappers to bypass VC++ bug
#define FB_DECLARE_CUSTOM_3(p_oper, p_vars, p_args) FB_PP_FOR(FB_PP_NARG p_args, FB_DECLARE_CUSTOM_OPER, (FB_PP_NARG p_args, p_oper, p_vars, (FB_PP_UNPACK(p_args), input), (begin), (end), (ns)))
#define FB_DECLARE_CUSTOM_2(p_oper, p_vars, p_args) FB_DECLARE_CUSTOM_3(p_oper, p_vars, p_args)
#define FB_DECLARE_CUSTOM_1(p_oper, p_vars, p_args) FB_DECLARE_CUSTOM_2(p_oper, p_vars, p_args)

// p_oper gets called with (p_name, FB_PP_UNPACK(p_vars)), where p_name is the last item after namespaces
// NOTE: p_vars needs to have at least one variable. If you simply don't need any, then just add dummy.
#define FB_DECLARE_CUSTOM(p_oper, p_vars, p_args) FB_DECLARE_CUSTOM_1(p_oper, p_vars, p_args)


// Forward declaration macro
#define FB_FORWARD_DECLARE_OPER(p_name, p_type) p_type p_name;
#define FB_FORWARD_DECLARE(p_type, p_args) FB_DECLARE_CUSTOM(FB_FORWARD_DECLARE_OPER, (p_type), p_args)

// Enum forward declaration macro
#define FB_FORWARD_DECLARE_ENUM_OPER(p_name, p_type) p_type p_name FB_ENUM_UNSIGNED;
#define FB_FORWARD_DECLARE_ENUM(p_type, p_args) FB_DECLARE_CUSTOM(FB_FORWARD_DECLARE_ENUM_OPER, (p_type), p_args)


// Declare class
#define FB_DECLARE(...) \
	namespace fb { FB_FORWARD_DECLARE(class, (__VA_ARGS__)) }

#define FB_DECLARE0(p_name) \
	namespace fb { class p_name; }

// Declare struct
#define FB_DECLARE_STRUCT(...) \
	namespace fb { FB_FORWARD_DECLARE(struct, (__VA_ARGS__)) }

#define FB_DECLARE_STRUCT0(p_name) \
	namespace fb { struct p_name; }

// Declare enum (only works for enums whose underlying type is defined as FB_ENUM_UNSIGNED)
#define FB_DECLARE_ENUM(...) \
	namespace fb { FB_FORWARD_DECLARE_ENUM(enum, (__VA_ARGS__)) }

#define FB_DECLARE_ENUM0(p_name) \
	namespace fb { enum p_name FB_ENUM_UNSIGNED; }

// Declare templated class
#define FB_DECLARE_TEMPLATED_CLASS(...) \
	namespace fb { FB_FORWARD_DECLARE(template<typename T> class, (__VA_ARGS__)) }

#define FB_DECLARE_TEMPLATED_CLASS0(p_className) \
	namespace fb { template<typename T> class p_className; }

// Declare templated struct
#define FB_DECLARE_TEMPLATED_STRUCT(...) \
	namespace fb { FB_FORWARD_DECLARE(template<typename T> struct, (__VA_ARGS__)) }

#define FB_DECLARE_TEMPLATED_STRUCT0(p_className) \
	namespace fb { template<typename T> struct p_className; }
