#pragma once

#include "LinearHashBase.h"

#include "fb/container/Pair.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/MemTools.h"

FB_PACKAGE1(container)

template<typename Key, typename Value>
struct HashPair
{
	Key key;
	Value value;

	HashPair(const Key &key_, const Value &value_): key(key_), value(value_) {}
	HashPair(Key &&key_, const Value &value_): key((Key&&)key_), value(value_) {}
	HashPair(const Key &key_, Value &&value_): key(key_), value((Value&&)value_) {}
	HashPair(Key &&key_, Value &&value_): key((Key&&)key_), value((Value&&)value_) {}
	HashPair(const HashPair &other): key(other.key), value(other.value) {}
	HashPair(HashPair &&other): key((Key&&)other.key), value((Value&&)other.value) {}

	void operator = (const HashPair &other) { key = other.key; value = other.value; }
	void operator = (HashPair &&other) { key = (Key&&) other.key; value = (Value&&) other.value; }

	bool operator == (const HashPair &other) const { return key == other.key; }
	bool operator == (const Key &otherKey) const { return key == otherKey; }
};

FB_END_PACKAGE1()

FB_PACKAGE0()

// Open addressing (closed) hash map. Uses robin hood hashing.
template<typename Key, typename Value, typename HashFunctor = container::DefaultHashFunctor>
struct LinearHashMap: public container::LinearHashBase
{
	typedef LinearHashMap<Key, Value, HashFunctor> ThisType;
	typedef Key KeyType;
	typedef Value ValueType;
	typedef container::HashPair<Key, Value> PairType;

protected:
	LinearHashMap(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve): LinearHashBase(staticPointer, staticSizeInElements, assertOnReserve) {}

	const PairType *getPairArray() const
	{
		const uint32_t *hashPointer = (const uint32_t*) getBytePointer();
		return (const PairType *) (hashPointer + getCapacity());
	}
	PairType *getPairArray()
	{
		uint32_t *hashPointer = (uint32_t*) getBytePointer();
		return (PairType *) (hashPointer + getCapacity());
	}

	void reserveImp(SizeType newCapacity)
	{
		newCapacity = newCapacity < 8 ? 8 : newCapacity;
		uint32_t localCapacity = getCapacity();
		if (newCapacity <= localCapacity)
			return;

		uint32_t *FB_RESTRICT oldHashArray = (uint32_t *) getBytePointer();
		PairType *FB_RESTRICT oldPairArray = (PairType *) (oldHashArray + localCapacity);
		uint32_t *FB_RESTRICT newHashArray = (uint32_t*) impGetNewPointerExact(sizeof(uint32_t) + sizeof(PairType), newCapacity);
		PairType *FB_RESTRICT newPairArray = (PairType *) (newHashArray + newCapacity);

		lang::MemSet::set(newHashArray, 0, newCapacity * sizeof(uint32_t));
		for (uint32_t i = 0; i < localCapacity; ++i)
		{
			uint32_t oldHashValue = oldHashArray[i];
			if (isValidHash(oldHashValue))
			{
				container::hashimp::insertImp<PairType>(oldHashValue, (PairType&&) oldPairArray[i], newHashArray, newPairArray, newCapacity);
				oldPairArray[i].~PairType();
			}
		}

		impSwapPointer(sizeof(uint32_t) + sizeof(PairType), newHashArray, newCapacity);
	}

public:

	LinearHashMap() {}
	LinearHashMap(const LinearHashMap &other) { copy(other); }
	LinearHashMap(LinearHashMap &&other) { move(other); }
	~LinearHashMap() { clear(); impReset(sizeof(uint32_t) + sizeof(PairType)); }
	void operator = (const LinearHashMap &other) { clear(); copy(other); }
	void operator = (LinearHashMap &&other) { move(other); }

