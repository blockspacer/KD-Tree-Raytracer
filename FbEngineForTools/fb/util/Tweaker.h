#pragma once

#include "fb/math/Vec2.h"
#include "fb/math/Vec3.h"
#include "fb/math/Vec4.h"
#include "fb/math/Color3.h"
#include "fb/math/Color4.h"
#include "fb/math/Quaternion.h"
#include "fb/string/StringLiteral.h"

FB_DECLARE0(StringRef)

FB_PACKAGE0()

#ifndef FB_TWEAKER_ENABLED
	#if FB_BUILD != FB_FINAL_RELEASE
		#define FB_TWEAKER_ENABLED FB_TRUE
	#else
		#define FB_TWEAKER_ENABLED FB_FALSE
	#endif
#endif

struct NoTweaker
{
	template<typename S> NoTweaker(const S &) {}
	~NoTweaker() {}
	void show() {}

	template<typename S, typename T> constexpr const T& get(const S &, const T& val) const { return val; }
	template<typename S, typename T> void print(const S &, const T&) const { }
};

#if FB_TWEAKER_ENABLED == FB_TRUE
struct Tweaker
{
	typedef string::StringLiteral String;

	Tweaker(const StringRef &name);
	~Tweaker();
	void show();
	float get(String variableName, float defaultValue);
	int32_t get(String variableName, int32_t defaultValue);
	SizeType get(String variableName, SizeType defaultValue);
	const char *get(String variableName, const char *defaultValue);
	bool get(String variableName, bool defaultValue);

	math::VC2 get(String variableName, const math::VC2& defaultValue);
	math::VC3 get(String variableName, const math::VC3& defaultValue);
	math::VC4 get(String variableName, const math::VC4& defaultValue);
	math::QUAT get(String variableName, const math::QUAT& defaultValue);
	math::COL get(String variableName, const math::COL& defaultValue);
	math::COL4 get(String variableName, const math::COL4& defaultValue);

	// NOTE: When creating new overloads for get or print, add them also to the FINAL_RELEASE version below

	void print(String variableName, bool value);
	void print(String variableName, float value);
	void print(String variableName, const char *value);

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// NOTE: This doesn't exist in FINAL_RELEASE
	// It is not private because free functions in the .cpp file are using it
	const char *tweakerName;
	SizeType tweakerNameLength;
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
};
#else
struct Tweaker : public NoTweaker
{
	template<typename S>
	Tweaker(const S &)
		: NoTweaker("")
	{
	}
};
#endif

FB_END_PACKAGE0()
