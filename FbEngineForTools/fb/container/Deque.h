#pragma once

#include "fb/lang/Types.h"
#include "fb/container/AFList.h"

FB_PACKAGE0()

/**
 * Deque is a container good for few things:
 *   1) Pushing and popping doesn't cause objects to be moved in memory
 *   2) Pushing and popping to and from front or back is equally fast
 *   3) Memory is reserved in blocks, not one item at a time like linked list
 *
 * Deque is also a quick hack needed to replace std::deque and std::list in very few marginal cases in engine. It's 
 * not really that efficient and doesn't do everything std::deque and std::list could (like inserting to or popping 
 * from the middle). You should normally consider using AFList instead, if possible.
 * 
 * Deque currently doesn't support inserting, only pushFront and pushBack. It wouldn't be too hard to add inserting 
 * support though, so just ask Jukka L., if you think you need it.
 */

/* Extreme debugging define. Most could be just removed now that Deque seems to be working. */
#define FB_DEQUE_DEBUGGING FB_FALSE

template<class T, bool IsPod=false>
class Deque
{
	typedef TinyAFList<T, IsPod> StorageUnit;
	typedef PodVector<StorageUnit*> Storage;
	struct ItemPointer
	{
		/* Index of list where item is */
		uint16_t listIndex;
		/* Index of the item in list */
		uint16_t indexInList;
	};

	struct ItemIndex
	{
		ItemIndex() : firstIndex(0), numIndexes(0) { }
		typedef PodVector<ItemPointer> IndexVector;
		IndexVector indexes;
		SizeType firstIndex;
		SizeType numIndexes;

		void makeSpaceIfNecessary(SizeType newSpaceNeeded)
		{
#if FB_DEQUE_DEBUGGING == FB_TRUE
			FB_PRINTF("Deque::ItemIndex::makeSpaceIfNecessary: MAKING SPACE for %d items\n", newSpaceNeeded);
#endif
			SizeType oldSize = indexes.getSize();
			if (numIndexes + newSpaceNeeded > oldSize)
			{
				SizeType oldlastIndex = numIndexes > 0 ? getLastIndex() : 0xFFFFFFFF;
				SizeType newSize = lang::max(numIndexes + newSpaceNeeded, oldSize * 2);
#if FB_DEQUE_DEBUGGING == FB_TRUE
				FB_PRINTF("Deque::ItemIndex::makeSpaceIfNecessary: Actually going from %d to %d items\n", oldSize, newSize);
#endif
				indexes.reserve(newSize);
#if FB_DEQUE_DEBUGGING == FB_TRUE
				if (indexes.getCapacity() != newSize)
				{
					FB_PRINTF("Deque::ItemIndex::makeSpaceIfNecessary: Actually actually went from %d to %d items\n", oldSize, indexes.getCapacity());
				}
#endif
				newSize = indexes.getCapacity();
				indexes.resize(newSize);
				if (numIndexes > 0 && oldlastIndex < firstIndex)
				{
					/* Copy values to correct place */
					SizeType numNewItems = newSize - oldSize;
					memmove(&indexes[firstIndex + numNewItems], &indexes[firstIndex], sizeof(typename IndexVector::ValueType) * (oldSize - firstIndex));
#if FB_DEQUE_DEBUGGING == FB_TRUE
					for (SizeType i = 0; i < numNewItems; ++i)
						indexes[firstIndex + i].listIndex = indexes[firstIndex + i].indexInList = 0xFFFF;
#endif
					/* Update first item pointer */
					firstIndex += numNewItems;
				}
			}
#if FB_DEQUE_DEBUGGING == FB_TRUE
			else
			{
				FB_PRINTF("Deque::ItemIndex::makeSpaceIfNecessary: Actually no need to do anything\n");
			}
#endif
			fb_assert(indexes.getSize() >= newSpaceNeeded + numIndexes);
		}

		const ItemPointer& operator[](SizeType index) const
		{
			fb_expensive_assert(isValidIndex((firstIndex + index) % indexes.getSize()));
			return indexes[(firstIndex + index) % indexes.getSize()];
		}

