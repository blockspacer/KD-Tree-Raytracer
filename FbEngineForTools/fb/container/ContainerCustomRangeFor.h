#if !defined(FB_CONTAINERIMP_CONTAINER_TYPE) || !defined(FB_CONTAINERIMP_TEMPLATE_PARAMS)
#error "Define these before including this file."
#endif

#ifndef FB_RANGEFOR_CUSTOM_CRAP
#define FB_RANGEFOR_CUSTOM_CRAP

	// These are totally bogus types which only purpose is to provide * -operator returning the original iterator.
	// As our assosiative iterators have been defined as getKey() / getValue() instead of dereferencing to actual value, 
	// that's the only way to make them work with range-for. Pointless template bloat ftw.

	// As a bonus, we can be included multiple times, so do the types only once.
	// And hope we don't end up to multiple namespaces as this will break, would have to move to separate header.

	template<typename IType>
	struct RangeForMutableIteratorWrapper
	{
		IType iterator;
		RangeForMutableIteratorWrapper(IType it): iterator(it) {}

		IType &operator* () { return iterator; }
		bool operator == (const RangeForMutableIteratorWrapper &other) const { return iterator != other.iterator; }
		bool operator != (const RangeForMutableIteratorWrapper &other) const { return iterator != other.iterator; }
		RangeForMutableIteratorWrapper operator ++() { ++iterator; return *this; }
	};

	template<typename IType>
	struct RangeForConstIteratorWrapper
	{
		IType iterator;
		RangeForConstIteratorWrapper(IType it): iterator(it) {}

		const IType &operator* () const { return iterator; }
		bool operator == (const RangeForConstIteratorWrapper &other) const { return iterator != other.iterator; }
		bool operator != (const RangeForConstIteratorWrapper &other) const { return iterator != other.iterator; }
		RangeForConstIteratorWrapper operator ++() { ++iterator; return *this; }
	};

#endif

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
RangeForMutableIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::Iterator> begin(FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return RangeForMutableIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::Iterator> (vec.getBegin());
}

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
RangeForMutableIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::Iterator> end(FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return RangeForMutableIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::Iterator>(vec.getEnd());
}

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
RangeForConstIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::ConstIterator> begin(const FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return RangeForConstIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::ConstIterator> (vec.getBegin());
}

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
RangeForConstIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::ConstIterator> end(const FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return RangeForConstIteratorWrapper<typename FB_CONTAINERIMP_CONTAINER_TYPE::ConstIterator>(vec.getEnd());
}

#undef FB_CONTAINERIMP_CONTAINER_TYPE
#undef FB_CONTAINERIMP_TEMPLATE_PARAMS
