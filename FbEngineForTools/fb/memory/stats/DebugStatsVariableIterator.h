#pragma once

#include "fb/memory/stats/DebugStatsVariable.h"

FB_DECLARE(memory, stats, DebugStatsScope)

FB_PACKAGE2(memory, stats)

class DebugStatsVariableIterator
{
public:
	/**
	 * Returns the next variable or null if no more variables.
	 */
	const DebugStatsVariable *next();

private:
	const DebugStatsScope *scope;
	SizeType nextVariableIndex;
	DebugStatsVariableIterator(const DebugStatsScope *scope) : scope(scope), nextVariableIndex(0) { }
	friend class DebugStatsScope;
};

FB_END_PACKAGE2()
