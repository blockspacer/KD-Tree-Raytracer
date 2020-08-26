#ifndef FB_AFLISTIMP_TYPE
	#error "Need to define FB_AFLISTIMP_TYPE before including this file."
#endif

template<typename ValueType, typename CompareValueType, typename ComparatorT>
typename FB_AFLISTIMP_TYPE<ValueType>::Iterator find(FB_AFLISTIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	for (typename FB_AFLISTIMP_TYPE<ValueType>::Iterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (ComparatorT::compare(*iter, value))
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
typename FB_AFLISTIMP_TYPE<ValueType>::ConstIterator find(const FB_AFLISTIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	for (typename FB_AFLISTIMP_TYPE<ValueType>::ConstIterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (ComparatorT::compare(*iter, value))
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType>
typename FB_AFLISTIMP_TYPE<ValueType>::Iterator find(FB_AFLISTIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	for (typename FB_AFLISTIMP_TYPE<ValueType>::Iterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (*iter == value)
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType>
typename FB_AFLISTIMP_TYPE<ValueType>::ConstIterator find(const FB_AFLISTIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	for (typename FB_AFLISTIMP_TYPE<ValueType>::ConstIterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (*iter == value)
			return iter;
	}
	return vec.getEnd();
}


#undef FB_AFLISTIMP_TYPE