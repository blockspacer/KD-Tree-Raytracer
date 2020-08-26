#ifndef FB_LOWLEVEL_CONTAINER_ALLOCFRIENDLYLISTITERATORINLINE_INCLUDE_OK
#error "AllocFriendlyListIteratorInline.h is supposed to be included by the AllocFriendlylist->h only."
#endif

/* This should be included directly inside AllocFriendlyList class definition */

public:

class ConstIterator;

class Iterator
{
public:
	friend class ConstIterator;
	Iterator()
		: list(nullptr), index(getInvalidIndex())
	{
		/* Allow constructing empty iterator */
	}

	Iterator(AllocFriendlyList<T, IndexType, IsPod>* list, IndexType index)
		: list(list), index(index)
	{
		validate();
	}

	Iterator(const typename AllocFriendlyList<T, IndexType, IsPod>::Iterator& other)
		: list(other.list), index(other.index)
	{
		validate();
	}

	Iterator& operator=(const typename AllocFriendlyList<T, IndexType, IsPod>::Iterator& other)
	{
		this->list = other.list;
		this->index = other.index;
		validate(other);
		return *this;
	}

	IndexType getIndex() const
	{
		validate();
		return index;
	}

	T& operator*()
	{
		validateForAccess();
		return list->values[index].item;
	}

	const T &operator*() const
	{
		validateForAccess();
		return list->values[index].item;
	}

	T *operator->()
	{
		validateForAccess();
		return &list->values[index].item;
	}

	const T *operator->() const
	{
		validateForAccess();
		return &list->values[index].item;
	}

	Iterator &operator++()
	{
		validate();
		if (index != getInvalidIndex())
			this->index = list->values[index].next;
		else
			this->index = list->firstNode;

		validate();
		return *this;
	}

	Iterator operator++(int)
	{
		validate();
		Iterator iter(list, index);
		++*this;
		validate();
		return iter;
	}

	Iterator &operator--()
	{
		validate();
		if (index != getInvalidIndex())
			this->index = list->values[index].previous;
		else
			this->index = list->lastNode;

		validate();
		return *this;
	}

	Iterator operator--(int)
	{
		validate();
		Iterator iter(list, index);
		--*this;
		validate();
		return iter;
	}

	Iterator& operator+=(SizeType value)
	{
		validate();
		while (value != 0)
		{
			++*this;
			--value;
		}
		validate();
		return *this;
	}

	Iterator operator+(SizeType value)
	{
		validate();
		Iterator iter(list, index);
		while (value != 0)
		{
			++iter;
			--value;
		}
		validate();
		return iter;
	}

	Iterator& operator-=(SizeType value)
	{
		validate();
		while (value != 0)
		{
			--*this;
			--value;
		}
		validate();
		return *this;
	}

	Iterator operator-(SizeType value)
	{
		validate();
		Iterator iter(list, index);
		while (value != 0)
		{
			--iter;
			--value;
		}
		validate();
		return iter;
	}

	bool operator==(const Iterator& other) const
	{
		validate(other);
		return this->index == other.index;
	}

	bool operator!=(const Iterator& other) const
	{
		validate(other);
		return this->index != other.index;
	}

	void validate(const Iterator& other) const
	{
		fb_expensive_assert(this->list == other.list);
		validate();
		other.validate();
	}

	void validate() const
	{
		if (this->index != getInvalidIndex())
		{
			validateForAccess();
		}
		else
		{
			fb_expensive_assert(this->index != getEmptyIndex());
		}

	}

	void validateForAccess() const
	{
		fb_expensive_assert(this->index != getEmptyIndex());
		fb_expensive_assert(list->values[index].isValid());
	}


private:
	AllocFriendlyList<T, IndexType, IsPod>* list;
	IndexType index;
};

class ConstIterator
{
public:
	ConstIterator()
		: list(nullptr), index(getInvalidIndex())
	{
	}

	ConstIterator(const AllocFriendlyList<T, IndexType, IsPod>* list, IndexType index)
		: list(list), index(index)
	{
		validate();
	}

	ConstIterator(const typename AllocFriendlyList<T, IndexType, IsPod>::Iterator& iter)
		: list(iter.list), index(iter.index)
	{
		validate();
	}

	ConstIterator& operator=(const typename AllocFriendlyList<T, IndexType, IsPod>::Iterator& other)
	{
		this->list = other.list;
		this->index = other.index;
		validate(other);
		return *this;
	}

	IndexType getIndex() const
	{
		validate();
		return index;
	}

	T& operator*()
	{
		validateForAccess();
		return list->values[index].item;
	}

	const T &operator*() const
	{
		validateForAccess();
		return list->values[index].item;
	}

	T *operator->()
	{
		validateForAccess();
		return &list->values[index].item;
	}

	const T *operator->() const
	{
		validateForAccess();
		return &list->values[index].item;
	}

	ConstIterator &operator++()
	{
		validate();
		if (index != getInvalidIndex())
			this->index = list->values[index].next;
		else
			this->index = list->firstNode;

		validate();
		return *this;
	}

	ConstIterator operator++(int)
	{
		validate();
		ConstIterator iter(list, index);
		++*this;
		validate();
		return iter;
	}

	ConstIterator &operator--()
	{
		validate();
		if (index != getInvalidIndex())
			this->index = list->values[index].previous;
		else
			this->index = list->lastNode;

		validate();
		return *this;
	}

	ConstIterator operator--(int)
	{
		validate();
		ConstIterator iter(list, index);
		--*this;
		validate();
		return iter;
	}

	ConstIterator& operator+=(SizeType value)
	{
		validate();
		while (value != 0)
		{
			++*this;
			--value;
		}
		validate();
		return *this;
	}

	ConstIterator operator+(SizeType value)
	{
		validate();
		ConstIterator iter(list, index);
		while (value != 0)
		{
			++iter;
			--value;
		}
		validate();
		return iter;
	}

	ConstIterator& operator-=(SizeType value)
	{
		validate();
		while (value != 0)
		{
			--*this;
			--value;
		}
		validate();
		return *this;
	}

	ConstIterator operator-(SizeType value)
	{
		validate();
		ConstIterator iter(list, index);
		while (value != 0)
		{
			--iter;
			--value;
		}
		validate();
		return iter;
	}

	bool operator==(const ConstIterator& other) const
	{
		validate(other);
		return this->index == other.index;
	}

	bool operator!=(const ConstIterator& other) const
	{
		validate(other);
		return this->index != other.index;
	}

	void validate(const ConstIterator& other) const
	{
		fb_expensive_assert(this->list == other.list);
		validate();
		other.validate();
	}

	void validate() const
	{
		if (this->index != getInvalidIndex())
		{
			validateForAccess();
		}
		else
		{
			fb_expensive_assert(this->index != getEmptyIndex());
		}

	}

	void validateForAccess() const
	{
		fb_expensive_assert(this->index != getEmptyIndex());
		fb_expensive_assert(list->values[index].isValid());
	}

private:
	const AllocFriendlyList<T, IndexType, IsPod>* list;
	IndexType index;
};

friend class Iterator;
friend class ConstIterator;