	void copy(const LinearHashMap &other)
	{
		if (this == &other)
			return;

		uint32_t *FB_RESTRICT hashArray = getHashArray();
		PairType *FB_RESTRICT pairArray = getPairArray();
		uint32_t localCapacity = getCapacity();
		for (uint32_t i = 0; i < localCapacity; ++i)
		{
			if (isValidHash(hashArray[i]))
			{
				hashArray[i] = 0;
				pairArray[i].~PairType();
			}
		}

		size = 0;
		uint32_t otherCapacity = other.getCapacity();
		uint32_t otherSize = other.getSize();
		reserveImp(getCapacityFromSize(otherSize));
		
		uint32_t newCapacity = getCapacity();
		uint32_t *FB_RESTRICT newHashArray = getHashArray();
		PairType *FB_RESTRICT newPairArray = getPairArray();

		const uint32_t *FB_RESTRICT otherHashArray = other.getHashArray();
		const PairType *FB_RESTRICT otherPairArray = other.getPairArray();
		for (uint32_t i = 0; i < otherCapacity; ++i)
		{
			if (isValidHash(otherHashArray[i]))
				container::hashimp::insertImp<PairType>(otherHashArray[i], PairType(otherPairArray[i]), newHashArray, newPairArray, newCapacity);
		}
		size = otherSize;
	}

	// ToDo: Fix move and swap when implementing static buffers!
	void swap(LinearHashMap &other) { lang::swap(intPointer, other.intPointer); lang::swap(size, other.size); lang::swap(capacity, other.capacity); }
	void move(LinearHashMap &other) { swap(other); }

	void clear()
	{
		if(size)
		{
			uint32_t *FB_RESTRICT hashArray = getHashArray();
			PairType *FB_RESTRICT pairArray = getPairArray();
			uint32_t localCapacity = getCapacity();
			for (uint32_t i = 0; i < localCapacity; ++i)
			{
				if (isValidHash(hashArray[i]))
				{
					hashArray[i] = 0;
					pairArray[i].~PairType();
				}
			}

			size = 0;
		}
	}

	// Reserve accepts the assumed number of elements stored to hash array.
	// Internal array size will be higher due to pow2 rounding and load factor compensation
	void reserve(SizeType expectedSize)
	{
		reserveImp(getCapacityFromSize(expectedSize));
	}

	void trimMemory()
	{
		reserveImp(getCapacityFromSize(getSize()));
	}

	struct Iterator
	{
		ThisType *hashMap;
		uint32_t index;

		Iterator(ThisType *hashMap_, uint32_t index_): hashMap(hashMap_), index(index_) {}

		const KeyType &getKey() const { fb_assert(index < hashMap->getCapacity()); return hashMap->getPairArray()[index].key; }
		const ValueType &getValue() const { fb_assert(index < hashMap->getCapacity()); return hashMap->getPairArray()[index].value; }
		ValueType &getValue() { fb_assert(index < hashMap->getCapacity());  return hashMap->getPairArray()[index].value; }

		Iterator &operator++() { index = hashMap->getNextIndex(index); return (*this); }
		Iterator operator++(int) { Iterator temp = *this; index = hashMap->getNextIndex(index); return temp; }
		bool operator==(const Iterator &other) const { return index == other.index; }
		bool operator!=(const Iterator &other) const { return index != other.index; }
	};

	struct ConstIterator
	{
		const ThisType *hashMap;
		uint32_t index;

		ConstIterator(const ThisType *hashMap_, uint32_t index_): hashMap(hashMap_), index(index_) {}
		ConstIterator(const Iterator &it): hashMap(it.hashMap), index(it.index) {}

		const KeyType &getKey() const { fb_assert(index < hashMap->getCapacity()); return hashMap->getPairArray()[index].key; }
		const ValueType &getValue() const { fb_assert(index < hashMap->getCapacity()); return hashMap->getPairArray()[index].value; }

		ConstIterator &operator++() { index = hashMap->getNextIndex(index); return (*this); }
		ConstIterator operator++(int) { ConstIterator temp = *this; index = hashMap->getNextIndex(index); return temp; }
		bool operator==(const ConstIterator &other) const { return index == other.index; }
		bool operator!=(const ConstIterator &other) const { return index != other.index; }
	};

	ConstIterator getBegin() const { return ConstIterator(this, getFirstIndex()); }
	Iterator getBegin() { return Iterator(this, getFirstIndex()); }
	ConstIterator getEnd() const { return ConstIterator(this, getCapacity()); }
	Iterator getEnd() { return Iterator(this, getCapacity()); }

