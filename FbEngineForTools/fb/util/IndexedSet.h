#pragma once

#include "fb/container/PodVector.h"

FB_PACKAGE1(util)

// So you want to store a big set of object pointers. You can do better than LinearHashSet<Object *> if each object is inserted to only one set at a time.
//
// This is your container:
//
//		PodVector<Object *> objects;
//
// This is your object class:
//
//		struct Object
//		{
//			uint32_t index = ~0u;
//			...
//		};
//
// Use these functions:
//
//		pushToIndexedSet(objects, obj, &Object::index);
//
//		swapOutFromIndexedSet(objects, obj, &Object::index);
//
//		isInIndexedSet(objects, obj, &Object::index);
//
//	Iterate the vector as you normally would.

template<class Vector, class T, class IndexType>
static void pushToIndexedSet(Vector &list, T &object, IndexType(T::*indexPointer))
{
	fb_assert(object.*indexPointer == (IndexType)~0u);
	object.*indexPointer = list.getSize();
	list.pushBack(&object);
}

template<class Entry, class T, class IndexType>
static Entry &pushToIndexedSet(PodVector<Entry> &list, T &object, IndexType(T::*indexPointer), T*(Entry::*objectPointer))
{
	fb_assert(object.*indexPointer == (IndexType)~0u);
	object.*indexPointer = list.getSize();
	Entry &entry = list.pushBack();
	entry.*objectPointer = &object;
	return entry;
}


template<class Vector, class T, class IndexType>
static void swapOutFromIndexedSet(Vector &list, T &object, IndexType(T::*indexPointer))
{
	IndexType index = object.*indexPointer;
	fb_assert(index < list.getSize());
	fb_assert(list[index] == &object);
	lang::swap(list[index], list.getBack());
	list[index]->*indexPointer = index;
	object.*indexPointer = (IndexType)~0u;
	list.popBack();
}

template<class Entry, class T, class IndexType>
static void swapOutFromIndexedSet(PodVector<Entry> &list, T &object, IndexType(T::*indexPointer), T*(Entry::*objectPointer))
{
	IndexType index = object.*indexPointer;
	fb_assert(index < list.getSize());
	Entry &entry = list[index];
	fb_assert(entry.*objectPointer == &object);
	lang::swap(list[index], list.getBack());
	Entry &backEntry = list[index];
	T *backObject = backEntry.*objectPointer;
	backObject->*indexPointer = index;
	object.*indexPointer = (IndexType)~0u;
	list.popBack();
}

template<class Vector, class T, class IndexType>
static bool isInIndexedSet(Vector &list, T &object, IndexType(T::*indexPointer))
{
	IndexType index = object.*indexPointer;
	return index < list.getSize() && list[index] == &object;
}

FB_END_PACKAGE1()
