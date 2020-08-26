#include "Precompiled.h"

#include "HeapAddressCheck.h"

#if (FB_COMPILER == FB_MSC)	
#include "fb/lang/IncludeWindows.h"
#endif

FB_PACKAGE1(memory)

const void *stackMax = 0;
const void *stackMin = 0;
uint32_t mainStackThreadId = 0;

#if ( FB_COMPILER == FB_MSC)
void initCheckHeapAddress(const void *ptrToStack)
{
	mainStackThreadId = GetCurrentThreadId();
	stackMax = ptrToStack;
	// 1MB stack is default, the pointer is probably a little bit below it
	fb_assert((intptr_t)stackMax > 768*1024);
	stackMin = ((const char *)ptrToStack) - 768*1024;
}

// this non-inline function may cause a quite significant performance hit if heap check is used a lot.
// but it is currently required so that only the main thread will do the heap checks (as other threads
// will have their heap at different addresses).
// another way would be to increment the mainStackBase at every new thread startup to max value of all stacks...
// (that, however, would probably break the main stack vs. heap checks pretty completely unless done in a bit
// more complex manner)
bool hasMainThreadHeap()
{
	return (GetCurrentThreadId() == mainStackThreadId);
}
#endif

FB_END_PACKAGE1()