	ConstIterator find(const Key &key) const { return ConstIterator(this, container::hashimp::getInternalIndex<KeyType, PairType> (container::hashimp::getHashValue<KeyType, HashFunctor>(key), key, getHashArray(), getPairArray(), getCapacity())); }
	Iterator find(const Key &key) { return Iterator(this, container::hashimp::getInternalIndex<KeyType, PairType>(container::hashimp::getHashValue<KeyType, HashFunctor>(key), key, getHashArray(), getPairArray(), getCapacity())); }

	#define FB_LINHASH_INDEX_OPERATION_IMP(insertKeyString) \
		uint32_t *FB_RESTRICT hashArray = getHashArray(); \
		PairType *FB_RESTRICT pairArray = getPairArray(); \
		uint32_t localCapacity = capacity; \
		uint32_t hashValue = container::hashimp::getHashValue<KeyType, HashFunctor>(key); \
		uint32_t index = container::hashimp::getInternalIndex(hashValue, key, hashArray, pairArray, localCapacity); \
		if (index != localCapacity) \
			return pairArray[index].value; \
		uint32_t localSize = getSize(); \
		if (shouldResize(localSize, localCapacity)) \
		{ \
			reserveImp(localCapacity * 2); \
			hashArray = getHashArray(); \
			pairArray = getPairArray(); \
			localCapacity = capacity; \
		} \
		index = container::hashimp::insertImp<PairType>(hashValue, container::HashPair<KeyType, ValueType> (key, ValueType()), hashArray, pairArray, localCapacity);  \
		size = localSize + 1;  \
		return pairArray[index].value

	Value &operator[](const Key &key) { FB_LINHASH_INDEX_OPERATION_IMP(key); }
	Value &operator[](Key &&key) { FB_LINHASH_INDEX_OPERATION_IMP((KeyType&&)key); }
	#undef FB_LINHASH_INDEX_OPERATION_IMP

	#define FB_LINHASH_INSERT_IMP(insertKeyString, insertValueString) \
		uint32_t *FB_RESTRICT hashArray = getHashArray(); \
		PairType *FB_RESTRICT pairArray = getPairArray(); \
		uint32_t localCapacity = capacity; \
		uint32_t hashValue = container::hashimp::getHashValue<KeyType, HashFunctor>(key);  \
		uint32_t index = container::hashimp::getInternalIndex(hashValue, key, hashArray, pairArray, localCapacity);  \
		if (index < localCapacity) \
		{ \
			FB_LINHASH_INSERT_ASSIGN_IMP(insertValueString); \
			return Pair<Iterator, bool>(Iterator(this, index), false); \
		} \
		uint32_t localSize = getSize(); \
		if (shouldResize(localSize, localCapacity))  \
		{  \
			reserveImp(localCapacity * 2);  \
			hashArray = getHashArray(); \
			pairArray = getPairArray(); \
			localCapacity = capacity; \
		} \
		index = container::hashimp::insertImp<PairType>(hashValue, container::HashPair<KeyType, ValueType> (insertKeyString, insertValueString), hashArray, pairArray, localCapacity); \
		size = localSize + 1; \
		return Pair<Iterator, bool> (Iterator(this, index), true)
	
	#define FB_LINHASH_INSERT_ASSIGN_IMP(insertValueString) \
		pairArray[index].value.~ValueType(); \
		new(&pairArray[index].value) ValueType(insertValueString); \

	Pair<Iterator, bool> insertOrAssign(const KeyType &key, const ValueType &value) { FB_LINHASH_INSERT_IMP(key, value); }
	Pair<Iterator, bool> insertOrAssign(KeyType &&key, const ValueType &value) { FB_LINHASH_INSERT_IMP((KeyType&&)key, value); }
	Pair<Iterator, bool> insertOrAssign(const KeyType &key, ValueType &&value) { FB_LINHASH_INSERT_IMP(key, (ValueType&&)value); }
	Pair<Iterator, bool> insertOrAssign(KeyType &&key, ValueType &&value) { FB_LINHASH_INSERT_IMP((KeyType&&)key, (ValueType&&)value); }

