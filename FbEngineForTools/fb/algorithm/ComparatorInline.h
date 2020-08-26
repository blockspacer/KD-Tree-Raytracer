
// Inline header! don't include directly!

FB_PACKAGE1(algorithm)

template<class T> bool DefaultDataComparator<T>::compare(T const &first, T const &second)
{
	// If this triggers, you are either mis-using the Comparator or you have not defined or included the traits for your data type
	fb_static_assert(algorithm::traits::CompareTraits<T>::doesSupportCompare);

	// Note, if some specialized compare wants to use this as base implementation, then this assert should be commented out
	// Otherwise, it is probably valid and indicates some incorrect Comparator usage.
	fb_static_assert(!algorithm::traits::CompareTraits<T>::hasSpecializedCompare);

	return first == second;
}

template<class T> CompareResult DefaultDataComparator<T>::compareWithSorting(T const &first, T const &second)
{
	// If this triggers, you are either mis-using the Comparator or you have not defined or included the traits for your data type
	fb_static_assert(algorithm::traits::CompareTraits<T>::doesSupportCompare);

	// Note, if some specialized compare wants to use this as base implementation, then this assert should be commented out
	// Otherwise, it is probably valid and indicates some incorrect Comparator usage.
	fb_static_assert(!algorithm::traits::CompareTraits<T>::hasSpecializedCompare);

	if (first < second)
	{
		return CompareResultFirstLessThanSecond;
	}
	else if (first > second)
	{
		return CompareResultFirstGreaterThanSecond;
	} else {
		fb_expensive_assert(first == second);
		return CompareResultEqual;
	}
}

template<class T> bool DefaultNonSortableComparator<T>::compare(T const &first, T const &second)
{
	// If this triggers, you are either mis-using the Comparator or you have not defined or included the traits for your data type
	fb_static_assert(algorithm::traits::CompareTraits<T>::doesSupportCompare);

	// (assuming this is the case since you got here.)
	fb_static_assert(!algorithm::traits::CompareTraits<T>::doesSupportSortingCompare);

	return first == second;
}

template<class T> CompareResult DefaultNonSortableComparator<T>::compareWithSorting(const T &first, const T &second)
{
	// trying to make a sorted compare on non-sortable data is considered an error.
	// - make sure to use the compareWithSorting() function, rather than using the implementing Comparator object directly.
	// - if your data is supposed to support sorting/ordering, make sure it has a appropriate traits class and it is included.
	fb_template_specialization_assert(0 && "Attempt to call compareWithSorting for a DefaultNonSortableComparator (which was probably selected because of non-sortable data).");
	return CompareResultEqual;
}

// Special comparator for handling non-existing comparison cases. (Don't use this directly.)
template<class T> class UnsupportedComparator
{
public:
	static bool compare(T const &first, T const &second)
	{
		// note, this function can get compiled as a replacement for non-supported Comparator but should never get called.
		fb_expensive_assert(0 && "UnsupportedComparator - Should never get here.");
		return false;
	}

	static CompareResult compareWithSorting(const T &first, const T &second)
	{
		// note, this function can get compiled as a replacement for non-supported Comparator but should never get called.
		fb_expensive_assert(0 && "UnsupportedComparator - Should never get here.");
		return CompareResultEqual;
	}

	// (don't touch this.)
	static const bool isUnsupported = true;
};


FB_END_PACKAGE1()

