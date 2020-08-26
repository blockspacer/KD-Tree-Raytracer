/* This file contains code for common functions in Vector and PodVector */

void insert(ConstIterator it, const T &element)
{
	insertIndex((SizeType)(it - getBegin()), element);
}

void insert(ConstIterator it, const T *elements, uint32_t elementAmount)
{
	insertIndex((SizeType)(it - getBegin()), elements, elementAmount);
}

void insert(ConstIterator it, const T *begin, const T *end)
{
	uint32_t index = (SizeType)(it - getBegin());
	uint32_t amount = (SizeType)(end - begin);
	insertIndex(index, begin, amount);
}

void swapOut(ConstIterator it)
{
	uint32_t index = (SizeType)(it - getBegin());
	swapOutIndex(index);
}

Iterator erase(ConstIterator it)
{
	uint32_t index = (SizeType)(it - getBegin());
	eraseIndex(index);
	return getBegin() + index;
}

Iterator erase(ConstIterator begin, ConstIterator end)
{
	uint32_t index = (SizeType)(begin - getBegin());
	uint32_t amount = (SizeType)(end - begin);
	eraseIndex(index, amount);
	return getBegin() + index;
}

T &pushBack()
{
	pushBack(T());
	return getBack();
}

FB_FORCEINLINE const T &operator[] (SizeType index) const
{
	FB_VECTOR_ASSERTF(index < getSize(), "%d < %d", index, getSize());
	return *(getPointer() + index);
}

FB_FORCEINLINE T &operator[] (SizeType index)
{
	FB_VECTOR_ASSERTF(index < getSize(), "%d < %d", index, getSize());
	return *(getPointer() + index);
}

const T &getFront() const
{
	FB_VECTOR_ASSERT(!isEmpty());
	return getPointer()[0];
}

T &getFront()
{
	FB_VECTOR_ASSERT(!isEmpty());
	return getPointer()[0];
}

const T &getBack() const
{
	FB_VECTOR_ASSERT(!isEmpty());
	return getPointer()[getSize() - 1];
}

T &getBack()
{
	FB_VECTOR_ASSERT(!isEmpty());
	return getPointer()[getSize() - 1];
}

FB_FORCEINLINE const T *getPointer() const
{
	return (const T*)getBytePointer();
}

FB_FORCEINLINE T *getPointer()
{
	return (T*)getBytePointer();
}

ConstIterator getBegin() const
{
	return getPointer();
}

ConstIterator getEnd() const
{
	return getPointer() + getSize();
}

Iterator getBegin()
{
	return getPointer();
}

Iterator getEnd()
{
	return getPointer() + getSize();
}

// Set external memory buffer to use. Can only be used when size == 0.
// Currently you don't really want to move these around, as move() is handled as-if having a static local buffer (copy if fits or switch to heap buffer).
// Possibly additional bit might make sense (whether pointer is movable), but honestly this is a special case you don't want to copy around.
// Just set the memory buffer when you've finished setting things up.
void initMemoryBuffer(void *memoryPointer, uint32_t memorySizeInBytes, bool assertOnReserve = true)
{
	impSwapToStaticPointer(sizeof(T), memoryPointer, memorySizeInBytes, assertOnReserve);
}
