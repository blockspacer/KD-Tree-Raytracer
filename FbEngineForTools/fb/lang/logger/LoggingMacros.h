#pragma once

/* Rather than including this, include fb/lang/logger/GlobalLogger.h */

#include "fb/lang/platform/Config.h"
#include "fb/lang/platform/Compiler.h"
#include "fb/lang/platform/LineFeed.h"

/* Config */
#if FB_BUILD == FB_DEBUG
	#define FB_SYS_LOGGER_ENABLED FB_TRUE
#elif FB_BUILD == FB_RELEASE
	#define FB_SYS_LOGGER_ENABLED FB_TRUE
#elif FB_NULL_RENDERER == FB_TRUE
	#define FB_SYS_LOGGER_ENABLED FB_TRUE
#elif FB_BUILD == FB_FINAL_RELEASE
	#define FB_SYS_LOGGER_ENABLED FB_FALSE
#else
	#error "Unknown build."
#endif


#if FB_ENGINE_FOR_TOOLS == FB_FALSE &&  FB_BUILD != FB_FINAL_RELEASE
	#define FB_FOCUS_TEST_LOGGING_ENABLED FB_TRUE
#else
	#define FB_FOCUS_TEST_LOGGING_ENABLED FB_FALSE
#endif

#if FB_BUILD == FB_FINAL_RELEASE && FB_FINAL_RELEASE_OBFUSCATION_ENABLED == FB_TRUE
	#define FB_IMPL_LOGGER_FUNC_MACRO_SIMPLE ""
	#define FB_IMPL_LOGGER_FUNC_MACRO_DETAILED ""
	#define FB_IMPL_LOGGER_FUNC_MACRO ""
#else
	// could also use __func__, but it lacks the classname =/
	#if FB_COMPILER == FB_MSC
		#define FB_IMPL_LOGGER_FUNC_MACRO_SIMPLE __func__
		#define FB_IMPL_LOGGER_FUNC_MACRO_DETAILED __FUNCSIG__
	#elif FB_COMPILER == FB_GNUC || FB_COMPILER == FB_CLANG
		#define FB_IMPL_LOGGER_FUNC_MACRO_SIMPLE __PRETTY_FUNCTION__
		#define FB_IMPL_LOGGER_FUNC_MACRO_DETAILED __PRETTY_FUNCTION__
	#else
		#define FB_IMPL_LOGGER_FUNC_MACRO_SIMPLE __func__
		#define FB_IMPL_LOGGER_FUNC_MACRO_DETAILED FB_IMPL_LOGGER_FUNC_MACRO_SIMPLE
	#endif

	#if FB_COMPILER == FB_MSC
		#define FB_IMPL_LOGGER_FUNC_MACRO __FUNCTION__
	#else
		#define FB_IMPL_LOGGER_FUNC_MACRO FB_IMPL_LOGGER_FUNC_MACRO_SIMPLE
	#endif
#endif

#define FB_LOGGER_LINEFEED "\r\n"
#define FB_LOGGER_LINEFEEDED_S "%s\r\n"

#if (FB_SYS_LOGGER_ENABLED == FB_TRUE)
	#define FB_LOG_ERROR(...) fb::GlobalLogger::getInstance().error(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
	#define FB_LOG_ERROR_FUNC(p_func, ...) fb::GlobalLogger::getInstance().error(p_func, __VA_ARGS__)

	#define FB_LOG_WARNING(...) fb::GlobalLogger::getInstance().warning(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
	#define FB_LOG_WARNING_FUNC(p_func, ...) fb::GlobalLogger::getInstance().warning(p_func, __VA_ARGS__)

	#define FB_LOG_INFO(...) fb::GlobalLogger::getInstance().info(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
	#define FB_LOG_INFO_FUNC(p_func, ...) fb::GlobalLogger::getInstance().info(p_func, __VA_ARGS__)

	#define FB_LOG_DEBUG(...) fb::GlobalLogger::getInstance().debug(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
	#define FB_LOG_DEBUG_FUNC(p_func, ...) fb::GlobalLogger::getInstance().debug(p_func, __VA_ARGS__)

#else
	#define FB_LOG_ERROR(...) fb::lang::ILogger::dummy()
	#define FB_LOG_ERROR_FUNC(...) fb::lang::ILogger::dummy()
	#define FB_LOG_WARNING(...) fb::lang::ILogger::dummy()
	#define FB_LOG_WARNING_FUNC(...) fb::lang::ILogger::dummy()
	#define FB_LOG_INFO(...) fb::lang::ILogger::dummy()
	#define FB_LOG_INFO_FUNC(...) fb::lang::ILogger::dummy()
	#define FB_LOG_DEBUG(...) fb::lang::ILogger::dummy()
	#define FB_LOG_DEBUG_FUNC(...) fb::lang::ILogger::dummy()
