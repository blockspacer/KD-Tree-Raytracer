#pragma once

#include "fb/container/VectorIteratorWrapper.h"
#include "fb/container/Vector.h"
#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE0()

/**
 * This can be used to further wrap any other object that provides iteration for entries with the next() method.
 *
 * This wrapper will create a temporary buffer of all the data from the iterating object, and then provides
 * those for iteration. This means that you "safely" iterate the values of a container, even if the container
 * would change during the iteration (or by another thread). It does NOT mean, however, that any objects
 * pointed by the iterated entries are valid if they get deleted / changed in the process!
 *
 * One trivial use case for this is to provide a safe (ref-counted) list for Lua iteration, for example,
 * the following returns a buffered smart-pointed iterator for the myVector elements.
 */
template<class IteratorT, class ElementT> 
	class BufferedIterator 
{
public:
	BufferedIterator(IteratorT &c, ElementT end) : currentItemIndex(0)
	{
		ElementT tmp;
		while ((tmp = c.next()) != end)
		{
			fb_expensive_assert(tmp != end);
			bufferQueue.pushBack(tmp);
		}
		bufferQueue.pushBack(end);
	}

	ElementT next()
	{
		if (currentItemIndex < bufferQueue.getSize())
		{
			ElementT item = bufferQueue[currentItemIndex];
			++currentItemIndex;
			return item;
		}
		return bufferQueue.getBack();
	}

private: 
	// safety checks... if you are assigning/copy constructing from another buffered iterator, you've probably screwed up.
	// this is because the buffered iterator will have to copy the entire buffer from one iterator instance to another.
	// (if you must pass the buffered iterator from one location to another, wrap it in shared_ptr)
	BufferedIterator(const BufferedIterator &other)
	{
		fb_expensive_assert(0 && "Copying a buffered iterator.");
		this->bufferQueue = other.bufferQueue;
	}
	BufferedIterator &operator= (const BufferedIterator &other)
	{
		fb_expensive_assert(0 && "Copying a buffered iterator.");
		this->bufferQueue = other.bufferQueue;
	}

public:
	PodVector<ElementT> bufferQueue;
	SizeType currentItemIndex;
};


// Partial specialization (optimization) of BufferedIterator for non-const vector iterator wrapper
template<class VectorT, class ElementT> 
class BufferedIterator<VectorIteratorWrapper<VectorT, ElementT>, ElementT> 
{
public:
	BufferedIterator(VectorIteratorWrapper<VectorT, ElementT> &c, ElementT end) : currentItemIndex(0)
	{
		bufferQueue.reserve(c.vector.getSize() + 1);
		if (!c.vector.isEmpty())
		{
			fb_expensive_assert(c.vector[c.vector.getSize() - 1] != end);
			bufferQueue.insert(bufferQueue.getEnd(), &c.vector.getFront(), (&c.vector.getBack()) + 1);
		}
		bufferQueue.pushBack(end);
		fb_assert(bufferQueue.getSize() == c.vector.getSize() + 1);
	}

	ElementT next()
	{
		if (currentItemIndex < bufferQueue.getSize())
		{
			ElementT item = bufferQueue[currentItemIndex];
			++currentItemIndex;
			return item;
		}
		return bufferQueue.getBack();
	}

private: 
	// safety checks... if you are assigning/copy constructing from another buffered iterator, you've probably screwed up.
	// this is because the buffered iterator will have to copy the entire buffer from one iterator instance to another.
	// (if you must pass the buffered iterator from one location to another, wrap it in shared_ptr)
	BufferedIterator(const BufferedIterator &other)
	{
		fb_expensive_assert(0 && "Copying a buffered iterator.");
		this->bufferQueue = other.bufferQueue;
	}
	BufferedIterator &operator= (const BufferedIterator &other)
	{
		fb_expensive_assert(0 && "Copying a buffered iterator.");
		this->bufferQueue = other.bufferQueue;
	}

public:
	PodVector<ElementT> bufferQueue;
	SizeType currentItemIndex;
};


// Partial specialization (optimization) of BufferedIterator for const vector iterator wrapper
template<class VectorT, class ElementT > 
class BufferedIterator<ConstVectorIteratorWrapper<VectorT, ElementT>, ElementT> 
{
public:
	BufferedIterator(ConstVectorIteratorWrapper<VectorT, ElementT> &c, ElementT end) : currentItemIndex(0)
	{
		bufferQueue.reserve(c.vector.getSize() + 1);
		if (!c.vector.isEmpty())
		{
			fb_expensive_assert(c.vector[c.vector.getSize() - 1] != end);
			bufferQueue.insert(bufferQueue.getEnd(), &c.vector.getFront(), (&c.vector.getBack()) + 1);
		}
		bufferQueue.pushBack(end);
		fb_assert(bufferQueue.getSize() == c.vector.getSize() + 1);
	}

	ElementT next()
	{
		if (currentItemIndex < bufferQueue.getSize())
		{
			ElementT item = bufferQueue[currentItemIndex];
			++currentItemIndex;
			return item;
		}
		return bufferQueue.getBack();
	}

private: 
	// safety checks... if you are assigning/copy constructing from another buffered iterator, you've probably screwed up.
	// this is because the buffered iterator will have to copy the entire buffer from one iterator instance to another.
	// (if you must pass the buffered iterator from one location to another, wrap it in shared_ptr)
	BufferedIterator(const BufferedIterator &other)
	{
		fb_expensive_assert(0 && "Copying a buffered iterator.");
		this->bufferQueue = other.bufferQueue;
	}
	BufferedIterator &operator= (const BufferedIterator &other)
	{
		fb_expensive_assert(0 && "Copying a buffered iterator.");
		this->bufferQueue = other.bufferQueue;
	}

public:
	Vector<ElementT> bufferQueue;
	SizeType currentItemIndex;
};

FB_END_PACKAGE0()
