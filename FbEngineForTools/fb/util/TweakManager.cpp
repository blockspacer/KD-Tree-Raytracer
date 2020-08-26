#include "Precompiled.h"
#include "TweakManager.h"

#include "fb/lang/thread/DataGuard.h"
#include "fb/container/StringMapFind.h"

FB_PACKAGE0()

TweakManager::TweakManager()
{
}
void TweakManager::startTweaker(const StringRef &name)
{
	Tweaks::ConstIterator it = StringMapFind::findIterator(tweaks, name);
	if (it != tweaks.getEnd())
		tweakerStartedSignal(it.getKey());
	else
		tweakerStartedSignal(StaticString::createFromAnyString(name));
}
void TweakManager::showTweaker(const StringRef &name)
{
	Tweaks::ConstIterator it = StringMapFind::findIterator(tweaks, name);
	if (it != tweaks.getEnd())
		tweakerShowSignal(it.getKey());
}

TweakVar& TweakManager::getVar(const StringRef &tweakerName, const StringRef &variable, TweakVar::Type type)
{
	Tweaks::Iterator it = StringMapFind::findIterator(tweaks, tweakerName);
	if (it == tweaks.getEnd())
	{
		fb_assertf(type != TweakVar::Unknown, "Couldn't find tweaker '%s' while looking for its variable '%s'.", tweakerName.getPointer(), variable.getPointer());

		it = tweaks.insert(StaticString::createFromAnyString(tweakerName), TweakMap()).first;
	}

	TweakMap& map = it.getValue();
	TweakMap::Iterator vit = StringMapFind::findIterator(map, variable);
	if (vit == map.getEnd())
	{
		fb_assertf(type != TweakVar::Unknown, "Couldn't find variable in '%s' tweaker '%s'.", tweakerName.getPointer(), variable.getPointer());

		vit = map.insert(StaticString::createFromAnyString(variable), TweakVar()).first;
		vit.getValue().type = type;
	}

	fb_assertf(vit.getValue().type == type || type == TweakVar::Unknown, "Variable type mismatch (%d != %d)", vit.getValue().type, type);
	return vit.getValue();
}

ScopedRef<TweakManager> TweakManager::getSingleton()
{
	static DataGuard<TweakManager> singleton("TweakManager DataGuard");
	return singleton;
}

FB_END_PACKAGE0()
