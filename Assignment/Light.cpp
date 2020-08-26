#include "Precompiled.h"
#include "Light.h"

FB_PACKAGE1(assignment)

void Light::setType(const LightType lightType)
{
	type = lightType;
}

void Light::setColor(const math::VC3 &lightColor)
{
	color = lightColor;
}

void Light::setPosition(const math::VC3 &lightPosition)
{
	pos = lightPosition;
}

LightType Light::getType() const
{
	return type;
}

const math::VC3 &Light::getColor() const
{
	return color;
}

const math::VC3 &Light::getPosition() const
{
	return pos;
}

FB_END_PACKAGE1()
