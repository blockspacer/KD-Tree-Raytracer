#include "Precompiled.h"
#include "InputLagDebugger.h"

#if FB_INPUT_LAG_DEBUGGER_ENABLED == FB_TRUE

#include "fb/lang/Atomics.h"
#include "fb/lang/time/HighResolutionTime.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/HeapString.h"

FB_PACKAGE1(profiling)

struct InputLagDebuggerData
{
	lang::AtomicUInt64 timeButtonPressedTime;
	lang::AtomicUInt64 timeQueryMessages;
	lang::AtomicUInt64 timeQueryMessagesStored;
	lang::AtomicUInt64 timeButtonSignal;
	lang::AtomicUInt64 timeRendered;

	lang::AtomicUInt64 triggered;
	lang::AtomicUInt64 trackedElementName;
	lang::AtomicUInt64 trackedEffectData;

	lang::AtomicUInt64 triggerCounter;
	lang::AtomicUInt64 isEnabled;
};
static InputLagDebuggerData data;

void InputLagDebugger::queryingInput()
{
	if (!getIsEnabled())
		return;

	FB_ZONE("inputLagDetection querying input");
	atomicStoreRelaxed(data.timeQueryMessages, getHighResolutionTimeValue());
}
void InputLagDebugger::buttonPressed(uint32_t elementNameEnum)
{
	if (!getIsEnabled())
		return;

	FB_ZONE("inputLagDetection button press registered");
	atomicStoreRelaxed(data.trackedElementName, elementNameEnum);
	atomicStoreRelaxed(data.timeQueryMessagesStored, atomicLoadRelaxed(data.timeQueryMessages));
	atomicStoreRelaxed(data.timeButtonPressedTime, getHighResolutionTimeValue());
}
bool InputLagDebugger::inputEventSignaled(uint32_t elementNameEnum, uint64_t effectData)
{
	if (elementNameEnum != atomicLoadRelaxed(data.trackedElementName))
		return false;

	FB_ZONE("inputLagDetection effects passed to GuiSystem");
	atomicStoreRelaxed(data.triggered, true);
	atomicStoreRelaxed(data.timeButtonSignal, getHighResolutionTimeValue());
	atomicStoreRelaxed(data.trackedEffectData, effectData);
	return true;
}
void InputLagDebugger::effectsRendered(uint64_t effectdata)
{
	if (effectdata != atomicLoadRelaxed(data.trackedEffectData))
	{
		// likely
	}
	else
	{
		FB_ZONE("inputLagDetection effects passed to gpu");
		atomicStoreRelaxed(data.triggered, false);
		atomicStoreRelaxed(data.timeRendered, getHighResolutionTimeValue());
		atomicIncRelaxed(data.triggerCounter);
		atomicStoreRelaxed(data.trackedEffectData, ~uint64_t(0));
	}
}

void InputLagDebugger::getDebugString(HeapString& outString)
{
	FB_ZONE("inputLagDetection outputting statistics string");
	auto diff = [](uint64_t  end, uint64_t  start)
	{
		return (end - start) / (getHighResolutionTimeFrequency() / (1000.0));
	};
	uint64_t  query = atomicLoadRelaxed(data.timeQueryMessagesStored);
	uint64_t  received = atomicLoadRelaxed(data.timeButtonPressedTime);
	uint64_t  buttonSignal = atomicLoadRelaxed(data.timeButtonSignal);
	uint64_t  guiRender = atomicLoadRelaxed(data.timeRendered);

	query = query != 0 ? query : received;
	uint64_t  start = query < received ? query : received;

	outString.doSprintf("Querying input: %" FB_FSU64 " ticks, %.3f ms\n", query - start, diff(query, start));
	outString.doSprintf("Button pressed: %" FB_FSU64 " ticks, %.3f ms\n", received - start, diff(received, start));
	outString.doSprintf("Button signaled: %" FB_FSU64 " ticks, %.3f ms\n", buttonSignal - start, diff(buttonSignal, start));
	outString.doSprintf("Effects rendered: %" FB_FSU64 " ticks, %.3f ms\n", guiRender - start, diff(guiRender, start));
}

uint64_t InputLagDebugger::getCounter()
{
	return atomicLoadRelaxed(data.triggerCounter);
}

bool InputLagDebugger::getIsEnabled()
{
	return atomicLoadRelaxed(data.isEnabled) != 0;
}
void InputLagDebugger::setIsEnabled(bool enabled)
{
	atomicStoreRelaxed(data.isEnabled, enabled ? 1U : 0U);
}

FB_END_PACKAGE1()

#endif