#endif

// logging macros that are always enabled
#define FB_FINAL_LOG_ERROR(...) fb::GlobalLogger::getInstance().error(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
#define FB_FINAL_LOG_ERROR_FUNC(p_func, ...) fb::GlobalLogger::getInstance().error(p_func, __VA_ARGS__)
#define FB_FINAL_LOG_WARNING(...) fb::GlobalLogger::getInstance().warning(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
#define FB_FINAL_LOG_WARNING_FUNC(p_func, ...) fb::GlobalLogger::getInstance().warning(p_func, __VA_ARGS__)
#define FB_FINAL_LOG_INFO(...) fb::GlobalLogger::getInstance().info(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
#define FB_FINAL_LOG_INFO_FUNC(p_func, ...) fb::GlobalLogger::getInstance().info(p_func, __VA_ARGS__)
#define FB_FINAL_LOG_DEBUG(...) fb::GlobalLogger::getInstance().debug(FB_IMPL_LOGGER_FUNC_MACRO, __VA_ARGS__)
#define FB_FINAL_LOG_DEBUG_FUNC(p_func, ...) fb::GlobalLogger::getInstance().debug(p_func, __VA_ARGS__)

/* Some helper macros for focus logging */
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	/* Creates event type, basically a StaticString */
	#define FB_FOCUS_DEFINE_EVENT_TYPE(p_event_type) FB_STATIC_CONST_STRING(focusEventType##p_event_type, #p_event_type);
	/* Just a basic wrapper. Needs to be delivered the logger. */
	#define FB_FOCUS_TEST_LOWLEVEL_EVENT(p_logger, p_event_type, p_position, p_msg) p_logger->focusTestEvent(p_event_type, p_position, p_msg);
	/* For Blobject based use. Presumes logger is available via getEnv(). */
	#define FB_FOCUS_TEST_EVENT(p_event_type, p_msg) GlobalLogger::getInstance().focusTestEvent(p_event_type, 0.0f, 0.0f, 0.0f, p_msg);
	#define FB_FOCUS_TEST_EVENT_POSITIONED(p_event_type, p_position, p_msg) GlobalLogger::getInstance().focusTestEvent(p_event_type, p_position.x, p_position.y, p_position.z, p_msg);
	/* For Entity / Component based use. Presumes owner and TransformComponent are available (and logger via getEnv()). */
	#define FB_FOCUS_TEST_EVENT_AUTO_POS(p_event_type, p_msg) \
	{ \
		const engine::instance::Entity* focusTestTempEntity = fb::dynamicCast<engine::instance::Entity>(getFinalOwner()); \
		const engine::component::TransformComponent* focusTestTempTC = focusTestTempEntity != nullptr ? focusTestTempEntity->getTransformComponent() : nullptr; \
		if (focusTestTempTC != nullptr) \
		{ \
			const math::VC3 &pos = focusTestTempTC->getPosition(); \
			GlobalLogger::getInstance().focusTestEvent(p_event_type, pos.x, pos.y, pos.z, FB_MSG("Entity: ", focusTestTempEntity->getName(), ", ", p_msg)); \
		} \
		else \
		{ \
			GlobalLogger::getInstance().focusTestEvent(p_event_type, 0.0f, 0.0f, 0.0f, "ERROR: Could not find transformComponent to determine position, non-positioned event follows"); \
			GlobalLogger::getInstance().focusTestEvent(p_event_type, 0.0f, 0.0f, 0.0f, p_msg); \
		} \
	}
	#define FB_FOCUS_TEST_EVENT_AUTO_POS_CUSTOM_ENTITY(p_event_type, p_msg, p_entity) \
	{ \
		const engine::instance::Entity* focusTestTempEntity = fb::dynamicCast<engine::instance::Entity>(p_entity); \
		const engine::component::TransformComponent* focusTestTempTC = focusTestTempEntity != nullptr ? focusTestTempEntity->getTransformComponent() : nullptr; \
		if (focusTestTempTC != nullptr) \
		{ \
			const math::VC3 &pos = focusTestTempTC->getPosition(); \
			GlobalLogger::getInstance().focusTestEvent(p_event_type, pos.x, pos.y, pos.z, FB_MSG("Entity: ", focusTestTempEntity->getName().getPointer(), ", ", p_msg)); \
		} \
		else \
		{ \
			GlobalLogger::getInstance().focusTestEvent(p_event_type, 0.0f, 0.0f, 0.0f, "ERROR: Could not find transformComponent to determine position, non-positioned event follows"); \
			GlobalLogger::getInstance().focusTestEvent(p_event_type, 0.0f, 0.0f, 0.0f, p_msg); \
		} \
	}
	#define FB_FOCUS_TEST_POSITION_EVENT(p_position, p_msg) \
	{ \
		FB_STATIC_CONST_STRING(focusTestPositionEventStr, "PlayerPosition"); \
		GlobalLogger::getInstance().focusTestEvent(focusTestPositionEventStr, p_position.x, p_position.y, p_position.z, p_msg); \
	}
	#define FB_FOCUS_TEST_POSITION_EVENT_AUTO(p_msg) \
	{ \
		FB_STATIC_CONST_STRING(focusTestPositionEventStr, "PlayerPosition"); \
		const engine::instance::Entity* focusTestTempEntity = fb::dynamicCast<engine::instance::Entity>(getFinalOwner()); \
		const engine::component::TransformComponent* focusTestTempTC = focusTestTempEntity != nullptr ? focusTestTempEntity->getTransformComponent() : nullptr; \
		if (focusTestTempTC != nullptr) \
		{ \
			TempString temp; temp += "Entity: "; temp += focusTestTempEntity->getName(); temp += ", "; temp += p_msg; \
			const math::VC3 &pos = focusTestTempTC->getPosition(); \
			GlobalLogger::getInstance().focusTestEvent(focusTestPositionEventStr, pos.x, pos.y, pos.z, temp.getPointer()); \
		} \
		else \
		{ \
			GlobalLogger::getInstance().focusTestEvent(focusTestPositionEventStr, 0.0f, 0.0f, 0.0f, "ERROR: Could not find transformComponent to determine position, non-positioned event follows"); \
			GlobalLogger::getInstance().focusTestEvent(focusTestPositionEventStr, 0.0f, 0.0f, 0.0f, p_msg); \
		} \
	}

