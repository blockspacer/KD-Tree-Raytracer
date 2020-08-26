#pragma once

#if FB_BUILD == FB_FINAL_RELEASE
	#define FB_INPUT_LAG_DEBUGGER_ENABLED FB_FALSE
#else
	#define FB_INPUT_LAG_DEBUGGER_ENABLED FB_TRUE
#endif

FB_DECLARE0(HeapString);

FB_PACKAGE1(profiling)

struct InputLagDebugger
{
#if FB_INPUT_LAG_DEBUGGER_ENABLED == FB_TRUE
	static void queryingInput();
	static void buttonPressed(uint32_t elementNameEnum);
	static bool inputEventSignaled(uint32_t elementNameEnum, uint64_t effectdata);
	static void effectsRendered(uint64_t effectdata);
	static void getDebugString(HeapString& outString);
	static uint64_t getCounter();
	static bool getIsEnabled();
	static void setIsEnabled(bool enabled);
#else
	static void queryingInput() {}
	static void buttonPressed(uint32_t elementNameEnum) {}
	static bool inputEventSignaled(uint32_t elementNameEnum, uint64_t effectdata) { return false; }
	static void effectsRendered(uint64_t effectdata) {}
	static void getDebugString(HeapString& outString) {}
	static uint64_t getCounter() { return 0; }
	static bool getIsEnabled() { return false; }
	static void setIsEnabled(bool) { }
#endif
};

FB_END_PACKAGE1()
