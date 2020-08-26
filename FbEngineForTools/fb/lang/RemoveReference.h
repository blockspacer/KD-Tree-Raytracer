#pragma once

FB_PACKAGE1(lang)

template< typename T > struct RemoveReference { typedef T type; };
template< typename T > struct RemoveReference<T&> { typedef T type; };
template< typename T > struct RemoveReference<T&&> { typedef T type; };

FB_END_PACKAGE1()
