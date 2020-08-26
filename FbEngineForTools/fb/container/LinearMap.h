#pragma once

#include "fb/algorithm/BinarySearch.h"
#include "Pair.h"
#include "Vector.h"
#include "fb/container/Pair.h"

FB_PACKAGE1(container)

struct DefaultLinearMapSorter
{
	// Note that you need all these variants

	template<typename PairType>
	bool operator() (const PairType &v1, const PairType &v2) const
	{
		return v1.key < v2.key;
	}

	template<typename PairType>
	bool operator() (const PairType &v1, const typename PairType::KeyType &v2) const
	{
		return v1.key < v2;
	}

	template<typename PairType>
	bool operator() (const typename PairType::KeyType &v1, const PairType &v2) const
	{
		return v1 < v2.key;
	}
};

template<typename Key, typename Value>
struct MapPair
{
	typedef Key KeyType;
	typedef Value ValueType;
	Key key;
	Value value;

	MapPair(const Key &key_, const Value &value_) : key(key_), value(value_) {}
	MapPair(Key &&key_, const Value &value_) : key((Key&&)key_), value(value_) {}
	MapPair(const Key &key_, Value &&value_) : key(key_), value((Value&&)value_) {}
	MapPair(Key &&key_, Value &&value_) : key((Key&&)key_), value((Value&&)value_) {}
	MapPair(const MapPair &other) : key(other.key), value(other.value) {}
	MapPair(MapPair &&other) : key((Key&&)other.key), value((Value&&)other.value) {}
	void operator = (const MapPair &other) { key = other.key; value = other.value; }
	void operator = (MapPair &&other) { key = (Key&&)other.key; value = (Value&&)other.value; }

	bool operator == (const MapPair &other) const { return key == other.key; }
	bool operator == (const Key &otherKey) const { return key == otherKey; }
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<typename KeyType, typename T, typename ImpVecType = fb::Vector<container::MapPair<KeyType, T> >, typename SortFunctor = container::DefaultLinearMapSorter>
class LinearMap: private ImpVecType
{
public:
	typedef T ValueType;
	typedef typename ImpVecType::ValueType PairType;
	typedef SortFunctor SortFunctorType;
	using typename ImpVecType::SizeType;

	// ----------------------------------------------------------------------------------------------------------------
	// Iterators are a thin wrapper over the (pair) vector iterator.
	// Iterate using getVectorImp() or use manual indexing if you care about removing the abstraction.
	// Annoyingly, as Iterator allows you to modify the value, we need some duplication compared to LinearSet.
	// ----------------------------------------------------------------------------------------------------------------
	struct Iterator
	{
		typename ImpVecType::Iterator it;

		Iterator() {}
		Iterator(typename ImpVecType::Iterator it_): it(it_) {}

		const KeyType &getKey() const { return it->key; }
		const ValueType &getValue() const { return it->value; }
		ValueType &getValue() { return it->value; }

		Iterator &operator++() { ++it; return (*this); }
		Iterator operator++(int) { Iterator temp = it; ++it; return temp; }
		bool operator==(const Iterator &other) const { return it == other.it; }
		bool operator!=(const Iterator &other) const { return it != other.it; }

		const PairType &operator*() const { return *it; }
	};

	struct ConstIterator
	{
		typename ImpVecType::ConstIterator it;

		ConstIterator() {}
		ConstIterator(typename ImpVecType::ConstIterator it_): it(it_) {}
		ConstIterator(const Iterator &it_): it(it_.it) {}

		const KeyType &getKey() const { return it->key; }
		const ValueType &getValue() const { return it->value; }

		ConstIterator &operator++() { ++it; return (*this); }
		ConstIterator operator++(int) { ConstIterator temp = it; ++it; return temp; }
		bool operator==(const ConstIterator &other) const { return it == other.it; }
		bool operator!=(const ConstIterator &other) const { return it != other.it; }
	};

