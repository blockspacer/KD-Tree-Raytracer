#ifndef FB_CONTAINER_DEQUEINLINE_INCLUDE_OK
	#error "DequeInline.h is supposed to be included by the Deque.h only."
#endif


FB_PACKAGE0()

// Deque::ConstIterator

template<class T, bool IsPod>
T Deque<T, IsPod>::ConstIterator::operator*()
{
	const Deque::ItemPointer& iPtr = deque->itemPointers[itemPointerIndex];
	return (*deque->items[iPtr.listIndex]).getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
const T &Deque<T, IsPod>::ConstIterator::operator*() const
{
	const Deque::ItemPointer& iPtr = deque->itemPointers[itemPointerIndex];
	return (*deque->items[iPtr.listIndex]).getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
T *Deque<T, IsPod>::ConstIterator::operator->() const
{
	Deque::ItemPointer& iPtr = deque->itemPointers[itemPointerIndex];
	return &(*deque->items[iPtr.listIndex]).getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::ConstIterator &Deque<T, IsPod>::ConstIterator::operator++()
{
	Deque::ItemPointer& iPtr = deque->itemPointers[itemPointerIndex];
	fb_expensive_assert(itemPointerIndex < deque->itemPointers.getSize() && "operator++ called for end iterator");
	if (itemPointerIndex != deque->itemIndex.getLastIndex())
		itemPointerIndex = deque->itemIndex.getNextIndexByOffset(itemPointerIndex, 1);
	else
		itemPointerIndex == deque->itemPointers.getSize();
	return *this;
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::ConstIterator Deque<T, IsPod>::ConstIterator::operator++(int)
{
	ConstIterator temp = *this;
	++(*this);
	return temp;
}


template<class T, bool IsPod>
bool Deque<T, IsPod>::ConstIterator::operator==(const ConstIterator &other) const
{
	return this->deque == other.deque && this->itemPointerIndex == other.itemPointerIndex;
}


template<class T, bool IsPod>
bool Deque<T, IsPod>::ConstIterator::operator!=(const ConstIterator &other) const
{
	return !(*this == other);
}


// Deque::Iterator

template<class T, bool IsPod>
T Deque<T, IsPod>::Iterator::operator*()
{
	const Deque::ItemPointer& iPtr = deque->itemIndex[itemPointerIndex];
	return (*deque->items[iPtr.listIndex]).getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
const T &Deque<T, IsPod>::Iterator::operator*() const
{
	const Deque::ItemPointer& iPtr = deque->itemPointers[itemPointerIndex];
	return (*deque->items[iPtr.listIndex]).getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
T *Deque<T, IsPod>::Iterator::operator->() const
{
	Deque::ItemPointer& iPtr = deque->itemPointers[itemPointerIndex];
	return &(*deque->items[iPtr.listIndex]).getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::Iterator &Deque<T, IsPod>::Iterator::operator++()
{
	fb_expensive_assert(itemPointerIndex < deque->itemIndex.getSize() && "operator++ called for end iterator");
	if (itemPointerIndex != deque->itemIndex.getLastIndex())
		itemPointerIndex = deque->itemIndex.getNextIndexByOffset(itemPointerIndex, 1);
	else
		itemPointerIndex = deque->itemIndex.getSize();
	return *this;
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::Iterator Deque<T, IsPod>::Iterator::operator++(int)
{
	Iterator temp = *this;
	++(*this);
	return temp;
}


template<class T, bool IsPod>
bool Deque<T, IsPod>::Iterator::operator==(const Iterator &other) const
{
	return this->deque == other.deque && this->itemPointerIndex == other.itemPointerIndex;
}


template<class T, bool IsPod>
bool Deque<T, IsPod>::Iterator::operator!=(const Iterator &other) const
{
	return !(*this == other);
}


// Deque


template<class T, bool IsPod>
Deque<T, IsPod>::Deque()
	: unitWithFreeSpace(uint16_t(StorageUnit::getMaxCapacity()))
{
	fb_assert(StorageUnit::getMaxCapacity() <= 1 << 16);
}


template<class T, bool IsPod>
Deque<T, IsPod>::Deque(const Deque<T, IsPod>& other)
	: unitWithFreeSpace(uint16_t(StorageUnit::getMaxCapacity()))
{
	SizeType numItems = other.getSize();
	reserve(numItems);
	for (ConstIterator it = other.getBegin(); it != other.getEnd(); ++it)
		this->pushBack(*it);
}


template<class T, bool IsPod>
Deque<T, IsPod>::~Deque()
{
	validate();
	while (!items.isEmpty())
	{
		delete items.getBack();
		items.popBack();
	}
}


template<class T, bool IsPod>
uint16_t Deque<T, IsPod>::getUnitToInsertTo()
{
	if (unitWithFreeSpace < items.getSize() && items[unitWithFreeSpace]->getSize() < items[unitWithFreeSpace]->getCapacity())
		return unitWithFreeSpace;

	for (SizeType i = 0, num = items.getSize(); i < num; ++i)
	{
		if (items[i]->getSize() < items[i]->getCapacity())
		{
			unitWithFreeSpace = uint16_t(i);
			return unitWithFreeSpace;
		}
	}

	items.pushBack(new StorageUnit());
	SizeType sizeToReserve = lang::min(StorageUnit::getMaxCapacity(), lang::max(itemIndex.getSize() * 2, getMinimumReserveSize()));
	items.getBack()->reserve(sizeToReserve);
	unitWithFreeSpace = uint16_t(items.getSize() - 1);
	return uint16_t(items.getSize() - 1);
}


template<class T, bool IsPod>
void Deque<T, IsPod>::pushBack(const T &t)
{
	validate();
	ItemPointer& iPtr = itemIndex.getNewPointerFromBack();
#if FB_DEQUE_DEBUGGING == FB_TRUE
	fb_assert(&iPtr == &itemIndex.getBack());
	iPtr.listIndex = iPtr.indexInList = 0xFFFF;
	itemIndex.popBack();
	validate();
	ItemPointer& iPtr2 = itemIndex.getNewPointerFromBack();
	fb_assert(&iPtr2 == &itemIndex.getBack() && &iPtr2 == &iPtr);
#endif
	iPtr.listIndex = getUnitToInsertTo();
#if FB_DEQUE_DEBUGGING == FB_TRUE
	SizeType capacityBefore = items[iPtr.listIndex]->getCapacity();
#endif
	SizeType index = items[iPtr.listIndex]->pushBack(t);
#if FB_DEQUE_DEBUGGING == FB_TRUE
	fb_assert(capacityBefore == items[iPtr.listIndex]->getCapacity() && "Unexpected capacity change.");
#endif
	fb_assert(index < 1 << 16);
	iPtr.indexInList = uint16_t(index);
	validate();
}


template<class T, bool IsPod>
void Deque<T, IsPod>::pushFront(const T &t)
{
	validate();
	ItemPointer& iPtr = itemIndex.getNewPointerFromFront();
	iPtr.listIndex = getUnitToInsertTo();
	SizeType index = items[iPtr.listIndex]->pushBack(t);
	fb_assert(index < 1 << 16);
	iPtr.indexInList = uint16_t(index);
	validate();
}


template<class T, bool IsPod>
void Deque<T, IsPod>::popBack()
{
	validate();
	ItemPointer& iPtr = itemIndex.getBack();
	items[iPtr.listIndex]->erase(iPtr.indexInList);
	itemIndex.popBack();
	validate();
}


template<class T, bool IsPod>
void Deque<T, IsPod>::popFront()
{
	validate();
	ItemPointer& iPtr = itemIndex.getFront();
	items[iPtr.listIndex]->erase(iPtr.indexInList);
	itemIndex.popFront();
	validate();
}


template<class T, bool IsPod>
T Deque<T, IsPod>::popFrontWithValue()
{
	/* Note: IMNSHO, this method is stupid. */

	// Note, use of this method is expensive with large data types as it makes temporaries of the data.
	// you can comment this out or change the limit if you think that your use case is justified.
	fb_expensive_assert(sizeof(T) <= 16);

	T ret = getFront();
	popFront();
	return ret;
}


template<class T, bool IsPod>
T Deque<T, IsPod>::popBackWithValue()
{
	/* Note: IMNSHO, this method is stupid. */

	// Note, use of this method is expensive with large data types as it makes temporaries of the data.
	// you can comment this out or change the limit if you think that your use case is justified.
	fb_expensive_assert(sizeof(T) <= 16);

	T ret = getBack();
	popBack();
	return ret;
}


template<class T, bool IsPod>
SizeType Deque<T, IsPod>::getSize() const
{
	validate();
	return itemIndex.getSize();
}


template<class T, bool IsPod>
SizeType Deque<T, IsPod>::getCapacity() const
{
	validate();
	SizeType count = 0;
	for (SizeType i = 0, num = items.getSize(); i < num; ++i)
		count += items[i]->getCapacity();

	return count;
}


template<class T, bool IsPod>
const T &Deque<T, IsPod>::operator[](SizeType index) const
{
	validate();
	ItemPointer& iPtr = itemIndex[index];
	return items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
T &Deque<T, IsPod>::operator[](SizeType index)
{
	validate();
	ItemPointer& iPtr = itemIndex[index];
	return items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
T &Deque<T, IsPod>::getFront()
{
	validate();
	ItemPointer& iPtr = itemIndex.getFront();
	return items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
const T &Deque<T, IsPod>::getFront() const
{
	validate();
	ItemPointer& iPtr = itemIndex.getFront();
	return items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
T &Deque<T, IsPod>::getBack()
{
	validate();
	ItemPointer& iPtr = itemIndex.getBack();
	return items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
const T &Deque<T, IsPod>::getBack() const
{
	validate();
	ItemPointer& iPtr = itemIndex.getBack();
	return items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::Iterator Deque<T, IsPod>::getBegin()
{
	validate();
	return Iterator(this, 0);
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::ConstIterator Deque<T, IsPod>::getBegin() const
{
	validate();
	return ConstIterator(this, 0);
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::Iterator Deque<T, IsPod>::getEnd()
{
	validate();
	return Iterator(this, itemIndex.getSize());
}


template<class T, bool IsPod>
typename Deque<T, IsPod>::ConstIterator Deque<T, IsPod>::getEnd() const
{
	validate();
	return ConstIterator(this, itemIndex.getSize());
}


template<class T, bool IsPod>
void Deque<T, IsPod>::clear()
{
	validate();
	itemIndex.clear();
	if (!items.isEmpty())
	{
		/* Allocation management: Save last storage unit, it should be the biggest (half of total). Free rest */
		StorageUnit* unit = items.getBack();
		unitWithFreeSpace = 0;
		items.popBack();
		unit->clear();
		while (!items.isEmpty())
		{
			delete items.getBack();
			items.popBack();
		}
		items.pushBack(unit);
	}
	validate();
}


template<class T, bool IsPod>
void Deque<T, IsPod>::reserve(SizeType size)
{
	validate();
	if (size > itemIndex.getSize())
		itemIndex.makeSpaceIfNecessary(size - itemIndex.getSize());

	SizeType capacity = getCapacity();
	if (capacity < size)
	{
		SizeType extraSizeNeeded = size - capacity;
		/* Reserve at least the size of last reservation */
		SizeType sizeToReserve = capacity > 0 ? items.getBack()->getCapacity() : getMinimumReserveSize();
		sizeToReserve = lang::max(sizeToReserve, extraSizeNeeded);
		/* Normally (with capacities smaller than 2^16), this while loop is skipped. */
		while (sizeToReserve > StorageUnit::getMaxCapacity())
		{
			items.pushBack(new StorageUnit());
			items.getBack()->reserve(StorageUnit::getMaxCapacity());
			sizeToReserve -= StorageUnit::getMaxCapacity();
		}
		items.pushBack(new StorageUnit());
		if (extraSizeNeeded > StorageUnit::getMaxCapacity())
		{
			/* We already did at least one reservation in while loop, might as well round this up */
			items.getBack()->reserve(StorageUnit::getMaxCapacity());
		}
		else
		{
			/* While loop was skipped, just reserve what was requested (more or less) */
			items.getBack()->reserve(sizeToReserve);
		}
	}

}


template<class T, bool IsPod>
bool Deque<T, IsPod>::isEmpty() const
{
	return getSize() == 0;
}


template<class T, bool IsPod>
void Deque<T, IsPod>::validate() const
{
#if FB_DEQUE_DEBUGGING == FB_TRUE
	fb_assert(itemIndex.indexes.getSize() == itemIndex.indexes.getCapacity());
	FB_PRINTF("Validating Deque: size: %d, storage amount: %d, index capacity: %d, firstIndex: %d, numIndexes: %d\n", itemIndex.getSize(), items.getSize(), itemIndex.indexes.getCapacity(), itemIndex.firstIndex, itemIndex.numIndexes);
	for (SizeType i = 0, num = itemIndex.getSize(); i < num; ++i)
	{
		const ItemPointer& iPtr = itemIndex[i];
		FB_PRINTF("\tItem at [%d]: %hu/%hu\n", i, iPtr.listIndex, iPtr.indexInList);
		fb_assert(iPtr.listIndex < 0xFFFF);
		fb_assert(iPtr.indexInList < 0xFFFF);
		FB_UNUSED_VAR(T&) = items[iPtr.listIndex]->getWithIndex(iPtr.indexInList);
		for (SizeType j = i + 1; j < num; ++j)
		{
			const ItemPointer& otherIPtr = itemIndex[j];
			fb_assert(iPtr.listIndex != otherIPtr.listIndex || iPtr.indexInList != otherIPtr.indexInList);
		}
	}
#endif
}

FB_END_PACKAGE0()
