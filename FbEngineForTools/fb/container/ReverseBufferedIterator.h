#pragma once

#include "fb/container/Stack.h"
#include "fb/container/PodStack.h"
#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE0()


// TODO: partially specialize for specific cases - such as the BufferedIterator for vector 
// (as that would not require this second buffering - just one buffer that is given out in reverse order!)

/**
 * An iterator reversing iterator (will create a temporary buffer of all the elements in the iterator for the operation). 
 * This is like BufferedIterator, but it gives out the results in the opposite (reversed) order.
 *
 * @see BufferedIterator.
 */
template<class IteratorT, class ElementT> 
	class ReverseBufferedIterator
{
public:
	ReverseBufferedIterator(IteratorT &c, ElementT end) : endValue(end) 
	{
		ElementT tmp;
		while ((tmp = c.next()) != end)
		{
			fb_expensive_assert(tmp != end);
			bufferStack.pushBack(tmp);
		}
	}

	ElementT next()
	{
		if (bufferStack.isEmpty())
			return endValue;

		return bufferStack.popBackWithValue();
	}

public:
	PodStack<ElementT> bufferStack;
	ElementT endValue;
};

FB_END_PACKAGE0()
