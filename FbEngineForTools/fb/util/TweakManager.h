#pragma once

#include "fb/container/LinearHashMap.h"
#include "fb/container/LinearMap.h"
#include "fb/lang/thread/ScopedRefDecl.h"
#include "fb/lang/Signal.h"
#include "fb/string/StaticString.h"

FB_PACKAGE0()

struct TweakVar
{
	enum Type { String, Float, Int, Bool, VC2, VC3, VC4, QUAT, COL, COL4, Unknown };
	Type type;
	bool userModified = false;
	bool readOnly = false;
	SizeType order = ~0U;

	DynamicString stringValue;
	float floatValue = 0;
	int intValue = 0;
	bool booleanValue = false;
	float vcValue[4];
};

class TweakManager
{
public:
	static ScopedRef<TweakManager> getSingleton();

	typedef LinearMap<StaticString, TweakVar> TweakMap;
	typedef LinearMap<StaticString, TweakMap> Tweaks;
	Tweaks tweaks;
	SignalBind stateCloseBind;
	Signal<void(const StaticString &tweakerName)> tweakerStartedSignal;
	Signal<void(const StaticString &tweakerName)> tweakerShowSignal;

	TweakManager();
	void startTweaker(const StringRef &tweakerName);
	void showTweaker(const StringRef &tweakerName);

	TweakVar& getVar(const StringRef &tweakerName, const StringRef &variable, TweakVar::Type type = TweakVar::Unknown);
};

FB_END_PACKAGE0()
