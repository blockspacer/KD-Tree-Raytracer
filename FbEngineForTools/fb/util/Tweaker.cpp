#include "Precompiled.h"
#include "Tweaker.h"

#if FB_TWEAKER_ENABLED == FB_TRUE

#include "fb/container/StringMapFind.h"
#include "fb/lang/thread/ScopedRef.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/HeapString.h"
#include "fb/util/TweakManager.h"

FB_PACKAGE0()

Tweaker::Tweaker(const StringRef &name)
	: tweakerName(name.getPointer())
	, tweakerNameLength(name.getLength())
{
	TweakManager::getSingleton()->startTweaker(DynamicString(tweakerName, tweakerNameLength));
}
Tweaker::~Tweaker()
{
}
void Tweaker::show()
{
	TweakManager::getSingleton()->showTweaker(DynamicString(tweakerName, tweakerNameLength));
}

TweakVar& getImpl(Tweaker* self, const StringRef &variableName, TweakVar::Type type)
{
	FB_ZONE("Tweaker::getImpl");
	ScopedRef<TweakManager> tweakManager = TweakManager::getSingleton();
	StringRef name(self->tweakerName);
	TweakVar &var = tweakManager->getVar(name, StringRef(variableName.getPointer(), variableName.getLength()), type);
	if (var.order == ~0U)
	{
		SizeType order = 0;
		TweakManager::TweakMap &tweakMap = StringMapFind::findOrInsert(tweakManager->tweaks, name);
		for (TweakManager::TweakMap::Iterator it : tweakMap)
			if (order <= it.getValue().order && it.getValue().order != ~0U)
				order = it.getValue().order + 1;

		var.order = order;
	}
	return var;
}

float Tweaker::get(String variableName, float value)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::Float);
	if (var.userModified)
		return var.floatValue;

	return var.floatValue = value;
}
int Tweaker::get(String variableName, int value)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::Int);
	if (var.userModified)
		return var.intValue;

	var.intValue = value;
	return value;
}
SizeType Tweaker::get(String variableName, SizeType value)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::Int);
	if (var.userModified)
	{
		if (var.intValue < 0)
			var.intValue = 0;

		return SizeType(var.intValue);
	}
	var.intValue = int(value);
	return value;
}
const char* Tweaker::get(String variableName, const char* value)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::String);
	if (var.userModified)
		return var.stringValue.getPointer();

	if (var.stringValue != value)
		var.stringValue = StringRef::make(value);

	return value;
}
bool Tweaker::get(String variableName, bool value)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::Bool);
	if (var.userModified)
		return var.booleanValue;

	var.booleanValue = value;
	return value;
}

math::VC2 Tweaker::get(String variableName, const math::VC2& defaultValue)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::VC2);
	if (var.userModified)
		return math::VC2(var.vcValue[0], var.vcValue[1]);

	var.vcValue[0] = defaultValue.x;
	var.vcValue[1] = defaultValue.y;
	var.vcValue[2] = 0;
	var.vcValue[3] = 0;
	return defaultValue;
}
math::VC3 Tweaker::get(String variableName, const math::VC3& defaultValue)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::VC3);
	if (var.userModified)
		return math::VC3(var.vcValue[0], var.vcValue[1], var.vcValue[2]);

	var.vcValue[0] = defaultValue.x;
	var.vcValue[1] = defaultValue.y;
	var.vcValue[2] = defaultValue.z;
	var.vcValue[3] = 0;
	return defaultValue;
}
math::VC4 Tweaker::get(String variableName, const math::VC4& defaultValue)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::VC4);
	if (var.userModified)
		return math::VC4(var.vcValue);

	var.vcValue[0] = defaultValue.x;
	var.vcValue[1] = defaultValue.y;
	var.vcValue[2] = defaultValue.z;
	var.vcValue[3] = defaultValue.w;
	return defaultValue;
}
math::QUAT Tweaker::get(String variableName, const math::QUAT& defaultValue)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::QUAT);
	if (var.userModified)
		return math::QUAT(var.vcValue);

	var.vcValue[0] = defaultValue.x;
	var.vcValue[1] = defaultValue.y;
	var.vcValue[2] = defaultValue.z;
	var.vcValue[3] = defaultValue.w;
	return defaultValue;
}
math::COL Tweaker::get(String variableName, const math::COL& defaultValue)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::COL);
	if (var.userModified)
		return math::COL(var.vcValue[0], var.vcValue[1], var.vcValue[2]);

	var.vcValue[0] = defaultValue.r;
	var.vcValue[1] = defaultValue.g;
	var.vcValue[2] = defaultValue.b;
	var.vcValue[3] = 0;
	return defaultValue;
}
math::COL4 Tweaker::get(String variableName, const math::COL4& defaultValue)
{
	TweakVar& var = getImpl(this, variableName, TweakVar::COL4);
	if (var.userModified)
		return math::COL4(var.vcValue[0], var.vcValue[1], var.vcValue[2], var.vcValue[3]);

	var.vcValue[0] = defaultValue.r;
	var.vcValue[1] = defaultValue.g;
	var.vcValue[2] = defaultValue.b;
	var.vcValue[3] = defaultValue.a;
	return defaultValue;
}


static void getFloatAsString(HeapString& s, float f)
{
	s.clear();
	s.doSprintf("%g", f);
}
void printImpl(Tweaker* self, const StringRef &variableName, const StringRef &value)
{
	TweakVar &var = getImpl(self, variableName, TweakVar::String);
	var.stringValue = value;
	var.readOnly = true;
}
void Tweaker::print(String variableName, bool value)
{
	printImpl(this, variableName, value ? string::StringLiteral("true") : string::StringLiteral("false"));
}
void Tweaker::print(String variableName, float value)
{
	TempString str;
	getFloatAsString(str, value);
	printImpl(this, variableName, str);
}
void Tweaker::print(String variableName, const char* value)
{
	printImpl(this, variableName, StringRef(value));
}

FB_END_PACKAGE0()

#endif

FB_PACKAGE0()

void testTweaker()
{
	{
		Tweaker twk("asdf");
		FB_UNUSED_NAMED_VAR(float, f) = twk.get("asdf", 1.0f);
		FB_UNUSED_NAMED_VAR(int, i) = twk.get("asdf", 1);
		FB_UNUSED_NAMED_VAR(bool, b) = twk.get("asdf", true);
		FB_UNUSED_NAMED_VAR(const char *, s) = twk.get("asdf", "asdf");
	}
	{
		NoTweaker twk("asdf");
		FB_UNUSED_NAMED_VAR(float, f) = twk.get("asdf", 1.0f);
		FB_UNUSED_NAMED_VAR(int, i) = twk.get("asdf", 1);
		FB_UNUSED_NAMED_VAR(bool, b) = twk.get("asdf", true);
		FB_UNUSED_NAMED_VAR(const char *, s) = twk.get("asdf", "asdf");
	}
}

FB_END_PACKAGE0()
