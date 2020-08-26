#pragma once

#include "ImpByteElementArray.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/IsTriviallyCopyable.h"
#include <initializer_list>

FB_PACKAGE0()

#if 1
// Simple vector for POD types.
// Provides type safety over ImpByteElementArray which is non-template base to help linker.
// Should compile to pretty minimal code.

template<typename T>
struct PodVector : public container::ImpByteElementArray
{
	// If this fires, use Vector instead. If you feel type should be POD'able, just remove copy-constructor/operator and destructor.
	fb_static_assert(lang::IsTriviallyCopyable<T>::value);

protected:
	PodVector(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve)
		: ImpByteElementArray(staticPointer, staticSizeInElements, assertOnReserve)
	{
	}

public:
	static const bool IsPod = false;
	typedef T ValueType;
	typedef T* Iterator;
	typedef const T* ConstIterator;

	PodVector()
	{
	}

	~PodVector()
	{
		impReset(sizeof(T));
	}

	PodVector(const PodVector<T> &other)
	{
		impCopyFromOrdered(other, sizeof(T));
	}

	PodVector(PodVector<T> &&other)
	{
		move(other);
	}

	PodVector(const std::initializer_list<T>& list)
	{
		fb_assert(list.size() < 0xFFFFFFFF);
		insertIndex(0, list.begin(), uint32_t(list.size()));
	}

	template <uint32_t N>
	PodVector(const T (&values)[N])
	{
		insertIndex(0, values, N);
	}

	void operator = (const PodVector<T> &other)
	{
		impCopyFromOrdered(other, sizeof(T));
	}

	void operator = (PodVector<T> &&other)
	{
		move(other);
	}

	bool operator == (const PodVector<T> &other) const
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

	bool operator != (const PodVector<T> &other) const
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

	void swap(PodVector<T> &other)
	{
		ImpByteElementArray::impSwapOrdered(other, sizeof(T));
	}

	void move(PodVector<T> &other)
	{
		ImpByteElementArray::impMoveOrdered(other, sizeof(T));
	}

	void reserve(SizeType expectedCapacity)
	{
		if (expectedCapacity <= getCapacity())
			return;

		impReserveInElementsExactOrdered(sizeof(T), expectedCapacity);
	}

	void reserveRounded(SizeType expectedCapacity)
	{
		expectedCapacity = impGetNextPow2(expectedCapacity);
		if (expectedCapacity <= getCapacity())
			return;

		impReserveInElementsRoundedOrdered(sizeof(T), expectedCapacity);
	}

	void trimMemory()
	{
		impTrimMemoryOrdered(sizeof(T));
	}

	void resize(SizeType newSize, const T &defaultValue = T())
	{
		impResizeOrdered(sizeof(T), newSize, (const void*) &defaultValue);
	}

	void uninitialisedResize(SizeType newSize)
	{
		impResizeOrdered(sizeof(T), newSize, nullptr);
	}

	void insertIndex(SizeType index, const T &element)
	{
		impPrepareInsertOrdered(index, &element, sizeof(T));
		getPointer()[index] = element;
	}

	void insertIndex(SizeType index, const T *elements, uint32_t elementAmount)
	{
		impInsertArrayOrdered(index, elements, elementAmount, sizeof(T));
	}

	void swapOutIndex(SizeType index)
	{
		uint32_t localSize = getSize();
		FB_VECTOR_ASSERTF(index < localSize, "%d < %d", index, localSize);

		T *data = getPointer();
		if (index + 1 < localSize)
			data[index] = data[localSize - 1];

		size = localSize - 1;
	}

	void eraseIndex(uint32_t index)
	{
		impEraseOrdered(index, sizeof(T));
	}

	void eraseIndex(uint32_t index, uint32_t amount)
	{
		impEraseRangeOrdered(index, amount, sizeof(T));
	}

	void pushBack(const T &element)
	{
		uint32_t localSize = getSize();
		if (localSize < getCapacity())
		{
			getPointer()[localSize] = element;
			size = localSize + 1;	
			return;
		}

		impPushOrdered((const void*) &element, sizeof(T));
	}

