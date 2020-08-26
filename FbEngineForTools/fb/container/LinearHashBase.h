#pragma once

#include "ImpByteElementArray.h"
#include "fb/lang/hash/Hash.h"
#include "fb/lang/PlacementNew.h"
#include "fb/lang/Swap.h"

FB_PACKAGE0()

// ---------------------------------------------------------------
// Generic hash functor. Treats Key as memory stream and hashes that.
// Be careful as Key containing pointers (strings, or somesuch) will not work correctly.
// ---------------------------------------------------------------
struct GenericHashFunctor
{
	template<typename Key>
	uint32_t operator () (const Key &key) const
	{
		return getHashValue((const char*)&key, sizeof(key));
	}
};

FB_END_PACKAGE0()

FB_PACKAGE1(container)

// ---------------------------------------------------------------
// Non-template base for linear hash containers
// ---------------------------------------------------------------
struct LinearHashBase: public ImpByteElementArray
{
protected:
	LinearHashBase(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve): ImpByteElementArray(staticPointer, staticSizeInElements, assertOnReserve) {}
	LinearHashBase() {}

	uint32_t getCapacityFromSize(uint32_t wantedSize) const;
	bool shouldResize(uint32_t localSize, uint32_t localCapacity) const
	{
		return ((localSize + 1) > (localCapacity / 2));
	}
	bool shouldResize() const
	{
		return shouldResize(getSize(), getCapacity());
	}

	bool isValidHash(uint32_t hashValue) const
	{
		return hashValue != 0;
	}

	uint32_t *getHashArray() { return (uint32_t *) getBytePointer(); }
	const uint32_t *getHashArray() const { return (const uint32_t *) getBytePointer(); }

	uint32_t getFirstIndex() const;
	uint32_t getNextIndex(uint32_t index) const;

public:
	static constexpr bool isPowerOf2(uint32_t v)
	{
		return v > 0 && ((v & (v - 1U)) == 0U);
	}

	float getAverageProbeCount() const;
};

// ---------------------------------------------------------------
// Some basic functionality for both HashMap and HashSet
// ---------------------------------------------------------------
namespace hashimp {

	static inline uint32_t getProbeDistance(uint32_t hashValue, uint32_t index, uint32_t capacityMask)
	{
		return (index - hashValue) & capacityMask;
	}

	template<typename KeyType, typename HashFunctor>
	static inline uint32_t getHashValue(const KeyType &key)
	{
		uint32_t hashValue = (uint32_t) HashFunctor()(key);
		hashValue = hashValue ? hashValue : 1;
		return hashValue;
	}

extern uint64_t totalFindSwapAmount;
extern uint64_t sumFindSwapDistance;
extern uint64_t totalFindAmount;
extern uint64_t sumFindDistance;

	// Returns capacity if not found
	template<typename KeyType, typename ValueType>
	uint32_t getInternalIndex(uint32_t hashValue, const KeyType &key, const uint32_t *FB_RESTRICT hashArray, const ValueType *FB_RESTRICT valueArray, uint32_t capacity)
	{
		if (!hashArray) // Empty container
			return capacity;

		uint32_t capacityMask = capacity - 1;
		uint32_t index = hashValue & capacityMask;
		uint32_t probeDistance = 0;

		for (;;)
		{
			uint32_t currentHashValue = hashArray[index];

			// Take away a branch by combining
			bool notFound = (currentHashValue == 0) ? true : false;
			notFound = probeDistance > getProbeDistance(currentHashValue, index, capacityMask) ? true : notFound;
			if (notFound)
				return capacity;
			else if ((currentHashValue == hashValue) && (valueArray[index] == key)) // Match
				return index;

			index = (index + 1) & capacityMask;
			++probeDistance;
		}
	}

	// Make sure you don't call insertImp if the value already exists.
	template<typename ValueType>
	uint32_t insertImp(uint32_t hashValue, ValueType &&value, uint32_t *FB_RESTRICT hashArray, ValueType *FB_RESTRICT valueArray, uint32_t capacity)
	{
		uint32_t capacityMask = capacity - 1;
		uint32_t probeDistance = 0;
		uint32_t index = hashValue & capacityMask;
		uint32_t insertIndex = 0xffffffff;

		for (;;)
		{
			uint32_t currentHashValue = hashArray[index];
			if (currentHashValue == 0)
			{
				new (valueArray + index) ValueType((ValueType &&) value);
				hashArray[index] = hashValue;
				return (insertIndex == 0xffffffff) ? index : insertIndex;
			}

			uint32_t currentProbeDistance = getProbeDistance(currentHashValue, index, capacityMask);
			if (currentProbeDistance < probeDistance)
			{
				lang::swap(hashArray[index], hashValue);
				lang::swap(valueArray[index], value);
				probeDistance = currentProbeDistance;
				insertIndex = (insertIndex == 0xffffffff) ? index : insertIndex;
			}

			index = (index + 1) & capacityMask;
			++probeDistance;
		}
	}

	template<typename ValueType>
	void eraseImp(uint32_t index, uint32_t *FB_RESTRICT hashArray, ValueType *FB_RESTRICT valueArray, uint32_t capacity)
	{
		valueArray[index].~ValueType();

		uint32_t originalIndex = index;
		uint32_t capacityMask = capacity - 1;
		for (;;)
		{
			uint32_t nextIndex = (index + 1) & capacityMask;
			uint32_t nextHashValue = hashArray[nextIndex];
			if ((nextIndex == originalIndex) || (!nextHashValue) || (getProbeDistance(nextHashValue, nextIndex, capacityMask) == 0))
				break;

			hashArray[index] = nextHashValue;
			new (valueArray + index) ValueType((ValueType&&) valueArray[nextIndex]);
			valueArray[nextIndex].~ValueType();

			index = nextIndex;
		}

		hashArray[index] = 0;
	}
} // hashimp

static inline uint32_t global_default_hash_function(uint64_t number) { return getNumberHashValue(number); }
static inline uint32_t global_default_hash_function(uint32_t number) { return getNumberHashValue(number); }
static inline uint32_t global_default_hash_function(int32_t number) { return getNumberHashValue((uint32_t)number); }
static inline uint32_t global_default_hash_function(int64_t number) { return getNumberHashValue((uint64_t)number); }
uint32_t global_default_hash_function(float number);

// ---------------------------------------------------------------
// Default hash functor. Deals with numbers/pointers.
// ---------------------------------------------------------------
struct DefaultHashFunctor
{
	template<typename T>
	inline uint32_t operator () (const T& t) { return global_default_hash_function(t); }
	template<typename T>
	inline uint32_t operator () (T* t) { return global_default_hash_function(intptr_t(t)); }
};

FB_END_PACKAGE1()
