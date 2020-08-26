#include "Precompiled.h"
#include "Megaton.h"

#include "fb/lang/FBPrintf.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/util/CreateTemporaryHeapString.h"

#include <cstring> // For strstr, TODO: Replace with our own equivalent

FB_PACKAGE0()

Megaton::Megaton()
	: fallbackMegaton(nullptr)
{
	debugMegatonType = "GlobalMegaton";

	for (SizeType i = 0; i < pointers.getSize(); ++i)
	{
		pointers.buffer[i] = nullptr;
	}

	megatonDebugger = new lang::MegatonDebugger(&pointers.buffer[0]);
}
Megaton::~Megaton()
{
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
	for (SizeType i = 0; false && i < megatonTypeIndexCounter; ++i)
	{
		FB_LOG_DEBUG(FB_FMT("Megaton usage: %3d: %8" FB_FSU64 ", fallback: %8" FB_FSU64 " - %s", 
			i, *(megatonDebugger->pointers[i].usage), *(megatonDebugger->pointers[i].fallback), *(megatonDebugger->pointers[i].name)));
	}
	{
		uint64_t usage = 0;
		uint32_t index = ~0U;
		uint64_t prevValue = ~0U;
		for (SizeType place = 0; place < 10; ++place)
		{
			index = ~0U;
			usage = 0;
			for (SizeType i = 0; i < megatonTypeIndexCounter; ++i)
			{
				uint64_t u = *(megatonDebugger->pointers[i].quickUsage);
				if (usage < u && u < prevValue)
				{
					index = i;
					usage = u;
				}
			}
			prevValue = usage;
			if (index >= megatonTypeIndexCounter)
				break;

			FB_LOG_DEBUG(FB_FMT("The %dth most used megaton with %8" FB_FSU64 " usages via quick getter is %s (index: %d).",
				place, usage, *megatonDebugger->pointers[index].name, index));
		}
	}
	{
		uint64_t usage = 0;
		uint32_t index = ~0U;
		uint64_t prevValue = ~0U;
		for (SizeType place = 0; place < 10; ++place)
		{
			index = ~0U;
			usage = 0;
			for (SizeType i = 0; i < megatonTypeIndexCounter; ++i)
			{
				uint64_t u = *(megatonDebugger->pointers[i].usage);
				if (usage < u && u < prevValue)
				{
					index = i;
					usage = u;
				}
			}
			prevValue = usage;
			if (index >= megatonTypeIndexCounter)
				break;

			FB_LOG_DEBUG(FB_FMT("The %dth most slowly used megaton with %8" FB_FSU64 " times used via slow getter is %s (index: %d).",
				place, usage, *megatonDebugger->pointers[index].name, index));
		}
	}
	{
		uint64_t usage = 0;
		uint32_t index = ~0U;
		uint64_t prevValue = ~0U;
		for (SizeType place = 0; place < 10; ++place)
		{
			index = ~0U;
			usage = 0;
			for (SizeType i = 0; i < megatonTypeIndexCounter; ++i)
			{
				uint64_t u = *(megatonDebugger->pointers[i].fallback);
				if (usage < u && u < prevValue)
				{
					index = i;
					usage = u;
				}
			}
			prevValue = usage;
			if (index >= megatonTypeIndexCounter)
				break;

			FB_LOG_DEBUG(FB_FMT("The %dth most fallbacked megaton with %8" FB_FSU64 " fallbacks in slow getter is %s (index: %d).",
				place, usage, *megatonDebugger->pointers[index].name, index));
		}
	}
#endif
	delete megatonDebugger;
}

void *Megaton::getWithFallbackImpl(SizeType index) const
{
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
	if (index < FB_MEGATON_MAX_POINTER_COUNT)
		++(lang::MegatonDebugger::megatonUsageCounters[index]);
#endif
	
	FB_MEGATON_ASSERTF(index < pointers.getSize(), "Megaton index is invalid: %d", index);

	if (pointers[index] != nullptr)
	{
		return pointers[index];
	}
	else if (fallbackMegaton)
	{
#if FB_MEGATON_VERBOSE_DEBUG_ENABLED == FB_TRUE
		if (index < FB_MEGATON_MAX_POINTER_COUNT)
			++(lang::MegatonDebugger::megatonFallbackCounters[index]);
#endif

		// Max depth is 3 so let's just manually inline this to avoid unnecessary boundary checks and stuff
		if (fallbackMegaton->pointers[index] != nullptr)
		{
			return fallbackMegaton->pointers[index];
		}
		else if (fallbackMegaton->fallbackMegaton)
		{
			return fallbackMegaton->fallbackMegaton->pointers[index];
		}
	}
	return nullptr;
}

const char *Megaton::getDebugMegatonType() const
{
	return debugMegatonType;
}

