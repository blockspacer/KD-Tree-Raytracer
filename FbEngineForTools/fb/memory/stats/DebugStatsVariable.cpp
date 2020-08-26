#include "Precompiled.h"
#include "DebugStatsVariable.h"

#include "fb/string/HeapString.h"

FB_PACKAGE2(memory, stats)


DebugStatsVariable::DebugStatsVariable(const StaticString &name, DebugStatsVariableImplementingType &implementingVariable, DebugStatsVariable::VariableType variableType, bool *breakpointFlag, SizeType numArrayElements)
	: name(name)
	, implementingVariable(&implementingVariable, numArrayElements)
	, variableType(variableType)
	, breakpointFlag(breakpointFlag)
{
}

DebugStatsVariable::~DebugStatsVariable()
{
}

const DynamicString &DebugStatsVariable::getName() const
{
	return name;
}

DebugStatsVariable::VariableType DebugStatsVariable::getVariableType() const
{
	return variableType;
}

const DebugStatsVariableImplementingType &DebugStatsVariable::getImplementingVariable() const
{
	fb_assert(variableType != VariableTypeIntArray);
	return implementingVariable[0];
}

float DebugStatsVariable::getBadness() const
{
	return getBadness(implementingVariable[0]);
}

float DebugStatsVariable::getBadness(DebugStatsVariableImplementingType value) const
{
	if (badnessLowLimit == 0 && badnessHighLimit == 0)
		return 0.0f;

	if (value <= badnessLowLimit)
		return 0.0f;

	if (value >= badnessHighLimit)
		return 1.0f;

	float range = float(badnessHighLimit - badnessLowLimit);
	return float(value - badnessLowLimit) / range;
}

void DebugStatsVariable::setBadnessLimits(DebugStatsVariableImplementingType lowLimit, DebugStatsVariableImplementingType highLimit)
{
	this->badnessLowLimit = lowLimit;
	this->badnessHighLimit = highLimit;
}


DebugStatsVariableImplementingType *DebugStatsVariable::getImplementingVariablePtr() 
{
	fb_assert(variableType != VariableTypeIntArray);
	return &(implementingVariable[0]);
}

const DebugStatsArrayVariableImplementingType &DebugStatsVariable::getImplementingArrayVariable() const
{
	fb_assert(variableType == VariableTypeIntArray);
	return implementingVariable;
}

DebugStatsVariable::DebugStatsVariable(const DebugStatsVariable &other)
	: name(other.name)
	, implementingVariable(const_cast<DebugStatsVariable&>(other).implementingVariable.getPointer(), other.implementingVariable.getSize())
	, variableType(other.variableType)
	, badnessLowLimit(other.badnessLowLimit)
	, badnessHighLimit(other.badnessHighLimit)
	, breakpointFlag(other.breakpointFlag)
{
}

DebugStatsVariable &DebugStatsVariable::operator= (const DebugStatsVariable &other)
{
	name = other.name;
	variableType = other.variableType;
	badnessLowLimit = other.badnessLowLimit;
	badnessHighLimit = other.badnessHighLimit;
	implementingVariable = other.implementingVariable;
	breakpointFlag = other.breakpointFlag;
	return *this;
}

FB_END_PACKAGE2()

