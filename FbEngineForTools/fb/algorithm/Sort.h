#pragma once

#include "fb/algorithm/IncludeAlgorithm.h"

FB_PACKAGE1(algorithm)

/**
 * Sorts elements in range with default comparison
 */
template<class Iterator> void sort(Iterator begin, Iterator end)
{
	std::sort(begin, end); 
}

/**
 * Sorts elements in range with custom comparison
 */
template<class Iterator, class Compare> void sort(Iterator begin, Iterator end, Compare compare)
{
	std::sort(begin, end, compare); 
}

/**
 * Sorts elements in range with default comparison. Maintains the relative order of records with equal keys.
 */
template<class Iterator> void stableSort(Iterator begin, Iterator end)
{
	std::stable_sort(begin, end); 
}

/**
 * Sorts elements in range with custom comparison. Maintains the relative order of records with equal keys.
 */
template<class Iterator, class Compare> void stableSort(Iterator begin, Iterator end, Compare compare)
{
	std::stable_sort(begin, end, compare); 
}

FB_END_PACKAGE1()
