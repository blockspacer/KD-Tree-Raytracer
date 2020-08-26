#include "Precompiled.h"
#include "MegatonDebugger.h"

#include "fb/lang/Megaton.h"

FB_PACKAGE1(lang)

SizeType MegatonDebugger::editorMegatonIndex = ~0U;
const char *MegatonDebugger::megatonNames[FB_MEGATON_MAX_POINTER_COUNT] = { 0 };
uint64_t MegatonDebugger::megatonQuickUsage[FB_MEGATON_MAX_POINTER_COUNT] = { 0 };
uint64_t MegatonDebugger::megatonUsageCounters[FB_MEGATON_MAX_POINTER_COUNT] = { 0 };
uint64_t MegatonDebugger::megatonFallbackCounters[FB_MEGATON_MAX_POINTER_COUNT] = { 0 };

MegatonDebugger::MegatonDebugger(void **originalPointers)
{
	for (SizeType i = 0; i < FB_MEGATON_MAX_POINTER_COUNT; ++i)
	{
		pointers[i].name = megatonNames + i;
		pointers[i].value = originalPointers + i;
		pointers[i].index = i;
		pointers[i].usage = megatonUsageCounters + i;
		pointers[i].fallback = megatonFallbackCounters + i;
		pointers[i].quickUsage = megatonQuickUsage + i;
	}
}

void *MegatonDebugger::getMegatonPointer(uint32_t megatonIndex)
{
	if (megatonIndex >= FB_MEGATON_MAX_POINTER_COUNT)
		return (void*)(0x1);

	if (g_globalMegaton->pointers.buffer[megatonIndex])
		return g_globalMegaton->pointers.buffer[megatonIndex];

	if (void *stateMegaton = GameStateMegaton::megatonIndex < FB_MEGATON_MAX_POINTER_COUNT ? g_globalMegaton->pointers.buffer[GameStateMegaton::megatonIndex] : nullptr)
	{
		if (void *result = ((Megaton*)stateMegaton)->pointers.buffer[megatonIndex])
			return result;

		if (void *sceneMegaton = SceneMegaton::megatonIndex < FB_MEGATON_MAX_POINTER_COUNT ? ((Megaton*)stateMegaton)->pointers.buffer[SceneMegaton::megatonIndex] : nullptr)
			if (void *result = ((Megaton*)sceneMegaton)->pointers.buffer[megatonIndex])
				return result;
	}

#if FB_EDITOR_ENABLED == FB_TRUE
	if (void *stateMegaton = EditorStateMegaton::megatonIndex < FB_MEGATON_MAX_POINTER_COUNT ? g_globalMegaton->pointers.buffer[EditorStateMegaton::megatonIndex] : nullptr)
	{
		if (void *result = ((Megaton*)stateMegaton)->pointers.buffer[megatonIndex])
			return result;

		if (void *sceneMegaton = SceneMegaton::megatonIndex < FB_MEGATON_MAX_POINTER_COUNT ? ((Megaton*)stateMegaton)->pointers.buffer[SceneMegaton::megatonIndex] : nullptr)
			if (void *result = ((Megaton*)sceneMegaton)->pointers.buffer[megatonIndex])
				return result;
	}
#endif

	return (void*)(0x2);
}

FB_END_PACKAGE1()
