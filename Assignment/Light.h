#pragma once

#include "Assignment/LightType.h"

FB_PACKAGE1(assignment)

class Light
{
public:
	void setType(const LightType lightType);
	void setColor(const math::VC3 &lightColor);
	void setPosition(const math::VC3 &lightPosition);

	LightType getType() const;
	const math::VC3 &getColor() const;
	const math::VC3 &getPosition() const;

private:
	LightType type = LightTypeNone;
	math::VC3 color;
	math::VC3 pos;
};

FB_END_PACKAGE1()
