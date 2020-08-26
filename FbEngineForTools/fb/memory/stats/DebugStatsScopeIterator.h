#ifndef FB_MEMORY_STATS_DEBUGSTATSSCOPEITERATOR_H
#define FB_MEMORY_STATS_DEBUGSTATSSCOPEITERATOR_H

FB_DECLARE(memory, stats, DebugStats)

FB_PACKAGE2(memory, stats)

class DebugStatsScopeIterator
{
public:
	/**
	 * Returns the next scope or null if no more scopes.
	 */
	const DebugStatsScope *next();

private:
	SizeType nextScopeIndex;
	DebugStatsScopeIterator(const DebugStats *debugStats) : nextScopeIndex(0), debugStats(debugStats) { }

	const DebugStats *debugStats;

	friend class DebugStats;
};

FB_END_PACKAGE2()

#endif
