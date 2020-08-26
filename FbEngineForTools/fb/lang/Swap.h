#pragma once

#include "fb/lang/Move.h"

FB_PACKAGE1(lang)

template<typename T>
void swap(T &a, T &b)
{
	T temp(lang::move(a));
	a = lang::move(b);
	b = lang::move(temp);
}

FB_END_PACKAGE1()
