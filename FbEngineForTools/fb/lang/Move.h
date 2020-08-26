#pragma once

#include "fb/lang/RemoveReference.h"

FB_PACKAGE1(lang)

template<typename T> inline
constexpr typename lang::RemoveReference<T>::type &&move(T &&t)
{
	return static_cast<typename lang::RemoveReference<T>::type&&>(t);
}

FB_END_PACKAGE1()