const char *Megaton::getOwnerMegatonType(SizeType index)
{
	GlobalMegaton &gm = getGlobalMegaton();
	SizeType pointerCount = gm.pointers.getSize();
	if (index >= pointerCount)
		return "NONE";

	if (gm.pointers[index] != nullptr)
		return gm.getDebugMegatonType();

	if (GameStateMegaton::megatonIndex < pointerCount
		&& g_gameStateMegaton != nullptr
		&& g_gameStateMegaton->pointers[index] != nullptr)
		return GameStateMegaton::getDebugMegatonName();

#if FB_EDITOR_ENABLED == FB_TRUE
	if (EditorStateMegaton::megatonIndex < pointerCount
		&& gm.pointers[EditorStateMegaton::megatonIndex] != nullptr
		&& ((EditorStateMegaton *)gm.pointers[EditorStateMegaton::megatonIndex])->pointers[index] != nullptr)
		return EditorStateMegaton::getDebugMegatonName();

	StateMegaton &stateMegaton = !g_megatonDefaultInEditor ? *static_cast<StateMegaton *>(g_gameStateMegaton) : *reinterpret_cast<StateMegaton *>(gm.pointers[EditorStateMegaton::megatonIndex]);
#else
	StateMegaton &stateMegaton = *g_gameStateMegaton;
#endif

	if (SceneMegaton::megatonIndex < pointerCount
		&& stateMegaton.pointers[SceneMegaton::megatonIndex] != nullptr
		&& ((SceneMegaton *)stateMegaton.pointers[SceneMegaton::megatonIndex])->pointers[index] != nullptr)
		return "SceneMegaton";

	return "NONE";
}
const char *Megaton::getOwnerMegatonType(SizeType index, void *pointer)
{
	GlobalMegaton &gm = getGlobalMegaton();
	SizeType pointerCount = gm.pointers.getSize();
	if (index >= pointerCount)
		return "NONE";

	if (gm.pointers[index] != nullptr)
		return gm.getDebugMegatonType();

	if (GameStateMegaton::megatonIndex < pointerCount
		&& g_gameStateMegaton != nullptr
		&& g_gameStateMegaton->pointers[index] == pointer)
		return GameStateMegaton::getDebugMegatonName();

#if FB_EDITOR_ENABLED == FB_TRUE
	if (EditorStateMegaton::megatonIndex < pointerCount
		&& gm.pointers[EditorStateMegaton::megatonIndex] != nullptr
		&& ((EditorStateMegaton *)gm.pointers[EditorStateMegaton::megatonIndex])->pointers[index] == pointer)
		return EditorStateMegaton::getDebugMegatonName();

	StateMegaton &stateMegaton = !g_megatonDefaultInEditor ? *static_cast<StateMegaton *>(g_gameStateMegaton) : *reinterpret_cast<StateMegaton *>(gm.pointers[EditorStateMegaton::megatonIndex]);
#else
	StateMegaton &stateMegaton = *g_gameStateMegaton;
#endif
	if (SceneMegaton::megatonIndex < pointerCount
		&& stateMegaton.pointers[SceneMegaton::megatonIndex] != nullptr
		&& ((SceneMegaton *)stateMegaton.pointers[SceneMegaton::megatonIndex])->pointers[index] == pointer)
		return SceneMegaton::getDebugMegatonName();

	return "NONE";
}

void Megaton::setImpl(SizeType &index, void *ptr, const char *debugMegatonName, const char *debugMegatonTypeParam)
{
	FB_MEGATON_ASSERTF(nullptr != ptr, "%s being set to %s is nullptr.", debugMegatonName, debugMegatonTypeParam);

	// If megatonIndex is missing from a class, add FB_MEGATON_CLASS_DECL() and FB_MEGATON_CLASS_IMPL(MyClass) macros to the class header and source files
	if (index == getUndefinedIndex())
	{
		// FIXME: thread safety, add mutex?
		index = megatonTypeIndexCounter++;
		FB_MEGATON_ASSERTF(megatonTypeIndexCounter <= pointers.getSize(), 
			"Max megaton pointer count exceeded while adding pointer type %s. Increase FB_MEGATON_MAX_POINTER_COUNT. Current pointer count: %d, FB_MEGATON_MAX_POINTER_COUNT: %d. Trying to add %s", 
			debugMegatonTypeParam, megatonTypeIndexCounter, FB_MEGATON_MAX_POINTER_COUNT, debugMegatonName);
	}

	FB_MEGATON_ASSERTF(pointers[index] == nullptr, 
		"%s is being double set in %s without the previous value being unset first. Old value: %p, new value: %p", 
		debugMegatonName, debugMegatonTypeParam, pointers[index], ptr);

	pointers[index] = ptr;
	lang::MegatonDebugger::megatonNames[index] = debugMegatonName;
}

FB_MEGATON_CLASS_IMPL(SceneMegaton);
FB_MEGATON_CLASS_IMPL(GameStateMegaton);

#if FB_EDITOR_ENABLED == FB_TRUE
FB_MEGATON_CLASS_IMPL(EditorStateMegaton);
#endif

SizeType megatonTypeIndexCounter = 0;
bool g_megatonDefaultInEditor = false;

GameStateMegaton *g_gameStateMegaton = nullptr;
GlobalMegaton globalMegatonImpl;
GlobalMegaton *g_globalMegaton = &globalMegatonImpl;


const char *Megaton::debugConcat(const char *a, const char *b, const char *c)
{
	auto stringCopy = [](char *&dst, const char *src, const char *end)
	{
		while (*src != '\0' && dst + 1 < end)
			*dst++ = *src++;
	};

	static char debugMsgBuffer[512];
	const char *end = debugMsgBuffer + sizeof(debugMsgBuffer) - 1;
	char *buffer = debugMsgBuffer;
	stringCopy(buffer, a, end);
	stringCopy(buffer, b, end);
	stringCopy(buffer, c, end);
	*buffer = '\0';
	return debugMsgBuffer;
}

void Megaton::checkMegatonOwner(SizeType i, const char *name, const char *megatonName)
{
	FB_MEGATON_ASSERTF(i == getUndefinedIndex()
		|| isMegaton(getOwnerMegatonType(i)) == 0
		|| (strstr(megatonName, "GameState") != nullptr && strstr(getOwnerMegatonType(i), "EditorState") != nullptr ),
		"%s (megatonIndex: %d) is not set in %s. But it set in %s.", name, i, megatonName, getOwnerMegatonType(i));
}

FB_END_PACKAGE0()