		ItemPointer& operator[](SizeType index)
		{
			fb_expensive_assert(isValidIndex((firstIndex + index) % indexes.getSize()));
			return indexes[(firstIndex + index) % indexes.getSize()];
		}

		ItemPointer& getBack()
		{
			fb_assert(!indexes.isEmpty());
			fb_assert(numIndexes > 0);
			return indexes[(firstIndex + numIndexes - 1) % indexes.getSize()];
		}

		ItemPointer& getFront()
		{
			fb_assert(!indexes.isEmpty());
			fb_assert(numIndexes > 0);
			return indexes[firstIndex];
		}

		void popBack()
		{
			fb_assert(!indexes.isEmpty());
			fb_assert(numIndexes > 0);
#if FB_DEQUE_DEBUGGING == FB_TRUE
			getBack().listIndex = 0xFFFF;
			getBack().indexInList = 0xFFFF;
#endif
			--numIndexes;
		}

		void popFront()
		{
			fb_assert(!indexes.isEmpty());
			fb_assert(numIndexes > 0);
#if FB_DEQUE_DEBUGGING == FB_TRUE
			getFront().listIndex = 0xFFFF;
			getFront().indexInList = 0xFFFF;
#endif
			--numIndexes;
			firstIndex = (firstIndex + 1) % indexes.getSize();
		}


		ItemPointer& getNewPointerFromBack()
		{
#if FB_DEQUE_DEBUGGING == FB_TRUE
			SizeType firstIndexBefore = firstIndex;
			SizeType numIndexesBefore = numIndexes;
			SizeType lastIndexBefore = numIndexes > 0 ? getLastIndex() : 0xFFFFFFFF;
#endif
			makeSpaceIfNecessary(1);
			++numIndexes;
#if FB_DEQUE_DEBUGGING == FB_TRUE
			FB_PRINTF("Deque::ItemIndex::getNewPointerFromBack: data before / after: firstIndex: %d / %d, numIndexes: %d / %d, lastIndex: %d / %d\n", firstIndexBefore, firstIndex, numIndexesBefore, numIndexes, lastIndexBefore, getLastIndex());
#endif
			return indexes[getLastIndex()];

		}


		ItemPointer& getNewPointerFromFront()
		{
			makeSpaceIfNecessary(1);
			++numIndexes;
			firstIndex = (firstIndex - 1) % indexes.getSize();
			return indexes[firstIndex];
		}

		SizeType getLastIndex() const
		{
			fb_expensive_assert(!indexes.isEmpty());
			return (firstIndex + numIndexes - 1) % indexes.getSize();
		}


		void clear()
		{
#if FB_DEQUE_DEBUGGING == FB_TRUE
			for (SizeType i = 0, num = indexes.getCapacity(); i < num; ++i)
				indexes[i].listIndex = indexes[i].indexInList = 0xFFFF;
#endif
			firstIndex = 0;
			numIndexes = 0;
		}


		SizeType getSize() const
		{
			return numIndexes;
		}

		bool isValidIndex(SizeType index) const
		{
			/* Calculating valid indexes efficiently requires modulo math. I don't expect to get it right the first time so using less efficient ways. */
			if (indexes.isEmpty())
				return false;

			/* Last index (except in zero size case, which is handled above) */
			SizeType lastIndex = getLastIndex();
			if (lastIndex > firstIndex)
			{
				/* Easy to visualize case */
				return index >= firstIndex && index <= lastIndex;
			}
			else
			{
				/* Not so easy to visualize case */
				return (index >= firstIndex && index < indexes.getSize()) || index <= lastIndex;
			}
		}

		SizeType getNextIndexByOffset(SizeType index, SizeType offset)
		{
			fb_expensive_assert(isValidIndex((firstIndex + index) % indexes.getSize()));
			SizeType newIndex = (index + offset) % indexes.getSize();
			return newIndex;
		}

		SizeType getPreviousIndexByOffset(SizeType index, SizeType offset)
		{
			fb_expensive_assert(isValidIndex((firstIndex + index) % indexes.getSize()));
			SizeType newIndex = (index - offset) % indexes.getSize();
			return newIndex;
		}
	};

