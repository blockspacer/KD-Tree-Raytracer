#pragma once

#include "fb/lang/Types.h"
#include "fb/lang/FBAssert.h"

FB_PACKAGE1(algorithm)

// These function require an sequence already sorted using < or comparator.

/**
 * Find first non-less value in range. Returned value is >= compared to t.
 */
template <typename Iterator, typename T>
Iterator lowerBound(Iterator begin, Iterator end, const T &t)
{
	fb_expensive_assert(begin <= end);

	SizeType count = SizeType(end - begin);
	while (count > 0)
	{
		Iterator it = begin;
		SizeType step = count / 2;
		it += step;
		if (*it < t)
		{
			begin = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return begin;
}

/**
 * Find first non-less value in range with custom comparison. Returned value is >= compared to t.
 */
template <typename Iterator, typename T, typename Compare>
Iterator lowerBound(Iterator begin, Iterator end, const T &t, Compare compare)
{
	fb_expensive_assert(begin <= end);

	SizeType count = SizeType(end - begin);
	while (count > 0)
	{
		Iterator it = begin;
		SizeType step = count / 2;
		it += step;
		if (compare(*it, t))
		{
			begin = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return begin;
}

/**
 * Find first greater value in range. Returned value is < compared to t.
 */
template <typename Iterator, typename T>
Iterator upperBound(Iterator begin, Iterator end, const T &t)
{
	fb_expensive_assert(begin <= end);

	SizeType count = SizeType(end - begin);
	while (count > 0)
	{
		Iterator it = begin;
		SizeType step = count / 2;
		it += step;
		if (!(t < *it))
		{
			begin = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return begin;
}

/**
 * Find first greater value in range with custom comparison. Returned value is < compared to t.
 */
template <typename Iterator, typename T, typename Compare>
Iterator upperBound(Iterator begin, Iterator end, const T &t, Compare compare)
{
	fb_expensive_assert(begin <= end);

	SizeType count = SizeType(end - begin);
	while (count > 0)
	{
		Iterator it = begin;
		SizeType step = count / 2;
		it += step;
		if (!compare(t, *it))
		{
			begin = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return begin;
}

/**
 * Find equal value iterator in range (iterator to given value). Returned iterator points to end if no matches. 
 */
template <typename Iterator, typename T>
Iterator binaryFind(Iterator begin, Iterator end, const T &t)
{
	Iterator it = lowerBound(begin, end, t);
	if (it != end && !(t < *it))
		return it;

	return end;
}

/**
 * Find equal value iterator in range (iterator to given value) with custom comparison. Returned iterator points to end if no matches. 
 */
template <typename Iterator, typename T, typename Compare>
Iterator binaryFind(Iterator begin, Iterator end, const T &t, Compare compare)
{
	Iterator it = lowerBound(begin, end, t, compare);
	if (it != end && !compare(t, *it))
		return it;

	return end;
}

template <typename T>
const T *binaryFindBranchFree(const T *begin, SizeType size, const T &t)
{
	if (size)
	{
		const T *base = begin;
		SizeType n = size;

		while (n > 1) 
		{
			SizeType half = n / 2;
			base = (base[half] < t) ? &base[half] : base;
			n -= half;
		}

		SizeType index = (*base < t) + SizeType(base - begin);
		if (index < size)
			return begin + index;
	}

	return begin;
}

FB_END_PACKAGE1()