	#undef FB_LINHASH_INSERT_ASSIGN_IMP
	#define FB_LINHASH_INSERT_ASSIGN_IMP(insertValueString) 

	Pair<Iterator, bool> insert(const KeyType &key, const ValueType &value) { FB_LINHASH_INSERT_IMP(key, value); }
	Pair<Iterator, bool> insert(KeyType &&key, const ValueType &value) { FB_LINHASH_INSERT_IMP((KeyType&&)key, value); }
	Pair<Iterator, bool> insert(const KeyType &key, ValueType &&value) { FB_LINHASH_INSERT_IMP(key, (ValueType&&)value); }
	Pair<Iterator, bool> insert(KeyType &&key, ValueType &&value) { FB_LINHASH_INSERT_IMP((KeyType&&)key, (ValueType&&)value); }

	#undef FB_LINHASH_INSERT_ASSIGN_IMP
	#undef FB_LINHASH_INSERT_IMP

	bool eraseKey(const KeyType &key)
	{
		uint32_t *FB_RESTRICT hashArray = getHashArray();
		PairType *FB_RESTRICT pairArray = getPairArray();
		uint32_t localCapacity = capacity;

		uint32_t hashValue = container::hashimp::getHashValue<KeyType, HashFunctor>(key);
		uint32_t index = container::hashimp::getInternalIndex<KeyType, PairType>(hashValue, key, hashArray, pairArray, localCapacity);
		if (index < getCapacity())
		{
			container::hashimp::eraseImp(index, hashArray, pairArray, localCapacity);
			--size;
			return true;
		}
	
		return false;
	}

	Iterator erase(const Iterator &it)
	{
		fb_expensive_assert(it.hashMap == this && getSize() > 0);

		uint32_t *FB_RESTRICT hashArray = getHashArray();
		PairType *FB_RESTRICT pairArray = getPairArray();
		uint32_t localCapacity = capacity;

		container::hashimp::eraseImp(it.index, hashArray, pairArray, localCapacity);
		--size;

		Iterator result = it;
		if (!isValidHash(hashArray[result.index]))
			result.index = getNextIndex(result.index);
		return result;
	}

	/* Some common for all maps utils */
	#include "MapCommonImp.h"
};

// Version which uses local buffer for initial storage.

template<typename Key, typename Value, int LocalCapacity, typename HashFunctor = container::DefaultHashFunctor>
struct CacheLinearHashMap : public LinearHashMap<Key, Value, HashFunctor>
{
protected:
	char buffer[LocalCapacity * (sizeof(uint32_t) + sizeof(typename LinearHashMap<Key, Value, HashFunctor>::PairType))] = { 0 };

public:
	typedef CacheLinearHashMap<Key, Value, LocalCapacity, HashFunctor> ThisClass;
	typedef LinearHashMap<Key, Value, HashFunctor> BaseClass;

	CacheLinearHashMap() : BaseClass(buffer, LocalCapacity, false)
	{
		fb_static_assert(LocalCapacity > 0);
		fb_static_assertf(container::LinearHashBase::isPowerOf2(uint32_t(LocalCapacity)), "CacheLinearHashMap LocalCapacity must be a power of two");
	}
	CacheLinearHashMap(const BaseClass &other) : BaseClass(buffer, LocalCapacity, false) { *this = other; }
	CacheLinearHashMap(BaseClass &&other) : BaseClass(buffer, LocalCapacity, false) { BaseClass::move(other); }
	CacheLinearHashMap(const ThisClass &other) : BaseClass(buffer, LocalCapacity, false) { *this = other; }
	CacheLinearHashMap(ThisClass &&other) : BaseClass(buffer, LocalCapacity, false) { BaseClass::move(other); }

	void operator = (const BaseClass &other) { BaseClass::operator= (other); }
	void operator = (BaseClass &&other) { BaseClass::move(other); }
	void operator = (const ThisClass &other) { BaseClass::operator= (other); }
	void operator = (ThisClass &&other) { BaseClass::move(other); }
};

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename Key, typename Value, typename HashFunctor
#define FB_CONTAINERIMP_CONTAINER_TYPE LinearHashMap<Key, Value, HashFunctor>
#include "ContainerCustomRangeFor.h"

FB_END_PACKAGE0()
