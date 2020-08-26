#pragma once

#include "fb/algorithm/CompareResult.h"
#include "fb/algorithm/traits/CompareTraits.h"
#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE1(algorithm)

// This is the template using comparator class.
// You should specialize this for you data type, should you have a need for a specialized comparator.
// You can also choose not to specialize this, but rather create a simple CompareTraits specialization for your data,
// this class will automatically use that to select the likely behaviour...


// The default data comparator. (using the operator==, operator< and operator>)

template<class T> class DefaultDataComparator
{
public:
	static bool compare(T const &first, T const &second);
	static CompareResult compareWithSorting(T const &first, T const &second);

	// (don't touch this, leave it false)
	static const bool isUnsupported = false;
};

// The default pointer comparator. (Assuming that only operator== is valid, greater than or less than not provided.)

template<class T> class DefaultNonSortableComparator
{
public:
	static bool compare(T const &first, T const &second);
	static CompareResult compareWithSorting(const T &first, const T &second);

	// (don't touch this, leave it false)
	static const bool isUnsupported = false;
};

FB_END_PACKAGE1()


#define FB_DEFAULT_COMPARATOR_FOR(p_datatype)
#define FB_DEFAULT_COMPARATOR_FOR_TEMPLATE(p_t_params, p_datatype) 


#include "ComparatorInline.h"
