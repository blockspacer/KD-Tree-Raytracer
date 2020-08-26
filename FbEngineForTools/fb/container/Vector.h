#pragma once

#include "ImpByteElementArray.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/IsTriviallyCopyable.h"
#include "fb/lang/PlacementNew.h"
#include "fb/lang/Swap.h"
#include <initializer_list>

FB_PACKAGE0()

// Vector for general types.
// Use PodVector if possible

template<typename T>
struct Vector: public container::ImpByteElementArray
{
	// If this fires, use PodVector instead as it generates much less overhead.
	fb_static_assert(!lang::IsTriviallyCopyable<T>::value);

protected:
	Vector(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve)
		: ImpByteElementArray(staticPointer, staticSizeInElements, assertOnReserve)
	{
	}

public:
	static const bool IsPod = false;
	typedef T ValueType;
	typedef T* Iterator;
	typedef const T* ConstIterator;

	Vector()
	{
	}

	Vector(const Vector &other)
	{
		*this = other;
	}

	Vector(Vector &&other)
	{
		move(other);
	}

	Vector(const std::initializer_list<T>& list)
	{
		fb_assert(list.size() < 0xFFFFFFFF);
		insertIndex(0, list.begin(), uint32_t(list.size()));
	}

	~Vector()
	{
		clear();
		impReset(sizeof(T));
	}

	void operator = (const Vector<T> &other)
	{
		if (this == &other)
			return;

		clear();

		uint32_t localCapacity = getCapacity();
		uint32_t otherSize = other.getSize();
		if (localCapacity < otherSize)
			reserve(otherSize);

		T *data = getPointer();
		const T *otherData = other.getPointer();
		for (uint32_t i = 0; i < otherSize; ++i)
			new (&data[i]) T(otherData[i]);

		size = otherSize;
	}

	void operator = (Vector<T> &&other)
	{
		move(other);
	}

	bool operator == (const Vector<T> &other) const
	{
		if (getSize() != other.getSize())
		{
			return false;
		}
		for (SizeType i = 0; i < getSize(); i++)
		{
			if (getPointer()[i] != other[i])
			{
				return false;
			}
		}
		return true;
	}

	bool operator != (const Vector<T> &other) const
	{
		if (getSize() != other.getSize())
		{
			return true;
		}
		for (SizeType i = 0; i < getSize(); i++)
		{
			if (getPointer()[i] != other[i])
			{
				return true;
			}
		}
		return false;
	}

	void swap(Vector<T> &other)
	{
		if (this == &other)
			return;

		// Easy case
		bool localStaticAllocation = isStaticAllocation();
		bool otherStaticAllocation = other.isStaticAllocation();
		if (!localStaticAllocation && !otherStaticAllocation)
		{
			impPointerSwap(other);
			return;
		}

		// This is (potentially) horribly slow
		lang::swap(*this, other);
	}

	void move(Vector<T> &other)
	{
		if (this == &other)
			return;

		bool localStaticAllocation = isStaticAllocation();
		bool otherStaticAllocation = other.isStaticAllocation();
		uint32_t localCapacity = getCapacity();
		uint32_t otherSize = other.getSize();

		clear();

		// In this specific case, we can get rid of our puny static pointer, and steal dynamic one from other.
		if ((localStaticAllocation == true && otherStaticAllocation == false) && localCapacity < otherSize)
		{
			impZeroPointer();
			localStaticAllocation = false;
			localCapacity = 0;
		}

		// Easy case
		if (!localStaticAllocation && !otherStaticAllocation)
		{
			impPointerSwap(other);
			return;
		}

		// Have to move the data into our buffer. 

		if (localCapacity < otherSize)
		{
			reserve(otherSize);
			localCapacity = getCapacity();
		}

		T *data = getPointer();
		const T *otherData = other.getPointer();
		for (uint32_t i = 0; i < otherSize; ++i)
		{
			new (&data[i]) T((T&&) otherData[i]);
			otherData[i].~T();
		}

		size = otherSize;
		other.size = 0;
	}

	void clear()
	{
		T *data = getPointer();
		uint32_t localSize = getSize();

		for (uint32_t i = 0; i < localSize; ++i)
			data[i].~T();
		size = 0;
	}

	void reserve(SizeType neededCapacity)
	{
		uint32_t localCapacity = getCapacity();
		FB_VECTOR_ASSERTF(neededCapacity >= getSize(), "%d >= %d", neededCapacity, getSize());
		if (localCapacity >= neededCapacity)
			return;

		uint32_t newCapacity = (uint32_t) neededCapacity;
		T *oldPointer = getPointer();
		T *newPointer = (T*) impGetNewPointerExact(sizeof(T), newCapacity);

		uint32_t localSize = getSize();
		for (uint32_t i = 0; i < localSize; ++i)
		{
			new (&newPointer[i]) T((T&&) oldPointer[i]);
			oldPointer[i].~T();
		}

		impSwapPointer(sizeof(T), newPointer, newCapacity);
	}

