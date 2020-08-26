#pragma once

#include "fb/container/AllocFriendlyList.h"

FB_PACKAGE0()

template<typename T, bool IsPod = false>
class AFList : public container::AllocFriendlyList<T, uint32_t, IsPod>
{

};

template<typename T, bool IsPod = false>
class TinyAFList : public container::AllocFriendlyList<T, uint16_t, IsPod>
{

};

#define FB_AFLISTIMP_TYPE AFList
#include "AllocFriendlyListUtilsImp.h"

#define FB_AFLISTIMP_TYPE TinyAFList
#include "AllocFriendlyListUtilsImp.h"

FB_END_PACKAGE0()
