#pragma once

#include "AFList.h"
#include "fb/lang/Types.h"

FB_PACKAGE0()

template<class T>
class Queue
{
public:

	typedef T ValueType;

	/* Note: with current implementation, adding pushFront() or popBack() would be efficient */
	void pushBack(const T &t) { queueImp.pushBack(t); }
	void popFront() { queueImp.popFront(); }

	// Note, use of popFrontWithValue() is recommended instead of the getFront(); popFront() pattern where
	// the contained data type size and copying is not an issue. (such as a pointer)
	T popFrontWithValue()
	{
		T ret = queueImp.getFront();
		queueImp.popFront();
		return ret;
	}

	SizeType getSize() const { return queueImp.getSize(); }
	SizeType getCapacity() const { return queueImp.getCapacity(); }
	void reserve(SizeType newCapacity) { queueImp.reserve(); }

	/* Same as getSize(). There was some philosphical discussion about why name getSize should not be used but it was 
	 * included anyway. I'd rather threw out this harder to guess option */
	//inline SizeType getElementCount() const;
	static SizeType getMaxCapacity() { return AFList<T>::getMaxCapacity(); }

	/* Due to the implementation of Queue (based on list) accessing items by index is slow. If you need to do that, 
	 * make a better implementation */
	 //inline const T &operator[](int index) const;
	 /* Maybe it is not OK to get a non-const element by index... */
	//inline T &operator[](int index);


	/* As we don't have pushFront() and above talk about it not being OK to get non-const element by index, is it any 
	 * better getting non-const first either? */
	T &getFront() { return queueImp.getFront(); }
	inline const T &getFront() const { return queueImp.getFront(); }

	inline T &getBack() { return queueImp.getBack(); }
	inline const T &getBack() const { return queueImp.getBack(); };

	inline void clear() { queueImp.clear(); }

	inline bool isEmpty() const { return queueImp.isEmpty(); }

private:
	AFList<T> queueImp;
};

FB_END_PACKAGE0()
