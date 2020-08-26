#include "Precompiled.h"
#include "StoreCallstack.h"

#if FB_STORE_CALLSTACK_ENABLED == FB_TRUE

#include "fb/lang/CallstackStorage.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/string/DynamicString.h"

FB_PACKAGE0()

void storeCallstack(const StringRef &name)
{
	if (name.getLength() > 0)
		CallstackStorage::storeCallstack(name, "");
}

void storeCallstack(const StringRef &name, const StringRef &message)
{
	if (name.getLength() > 0)
		CallstackStorage::storeCallstack(name, message);
}

void registerCallstackInfo(const StringRef &name, SizeType maxCount, SizeType offset)
{
	if (name.getLength() > 0)
		CallstackStorage::registerCallstackInfo(name, maxCount, offset);
}

FB_END_PACKAGE0()

#endif

#ifndef FB_STORE_CALLSTACK_ENABLED
#error "FB_STORE_CALLSTACK_ENABLED is not defined"
#endif
