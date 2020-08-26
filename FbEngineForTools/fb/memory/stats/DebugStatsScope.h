#pragma once

#include "fb/container/Vector.h"
#include "fb/memory/stats/DebugStatsVariableIterator.h"
#include "fb/string/StaticString.h"

FB_PACKAGE2(memory, stats)

class DebugStatsScopeImpl;

class DebugStatsScope
{
public:
	DebugStatsScope() { }
	DebugStatsScope(const StaticString &name);
	DebugStatsScope(DebugStatsScope &&other);
	~DebugStatsScope();

	const DynamicString &getName() const;

	/**
	 * Returns the debug stats variable iterator for this scope.
	 * Notice, that the iterator iterates the variables as const, because you should not fiddle around with their state.
	 */
	DebugStatsVariableIterator getVariables() const;

	/* Adds the DebugStatsVariable to this scope. Will make an own copy of the given const parameter */
	DebugStatsVariable &addVariable(const DebugStatsVariable &variable);

	bool isValid() const { return !name.isEmpty(); }

private:
	StaticString name;
	Vector<DebugStatsVariable> variables;

	friend class DebugStatsVariableIterator;
};

FB_END_PACKAGE2()
