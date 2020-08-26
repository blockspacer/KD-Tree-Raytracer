#pragma once

#include "fb/algorithm/BinarySearch.h"
#include "Vector.h"
#include "fb/container/Pair.h"

FB_PACKAGE1(container)

struct DefaulLinearSetSorter
{
	template<typename ValueType>
	bool operator() (const ValueType &v1, const ValueType &v2) const
	{
		return v1 < v2;
	}
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<typename T, typename ImpVecType = fb::Vector<T>, typename SortFunctor = container::DefaulLinearSetSorter>
class LinearSet: private ImpVecType
{
public:
	typedef SortFunctor SortFunctorType;
	using typename ImpVecType::ValueType;
	using typename ImpVecType::SizeType;

	using typename ImpVecType::ConstIterator;
	typedef ConstIterator Iterator;

	// ----------------------------------------------------------------------------------------------------------------
	// Swap(), copy and move semantics will work as long as LinearSets have compatible vector types 
	// (eg, Vector<T> and CacheVector<T, n>). Compiler error otherwise.
	// Sort functor has to be same, as otherwise you might end up with wrong sorting order given your different sorting logic.
	// If you need to hack something special, do move/swap using getVectorImp() directly.
	// ----------------------------------------------------------------------------------------------------------------

	LinearSet() {}	
	LinearSet(const LinearSet &other) { copy(other); }
	LinearSet(LinearSet &&other) { move(other); }
	template<typename OtherImpVecType>
	LinearSet(const LinearSet<T, OtherImpVecType, SortFunctor> &other) { copy(other); }
	template<typename OtherImpVecType>
	LinearSet(LinearSet<T, OtherImpVecType, SortFunctor> &&other) { move(other); }
	~LinearSet() {}	

	void operator = (const LinearSet &other) { copy(other); }
	void operator = (LinearSet &&other) { move(other); }
	template<typename OtherImpVecType>
	void operator = (const LinearSet<T, OtherImpVecType, SortFunctor> &other) { copy(other); }
	template<typename OtherImpVecType>
	void operator = (LinearSet<T, OtherImpVecType, SortFunctor> &&other) { move(other); }

	void swap(LinearSet &other) { ImpVecType::swap(other); }
	void move(LinearSet &other) { ImpVecType::swap(other); }
	void copy(const LinearSet &other) { ImpVecType::operator= (other); }
	template<typename OtherImpVecType>
	void swap(LinearSet<T, OtherImpVecType, SortFunctor> &other) { ImpVecType::swap(other); }
	template<typename OtherImpVecType>
	void move(LinearSet<T, OtherImpVecType, SortFunctor> &other) { ImpVecType::swap(other); }
	template<typename OtherImpVecType>
	void copy(const LinearSet<T, OtherImpVecType, SortFunctor> &other) { ImpVecType::operator= (other); }

	// --------------------------------------------
	// Generic vector stuff which is safe to expose
	// --------------------------------------------

	using ImpVecType::isEmpty;
	using ImpVecType::getSize;
	using ImpVecType::getCapacity;
	using ImpVecType::reserve;
	using ImpVecType::trimMemory;
	using ImpVecType::clear;

	// ----------------------------------------------------------------------------------------------------------------
	// Only expose const iteration/getters by default
	// ----------------------------------------------------------------------------------------------------------------

	Iterator getBegin() { return Iterator(ImpVecType::getBegin()); }
	Iterator getEnd() { return Iterator(ImpVecType::getEnd()); }
	ConstIterator getBegin() const { return ConstIterator(ImpVecType::getBegin()); }
	ConstIterator getEnd() const { return ConstIterator(ImpVecType::getEnd()); }
	const T& getFront() const { return ImpVecType::getBack(); }
	const T& getBack() const { return ImpVecType::getBack(); }
	const ValueType &getByIndex(SizeType index) { return ImpVecType::operator[](index); }