	void popBack()
	{
		FB_VECTOR_ASSERT(!isEmpty());
		--size;
	}

	/* Most functions are in VectorCommonImp.h */
	#include "VectorCommonImp.h"
};

#else

// Version to test POD-vector specific bugs

template<typename T>
struct PodVector : public Vector<T>
{
protected:
	PodVector(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve) : Vector<T>(staticPointer, staticSizeInElements, assertOnReserve) {}

public:
	PodVector() {}
	PodVector(const PodVector<T> &other) { *this = other; }
	PodVector(PodVector<T> &&other) { Vector<T>::move(other); }

	void operator = (const PodVector &other) { Vector<T>::operator= (other); }
	void operator = (PodVector &&other) { Vector<T>::move(other); }
};

#endif

// Version which uses local buffer for initial storage.

template<typename T, int LocalCapacity>
struct CachePodVector: public PodVector<T>
{
protected:
	char buffer[LocalCapacity * sizeof(T)];

public:
	CachePodVector(): PodVector<T> (buffer, LocalCapacity, false) {}
	CachePodVector(const PodVector<T> &other): PodVector<T> (buffer, LocalCapacity, false) { *this = other; }
	CachePodVector(PodVector<T> &&other): PodVector<T> (buffer, LocalCapacity, false) { PodVector<T>::move(other); }
	CachePodVector(const CachePodVector<T, LocalCapacity> &other): PodVector<T> (buffer, LocalCapacity, false) { *this = other; }
	CachePodVector(CachePodVector<T, LocalCapacity> &&other): PodVector<T> (buffer, LocalCapacity, false) { PodVector<T>::move(other); }
	template <uint32_t N>
	CachePodVector(const T (&values)[N]): PodVector<T> (buffer, LocalCapacity, false) { PodVector<T>::insertIndex(0, values, N); }

	void operator = (const PodVector<T> &other) { PodVector<T>::operator= (other); }
	void operator = (PodVector<T> &&other) { PodVector<T>::move(other); }
	void operator = (const CachePodVector<T, LocalCapacity> &other) { PodVector<T>::operator= (other); }
	void operator = (CachePodVector<T, LocalCapacity> &&other) { PodVector<T>::move(other); }
};

// Version which uses local buffer for initial storage. Asserts if having to reserve more capacity.

template<typename T, int LocalCapacity>
struct StaticPodVector: public PodVector<T>
{
protected:
	char buffer[LocalCapacity * sizeof(T)];

public:
	enum { Capacity = LocalCapacity };

	StaticPodVector(): PodVector<T> (buffer, LocalCapacity, true) {}
	StaticPodVector(const PodVector<T> &other): PodVector<T> (buffer, LocalCapacity, true) { *this = other; }
	StaticPodVector(PodVector<T> &&other): PodVector<T> (buffer, LocalCapacity, true) { PodVector<T>::move(other); }
	StaticPodVector(const StaticPodVector<T, LocalCapacity> &other): PodVector<T> (buffer, LocalCapacity, true) { *this = other; }
	StaticPodVector(StaticPodVector<T, LocalCapacity> &&other): PodVector<T> (buffer, LocalCapacity, true) { PodVector<T>::move(other); }
	template <uint32_t N>
	StaticPodVector(const T (&values)[N]): PodVector<T> (buffer, LocalCapacity, false) { PodVector<T>::insertIndex(0, values, N); }

	void operator = (const PodVector<T> &other) { PodVector<T>::operator= (other); }
	void operator = (PodVector<T> &&other) { PodVector<T>::move(other); }
	void operator = (const StaticPodVector<T, LocalCapacity> &other) { PodVector<T>::operator= (other); }
	void operator = (StaticPodVector<T, LocalCapacity> &&other) { PodVector<T>::move(other); }
};

#define FB_VECIMP_TYPE PodVector
#include "VectorUtilsImp.h"

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename ValueType
#define FB_CONTAINERIMP_CONTAINER_TYPE PodVector<ValueType>
#include "ContainerRangeFor.h"

FB_END_PACKAGE0()
