#pragma once

#include "fb/lang/Config.h"

FB_PACKAGE0()

/**
 * FB_EXPLICIT_TYPEDEF is used similarly to normal C++ typedef. It does, however, create an explicit (strong) type 
 * which does not get converted implicitly to the other implementing type.
 *
 * If the FB_EXPLICIT_TYPEDEF_ENABLED != FB_TRUE, then the macro get expanded to the normal C++ typedef (this is 
 * probably the case one may want for the final release version of the application.
 *
 * There is also the macro FB_EXPLICIT_TYPEDEF_ALWAYS which implements the explicit type behaviour, even if the normal 
 * macro is not enabled. You should only use the always if you intend to use the explicit type to automatically detect 
 * between different parameter types or such, instead of just type safeness.
 *
 * This code is mostly adapted from Boost's strong typedef with some FB styling and additional operators. I think this 
 * could be done more cleanly (at least for some values of clean) in C++11, but every example I ran across was in 
 * practice just as convoluted.
 */

namespace explicittypedef
{
	template <typename T>
	class EmptyBase
	{
	};

	template <class T, class B = EmptyBase<T> >
	struct LessThanComparable1 : B
	{
		friend bool operator>(const T& x, const T& y) { return y < x; }
		friend bool operator<=(const T& x, const T& y) { return !(y < x); }
		friend bool operator>=(const T& x, const T& y) { return !(x < y); }
	};

	template <class T, class U, class B = EmptyBase<T> >
	struct LessThanComparable2 : B
	{
		friend bool operator<=(const T& x, const U& y) { return !(x > y); }
		friend bool operator>=(const T& x, const U& y) { return !(x < y); }
		friend bool operator>(const U& x, const T& y) { return y < x; }
		friend bool operator<(const U& x, const T& y) { return y > x; }
		friend bool operator<=(const U& x, const T& y) { return !(y < x); }
		friend bool operator>=(const U& x, const T& y) { return !(y > x); }
	};

	template <class T, class B = EmptyBase<T> >
	struct EqualityComparable1 : B
	{
		friend bool operator!=(const T& x, const T& y) { return !(x == y); }
	};

	template <class T, class U, class B = EmptyBase<T> >
	struct EqualityComparable2 : B
	{
		friend bool operator==(const U& y, const T& x) { return x == y; }
		friend bool operator!=(const U& y, const T& x) { return !(x == y); }
		friend bool operator!=(const T& y, const U& x) { return !(y == x); }
	};

	template <class T, class B = EmptyBase<T> >
	struct TotallyOrdered1 : LessThanComparable1<T, EqualityComparable1<T, B > >
	{
	};

	template <class T, class U, class B = EmptyBase<T> >
	struct TotallyOrdered2 : LessThanComparable2<T, U, EqualityComparable2<T, U, B> >
	{
	};

}

#define FB_EXPLICIT_TYPEDEF_ALWAYS(p_implementing_type, p_new_typename) \
	struct p_new_typename : fb::explicittypedef::TotallyOrdered1<p_new_typename, fb::explicittypedef::TotallyOrdered2<p_new_typename, p_implementing_type> > \
	{ \
		explicit p_new_typename(const p_implementing_type &impType) : impType(impType) { } \
		explicit p_new_typename(const p_implementing_type &&impType) : impType(lang::move(impType)) { } \
	    p_new_typename() = default; \
		p_new_typename(const p_new_typename &) = default; \
		p_new_typename(p_new_typename&&) = default; \
		p_new_typename &operator=(const p_new_typename&) = default; \
		p_new_typename &operator=(p_new_typename&&) = default; \
		operator p_implementing_type& () { return impType; } \
		operator const p_implementing_type& () const { return impType; } \
		bool operator==(const p_new_typename &other) const { return impType == other.impType; } \
		bool operator<(const p_new_typename &other) const { return impType < other.impType; } \
		p_implementing_type impType; \
	};

#if (FB_EXPLICIT_TYPEDEF_ENABLED == FB_TRUE)

	#define FB_EXPLICIT_TYPEDEF(p_implementing_type, p_new_typename) FB_EXPLICIT_TYPEDEF_ALWAYS(p_implementing_type, p_new_typename)

#else

	#define FB_EXPLICIT_TYPEDEF(p_implementing_type, p_new_typename) typedef p_implementing_type p_new_typename

#endif

FB_END_PACKAGE0()
