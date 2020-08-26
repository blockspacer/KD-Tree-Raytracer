#include "Precompiled.h"
#include "DebugStatsScope.h"

FB_PACKAGE2(memory, stats)

DebugStatsScope::DebugStatsScope(const StaticString &name)
	: name(name)
{
}

DebugStatsScope::~DebugStatsScope()
{
}

const DynamicString &DebugStatsScope::getName() const
{
	return name;
}

DebugStatsVariableIterator DebugStatsScope::getVariables() const
{
	return DebugStatsVariableIterator(this);
}

DebugStatsVariable &DebugStatsScope::addVariable(const DebugStatsVariable &variable)
{
	variables.pushBack(variable);
	return variables.getBack();
}

const DebugStatsVariable *DebugStatsVariableIterator::next()
{ 
	fb_expensive_assert(nextVariableIndex <= scope->variables.getSize());
	if (nextVariableIndex < scope->variables.getSize())
		return &scope->variables[nextVariableIndex++];
	else
		return nullptr;
}


DebugStatsScope::DebugStatsScope(DebugStatsScope &&other)
	: name(lang::move(other.name))
	, variables(lang::move(other.variables))
{
}

FB_END_PACKAGE2()

