#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE0()


// Do NOT use this for gameplay or similar, this is only meant for development timing.

/// Get high resolution time value in undefined origin
uint64_t getHighResolutionTimeValue();

/// Get high resolution frequency value
uint64_t getHighResolutionTimeFrequency();

/// Get time difference in tenth of a millisecond units
uint64_t getHighResolutionDeltaAsTms(uint64_t start, uint64_t end);


FB_END_PACKAGE0()