	void reserveRounded(SizeType neededCapacity)
	{
		uint32_t localCapacity = getCapacity();
		FB_VECTOR_ASSERTF(neededCapacity >= getSize(), "%d >= %d", neededCapacity, getSize());
		if (localCapacity >= neededCapacity)
			return;

		uint32_t newCapacity = (uint32_t) neededCapacity;
		T *oldPointer = getPointer();
		T *newPointer = (T*) impGetNewPointer(sizeof(T), newCapacity);

		uint32_t localSize = getSize();
		for (uint32_t i = 0; i < localSize; ++i)
		{
			new (&newPointer[i]) T((T&&) oldPointer[i]);
			oldPointer[i].~T();
		}

		impSwapPointer(sizeof(T), newPointer, newCapacity);
	}

	void trimMemory()
	{
		uint32_t localCapacity = getCapacity();
		uint32_t localSize = getSize();
		if ((localCapacity == localSize) || isStaticAllocation())
			return;

		T *oldPointer = getPointer();
		T *newPointer = (T*) impGetNewPointerExact(sizeof(T), localSize);

		for (uint32_t i = 0; i < localSize; ++i)
		{
			new (&newPointer[i]) T((T&&) oldPointer[i]);
			oldPointer[i].~T();
		}

		impSwapPointer(sizeof(T), newPointer, localSize);
	}

	void resize(SizeType newSize, const T &defaultValue = T())
	{
		uint32_t localCapacity = getCapacity();
		if (localCapacity < newSize)
			reserve(newSize);

		T *data = getPointer();
		uint32_t localSize = getSize();

		if (newSize >= localSize)
		{
			for (uint32_t i = localSize; i < newSize; ++i)
				new (&data[i]) T(defaultValue);
		}
		else
		{
			for (uint32_t i = newSize; i < localSize; ++i)
				data[i].~T();
		}

		size = newSize;
	}

	void insertIndex(SizeType index, const T &element)
	{
		uint32_t localCapacity = getCapacity();
		uint32_t localSize = getSize();
		if (localSize == localCapacity)
			reserveRounded(localCapacity + 1);

		T *data = getPointer();
		if (index < localSize)
		{
			// Move stuff to make space
			for (uint32_t i = localSize; i != index; --i)
			{
				new (&data[i]) T((T&&) data[i - 1]);
				(&data[i - 1])->~T();
			}
		}

		new (&data[index]) T(element);
		size = localSize + 1;
	}

	void insertIndex(SizeType index, T &&element)
	{
		uint32_t localCapacity = getCapacity();
		uint32_t localSize = getSize();
		if (localSize == localCapacity)
			reserveRounded(localCapacity + 1);

		T *data = getPointer();
		if (index < localSize)
		{
			// Move stuff to make space
			for (uint32_t i = localSize; i != index; --i)
			{
				new (&data[i]) T((T&&) data[i - 1]);
				(&data[i - 1])->~T();
			}
		}

		new (&data[index]) T((T&&)element);
		size = localSize + 1;
	}

	void insertIndex(SizeType index, const T *elements, SizeType elementAmount)
	{
		if (elementAmount == 0)
			return;

		uint32_t localCapacity = getCapacity();
		uint32_t localSize = getSize();
		if (localSize + elementAmount > localCapacity)
			reserveRounded(localCapacity + elementAmount);

		T *data = getPointer();
		if (index < localSize)
		{
			// Move stuff to make space
			for (uint32_t i = localSize; i != index; --i)
			{
				new (&data[i + elementAmount - 1]) T((T&&) data[i - 1]);
				(&data[i - 1])->~T();
			}
		}

		for (uint32_t i = 0; i < elementAmount; ++i)
			new (&data[i + index]) T(elements[i]);
		size = localSize + elementAmount;
	}

	void swapOutIndex(SizeType index)
	{
		uint32_t localSize = getSize();
		FB_VECTOR_ASSERTF(index < localSize, "%d < %d", index, localSize);

		T *data = getPointer();
		data[index].~T();

		if (index + 1 < localSize)
		{
			new (&data[index]) T((T&&) data[localSize - 1]);
			data[localSize - 1].~T();
		}
		size = localSize - 1;
	}

