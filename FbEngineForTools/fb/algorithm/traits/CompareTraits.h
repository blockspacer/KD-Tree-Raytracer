#pragma once

FB_PACKAGE2(algorithm, traits)

/**
 * Compare traits for any data type.
 * The correct/simplest way to use the compare() and compareWithSorting() functions is to have these CompareTraits 
 * specialized for your data. 
 *
 * If your data has the ==, < and > operators, then you probably just want to define the doesSupportCompare and 
 * doesSupportSortingCompare to be true. Leave the hasSpecializedCompare and hasSpecializedSortingCompare to false.
 *
 * If your data does not have the operators, then you will have to provide your own Comparator template specialization 
 * and set the hasSpecializedCompare and/or hasSpecializedSortingCompare to true.
 */
template<class T> class CompareTraits
{
public:
	/**
	 * If this trait returns true, then the data can be compared for equality. 
	 * Otherwise, the equality compare may return an undefined value.
	 */
	static const bool doesSupportCompare;

	/**
	 * If this trait returns true, then the data can be compared for sorting.
	 * (In other words, the data can be ordered - it has less-than and greater-than comparison in addition to equality)
	 * Otherwise, the sorting compare may return an undefined value.
	 */
	static const bool doesSupportSortingCompare;

	/**
	 * This should return true for any data that has specialized compare.
	 * (A specialized compare is one that does not a call operator== like the default compare)
	 */
	static const bool hasSpecializedCompare;

	/**
	 * This should return true for any data that has specialized compareWithSorting.
	 * (A specialized compare is one that does not a call operator==, operator< and operator> like the default compareWithSorting)
	 */
	static const bool hasSpecializedSortingCompare;
};

// The usual cases...
template<class T> class DefaultCompareTraits
{
public:
	static const bool doesSupportCompare = true;
	static const bool doesSupportSortingCompare = true;
	static const bool hasSpecializedCompare = false;
	static const bool hasSpecializedSortingCompare = false;
};
template<class T> class DefaultNonSortableCompareTraits
{
public:
	static const bool doesSupportCompare = true;
	static const bool doesSupportSortingCompare = false;
	static const bool hasSpecializedCompare = false;
	static const bool hasSpecializedSortingCompare = false;
};

// some primite type cases...
template<> class CompareTraits<short> : public DefaultCompareTraits<short> { };
template<> class CompareTraits<char> : public DefaultCompareTraits<char> { };
template<> class CompareTraits<int> : public DefaultCompareTraits<int> { };
template<> class CompareTraits<bool> : public DefaultNonSortableCompareTraits<bool> { }; 
template<> class CompareTraits<float> : public DefaultCompareTraits<float> { };

// Macros to make your data comparable. (the most common cases)
// Use outside of any FB_PACKAGE!

#define FB_COMPARABLE(p_datatype) \
	namespace fb { namespace algorithm { namespace traits { \
		template<> class CompareTraits<p_datatype> : public fb::algorithm::traits::DefaultCompareTraits<p_datatype> { }; \
		template<> class CompareTraits<const p_datatype> : public fb::algorithm::traits::DefaultCompareTraits<const p_datatype> { }; \
} } }

#define FB_COMPARABLE_NON_SORTABLE(p_datatype) \
	namespace fb { namespace algorithm { namespace traits { \
		template<> class CompareTraits<p_datatype> : public fb::algorithm::traits::DefaultNonSortableCompareTraits<p_datatype> { }; \
		template<> class CompareTraits<const p_datatype> : public fb::algorithm::traits::DefaultNonSortableCompareTraits<const p_datatype> { }; \
	} } }

#define FB_COMPARABLE_TEMPLATE(p_t_params, p_datatype) \
	namespace fb { namespace algorithm { namespace traits { \
	template<p_t_params > class CompareTraits<p_datatype > : public fb::algorithm::traits::DefaultCompareTraits<p_datatype > { }; \
	template<p_t_params > class CompareTraits<const p_datatype > : public fb::algorithm::traits::DefaultCompareTraits<const p_datatype > { }; \
} } }

#define FB_COMPARABLE_NON_SORTABLE_TEMPLATE(p_t_params, p_datatype) \
	namespace fb { namespace algorithm { namespace traits { \
	template<p_t_params > class CompareTraits<p_datatype > : public fb::algorithm::traits::DefaultNonSortableCompareTraits<p_datatype > { }; \
	template<p_t_params > class CompareTraits<const p_datatype > : public fb::algorithm::traits::DefaultNonSortableCompareTraits<const p_datatype > { }; \
} } }

FB_END_PACKAGE2()
