#ifndef FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLISTINLINE_INCLUDE_OK
#error "AllocFriendlyListInline.h is supposed to be included by the AllocFriendlyList.h only."
#endif

#include "fb/lang/MemTools.h"

FB_PACKAGE1(container)

template<class T, typename IndexType, bool IsPod>
IndexType AllocFriendlyList<T, IndexType, IsPod>::allocIndex()
{
	fb_expensive_assert(firstFreeIndex != getInvalidIndex());
	fb_expensive_assert(firstFreeIndex <= size);

	if (firstFreeIndex >= capacity)
	{
		fb_assert(size == capacity);
		/* Make space */
		uint64_t newCapacity = capacity + capacity / 2U;
		if (newCapacity >= getMaxCapacity())
		{
			/* Can't go over max. If that can't be helped, let reserve assert. */
			if (capacity < getMaxCapacity())
				newCapacity = getMaxCapacity() - 1;
		}
		reserve(SizeType(newCapacity + 1));
		/* Return and increment */
		++size;
		return firstFreeIndex++;
	}
	++size;
	/* Find next free slot or if none, leave firstFreeIndex point to capacity */
	IndexType result = firstFreeIndex;
	for (++firstFreeIndex; firstFreeIndex < capacity; ++firstFreeIndex)
	{
		if (!values[firstFreeIndex].isValid())
			return result;
	}

	return result;
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::freeIndex(IndexType index)
{
	fb_expensive_assert(capacity > index);

	Node& node = values[index];
	fb_expensive_assert(node.isValid());

	if (node.next != getInvalidIndex())
		values[node.next].previous = node.previous;

	if (node.previous != getInvalidIndex())
		values[node.previous].next = node.next;

	if (IsPod)
		values[index].reset();
	else
		(&values[index])->~Node();

	overwriteMemory(&node.item, sizeof(T));
	--size;

	if (index < firstFreeIndex)
		firstFreeIndex = index;
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::checkInternalState()
{
#if FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLIST_DEBUG_ENABLED == FB_TRUE
	fb_assert(size <= capacity);
	fb_assert((capacity > 0 && values != nullptr) || (capacity == 0 && values == nullptr));
	fb_assert(firstFreeIndex <= capacity);
	fb_assert(firstFreeIndex == capacity || !values[firstFreeIndex].isValid());
	fb_assert(firstNode != getEmptyIndex());
	fb_assert(lastNode != getEmptyIndex());
	fb_assert(size == 0 || firstNode != getInvalidIndex());
	fb_assert((firstNode != getInvalidIndex() && lastNode != getInvalidIndex()) ||
		(firstNode == getInvalidIndex() && lastNode == getInvalidIndex()));
	fb_assert(firstNode == getInvalidIndex() || values[firstNode].isValid());
	fb_assert(lastNode == getInvalidIndex() || values[lastNode].isValid());
	fb_assert(size <= 1 || values[firstNode].next != getInvalidIndex());
	fb_assert(size <= 1 || values[lastNode].previous != getInvalidIndex());

	{
		SizeType nodesFound = 0;
		SizeType linksFound = 0;
		for (SizeType i = 0; i < capacity; ++i)
		{
			Node& node = values[i];
			if (node.isValid())
			{
				++nodesFound;
				if (node.next != getInvalidIndex())
				{
					++linksFound;
					fb_assert(values[node.next].isValid());
				}
				if (node.previous != getInvalidIndex())
				{
					++linksFound;
					fb_assert(values[node.previous].isValid());
				}
			}
		}
		fb_assert(nodesFound == size);
		fb_assert(size == 0 || linksFound == size * 2 - 2U);
	}
	if (size > 0)
	{
		SizeType nodesFound = 0;
		IndexType currentIndex = firstNode;
		/* Traverse forward */
		while (currentIndex != getInvalidIndex())
		{
			Node& node = values[currentIndex];
			++nodesFound;
			if (node.next != getInvalidIndex())
			{
				fb_assert(values[node.next].previous == currentIndex);
			}
			currentIndex = node.next;
		}
		fb_assert(nodesFound == size);

		nodesFound = 0;
		currentIndex = lastNode;
		/* Traverse backward */
		while (currentIndex != getInvalidIndex())
		{
			Node& node = values[currentIndex];
			++nodesFound;
			if (node.previous != getInvalidIndex())
			{
				fb_assert(values[node.previous].next == currentIndex);
			}
			currentIndex = node.previous;
		}
		fb_assert(nodesFound == size);
	}

#endif
}


template<class T, typename IndexType, bool IsPod>
AllocFriendlyList<T, IndexType, IsPod>::AllocFriendlyList()
	: values(nullptr)
	, size(0)
	, capacity(0)
	, firstNode(getInvalidIndex())
	, lastNode(getInvalidIndex())
	, firstFreeIndex(0)
{

}


template<class T, typename IndexType, bool IsPod>
AllocFriendlyList<T, IndexType, IsPod>::~AllocFriendlyList()
{
	clear();
	trimMemory();
}


template<class T, typename IndexType, bool IsPod>
AllocFriendlyList<T, IndexType, IsPod>::AllocFriendlyList(const AllocFriendlyList& other)
	: values(nullptr)
	, size(0)
	, capacity(0)
	, firstNode(getInvalidIndex())
	, lastNode(getInvalidIndex())
	, firstFreeIndex(0)
{
	*this = other;
}


template<class T, typename IndexType, bool IsPod>
AllocFriendlyList<T, IndexType, IsPod>& AllocFriendlyList<T, IndexType, IsPod>::operator=(const AllocFriendlyList<T, IndexType, IsPod>& other)
{
	if (&other == this)
		return *this;

	clear();
	if (this->capacity != other.getCapacity())
	{
		trimMemory();
		reserve(other.getCapacity());
	}

	/* Call placement copy constructor for valid items, reset for invalid. */
	for (SizeType i = 0; i < this->capacity; ++i)
	{
		if (IsPod)
		{
			memcpy(this->values, other.values, this->capacity * sizeof(Node));
		}
		else
		{
			if (other.values[i].isValid())
				new (&this->values[i]) Node(other.values[i]);
			else
				this->values[i].reset();
		}
	}
	this->size = other.size;
	fb_expensive_assert(this->capacity == other.capacity);
	this->firstNode = other.firstNode;
	this->lastNode = other.lastNode;
	this->firstFreeIndex = other.firstFreeIndex;

	checkInternalState();
	return *this;
}


template<class T, typename IndexType, bool IsPod>
bool AllocFriendlyList<T, IndexType, IsPod>::isEmpty() const
{
	return size == 0;
}

template<class T, typename IndexType, bool IsPod>
SizeType AllocFriendlyList<T, IndexType, IsPod>::getSize() const
{
	return size;
}


template<class T, typename IndexType, bool IsPod>
SizeType AllocFriendlyList<T, IndexType, IsPod>::getCapacity() const
{
	return capacity;
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::overwriteMemory(void* ptr, SizeType numBytes)
{
	/* Note: This is called even in final release, so leave it empty for that. */
#if FB_BUILD == FB_DEBUG
	typedef uint32_t Filler;
	const Filler filler(0xCAC11CAA);
	SizeType numFillers = numBytes / sizeof(Filler);
	for (SizeType i = 0; i < numFillers; ++i)
		reinterpret_cast<Filler*>(ptr)[i] = filler;

	for (SizeType i = 0; i < numBytes % sizeof(Filler); ++i)
		reinterpret_cast<unsigned char*>(ptr)[numBytes - i - 1] = (unsigned char)0xCA;
#endif
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::reserve(SizeType newCapacity)
{
	fb_assert(newCapacity <= getMaxCapacity());
	if (this->capacity >= newCapacity)
		return;

	if (IsPod)
	{
		values = static_cast<Node*>(lang::reallocateFixed(values, this->capacity * sizeof(Node), newCapacity * sizeof(Node)));
	}
	else
	{
		Node* newValues = static_cast<Node*>(lang::allocateFixed(newCapacity * sizeof(Node)));

		/* Call placement copy constructor and destructor for valid items. */
		for (SizeType i = 0; i < this->capacity; ++i)
		{
			if (values[i].isValid())
			{
				new (&newValues[i]) Node(values[i]);
				(&values[i])->~Node();
			}
			else
			{
				newValues[i].reset();
			}
		}

		lang::freeFixed(values, this->capacity * sizeof(Node));
		values = newValues;
	}
	/* Reset newly allocated nodes (Note: doesn't touch T item, so values aren't really valid). */
	for (SizeType i = this->capacity; i < newCapacity; ++i)
		values[i].reset();

	this->capacity = IndexType(newCapacity);
	checkInternalState();
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::trimMemory()
{
	if (this->size == this->capacity)
		return;

	if (isEmpty())
	{
		lang::freeFixed(values, capacity * sizeof(Node));
		values = nullptr;
	}
	else
	{
		/* Move all items to start of memory block. */
		compact();
		if (IsPod)
		{
			values = static_cast<Node*> (lang::reallocateFixed(values, capacity * sizeof(Node), size * sizeof(Node)));
		}
		else
		{
			Node *newValues = static_cast<Node*>(lang::allocateFixed(size * sizeof(Node)));
			/* Call placement copy constructor and destructor for valid items. */
			for (SizeType i = 0; i < size; ++i)
			{
				if (values[i].isValid())
				{
					new (&newValues[i]) Node(values[i]);
					(&values[i])->~Node();
				}
				else
				{
					newValues[i].reset();
				}
			}

			lang::freeFixed(values, capacity * sizeof(Node));
			values = newValues;
		}
	}

	this->capacity = this->size;
	checkInternalState();
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::resize(SizeType newSize, const T& initValue)
{
	while (getSize() > newSize)
		popBack();

	while (getSize() < newSize)
		pushBack(initValue);
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::compact()
{
	SizeType numProcessed = 0;
	for (SizeType slotIndex = 0, fillerIndex = capacity - 1U; numProcessed < size && slotIndex < fillerIndex; ++slotIndex)
	{
		if (!values[slotIndex].isValid())
		{
			/* Found empty hole */
			for ( ; slotIndex < fillerIndex; --fillerIndex)
			{
				if (values[fillerIndex].isValid())
				{
					Node& node = values[fillerIndex];
					if (IsPod)
					{
						values[slotIndex] = node;
						node.reset();
					}
					else
					{
						new (&values[slotIndex]) Node(node);
						(&node)->~Node();
					}
					Node& newNode = values[slotIndex];
					if (newNode.next != getInvalidIndex())
						values[newNode.next].previous = IndexType(slotIndex);

					if (newNode.previous != getInvalidIndex())
						values[newNode.previous].next = IndexType(slotIndex);

					++numProcessed;
				}
			}
		}
	}
	firstFreeIndex = size;
	checkInternalState();
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::clear()
{
	/* For almost full lists, this is less optimal than just iterating through the memory block 
	 * would be. But oh so much easier. */
	while (!isEmpty())
		popBack();
}


template<class T, typename IndexType, bool IsPod>
T& AllocFriendlyList<T, IndexType, IsPod>::getFront()
{
	fb_expensive_assert(!isEmpty());
	return values[firstNode].item;
}


template<class T, typename IndexType, bool IsPod>
const T& AllocFriendlyList<T, IndexType, IsPod>::getFront() const
{
	fb_expensive_assert(!isEmpty());
	return values[firstNode].item;
}


template<class T, typename IndexType, bool IsPod>
T& AllocFriendlyList<T, IndexType, IsPod>::getBack()
{
	fb_expensive_assert(!isEmpty());
	return values[lastNode].item;
}


template<class T, typename IndexType, bool IsPod>
const T& AllocFriendlyList<T, IndexType, IsPod>::getBack() const
{
	fb_expensive_assert(!isEmpty());
	return values[lastNode].item;
}


template<class T, typename IndexType, bool IsPod>
SizeType AllocFriendlyList<T, IndexType, IsPod>::pushFront(const T& item)
{
	return insert(firstNode, item);
}


template<class T, typename IndexType, bool IsPod>
SizeType AllocFriendlyList<T, IndexType, IsPod>::pushBack(const T& item)
{
	return insert(getInvalidIndex(), item);
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::popFront()
{
	erase(firstNode);
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::popBack()
{
	erase(lastNode);
}


template<class T, typename IndexType, bool IsPod>
T& AllocFriendlyList<T, IndexType, IsPod>::operator[](SizeType i)
{
	return *(getBegin() + i);
}


template<class T, typename IndexType, bool IsPod>
const T& AllocFriendlyList<T, IndexType, IsPod>::operator[](SizeType i) const
{
	return *(getBegin() + i);
}


template<class T, typename IndexType, bool IsPod>
T& AllocFriendlyList<T, IndexType, IsPod>::getWithIndex(SizeType i)
{
	fb_assert(capacity > i);
	fb_assert(values[i].isValid());
	return values[i].item;
}


template<class T, typename IndexType, bool IsPod>
const T& AllocFriendlyList<T, IndexType, IsPod>::getWithIndex(SizeType i) const
{
	fb_assert(capacity > i);
	fb_assert(values[i].isValid());
	return values[i].item;
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::Iterator AllocFriendlyList<T, IndexType, IsPod>::getBegin()
{
	return Iterator(this, firstNode);
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::ConstIterator AllocFriendlyList<T, IndexType, IsPod>::getBegin() const
{
	return ConstIterator(this, firstNode);
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::Iterator AllocFriendlyList<T, IndexType, IsPod>::getEnd()
{
	return Iterator(this, getInvalidIndex());
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::ConstIterator AllocFriendlyList<T, IndexType, IsPod>::getEnd() const
{
	return ConstIterator(this, getInvalidIndex());
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::Iterator AllocFriendlyList<T, IndexType, IsPod>::insert(const ConstIterator& whereIterator, const T& item)
{
	return Iterator(this, insert(whereIterator.getIndex(), item));
}


template<class T, typename IndexType, bool IsPod>
IndexType AllocFriendlyList<T, IndexType, IsPod>::insert(IndexType whereIndex, const T& item)
{
	fb_expensive_assert(whereIndex != getEmptyIndex());
	IndexType newIndex = allocIndex();
	Node& newNode = values[newIndex];
	if (IsPod)
		newNode.item = item;
	else
		new (&newNode.item) T(item);

	if (whereIndex != getInvalidIndex())
	{
		Node& whereNode = values[whereIndex];
		newNode.previous = whereNode.previous;
		if (whereNode.previous != getInvalidIndex())
			values[whereNode.previous].next = newIndex;

		whereNode.previous = newIndex;
	}
	else
	{
		if (lastNode != getInvalidIndex())
			values[lastNode].next = newIndex;

		newNode.previous = lastNode;
		lastNode = newIndex;
	}

	if (newNode.previous == getInvalidIndex())
		firstNode = newIndex;

	newNode.next = whereIndex;
	checkInternalState();
	return newIndex;
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::Iterator AllocFriendlyList<T, IndexType, IsPod>::insert(const ConstIterator& whereIterator, const ConstIterator& begin, const ConstIterator& end)
{
	return Iterator(this, insert(whereIterator.getIndex(), begin, end));
}


template<class T, typename IndexType, bool IsPod>
IndexType AllocFriendlyList<T, IndexType, IsPod>::insert(IndexType whereIndex, const ConstIterator& begin, const ConstIterator& end)
{
	fb_expensive_assert(whereIndex != getEmptyIndex());
	ConstIterator iter = end;
	SizeType indexOfBeginOnList = whereIndex;
	while (iter != begin)
	{
		--iter;
		indexOfBeginOnList = insert(whereIndex, *iter);
	}
	return indexOfBeginOnList;
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::Iterator AllocFriendlyList<T, IndexType, IsPod>::erase(const ConstIterator& whereIterator)
{
	fb_expensive_assert(whereIterator.getIndex() != getInvalidIndex());
	fb_expensive_assert(whereIterator.getIndex() != getEmptyIndex());

	Node& whereNode = values[whereIterator.getIndex()];
	fb_expensive_assert(whereNode.isValid());
	Iterator iter(this, whereNode.next);

	checkInternalState();

	if (whereNode.next != getInvalidIndex())
		values[whereNode.next].previous = whereNode.previous;

	if (whereNode.previous != getInvalidIndex())
		values[whereNode.previous].next = whereNode.next;

	/* Update first and last node indexes. When list size is about to drop to 1 and 0, some special 
	 * handling is required. */

	if (size > 2)
	{
		/* "Normal" case */
		fb_expensive_assert(firstNode != lastNode);
		if (whereIterator.getIndex() == firstNode)
			firstNode = whereNode.next;

		if (whereIterator.getIndex() == lastNode)
			lastNode = whereNode.previous;
	}
	else if (size == 2)
	{
		/* List size is dropping to 1 */
		fb_expensive_assert(firstNode != lastNode);
		if (whereIterator.getIndex() == firstNode)
			firstNode = lastNode;
		else
			lastNode = firstNode;

		fb_expensive_assert(firstNode == lastNode);
	}
	else
	{
		/* List size is dropping to 0 */
		firstNode = lastNode = getInvalidIndex();
		fb_expensive_assert(iter.getIndex() == getInvalidIndex());
	}

	freeIndex(whereIterator.getIndex());
	checkInternalState();
	return iter;
}


template<class T, typename IndexType, bool IsPod>
void AllocFriendlyList<T, IndexType, IsPod>::erase(IndexType whereIndex)
{
	erase(ConstIterator(this, whereIndex));
}


template<class T, typename IndexType, bool IsPod>
typename AllocFriendlyList<T, IndexType, IsPod>::Iterator AllocFriendlyList<T, IndexType, IsPod>::erase(const ConstIterator& begin, const ConstIterator& end)
{
	/* Well... It may not be pretty, but it works */
	ConstIterator iter(begin);
	while (iter != end)
	{
		erase(iter++);
	}
	return iter;
}

FB_END_PACKAGE1()
