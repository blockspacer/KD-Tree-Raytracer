
// This file allows a "templated" (inline file included) way of adding certain debug stats that are common for 
// containers and container like objects, such as strings.

// Usage: 
//
// create a new usual header file with include guards and stuff, and within it write something like:
//
// #define CONTAINER_DEBUG_STATS_SCOPE fb::container
// #define CONTAINER_DEBUG_STATS_PACKAGE FB_PACKAGE1(container)
// #define CONTAINER_DEBUG_STATS_END_PACKAGE FB_END_PACKAGE1()
// #define CONTAINER_DEBUG_STATS_CLASS YourContainerClass
// #include "fb/memory/stats/ContainerDebugStatsHeaderInline.h"
// 
// (note, the included file will finally undef the defines above)
//
// create a new source file and within it add something like:
//
// #include "YourStatsHeaderFile.h"
//
// #define CONTAINER_DEBUG_STATS_SCOPE fb::container
// #define CONTAINER_DEBUG_STATS_PACKAGE FB_PACKAGE1(container)
// #define CONTAINER_DEBUG_STATS_END_PACKAGE FB_END_PACKAGE1()
// #define CONTAINER_DEBUG_STATS_CLASS YourContainerClass
// #include "fb/memory/stats/ContainerDebugStatsSourceInline.h"
//

// Now, you can use the stats from your container using the calls:
//
// #include "YourStatsHeaderFile.h"
// ...
// YourContainerClass_AllocInc(); // called within the container constructor
// YourContainerClass_AllocDec(); // called within the container destructor
// YourContainerClass_ElementsAmountChange(oldAmount, newAmount); // called whenever the amount of elements changes
// (use -1 for oldAmount if a newly created container, and for newAmount if container being destroyed)

// additional defines
// #define TRACK_CONTAINER_DEBUG_STAT_SIZES_UP_TO_SIZE 256
// - any positive non-zero value enables tracking of containers of different sizes


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
	#define IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE2(a,b) a##b
	#define IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(a,b) IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE2(a,b)
	#define IMPL_FB_EXPAND_FIRST(x) x
#endif
	
CONTAINER_DEBUG_STATS_PACKAGE

void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(IMPL_FB_EXPAND_FIRST(CONTAINER_DEBUG_STATS_CLASS),_AllocInc)();
void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(IMPL_FB_EXPAND_FIRST(CONTAINER_DEBUG_STATS_CLASS),_AllocDec)();

// -1 in either parameter indicates non-existence of the object. (either creation or deletion)
void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(CONTAINER_DEBUG_STATS_CLASS,_ElementsAmountChange)(int oldAmount, int newAmount);

void IMPL_CONTAINER_DEBUG_STATS_SCOPE_MERGE(CONTAINER_DEBUG_STATS_CLASS,_CapacityAmountChange)(int oldAmount, int newAmount);

CONTAINER_DEBUG_STATS_END_PACKAGE

#undef CONTAINER_DEBUG_STATS_CLASS
#undef CONTAINER_DEBUG_STATS_END_PACKAGE
#undef CONTAINER_DEBUG_STATS_PACKAGE
#undef CONTAINER_DEBUG_STATS_SCOPE

