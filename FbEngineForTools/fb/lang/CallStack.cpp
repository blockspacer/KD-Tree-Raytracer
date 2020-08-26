#include "Precompiled.h"
#include "CallStack.h"
#include "CallStackListener.h"
#include "MemoryFunctions.h"

// Hacky internal defines to avoid rebuilding
#define DISABLE_CALLSTACK

FB_PACKAGE2(lang, detail)
namespace {

	CallStackListener **listeners = 0;
	int listenerAmount = 0;
}

void pushIdentifier(const char *className, const char *functionName)
{
	#ifdef DISABLE_CALLSTACK
		return;
	#else
		for (int i = 0; i < listenerAmount; ++i)
		{
			CallStackListener *l = listeners[i];
			if (l)
				l->pushFunction(className, functionName);
		}
	#endif
}

void pushIdentifier(const char *id)
{
	#ifdef DISABLE_CALLSTACK
		return;
	#else
		pushIdentifier(0, id);
	#endif
}

void popIdentifier(const char *className, const char *functionName)
{
	#ifdef DISABLE_CALLSTACK
		return;
	#else
		for (int i = 0; i < listenerAmount; ++i)
		{
			CallStackListener *l = listeners[i];
			if (l)
				l->popFunction(className, functionName);
		}
	#endif
}

FB_END_PACKAGE2()

FB_PACKAGE1(lang)

void addStackListener(CallStackListener *listener)
{
	++detail::listenerAmount;
	detail::listeners = reinterpret_cast<CallStackListener **> (reallocateMemory(detail::listeners, detail::listenerAmount * sizeof(CallStackListener *)));
	detail::listeners[detail::listenerAmount - 1] = listener;
}

void removeStackListener(CallStackListener *listener)
{
	for (int i = 0; i < detail::listenerAmount; ++i)
	{
		// KISS, so just zero it
		if (detail::listeners[i] == listener)
			detail::listeners[i] = 0;
	}
}

FB_END_PACKAGE1()