	ItemIndex itemIndex;
	Storage items;
	uint16_t unitWithFreeSpace;

public:

	typedef T ValueType;

	class Iterator;
	friend class Iterator;
	friend class ConstIterator;

	class ConstIterator
	{
	public:
		ConstIterator(const Deque* deque, SizeType itemPointerIndex) : deque(deque), itemPointerIndex(itemPointerIndex) { }

		ConstIterator(const Iterator& other) : deque(other.deque), itemPointerIndex(other->itemPointerIndex) { }

		inline T operator*();
		inline const T &operator*() const;
		inline T *operator->() const;

		inline ConstIterator &operator++();
		inline ConstIterator operator++(int);

		inline bool operator==(const ConstIterator &other) const;
		inline bool operator!=(const ConstIterator &other) const;

	private:
		friend class Deque;
		const Deque* deque;
		SizeType itemPointerIndex;
	};

	class Iterator
	{
	public:
		Iterator(Deque* deque, SizeType itemPointerIndex) : deque(deque), itemPointerIndex(itemPointerIndex) { }

		inline T operator*();
		inline const T &operator*() const;
		inline T *operator->() const;

		inline Iterator &operator++();
		inline Iterator operator++(int);

		inline bool operator==(const Iterator &other) const;
		inline bool operator!=(const Iterator &other) const;

	private:
		friend class Deque;
		Deque* deque;
		SizeType itemPointerIndex;
	};

	Deque();
	Deque(const Deque<T, IsPod>& other);
	~Deque();

	// "optimal push" (this not guaranteed to push to any specific order)
	/* What? Get rid of this. */
	inline SizeType push(const T &t) { pushBack(t); return getSize() - 1; }

	/* O[1] performance */
	inline void pushBack(const T &t);
	/* O[1] performance */
	inline void pushFront(const T &t);
	/* O[1] performance */
	inline void popBack();
	/* O[1] performance */
	inline void popFront();

	// Note, use of popFrontWithValue() is recommended instead of the getFront(); popFront() pattern where
	// the contained data type size and copying is not an issue. (such as a pointer)
	inline T popFrontWithValue();		
	inline T popBackWithValue();

	/* Returns number of items in Deque. */
	inline SizeType getSize() const;
	/* Returns reserved capacity. Note that there may be holes, so this doesn't really tell anything useful except 
		* in controlled conditions. Note that this is something like O[log N], not O[1] as usual. */
	inline SizeType getCapacity() const;
	/* Returns max capacity of deque (or close enough). */
	static SizeType getMaxCapacity() { return Storage::getMaxCapacity(); }

	/* O[1] performance */
	inline const T &operator[](SizeType index) const;
	inline T &operator[](SizeType index);

	/* O[1] performance */
	inline T &getFront();
	inline const T &getFront() const;

	/* O[1] performance */
	inline T &getBack();
	inline const T &getBack() const;

	inline Iterator getBegin();
	inline ConstIterator getBegin() const;

	inline Iterator getEnd();
	inline ConstIterator getEnd() const;

	/* Talk to Jukka L., if you need these. */
	//inline void insert(Iterator &wher, Iterator &begin, Iterator &end);
	//inline void erase(Iterator &it);

	/* Clears the Deque, but doesn't release (all) memory */
	inline void clear();
	/* Reserve memory. Works best when called for empty Deque. */
	void reserve(SizeType size);

	inline bool isEmpty() const;


private:
	/* Don't reserve miniscule sizes */
	SizeType getMinimumReserveSize() { return 4; }
	/* Return StorageUnit that has space at least for one item */
	uint16_t getUnitToInsertTo();
	/* Check validity of deque. Not kind to performance. */
	void validate() const;
};

FB_END_PACKAGE0()

#define FB_CONTAINER_DEQUEINLINE_INCLUDE_OK
#include "DequeInline.h"
#undef FB_CONTAINER_DEQUEINLINE_INCLUDE_OK
