#ifndef FB_MEMORY_STATS_IDEBUGSTATSBREAKPOINTLISTENER_H
#define FB_MEMORY_STATS_IDEBUGSTATSBREAKPOINTLISTENER_H

#include "DebugStatsVariableImplementingType.h"

FB_DECLARE(memory, stats, DebugStatsVariable)

FB_PACKAGE2(memory, stats)

class IDebugStatsBreakpointListener
{
public:
	virtual ~IDebugStatsBreakpointListener() { };

	virtual void breakOccured(const DebugStatsVariable &variable, DebugStatsVariableImplementingType value) = 0;
};

FB_END_PACKAGE2()

#endif
