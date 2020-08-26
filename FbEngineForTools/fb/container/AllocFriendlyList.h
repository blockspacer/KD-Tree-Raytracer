#pragma once

#include "fb/lang/FBAssert.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/Types.h"

FB_PACKAGE1(container)

/**
 * AllocFriendlyList is double-linked list that is backed by continuous block of memory. It only 
 * frees memory when requested. 
 * 
 * List is double-linked and can be iterated forward or backwards. ++:ing iterator returned by 
 * getEnd() returns getBegin, while --getBegin() returns getEnd(), making it easy to write either 
 * forward to back or back to forward iterator. Note that (as usual) getEnd() points outside the 
 * list.
 * 
 * NOTE: AllocFriendlyList doesn't preserve items' location in memory, when list's capacity changes 
 * (for example, due to new items being inserted). Pointers to items should only be held 
 * temporarily. It does, however, preserve indexes obtained from iterators, or returned by 
 * pushFront() or pushBack(), unless compact() or trimMemory() is called. Indexes are preserved 
 * when list is copied or copy constructed.
 *
 * NOTE on implementation: IndexType is used to specify size of the list and affects how much space 
 * the list takes. In practice, it is either uint16_t or uint32_t. SizeType is still used for 
 * calculations to avoid performance penalties when IndexType is 16 bit.
 **/

 /* Enable this to get super poor performance and lots of checks. */
#define FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLIST_DEBUG_ENABLED FB_FALSE


template<class T, typename IndexType, bool IsPod = false>
class AllocFriendlyList
{
#define FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLISTITERATORINLINE_INCLUDE_OK
#include "AllocFriendlyListIteratorInline.h"
#undef FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLISTITERATORINLINE_INCLUDE_OK

public:
	typedef T ValueType;

	AllocFriendlyList();
	~AllocFriendlyList();
	AllocFriendlyList(const AllocFriendlyList& other);
	AllocFriendlyList& operator=(const AllocFriendlyList& other);

	bool isEmpty() const;
	/* Returns number of items in list. */
	SizeType getSize() const;
	/* Returns capacity of list */
	SizeType getCapacity() const;
	/* Reserves requested capacity. Never decreases reservation. */
	void reserve(SizeType capacity);
	/* Decreases capacity to current size of list (drops all memory if size == 0) */
	void trimMemory();
	/* Changes list size to given capacity. If new size is smaller than current size, popBacks 
	 * items until that is no longer the case. */
	void resize(SizeType size, const T& initValue = T());
	/* Moves all items in backing storage to start of storage. */
	void compact();
	/* Empties the list, but doesn't adjust reserved capacity. */
	void clear();

	T& getFront();
	const T& getFront() const;
	T& getBack();
	const T& getBack() const;

	SizeType pushFront(const T& item);
	SizeType pushBack(const T& item);
	void popFront();
	void popBack();

	/* Returns ith item (corresponds to *(getBegin() + i). O(n) performance. */
	T& operator[](SizeType i);
	const T& operator[](SizeType i) const;
	/* Gets item with index returned by pushBack, pushFront or iterator. O(1) performance. 
	 * Note limits to index validity. */
	T& getWithIndex(SizeType i);
	const T& getWithIndex(SizeType i) const;

	Iterator getBegin();
	ConstIterator getBegin() const;
	Iterator getEnd();
	ConstIterator getEnd() const;

	/* Inserts item before given iterator. Returns Iterator that points to newly inserted item. */
	Iterator insert(const ConstIterator& where, const T& item);
	/* Inserts item before given index. Returns index of newly inserted item. */
	IndexType insert(IndexType where, const T& item);
	/* Items inserted include begin but not end. Returns iterator that points to first of newly 
	 * inserted items. */
	Iterator insert(const ConstIterator& where, const ConstIterator& begin, const ConstIterator& end);
	/* Items inserted include begin but not end. Returns index of first of newly inserted items. */
	IndexType insert(IndexType where, const ConstIterator& begin, const ConstIterator& end);

	/* Returns iterator that points to an element after deleted element (getEnd(), if last element on list was deleted). */
	Iterator erase(const ConstIterator& where);
	void erase(IndexType where);
	/* Items erased include begin but not end. Returns iterator that points to an element after last 
	 * erased element (getEnd(), if last element on list was erased). */
	Iterator erase(const ConstIterator& begin, const ConstIterator& end);

	static SizeType getMaxCapacity() { return IndexType(-1) - 1; }

private:
	/* Additional debug helper */
	static void overwriteMemory(void* ptr, SizeType numBytes);

	static IndexType getInvalidIndex() { return IndexType(getMaxCapacity()); }
	static IndexType getEmptyIndex() { return IndexType(getMaxCapacity() + 1); }
	static IndexType getMaxIndex() { return IndexType(getMaxCapacity() - 1); }

	IndexType allocIndex();
	void freeIndex(IndexType index);

	struct Node
	{
		Node() { }
		Node(const T& item, IndexType next, IndexType previous) : item(item), next(next), previous(previous) { }
		~Node() { reset(); }

		void validate() const
		{
			fb_expensive_assert((next != getEmptyIndex() && previous != getEmptyIndex()) || (next == getEmptyIndex() && previous == getEmptyIndex()));
		}
		bool isValid() const { validate(); return next != getEmptyIndex(); }
		void reset() { next = previous = getEmptyIndex(); }
		
		T item;
		IndexType next;
		IndexType previous;
	};

	void checkInternalState();

	Node *values;
	/* Size isn't really needed, but it's handy and someone probably wants to track it somewhere anyway */
	IndexType size;
	IndexType capacity;
	IndexType firstNode;
	IndexType lastNode;
	IndexType firstFreeIndex;
};

#define FB_CONTAINERIMP_TEMPLATE_PARAMS typename ValueType, typename IndexType, bool IsPod
#define FB_CONTAINERIMP_CONTAINER_TYPE AllocFriendlyList<ValueType, IndexType, IsPod>
#include "ContainerRangeFor.h"

FB_END_PACKAGE1()

#define FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLISTINLINE_INCLUDE_OK
#include "AllocFriendlyListInline.h"
#undef FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLISTINLINE_INCLUDE_OK