	// ----------------------------------------------------------------------------------------------------------------
	// Assosiative interface 
	// ----------------------------------------------------------------------------------------------------------------

	// findIndex() returns getSize() if no result found
	static const SizeType InvalidFindIndex = 0xffffffff;
	SizeType findIndex(const ValueType &value) const
	{
		ConstIterator localBegin = getBegin();
		ConstIterator result = algorithm::binaryFind(localBegin, getEnd(), value, SortFunctor());
		return (SizeType) (result - localBegin);
	}

	SizeType findLowerBoundIndex(const ValueType &value) const
	{
		ConstIterator localBegin = getBegin();
		ConstIterator result = algorithm::lowerBound(localBegin, getEnd(), value, SortFunctor());
		return (SizeType) (result - localBegin);
	}
	SizeType findUpperBoundIndex(const ValueType &value) const
	{
		ConstIterator localBegin = getBegin();
		ConstIterator result = algorithm::upperBound(localBegin, getEnd(), value, SortFunctor());
		return (SizeType) (result - localBegin);
	}

	ConstIterator find(const ValueType &value) const { return getBegin() + findIndex(value); }
	ConstIterator findLowerBound(const ValueType &value) const { return getBegin() + findLowerBoundIndex(value); }
	ConstIterator findUpperBound(const ValueType &value) const { return getBegin() + findUpperBoundIndex(value); }

	#define FB_LINSET_INSERT_IMP() \
		uint32_t insertIndex = findLowerBoundIndex(value); \
		if (insertIndex != getSize() && !SortFunctor()(value, getByIndex(insertIndex))) \
			return Pair<Iterator, bool>(ImpVecType::getBegin() + insertIndex, false); \
		ImpVecType::insertIndex(insertIndex, value); \
		return Pair<Iterator, bool>(ImpVecType::getBegin() + insertIndex, true)

	Pair<Iterator, bool> insert(const ValueType &value) { FB_LINSET_INSERT_IMP(); }
	Pair<Iterator, bool> insert(ValueType &&value) { FB_LINSET_INSERT_IMP(); }
	#undef FB_LINSET_INSERT_IMP

	bool erase(const ValueType &value)
	{
		Iterator it = find(value);
		if (it != getEnd())
		{
			erase(it);
			return true;
		}
		return false;
	}
	using ImpVecType::erase;
	//Iterator erase(ConstIterator it) { return ImpVecType::erase(it); }
	//Iterator erase(ConstIterator first, ConstIterator last) { return impVec::erase(first, last); }

	// ----------------------------------------------------------------------------------------------------------------
	// If you need generic 'Trust me, I know what I'm doing(tm)' access, use vector directly.
	// It's your responsibility to make sure you leave data sorted. Otherwise you can get infinite loops etc later.
	// Also make sure there are no duplicates.
	// It is generally more efficient to fill underlying vector with pushBack() and sort it when you are done.
	// ----------------------------------------------------------------------------------------------------------------
	
	const ImpVecType &getVectorImp() const { return *this; }
	ImpVecType &getVectorImp() { return *this; }
	// void sort() <-- This function can now be found in LinearSetSort.h
	// For set, this will simply index the underlying vector
	const ValueType &operator[] (SizeType index) const { return ImpVecType::operator[](index); }
};

template<typename T, int LocalCapacity, typename SortFunctor = container::DefaulLinearSetSorter>
using CacheLinearSet = LinearSet<T, CacheVector<T, LocalCapacity>, SortFunctor>;
template<typename T, int LocalCapacity, typename SortFunctor = container::DefaulLinearSetSorter>
using StaticLinearSet = LinearSet<T, StaticVector<T, LocalCapacity>, SortFunctor>;

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename T, typename ImpVecType, typename SortFunctor
#define FB_CONTAINERIMP_CONTAINER_TYPE LinearSet<T, ImpVecType, SortFunctor>
#include "ContainerRangeFor.h"

FB_END_PACKAGE0()
