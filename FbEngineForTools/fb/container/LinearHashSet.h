#pragma once

#include "LinearHashBase.h"
#include "fb/container/Pair.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/Move.h"

FB_PACKAGE0()

// Open addressing (closed) hash map. Uses robin hood hashing.
template<typename Key, typename HashFunctor = container::DefaultHashFunctor>
struct LinearHashSet: public container::LinearHashBase
{
	typedef LinearHashSet<Key, HashFunctor> ThisType;
	typedef Key KeyType;
	typedef Key ValueType;

protected:
	LinearHashSet(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve): LinearHashBase(staticPointer, staticSizeInElements, assertOnReserve) {}

	const KeyType *getKeyArray() const
	{
		const uint32_t *hashPointer = (const uint32_t*) getBytePointer();
		return (const KeyType *) (hashPointer + getCapacity());
	}
	KeyType *getKeyArray()
	{
		uint32_t *hashPointer = (uint32_t*) getBytePointer();
		return (KeyType *) (hashPointer + getCapacity());
	}

	void reserveImp(SizeType newCapacity)
	{
		newCapacity = newCapacity < 8 ? 8 : newCapacity;
		uint32_t localCapacity = getCapacity();
		if (newCapacity <= localCapacity)
			return;

		uint32_t *FB_RESTRICT oldHashArray = (uint32_t *) getBytePointer();
		KeyType *FB_RESTRICT oldKeyArray = (KeyType *) (oldHashArray + localCapacity);
		uint32_t *FB_RESTRICT newHashArray = (uint32_t*) impGetNewPointerExact(sizeof(uint32_t) + sizeof(KeyType), newCapacity);
		KeyType *FB_RESTRICT newKeyArray = (KeyType *) (newHashArray + newCapacity);

		lang::MemSet::set(newHashArray, 0, newCapacity * sizeof(uint32_t));
		for (uint32_t i = 0; i < localCapacity; ++i)
		{
			uint32_t oldHashValue = oldHashArray[i];
			if (isValidHash(oldHashValue))
			{
				container::hashimp::insertImp<KeyType>(oldHashValue, (KeyType&&)oldKeyArray[i], newHashArray, newKeyArray, newCapacity);
				oldKeyArray[i].~KeyType();
			}
		}

		impSwapPointer(sizeof(uint32_t) + sizeof(KeyType), newHashArray, newCapacity);
	}

	KeyType forceCopy(const KeyType &key) const { return key; }

public:

	LinearHashSet() {}
	LinearHashSet(const LinearHashSet &other) { copy(other); }
	LinearHashSet(LinearHashSet &&other) { move(other); }
	~LinearHashSet() { clear(); impReset(sizeof(uint32_t) + sizeof(KeyType)); }
	void operator = (const LinearHashSet &other) { clear(); copy(other); }
	void operator = (LinearHashSet &&other) { move(other); }

	void copy(const LinearHashSet &other)
	{
		if (this == &other)
			return;

		uint32_t *FB_RESTRICT hashArray = getHashArray();
		KeyType *FB_RESTRICT keyArray = getKeyArray();
		uint32_t localCapacity = getCapacity();
		for (uint32_t i = 0; i < localCapacity; ++i)
		{
			if (isValidHash(hashArray[i]))
			{
				hashArray[i] = 0;
				keyArray[i].~KeyType();
			}
		}

		size = 0;
		uint32_t otherCapacity = other.getCapacity();
		uint32_t otherSize = other.getSize();
		reserveImp(getCapacityFromSize(otherSize));
		
		uint32_t newCapacity = getCapacity();
		uint32_t *FB_RESTRICT newHashArray = getHashArray();
		KeyType *FB_RESTRICT newKeyArray = getKeyArray();

		const uint32_t *FB_RESTRICT otherHashArray = other.getHashArray();
		const KeyType *FB_RESTRICT otherKeyArray = other.getKeyArray();
		for (uint32_t i = 0; i < otherCapacity; ++i)
		{
			if (isValidHash(otherHashArray[i]))
			{
				KeyType otherKey(otherKeyArray[i]);
				container::hashimp::insertImp<KeyType>(otherHashArray[i], lang::move(otherKey), newHashArray, newKeyArray, newCapacity);
			}
		}
		size = otherSize;
	}

