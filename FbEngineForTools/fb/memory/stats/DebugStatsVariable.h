#pragma once

#include "DebugStatsVariableImplementingType.h"
#include "fb/lang/FBExplicitTypedef.h"
#include "fb/string/StaticString.h"

FB_DECLARE(memory, stats, DebugStats);

FB_PACKAGE2(memory, stats)

class DebugStatsVariableImpl;
class BreakpointListenerImpl;

class DebugStatsVariable
{
public:
	// there really is no support for other than int types, some of them are just allocation count ints,
	// which have some specific boundary checks and leak detections.
	enum VariableType
	{
		VariableTypeUndefinedInt, // just some unspecified (integer) type
		VariableTypeAllocationCount, // allocation count.
		VariableTypeIntArray // an int array
	};

	DebugStatsVariable(const StaticString &name, DebugStatsVariableImplementingType &implementingVariable, VariableType variableType, bool *breakpointFlag, SizeType numElements = 1);
	DebugStatsVariable(const DebugStatsVariable &other);
	~DebugStatsVariable();

	DebugStatsVariable &operator= (const DebugStatsVariable &other);

	const DynamicString &getName() const;

	VariableType getVariableType() const;

	const DebugStatsVariableImplementingType &getImplementingVariable() const;
	float getBadness() const;
	float getBadness(DebugStatsVariableImplementingType value) const;
	void setBadnessLimits(DebugStatsVariableImplementingType lowLimit, DebugStatsVariableImplementingType highLimit);

	const DebugStatsArrayVariableImplementingType &getImplementingArrayVariable() const;

	// (intended for breakpoint use only)
	DebugStatsVariableImplementingType *getImplementingVariablePtr();

private:
	StaticString name;
	DebugStatsArrayVariableImplementingType implementingVariable;
	DebugStatsVariable::VariableType variableType;

	DebugStatsVariableImplementingType badnessLowLimit = 0;
	DebugStatsVariableImplementingType badnessHighLimit = 0;

	// breakpoint stuff.
	bool *breakpointFlag;

	friend class DebugStats;
	friend class BreakpointListenerImpl;
};

FB_END_PACKAGE2()
