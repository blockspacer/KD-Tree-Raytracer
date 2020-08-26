
#include "fb/memory/stats/DebugStats.h"

#ifndef CONTAINER_DEBUG_STATS_CLASS
	#error "CONTAINER_DEBUG_STATS_CLASS define expected."
#endif
#ifndef CONTAINER_DEBUG_STATS_END_PACKAGE
	#error "CONTAINER_DEBUG_STATS_END_PACKAGE define expected."
#endif
#ifndef CONTAINER_DEBUG_STATS_PACKAGE
	#error "CONTAINER_DEBUG_STATS_PACKAGE define expected."
#endif
#ifndef CONTAINER_DEBUG_STATS_SCOPE
	#error "CONTAINER_DEBUG_STATS_SCOPE define expected."
#endif

#ifndef IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE
	#error "IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE define missing, make sure you have included your debug stats header file."
#endif

#define CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS CONTAINER_DEBUG_STATS_SCOPE::CONTAINER_DEBUG_STATS_CLASS

// get around the problematic order of macro expansion 
#define FB_IMPL_GET(x) FB_DEBUG_STATS_GET(x)
#define FB_IMPL_VAR(p_scope, p_var) FB_DEBUG_STATS_VAR_NO_LIMIT(p_scope, p_var)
#define FB_IMPL_ARRAY_VAR(p_scope, p_var, p_numelems) FB_DEBUG_STATS_ARRAY_VAR_NO_LIMIT(p_scope, p_var, p_numelems)
#define FB_IMPL_INC(x) FB_DEBUG_STATS_INC(x)
#define FB_IMPL_DEC(x) FB_DEBUG_STATS_DEC(x)

// these would be preferable, but will cause problems with uninitialization of some static containers...
// note, this cannot be an ALLOC type debug stat, because that would cause the leak check to fail - because 
// there are several static container, the uninit order of which cannot be determined.
//#define FB_IMPL_ALLOC_VAR(p_scope, p_var) FB_DEBUG_STATS_ALLOC_VAR_NO_LIMIT(p_scope, p_var);
//#define FB_IMPL_INC(x) FB_DEBUG_STATS_ALLOC_INC(x)
//#define FB_IMPL_DEC(x) FB_DEBUG_STATS_ALLOC_DEC(x)

CONTAINER_DEBUG_STATS_PACKAGE

#ifndef TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE 
	#define TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE 128
#endif

FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, CONTAINER_DEBUG_STATS_CLASS);
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, amountOfElements);
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, averageElementsPerInstanceRounded);
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, amountOfTotalCapacity);
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, averageCapacityPerInstanceRounded);
#if (TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE == 0)
	// no individual size tracking
#else
	FB_IMPL_ARRAY_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, instancesOfLength, (TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE + 1));
	FB_IMPL_ARRAY_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, instancesOfCapacity, (TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE + 1));
#endif
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, peakInstanceLength);
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, peakInstanceCapacity);
FB_IMPL_VAR(CONTAINER_DEBUG_STATS_SCOPE_AND_CLASS, capacityUsagePercentage);

void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(CONTAINER_DEBUG_STATS_CLASS,_AllocInc)()
{
	FB_IMPL_INC(CONTAINER_DEBUG_STATS_CLASS);
}

void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(CONTAINER_DEBUG_STATS_CLASS,_AllocDec)()
{
	FB_IMPL_DEC(CONTAINER_DEBUG_STATS_CLASS);
}

void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(CONTAINER_DEBUG_STATS_CLASS,_ElementsAmountChange)(int oldAmount, int newAmount)
{
	if (oldAmount == newAmount)
		return;

	// track individual container sizes...
#if (TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE == 0)
	// no individual size tracking
#else
	int clampedOld = (oldAmount < TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE) ? oldAmount : TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE;
	int clampedNew = (newAmount < TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE) ? newAmount : TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE;
	if (clampedOld >= 0)
	{
		FB_DEBUG_STATS_ARRAY_DEC(instancesOfLength, clampedOld);
	}
	if (clampedNew >= 0)
	{
		FB_DEBUG_STATS_ARRAY_INC(instancesOfLength, clampedNew);
	}
#endif

	// general summed up container statistics
	int amountOfElementsNew = FB_DEBUG_STATS_GET(amountOfElements) + (newAmount - oldAmount);
	FB_DEBUG_STATS_SET(amountOfElements, amountOfElementsNew);
	if (FB_IMPL_GET(CONTAINER_DEBUG_STATS_CLASS) > 0)
	{
		FB_DEBUG_STATS_SET(averageElementsPerInstanceRounded, amountOfElementsNew / FB_IMPL_GET(CONTAINER_DEBUG_STATS_CLASS));
	} else {
		FB_DEBUG_STATS_SET(averageElementsPerInstanceRounded, 0);
	}
	if (newAmount > FB_DEBUG_STATS_GET(peakInstanceLength))
	{
		FB_DEBUG_STATS_SET(peakInstanceLength, newAmount );
	}
}

void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(CONTAINER_DEBUG_STATS_CLASS,_CapacityAmountChange)(int oldAmount, int newAmount)
{
	if (oldAmount == newAmount)
		return;

	// track individual container capacity sizes...
#if (TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE == 0)
	// no individual size tracking
#else
	int clampedOld = (oldAmount < TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE) ? oldAmount : TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE;
	int clampedNew = (newAmount < TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE) ? newAmount : TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE;
	if (clampedOld >= 0)
	{
		FB_DEBUG_STATS_ARRAY_DEC(instancesOfCapacity, clampedOld);
	}
	if (clampedNew >= 0)
	{
		FB_DEBUG_STATS_ARRAY_INC(instancesOfCapacity, clampedNew);
	}
#endif

	// general summed up container statistics
	int amountOfTotalCapacityNew = FB_DEBUG_STATS_GET(amountOfTotalCapacity) + (newAmount - oldAmount);
	FB_DEBUG_STATS_SET(amountOfTotalCapacity, amountOfTotalCapacityNew);
	if (FB_IMPL_GET(CONTAINER_DEBUG_STATS_CLASS) > 0)
	{
		FB_DEBUG_STATS_SET(averageCapacityPerInstanceRounded, amountOfTotalCapacityNew / FB_IMPL_GET(CONTAINER_DEBUG_STATS_CLASS));
	} else {
		FB_DEBUG_STATS_SET(averageCapacityPerInstanceRounded, 0);
	}
	if (newAmount > FB_DEBUG_STATS_GET(peakInstanceCapacity))
	{
		FB_DEBUG_STATS_SET(peakInstanceCapacity, newAmount );
	}
	if (FB_IMPL_GET(amountOfTotalCapacity) > 0)
	{
		FB_DEBUG_STATS_SET(capacityUsagePercentage, (100 * FB_DEBUG_STATS_GET(amountOfElements)) / FB_IMPL_GET(amountOfTotalCapacity));
	} else {
		FB_DEBUG_STATS_SET(capacityUsagePercentage, 0);
	}
}


CONTAINER_DEBUG_STATS_END_PACKAGE

#undef CONTAINER_DEBUG_STATS_CLASS
#undef CONTAINER_DEBUG_STATS_END_PACKAGE
#undef CONTAINER_DEBUG_STATS_PACKAGE
#undef CONTAINER_DEBUG_STATS_SCOPE
