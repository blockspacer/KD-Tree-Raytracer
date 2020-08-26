#pragma once

#include "fb/string/util/SplitString.h"

FB_PACKAGE1(util)

/**
 * Verify macros are designed to make checks like 
 *
 *     if (some_pointer == NULL)
 *     {
 *         TAG_ERROR(message);
 *         return something;
 *     }
 *
 * prettier. Especially in case where there are many pointers to check. Macros have versions to add custom message and 
 * return value, or to not raise an error (just return). See further below for per macro descriptions.
 */


#if FB_ENGINE_BASE_ERRORTAG_ERROR_TAGGER_ENABLED == FB_TRUE || FB_ENGINE_BASE_ERRORTAG_REPLACE_TAGS_WITH_LOG == FB_TRUE
	#define FB_USE_ERROR_MACROS FB_TRUE
#elif FB_ENGINE_BASE_ERRORTAG_ERROR_TAGGER_ENABLED == FB_FALSE && FB_ENGINE_BASE_ERRORTAG_REPLACE_TAGS_WITH_LOG == FB_FALSE
	#define FB_USE_ERROR_MACROS FB_FALSE
#else
	#error "FB_ENGINE_BASE_ERRORTAG_ERROR_TAGGER_ENABLED or FB_ENGINE_BASE_ERRORTAG_REPLACE_ERRORS_WITH_ASSERT definition invalid or missing"
#endif

#if FB_USE_ERROR_MACROS == FB_TRUE

