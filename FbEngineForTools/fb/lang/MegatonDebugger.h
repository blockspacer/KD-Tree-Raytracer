#pragma once

#include "fb/lang/Types.h"

/* This is also used by Megaton. Should really be in some MegatonConfig.h, but let's not get too purist */
#define FB_MEGATON_MAX_POINTER_COUNT (SizeType)(256)

FB_PACKAGE1(lang)

struct MegatonDebugPointer
{
	const char **name;
	void **value;
	SizeType index;
	uint64_t *usage;
	uint64_t *fallback;
	uint64_t *quickUsage;
};

class MegatonDebugger
{
public:
	MegatonDebugPointer pointers[FB_MEGATON_MAX_POINTER_COUNT];
	MegatonDebugger(void **originalPointers);

	// Used by natvis visualizers instead of EditorStateMegaton::megatonIndex because that doesn't exist when editor is not enabled
	static SizeType editorMegatonIndex;

	// Collect class names to same indices as their megaton pointer
	static const char *megatonNames[FB_MEGATON_MAX_POINTER_COUNT];
	static uint64_t megatonQuickUsage[FB_MEGATON_MAX_POINTER_COUNT];

	static uint64_t megatonUsageCounters[FB_MEGATON_MAX_POINTER_COUNT];
	static uint64_t megatonFallbackCounters[FB_MEGATON_MAX_POINTER_COUNT];

	static void *getMegatonPointer(uint32_t megatonIndex);
};


FB_END_PACKAGE1()
