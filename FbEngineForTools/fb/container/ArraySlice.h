#pragma once

#include "fb/lang/FBAssert.h"

#include <initializer_list>

FB_PACKAGE0()
template <typename T>
class ArraySlice;
FB_END_PACKAGE0()

FB_PACKAGE1(container)

class ArraySliceBase
{
public:
	typedef fb::SizeType SizeType;
	ArraySliceBase(void *rawValues, SizeType size)
		: rawValues(rawValues)
		, size(size)
	{
	}
	SizeType getSize() const { return size; }
	bool isEmpty() const { return getSize() == 0; }

	template <typename T>
	ArraySlice<T> cast();

protected:
	bool operator==(const ArraySliceBase& other) const
	{
		return rawValues == other.rawValues && size == other.size;
	}
	bool operator!=(const ArraySliceBase& other) const
	{
		return !(*this == other);
	}

	void *rawValues;
	SizeType size;
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template <typename T>
class ArraySlice : public container::ArraySliceBase
{
public:
	typedef T* Iterator;
	typedef const T* ConstIterator;

	ArraySlice()
		: ArraySliceBase(nullptr, 0)
	{
	}
	ArraySlice(T* values, SizeType size)
		: ArraySliceBase((void*)values, size)
	{
	}
	ArraySlice(T* begin, T* end)
		: ArraySliceBase((void*)begin, end - begin)
	{
	}

	T* getPointer() { return (T*)rawValues; }
	const T* getPointer() const { return (const T*)rawValues; }

	Iterator getBegin() { return (T*)rawValues; }
	Iterator getEnd() { return (T*)rawValues + size; }
	const ConstIterator getBegin() const { return (const T*)rawValues; }
	const ConstIterator getEnd() const { return (const T*)rawValues + size; }

	T& getFront() { return (*this)[0]; }
	T& getBack() { return (*this)[size - 1]; }
	const T& getFront() const { return (*this)[0]; }
	const T& getBack() const { return (*this)[size - 1]; }

	const ArraySlice<const T> constCast()
	{
		return cast<const T>();
	}

	SizeType getSizeInBytes() const
	{
		return getSize() * sizeof(T);
	}

	T& operator[](SizeType index)
	{
		fb_assert(index < size);
		return ((T*)rawValues)[index];
	}
	const T& operator[](SizeType index) const
	{
		fb_assert(index < size);
		return ((const T*)rawValues)[index];
	}

	bool operator==(const ArraySlice<T>& other) const
	{
		return ArraySliceBase::operator==(other);
	}
	bool operator!=(const ArraySlice<T>& other) const
	{
		return ArraySliceBase::operator!=(other);
	}
};

template <typename T>
ArraySlice<T> container::ArraySliceBase::cast()
{
	return ArraySlice<T>((T*)rawValues, size);
}

template <typename T>
ArraySlice<typename T::ValueType> sliceFromVector(T& t)
{
#if FB_BUILD == FB_DEBUG
	if (t.getSize() == 0)
		return ArraySlice<typename T::ValueType>();
	else
		return ArraySlice<typename T::ValueType>(&*t.getBegin(), t.getSize());
#else
	return ArraySlice<typename T::ValueType>(t.getBegin(), t.getSize());
#endif
}

template <typename T>
ArraySlice<const typename T::ValueType> sliceFromVector(const T& t)
{
#if FB_BUILD == FB_DEBUG
	if (t.getSize() == 0)
		return ArraySlice<const typename T::ValueType>();
	else
		return ArraySlice<const typename T::ValueType>(&*t.getBegin(), t.getSize());
#else
	return ArraySlice<const typename T::ValueType>(t.getBegin(), t.getSize());
#endif
}

template <typename T>
ArraySlice<const typename T::ValueType> sliceFromVectorConst(T& t)
{
#if FB_BUILD == FB_DEBUG
	if (t.getSize() == 0)
		return ArraySlice<const typename T::ValueType>();
	else
		return ArraySlice<const typename T::ValueType>(&*t.getBegin(), t.getSize());
#else
	return ArraySlice<const typename T::ValueType>(t.getBegin(), t.getSize());
#endif
}

template <typename T, int N>
ArraySlice<T> sliceFromArray(T (&arr)[N])
{
	return ArraySlice<T>(arr, N);
}

template <typename T>
ArraySlice<const T> sliceFromInitializerList(const std::initializer_list<T>& arr)
{
	return ArraySlice<const T>(arr.begin(), (SizeType)arr.size());
}

template <typename T>
ArraySlice<T> sliceFromPointer(T* ptr, SizeType count)
{
	return ArraySlice<T>(ptr, count);
}


// TODO: Split VectorUtilsImp.h into mutation and find functions and only use the find functions for ArraySlice
template<typename ValueType, typename CompareValueType>
typename ArraySlice<ValueType>::ConstIterator find(const ArraySlice<ValueType> &vec, const CompareValueType &value)
{
	for (typename ArraySlice<ValueType>::ConstIterator iter = vec.getBegin(), end = vec.getEnd(); iter != end; ++iter)
	{
		if (*iter == value)
			return iter;
	}
	return vec.getEnd();
}

template<typename ValueType, typename CompareValueType>
bool findIfContains(const ArraySlice<ValueType> &vec, const CompareValueType &value)
{
	return find(vec, value) != vec.getEnd();
}


#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename T
#define FB_CONTAINERIMP_CONTAINER_TYPE ArraySlice<T>
#include "fb/container/ContainerRangeFor.h"

FB_END_PACKAGE0()
