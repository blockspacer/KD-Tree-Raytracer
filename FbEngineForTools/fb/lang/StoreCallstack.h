#pragma once

#include "fb/lang/Types.h"

FB_DECLARE0(StringRef)

FB_PACKAGE0()

#ifndef FB_STORE_CALLSTACK_ENABLED
#if FB_BUILD != FB_FINAL_RELEASE
#define FB_STORE_CALLSTACK_ENABLED FB_TRUE
#else
#define FB_STORE_CALLSTACK_ENABLED FB_FALSE
#endif
#endif

#if FB_STORE_CALLSTACK_ENABLED == FB_TRUE

extern void storeCallstack(const StringRef &name);
extern void storeCallstack(const StringRef &name, const StringRef &message);
extern void registerCallstackInfo(const StringRef &name, SizeType maxCount, SizeType offset);

#else

inline void storeCallstack(const StringRef &name)
{
}
inline void storeCallstack(const StringRef &name, const StringRef &message)
{
}
inline void registerCallstackInfo(const StringRef &name, SizeType maxCount, SizeType offset)
{
}

#endif

FB_END_PACKAGE0()