	// ----------------------------------------------------------------------------------------------------------------
	// Swap(), copy and move semantics will work as long as LinearSets have compatible vector types 
	// (eg, Vector<T> and CacheVector<T, n>). Compiler error otherwise.
	// Sort functor has to be same, as otherwise you might end up with wrong sorting order given your different sorting logic.
	// If you need to hack something special, do move/swap using getVectorImp() directly.
	// ----------------------------------------------------------------------------------------------------------------

	LinearMap() {}	
	LinearMap(const LinearMap &other) { copy(other); }
	LinearMap(LinearMap &&other) { move(other); }
	template<typename OtherImpVecType>
	LinearMap(const LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &other) { copy(other); }
	template<typename OtherImpVecType>
	LinearMap(LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &&other) { move(other); }
	~LinearMap() {}	

	void operator = (const LinearMap &other) { copy(other); }
	void operator = (LinearMap &&other) { move(other); }
	template<typename OtherImpVecType>
	void operator = (const LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &other) { copy(other); }
	template<typename OtherImpVecType>
	void operator = (LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &&other) { move(other); }

	void swap(LinearMap &other) { ImpVecType::swap(other); }
	void move(LinearMap &other) { ImpVecType::swap(other); }
	void copy(const LinearMap &other) { ImpVecType::operator= (other); }
	template<typename OtherImpVecType>
	void swap(LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &other) { ImpVecType::swap(other); }
	template<typename OtherImpVecType>
	void move(LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &other) { ImpVecType::swap(other); }
	template<typename OtherImpVecType>
	void copy(const LinearMap<KeyType, ValueType, OtherImpVecType, SortFunctor> &other) { ImpVecType::operator= (other); }

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
	const PairType& getFront() const { return ImpVecType::getFront(); }
	const PairType& getBack() const { return ImpVecType::getBack(); }
	const PairType& getByIndex(SizeType index) const { return ImpVecType::operator[](index); }
	const PairType& getByIndex(SizeType index) { return ImpVecType::operator[](index); }

	// ----------------------------------------------------------------------------------------------------------------
	// Assosiative interface 
	// ----------------------------------------------------------------------------------------------------------------

	// findIndex() returns getSize() if no result found
	SizeType findIndex(const KeyType &key) const
	{
		ConstIterator localBegin = getBegin();
		ConstIterator localEnd = getEnd();
		typename ImpVecType::ConstIterator result = algorithm::binaryFind(localBegin.it, localEnd.it, key, SortFunctor());
		return (SizeType) (result - localBegin.it);
	}

	SizeType findLowerBoundIndex(const KeyType &key) const
	{
		ConstIterator localBegin = getBegin();
		typename ImpVecType::ConstIterator result = algorithm::lowerBound(localBegin.it, getEnd().it, key, SortFunctor());
		return (SizeType) (result - localBegin.it);
	}
	SizeType findUpperBoundIndex(const KeyType &key) const
	{
		ConstIterator localBegin = getBegin();
		typename ImpVecType::ConstIterator result = algorithm::upperBound(localBegin.it, getEnd().it, key, SortFunctor());
		return (SizeType) (result - localBegin.it);
	}

	ConstIterator find(const KeyType &key) const { return ImpVecType::getBegin() + findIndex(key); }
	Iterator find(const KeyType &key) { return ImpVecType::getBegin() + findIndex(key); }
	ConstIterator findLowerBound(const KeyType &key) const { return ImpVecType::getBegin() + findLowerBoundIndex(key); }
	Iterator findLowerBound(const KeyType &key) { return ImpVecType::getBegin() + findLowerBoundIndex(key); }
	ConstIterator findUpperBound(const KeyType &key) const { return ImpVecType::getBegin() + findUpperBoundIndex(key); }
	Iterator findUpperBound(const KeyType &key) { return ImpVecType::getBegin() + findUpperBoundIndex(key); }

	// For map, this will default-construct key/value pair (if it doesn't exist already) and return reference to value

