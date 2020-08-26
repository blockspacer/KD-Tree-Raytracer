#ifndef FB_VECIMP_TYPE
	#error "Need to define FB_VECIMP_TYPE before including this file."
#endif
	
// Utils

template<typename ValueType>
void reserveHint(FB_VECIMP_TYPE<ValueType> &vec, typename FB_VECIMP_TYPE<ValueType>::SizeType sizeHint) 
{ 
	if (sizeHint > vec.getCapacity())
		vec.reserve(sizeHint);
}

template<typename ValueType>
void push(FB_VECIMP_TYPE<ValueType> &vec, const ValueType &value) 
{ 
	vec.pushBack(value);
}

template<typename ValueType>
void push(FB_VECIMP_TYPE<ValueType> &vec, ValueType &&value) 
{ 
	vec.pushBack(value);
}

template<typename ValueType>
bool pushBackIfSpace(FB_VECIMP_TYPE<ValueType> &vec, const ValueType &value)
{
	if (vec.getSize() < vec.getCapacity())
	{
		vec.pushBack(value);
		return true;
	}
	return false;
}

template<typename ValueType>
bool pushBackIfSpace(FB_VECIMP_TYPE<ValueType> &vec, ValueType &&value)
{
	if (vec.getSize() < vec.getCapacity())
	{
		vec.pushBack(value);
		return true;
	}
	return false;
}

template<typename ValueType>
typename FB_VECIMP_TYPE<ValueType>::ValueType popBackWithValue(FB_VECIMP_TYPE<ValueType> &vec) 
{ 
	ValueType value = vec.getBack(); 
	vec.popBack();
	return value; 
}

template<typename ValueType>
typename FB_VECIMP_TYPE<ValueType>::Iterator swapOut(FB_VECIMP_TYPE<ValueType> &vec, typename FB_VECIMP_TYPE<ValueType>::ConstIterator iter)
{
	uint32_t index = (uint32_t) (iter - vec.getBegin());
	vec.swapOutIndex(index);
	return vec.getBegin() + index;
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
typename FB_VECIMP_TYPE<ValueType>::Iterator find(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	for (typename FB_VECIMP_TYPE<ValueType>::Iterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (ComparatorT::compare(*iter, value))
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
typename FB_VECIMP_TYPE<ValueType>::ConstIterator find(const FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	for (typename FB_VECIMP_TYPE<ValueType>::ConstIterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (ComparatorT::compare(*iter, value))
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType>
typename FB_VECIMP_TYPE<ValueType>::Iterator find(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	for (typename FB_VECIMP_TYPE<ValueType>::Iterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (*iter == value)
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType>
typename FB_VECIMP_TYPE<ValueType>::ConstIterator find(const FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	for (typename FB_VECIMP_TYPE<ValueType>::ConstIterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (*iter == value)
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
bool findIfContains(const FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	return find(vec, value, comparator) != vec.getEnd();
}

template<typename ValueType, typename CompareValueType>
SizeType findIndex(const FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	for (typename FB_VECIMP_TYPE<ValueType>::ConstIterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (*iter == value)
			return SizeType(iter - vec.getBegin());
	}
	return ~0U;
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
SizeType findIndex(const FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	typename FB_VECIMP_TYPE<ValueType>::ConstIterator iter = find(vec, value, comparator);
	if (iter != vec.getEnd())
		return SizeType(iter - vec.getBegin());

	return ~0U;
}

template<typename ValueType, typename CompareValueType>
bool findIfContains(const FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	return find(vec, value) != vec.getEnd();
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
bool findAndRemove(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	typename FB_VECIMP_TYPE<ValueType>::Iterator iter = find(vec, value, comparator);
	if (iter != vec.getEnd())
	{
		vec.erase(iter);
		return true;
	}
	else
	{
		return false;
	}
}

template<typename ValueType, typename CompareValueType>
bool findAndRemove(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	typename FB_VECIMP_TYPE<ValueType>::Iterator iter = find(vec, value);
	if (iter != vec.getEnd())
	{
		vec.erase(iter);
		return true;
	}
	else
	{
		return false;
	}
}

template<typename ValueType, typename CompareValueType, typename ComparatorT>
bool findAndSwapOut(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value, const ComparatorT &comparator)
{
	typename FB_VECIMP_TYPE<ValueType>::Iterator iter = find(vec, value, comparator);
	if (iter != vec.getEnd())
	{
		swapOut(vec, iter);
		return true;
	}
	else
	{
		return false;
	}
}

template<typename ValueType, typename CompareValueType>
bool findAndSwapOut(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &value)
{
	typename FB_VECIMP_TYPE<ValueType>::Iterator iter = find(vec, value);
	if (iter != vec.getEnd())
	{
		swapOut(vec, iter);
		return true;
	}
	else
	{
		return false;
	}
}

/* Replaces at most numToReplace of occurrances of oldValue with newValue. Returns number of replacements */
template<typename ValueType, typename CompareValueType, typename ComparatorT>
SizeType findAndReplaceSomeValues(FB_VECIMP_TYPE<ValueType> &vec, SizeType numToReplace, const CompareValueType &oldValue, const ValueType &newValue, const ComparatorT &comparator)
{
	SizeType numReplaced = 0;
	for (typename FB_VECIMP_TYPE<ValueType>::Iterator iter = vec.getBegin(), end = vec.getEnd(); iter != end && numToReplace > 0; ++iter)
	{
		if (ComparatorT::compare(*iter, oldValue))
		{
			*iter = newValue;
			--numToReplace;
			++numReplaced;
		}
	}
	return numReplaced;
}

/* Replaces at most numToReplace of occurrances of oldValue with newValue. Returns number of replacements */
template<typename ValueType, typename CompareValueType>
SizeType findAndReplaceSomeValues(FB_VECIMP_TYPE<ValueType> &vec, SizeType numToReplace, const CompareValueType &oldValue, const ValueType &newValue)
{
	SizeType numReplaced = 0;
	for (typename FB_VECIMP_TYPE<ValueType>::Iterator iter = vec.getBegin(), end = vec.getEnd(); iter != end && numToReplace > 0; ++iter)
	{
		if (*iter == oldValue)
		{
			*iter = newValue;
			--numToReplace;
			++numReplaced;
		}
	}
	return numReplaced;
}

/* Replaces all occurrances of oldValue with newValue. Returns number of replacements */
template<typename ValueType, typename CompareValueType, typename ComparatorT>
SizeType findAndReplaceValues(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &oldValue, const ValueType &newValue, const ComparatorT &comparator)
{
	return findAndReplaceSomeValues(vec, FB_VECIMP_TYPE<ValueType>::getMaxCapacity(), oldValue, newValue, comparator);
}

/* Replaces all occurrances of oldValue with newValue. Returns number of replacements */
template<typename ValueType, typename CompareValueType>
SizeType findAndReplaceValues(FB_VECIMP_TYPE<ValueType> &vec, const CompareValueType &oldValue, const ValueType &newValue)
{
	return findAndReplaceSomeValues(vec, FB_VECIMP_TYPE<ValueType>::getMaxCapacity(), oldValue, newValue);
}

// Just to be safe
#undef FB_VECIMP_TYPE