	void eraseIndex(SizeType index)
	{
		uint32_t localSize = getSize();
		FB_VECTOR_ASSERTF(index < localSize, "%d < %d", index, localSize);

		T *data = getPointer();
		data[index].~T();

		for (uint32_t i = index + 1; i < localSize; ++i)
		{
			new (&data[i-1]) T((T&&) data[i]);
			data[i].~T();
		}

		size = localSize - 1;
	}

	void eraseIndex(SizeType index, uint32_t amount)
	{
		if (amount == 0)
			return;

		uint32_t localSize = getSize();
		FB_VECTOR_ASSERTF(index + amount <= localSize, "%d + %d <= %d", index, amount, localSize);

		T *data = getPointer();

		for (SizeType i = index; i < index + amount; ++i)
			data[i].~T();

		for (SizeType i = index; i < localSize - amount; ++i)
		{
			new (&data[i]) T((T&&) data[i + amount]);
			data[i + amount].~T();
		}

		size = localSize - amount;
	}

	void pushBack(const T &element)
	{
		uint32_t localSize = getSize();
		uint32_t localCapacity = getCapacity();
		if (localSize == localCapacity)
			reserveRounded(localCapacity + 1);

		T *data = getPointer();
		new (&data[localSize]) T(element);
		size = localSize + 1;
	}

	void pushBack(T &&element)
	{
		uint32_t localSize = getSize();
		uint32_t localCapacity = getCapacity();
		if (localSize == localCapacity)
			reserveRounded(localCapacity + 1);

		T *data = getPointer();
		new (&data[localSize]) T((T&&)element);
		size = localSize + 1;
	}

	void popBack() 
	{ 
		FB_VECTOR_ASSERT(!isEmpty());
		uint32_t localSize = getSize();
		getPointer()[localSize - 1].~T();
		size = localSize - 1; 
	}

	/* Most functions are in VectorCommonImp.h */
	#include "VectorCommonImp.h"
};

// Version which uses local buffer for initial storage.

template<typename T, int LocalCapacity>
struct CacheVector: public Vector<T>
{
protected:
	char buffer[LocalCapacity * sizeof(T)];

public:
	CacheVector(): Vector<T> (buffer, LocalCapacity, false) {}
	CacheVector(const Vector<T> &other): Vector<T> (buffer, LocalCapacity, false) { *this = other; }
	CacheVector(Vector<T> &&other): Vector<T> (buffer, LocalCapacity, false) { Vector<T>::move(other); }
	CacheVector(const CacheVector<T, LocalCapacity> &other): Vector<T> (buffer, LocalCapacity, false) { *this = other; }
	CacheVector(CacheVector<T, LocalCapacity> &&other): Vector<T> (buffer, LocalCapacity, false) { Vector<T>::move(other); }

	void operator = (const Vector<T> &other) { Vector<T>::operator= (other); }
	void operator = (Vector<T> &&other) { Vector<T>::move(other); }
	void operator = (const CacheVector<T, LocalCapacity> &other) { Vector<T>::operator= (other); }
	void operator = (CacheVector<T, LocalCapacity> &&other) { Vector<T>::move(other); }
};

// Version which uses local buffer for initial storage. Asserts if having to reserve more capacity.

template<typename T, int LocalCapacity>
struct StaticVector: public Vector<T>
{
protected:
	char buffer[LocalCapacity * sizeof(T)];

public:
	enum { Capacity = LocalCapacity };

	StaticVector(): Vector<T> (buffer, LocalCapacity, true) {}
	StaticVector(const Vector<T> &other): Vector<T> (buffer, LocalCapacity, true) { *this = other; }
	StaticVector(Vector<T> &&other): Vector<T> (buffer, LocalCapacity, true) { Vector<T>::move(other); }
	StaticVector(const StaticVector<T, LocalCapacity> &other): Vector<T> (buffer, LocalCapacity, true) { *this = other; }
	StaticVector(StaticVector<T, LocalCapacity> &&other): Vector<T> (buffer, LocalCapacity, true) { Vector<T>::move(other); }

	void operator = (const Vector<T> &other) { Vector<T>::operator= (other); }
	void operator = (Vector<T> &&other) { Vector<T>::move(other); }
	void operator = (const StaticVector<T, LocalCapacity> &other) { Vector<T>::operator= (other); }
	void operator = (StaticVector<T, LocalCapacity> &&other) { Vector<T>::move(other); }
};

#define FB_VECIMP_TYPE Vector
#include "VectorUtilsImp.h"

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename ValueType
#define FB_CONTAINERIMP_CONTAINER_TYPE Vector<ValueType>
#include "ContainerRangeFor.h"

FB_END_PACKAGE0()