	#define FB_LINMAP_INDEX_OPERATION_IMP() \
		uint32_t insertIndex = findLowerBoundIndex(key); \
		if (insertIndex != getSize() && !SortFunctor()(key, getByIndex(insertIndex))) \
			return ImpVecType::operator[] (insertIndex).value; \
		ImpVecType::insertIndex(insertIndex, PairType(key, ValueType())); \
		return ImpVecType::operator[] (insertIndex).value

	typename LinearMap::ValueType &operator[] (const KeyType &key) { FB_LINMAP_INDEX_OPERATION_IMP(); }
	typename LinearMap::ValueType &operator[] (KeyType &&key) { FB_LINMAP_INDEX_OPERATION_IMP(); }
	#undef FB_LINMAP_INDEX_OPERATION_IMP

	#define FB_LINMAP_INSERT_IMP(insertKeyString, insertValueString) \
		uint32_t insertIndex = findLowerBoundIndex(key); \
		if (insertIndex != getSize() && !SortFunctor()(key, getByIndex(insertIndex))) \
			return Pair<Iterator, bool>(ImpVecType::getBegin() + insertIndex, false); \
		ImpVecType::insertIndex(insertIndex, PairType(insertKeyString, insertValueString)); \
		return Pair<Iterator, bool>(ImpVecType::getBegin() + insertIndex, true);

	Pair<Iterator, bool> insert(const KeyType &key, const ValueType &value) { FB_LINMAP_INSERT_IMP(key, value); }
	Pair<Iterator, bool> insert(KeyType &&key, const ValueType &value) { FB_LINMAP_INSERT_IMP((KeyType&&)key, value); }
	Pair<Iterator, bool> insert(const KeyType &key, ValueType &&value) { FB_LINMAP_INSERT_IMP(key, (ValueType&&)value); }
	Pair<Iterator, bool> insert(KeyType &&key, ValueType &&value) { FB_LINMAP_INSERT_IMP((KeyType&&)key, (ValueType&&)value); }
	#undef FB_LINMAP_INSERT_IMP

	bool eraseKey(const KeyType &key)
	{	
		Iterator it = find(key);
		if (it != getEnd())
		{
			erase(it);
			return true;
		}
		return false;
	}
	Iterator erase(ConstIterator it) { return Iterator(ImpVecType::erase(it.it)); }
	Iterator erase(ConstIterator first, ConstIterator last) { return Iterator(ImpVecType::erase(first.it, last.it)); }

	// ----------------------------------------------------------------------------------------------------------------
	// If you need generic 'Trust me, I know what I'm doing(tm)' access, use vector directly.
	// It's your responsibility to make sure you leave data sorted. Otherwise you can get infinite loops etc later.
	// Also make sure there are no duplicates.
	// It is generally more efficient to fill underlying vector with pushBack() and sort it when you are done.
	// ----------------------------------------------------------------------------------------------------------------
	
	const ImpVecType &getVectorImp() const { return *this; }
	ImpVecType &getVectorImp() { return *this; }
	// void sort() <-- This function can now be found in LinearMapSort.h

	/* Some common for all maps utils */
	#include "MapCommonImp.h"
};

template<typename Key, typename T, int LocalCapacity, typename SortFunctor = container::DefaultLinearMapSorter>
using CacheLinearMap = LinearMap< Key, T, CacheVector<container::MapPair<Key, T>, LocalCapacity>, SortFunctor>;
template<typename Key, typename T, int LocalCapacity, typename SortFunctor = container::DefaultLinearMapSorter>
using StaticLinearMap = LinearMap< Key, T, StaticVector<container::MapPair<Key, T>, LocalCapacity>, SortFunctor>;

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename KeyType, typename ValueType, typename ImpVecType, typename SortFunctor
#define FB_CONTAINERIMP_CONTAINER_TYPE LinearMap<KeyType, ValueType, ImpVecType, SortFunctor>
#include "ContainerCustomRangeFor.h"

FB_END_PACKAGE0()
