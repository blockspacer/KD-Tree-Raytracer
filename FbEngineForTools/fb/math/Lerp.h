#pragma once

#include "fb/math/Vec2.h"
#include "fb/math/Vec3.h"

FB_PACKAGE1(math)

	FB_FORCEINLINE float lerp(float a, float b, float delta)
	{
		return a + (b - a) * delta;
	}

	FB_FORCEINLINE double lerp(double a, double b, double delta)
	{
		return a + (b - a) * delta;
	}

	FB_FORCEINLINE math::VC2 lerp(const math::VC2& a, const math::VC2& b, float delta)
	{
		return a + (b - a) * delta;
	}

	FB_FORCEINLINE math::VC3 lerp(const math::VC3& a, const math::VC3& b, float delta)
	{
		return a + (b - a) * delta;
	}

FB_END_PACKAGE1()