	// ToDo: Fix move and swap when implementing static buffers!
	void swap(LinearHashSet &other) { lang::swap(intPointer, other.intPointer); lang::swap(size, other.size); lang::swap(capacity, other.capacity); }
	void move(LinearHashSet &other) { swap(other); }

	void clear()
	{
		if(size)
		{
			uint32_t *FB_RESTRICT hashArray = getHashArray();
			KeyType *FB_RESTRICT keyArray = getKeyArray();
			uint32_t localCapacity = getCapacity();
			for (uint32_t i = 0; i < localCapacity; ++i)
			{
				if (isValidHash(hashArray[i]))
				{
					hashArray[i] = 0;
					keyArray[i].~KeyType();
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
		const ThisType *hashSet;
		uint32_t index;

		Iterator(const ThisType *const hashSet_, uint32_t index_) : hashSet(hashSet_), index(index_) {}

		const KeyType &operator *() const
		{
			return hashSet->getKeyArray()[index];
		}

		Iterator &operator++() { index = hashSet->getNextIndex(index); return (*this); }
		Iterator operator++(int) { Iterator temp = *this; index = hashSet->getNextIndex(index); return temp; }
		bool operator==(const Iterator &other) const { return index == other.index; }
		bool operator!=(const Iterator &other) const { return index != other.index; }
	};

	struct ConstIterator
	{
		const ThisType *hashSet;
		uint32_t index;

		ConstIterator(const ThisType *const hashSet_, uint32_t index_) : hashSet(hashSet_), index(index_) {}
		ConstIterator(const Iterator &copy) : hashSet(copy.hashSet), index(copy.index) {}
		ConstIterator& operator=(const Iterator &copy)
		{
			hashSet = copy.hashSet;
			index = copy.index;
			return *this;
		}

		const KeyType &operator *() const
		{
			return hashSet->getKeyArray()[index];
		}

		ConstIterator &operator++() { index = hashSet->getNextIndex(index); return (*this); }
		ConstIterator operator++(int) { ConstIterator temp = *this; index = hashSet->getNextIndex(index); return temp; }
		bool operator==(const ConstIterator &other) const { return index == other.index; }
		bool operator!=(const ConstIterator &other) const { return index != other.index; }
	};

	ConstIterator getBegin() const { return ConstIterator(this, getFirstIndex()); }
	Iterator getBegin() { return Iterator(this, getFirstIndex()); }
	ConstIterator getEnd() const { return ConstIterator(this, getCapacity()); }
	Iterator getEnd() { return Iterator(this, getCapacity()); }

	ConstIterator find(const Key &key) const { return ConstIterator(this, container::hashimp::getInternalIndex<KeyType, KeyType> (container::hashimp::getHashValue<KeyType, HashFunctor>(key), key, getHashArray(), getKeyArray(), getCapacity())); }
	Iterator find(const Key &key) { return Iterator(this, container::hashimp::getInternalIndex<KeyType, KeyType>(container::hashimp::getHashValue<KeyType, HashFunctor>(key), key, getHashArray(), getKeyArray(), getCapacity())); }

	#define FB_LINHASH_INSERT_IMP(insertKeyString) \
		uint32_t *FB_RESTRICT hashArray = getHashArray(); \
		KeyType *FB_RESTRICT keyArray = getKeyArray(); \
		uint32_t localCapacity = capacity; \
		uint32_t hashValue = container::hashimp::getHashValue<KeyType, HashFunctor>(key);  \
		uint32_t index = container::hashimp::getInternalIndex(hashValue, key, hashArray, keyArray, localCapacity);  \
		if (index < localCapacity) \
			return Pair<Iterator, bool>(Iterator(this, index), false); \
		uint32_t localSize = getSize(); \
		if (shouldResize(localSize, localCapacity))  \
		{  \
			reserveImp(localCapacity * 2);  \
			hashArray = getHashArray(); \
			keyArray = getKeyArray(); \
			localCapacity = capacity; \
		} \
		index = container::hashimp::insertImp<KeyType>(hashValue, insertKeyString, hashArray, keyArray, localCapacity); \
		size = localSize + 1; \
		return Pair<Iterator, bool> (Iterator(this, index), true)

	Pair<Iterator, bool> insert(const KeyType &key) { FB_LINHASH_INSERT_IMP( forceCopy(key) ); }
	Pair<Iterator, bool> insert(KeyType &&key) { FB_LINHASH_INSERT_IMP((KeyType&&)key); }
	#undef FB_LINHASH_INSERT_IMP

	bool eraseKey(const KeyType &key)
	{
		uint32_t *FB_RESTRICT hashArray = getHashArray();
		KeyType *FB_RESTRICT keyArray = getKeyArray();
		uint32_t localCapacity = capacity;

		uint32_t hashValue = container::hashimp::getHashValue<KeyType, HashFunctor>(key);
		uint32_t index = container::hashimp::getInternalIndex<KeyType, KeyType>(hashValue, key, hashArray, keyArray, localCapacity);
		if (index < getCapacity())
		{
			container::hashimp::eraseImp(index, hashArray, keyArray, localCapacity);
			--size;
			return true;
		}
	
		return false;
	}

	Iterator erase(const ConstIterator &it)
	{
		fb_expensive_assert(it.hashSet == this && getSize() > 0);

		uint32_t *FB_RESTRICT hashArray = getHashArray();
		KeyType *FB_RESTRICT keyArray = getKeyArray();
		const uint32_t localCapacity = capacity;

		container::hashimp::eraseImp(it.index, hashArray, keyArray, localCapacity);
		--size;

		Iterator result(this, it.index);
		if (!isValidHash(hashArray[result.index]))
			result.index = getNextIndex(result.index);
		return result;
	}
};

// Version which uses local buffer for initial storage.

template<typename Key, int LocalCapacity, typename HashFunctor = container::DefaultHashFunctor>
struct CacheLinearHashSet : public LinearHashSet<Key, HashFunctor>
{
protected:
	char buffer[LocalCapacity * (sizeof(uint32_t) + sizeof(Key))] = { 0 };

public:
	typedef CacheLinearHashSet<Key, LocalCapacity, HashFunctor> ThisClass;
	typedef LinearHashSet<Key, HashFunctor> BaseClass;

	CacheLinearHashSet() : BaseClass(buffer, LocalCapacity, false)
	{
		fb_static_assert(LocalCapacity > 0);
		fb_static_assertf(container::LinearHashBase::isPowerOf2(uint32_t(LocalCapacity)), "CacheLinearHashMap LocalCapacity must be a power of two");
	}
	CacheLinearHashSet(const BaseClass &other) : BaseClass(buffer, LocalCapacity, false) { *this = other; }
	CacheLinearHashSet(BaseClass &&other) : BaseClass(buffer, LocalCapacity, false) { BaseClass::move(other); }
	CacheLinearHashSet(const ThisClass &other) : BaseClass(buffer, LocalCapacity, false) { *this = other; }
	CacheLinearHashSet(ThisClass &&other) : BaseClass(buffer, LocalCapacity, false) { BaseClass::move(other); }

	void operator = (const BaseClass &other) { BaseClass::operator= (other); }
	void operator = (BaseClass &&other) { BaseClass::move(other); }
	void operator = (const ThisClass &other) { BaseClass::operator= (other); }
	void operator = (ThisClass &&other) { BaseClass::move(other); }
};

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename Key, typename HashFunctor
#define FB_CONTAINERIMP_CONTAINER_TYPE LinearHashSet<Key, HashFunctor>
#include "ContainerRangeFor.h"

FB_END_PACKAGE0()