#else

	#define FB_FOCUS_DEFINE_EVENT_TYPE(p_event_type)
	#define FB_FOCUS_TEST_LOWLEVEL_EVENT(p_logger, p_event_type, p_position, p_msg)
	#define FB_FOCUS_TEST_EVENT(p_event_type, p_msg)
	#define FB_FOCUS_TEST_EVENT_POSITIONED(p_event_type, p_position, p_msg)
	#define FB_FOCUS_TEST_EVENT_AUTO_POS(p_event_type, p_msg)
	#define FB_FOCUS_TEST_EVENT_AUTO_POS_CUSTOM_ENTITY(p_event_type, p_msg, p_entity)
	#define FB_FOCUS_TEST_POSITION_EVENT(p_position, p_msg)
	#define FB_FOCUS_TEST_POSITION_EVENT_AUTO(p_msg)

#endif

#if FB_BUILD == FB_FINAL_RELEASE && FB_FINAL_RELEASE_OBFUSCATION_ENABLED == FB_TRUE
	#define FB_FUNCTION ""
#else
	#define FB_FUNCTION __FUNCTION__
#endif

/* Helper macro to limit amount of spam logging may generate. 
 * Usage: FB_LIMIT_SPAM(100, FB_LOG_ERROR("Something went wrong"));*/
#define FB_LIMIT_SPAM(p_count, ...) \
	{ \
		static const uint32_t errorLimit = p_count; \
		static uint32_t errorCount = 0; \
		++errorCount; \
		if (errorCount <= errorLimit) \
		{ \
			__VA_ARGS__; \
			if (errorCount == errorLimit) \
			{ \
				FB_LOG_ERROR("Spam limit reached. Further messages will be suppressed"); \
			} \
		} \
	}

#define FB_LIMIT_SPAM_NO_ERROR(p_count, ...) \
	{ \
		static const uint32_t errorLimit = p_count; \
		static uint32_t errorCount = 0; \
		++errorCount; \
		if (errorCount <= errorLimit) \
		{ \
			__VA_ARGS__; \
			if (errorCount == errorLimit) \
			{ \
				FB_LOG_DEBUG("Spam limit reached. Further messages will be suppressed"); \
			} \
		} \
	}

#define FB_LIMIT_SPAM_NO_MESSAGE(p_count, ...) \
	{ \
		static const uint32_t errorLimit = p_count; \
		static uint32_t errorCount = 0; \
		++errorCount; \
		if (errorCount <= errorLimit) \
		{ \
			__VA_ARGS__; \
		} \
	}