#define FB_VERIFY_MACROS_CREATE_LOG_TAG(p_log_macro, ...) \
	string::SplitString names(StringRef(#__VA_ARGS__), StringRef(",")); \
	TempString nameString("A depended pointer ("); \
	for (SizeType iiii = 0; iiii < names.getNumPieces() && iiii < (sizeof pointers)/(sizeof pointers[0]); ++iiii) \
	{ \
		if (pointers[iiii] == NULL) \
		{ \
			nameString += names[iiii]; \
			break; \
		} \
	} \
	nameString += ") is NULL"; \
	p_log_macro(nameString);


#define FB_VERIFY_MACROS_CREATE_ERROR_TAG(...) \
	FB_VERIFY_MACROS_CREATE_LOG_TAG(TAG_CUSTOM_ERROR, __VA_ARGS__)


#define FB_VERIFY_MACROS_CREATE_GLOBAL_ERROR_TAG(...) \
	FB_VERIFY_MACROS_CREATE_LOG_TAG(TAG_GLOBAL_CUSTOM_ERROR, __VA_ARGS__)

#else
#define FB_VERIFY_MACROS_CREATE_ERROR_TAG(...)
#define FB_VERIFY_MACROS_CREATE_GLOBAL_ERROR_TAG(...)
#endif


#define FB_IF_HAS_NULLS_IMPL(...) \
	bool anyNulls = false; \
	const void* pointers[] = { __VA_ARGS__ }; \
	for (int iiii = 0; iiii < sizeof(pointers) / sizeof(pointers[0]); iiii++) \
	{ \
		if (pointers[iiii] != NULL) \
		{ \
		} \
		else \
		{ \
			anyNulls = true; \
			break; \
		} \
	} \
	if (!anyNulls) \
	{ \
	} \
	else \



#define FB_VERIFY_POINTERS_CET_IMPL(...) \
	{ \
		FB_IF_HAS_NULLS_IMPL(__VA_ARGS__) \
		{ \
			FB_VERIFY_MACROS_CREATE_ERROR_TAG(__VA_ARGS__) \


#define FB_VERIFY_POINTERS_CET_WITH_MSG_IMPL(p_msg, ...) \
	{ \
		FB_IF_HAS_NULLS_IMPL(__VA_ARGS__) \
		{ \
			TAG_CUSTOM_ERROR(p_msg); \


#define FB_VERIFY_POINTERS_GCET_IMPL(...) \
	{ \
		FB_IF_HAS_NULLS_IMPL(__VA_ARGS__) \
		{ \
			FB_VERIFY_MACROS_CREATE_GLOBAL_ERROR_TAG(__VA_ARGS__) \


#define FB_VERIFY_POINTERS_GCET_WITH_MSG_IMPL(p_msg, ...) \
	{ \
		FB_IF_HAS_NULLS_IMPL(__VA_ARGS__) \
		{ \
			TAG_GLOBAL_CUSTOM_ERROR(p_msg); \


#define FB_VERIFY_POINTER_IMPL(p_pointer) \
	DEF_LOCAL_ERROR(engine::base::Blobject *, value == NULL, p_pointer##IsNull, "Pointer is NULL."); \
	if (p_pointer != NULL) \
	{ \
	} \
	else \
	{ \
		FB_UNUSED_VAR(bool) = TAG_ERROR_W_MESSAGE(p_pointer, p_pointer##IsNull, FB_MSG("Depended pointer ", #p_pointer, " is NULL")); \


/* Verify single given pointer. Optionally return second parameter on fail. TAGS object on error. */
#define FB_VERIFY_POINTER(p_pointer, ...) \
	FB_VERIFY_POINTER_IMPL(p_pointer) \
		return __VA_ARGS__; \
	} \


/* Same as above but with custom message in case of error. */
#define FB_VERIFY_POINTER_WITH_MSG(p_pointer, p_msg, ...) \
	DEF_LOCAL_ERROR(engine::base::Blobject *, value == NULL, p_pointer##IsNull, p_msg); \
	if (p_pointer == NULL) \
	{ \
		FB_UNUSED_VAR(bool) = TAG_ERROR_W_MESSAGE(p_pointer, p_pointer##IsNull, p_msg); \
		return __VA_ARGS__; \
	} \


/* Same as above, but uses global error */
#define FB_VERIFY_POINTER_WITH_GLOBAL_MSG(p_pointer, p_msg, ...) \
	DEF_LOCAL_ERROR(engine::base::Blobject *, value == NULL, p_pointer##IsNull, p_msg); \
	if (p_pointer == NULL) \
	{ \
		TAG_GLOBAL_CUSTOM_ERROR(p_msg); \
		return __VA_ARGS__; \
	} \


/* Verify arbitrary number of pointers. Raise an error if any of them is NULL and return. */
#define FB_VERIFY_POINTERS_CET(...) \
	FB_VERIFY_POINTERS_CET_IMPL(__VA_ARGS__) \
			return; \
		} \
	} \

/* Same as above, but uses global error message */
#define FB_VERIFY_POINTERS_GCET(...) \
	FB_VERIFY_POINTERS_GCET_IMPL(__VA_ARGS__) \
			return; \
		} \
	} \



/* Same as above with additional custom error message. */
#define FB_VERIFY_POINTERS_CET_WITH_MSG(p_msg, ...) \
	FB_VERIFY_POINTERS_CET_WITH_MSG_IMPL(p_msg, __VA_ARGS__) \
			return; \
		} \
	} \


/** Same as above, but uses global error message. */
#define FB_VERIFY_POINTERS_GCET_WITH_MSG(p_msg, ...) \
	FB_VERIFY_POINTERS_GCET_WITH_MSG_IMPL(p_msg, __VA_ARGS__) \
			return; \
		} \
	} \


/* Same as pair above without any error message. */
#define FB_VERIFY_POINTERS_SILENT(...) \
	{ \
		FB_IF_HAS_NULLS_IMPL(__VA_ARGS__) \
			return; \
	} \


/* Verify arbitrary number of pointers. Raise and error if any of them is NULL and return first parameter. */
#define FB_VERIFY_POINTERS_CET_WITH_RETURN_VALUE(p_return_value, ...) \
	FB_VERIFY_POINTERS_CET_IMPL(__VA_ARGS__) \
	return p_return_value; \
		} \
	} \

/* Same as above, but uses global error message */
#define FB_VERIFY_POINTERS_GCET_WITH_RETURN_VALUE(p_return_value, ...) \
	FB_VERIFY_POINTERS_GCET_IMPL(__VA_ARGS__) \
			return p_return_value; \
		} \
	} \


/* Same as above with additional custom error message. */
#define FB_VERIFY_POINTERS_CET_WITH_MSG_AND_RETURN_VALUE(p_msg, p_return_value, ...) \
	FB_VERIFY_POINTERS_CET_WITH_MSG_IMPL(p_msg, __VA_ARGS__) \
			return p_return_value; \
		} \
	} \


/** Same as above, but uses global error message. */
#define FB_VERIFY_POINTERS_GCET_WITH_MSG_AND_RETURN_VALUE(p_msg, p_return_value, ...) \
	FB_VERIFY_POINTERS_GCET_WITH_MSG_IMPL(p_msg, __VA_ARGS__) \
			return p_return_value; \
		} \
	} \

/* Same as pair above without any error message. */
#define FB_VERIFY_POINTERS_WITH_RETURN_VALUE_SILENT(p_return_value, ...) \
	{ \
		FB_IF_HAS_NULLS_IMPL(__VA_ARGS__) \
			return p_return_value; \
	} \


#undef FB_USE_ERROR_MACROS

FB_END_PACKAGE1()
