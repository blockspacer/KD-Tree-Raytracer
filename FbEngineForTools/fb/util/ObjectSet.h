#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE1(util)

	/**
	* A container similar to std::set<T*> where every unique object exists only once.
	*
	* Inserting into ObjectSet is a fast operation because the "duplicate check"
	* is implemented by raising/lowering a flag in the object itself.
	*
	* Example usage:
	*
	* class MyObject
	* {
	* public:
	*   MyObject() : flags(0) {}
	*   int flags;
	*   enum Flags { FlagAddedToTestSet = (1<<0) }
	* };
	* ObjectSet<MyObject, int, FlagAddedToTestSet> testSet(&MyObject::flags);
	* MyObject myObject;
	* testSet.insert(&myObject);
	* // now testSet[0] == &myObject
	*
	*/
	template<class T, class BitMaskClass, int flagMask>
	class ObjectSet
	{
	public:
		typedef fb::SizeType SizeType;
		typedef BitMaskClass T::*FlagsPointer;
		ObjectSet(FlagsPointer flagsPointer)
			: flagsPointer(flagsPointer)
		{
		}

		~ObjectSet()
		{
			clearWithoutReset();
		}

		/**
		 * Adds object to set.
		 */
		void insert(T *object)
		{
			if (!( ((*object).*flagsPointer) & flagMask))
			{
				((*object).*flagsPointer) |= flagMask;
				objects.pushBack(object);
			}
		}

		bool isEmpty() const
		{
			return objects.isEmpty();
		}

		SizeType getSize() const
		{
			return objects.getSize();
		}

		T *operator[](SizeType index) const
		{
			fb_expensive_assert(index < objects.getSize());
			return objects[index];
		}

		void clear()
		{
			typename ObjectList::Iterator it = objects.getBegin();
			typename ObjectList::Iterator itEnd = objects.getEnd();
			for (;it != itEnd;it++)
			{
				((*(*it)).*flagsPointer) &= ~flagMask;
			}
			objects.clear();
		}

		/**
		 * Clears set without resetting flags (useful when object pointers are no longer valid)
		 */
		void clearWithoutReset()
		{
			objects.clear();
		}

		/**
		 * Removes object from set
		 */
		bool remove(T *object)
		{
			if (!( ((*object).*flagsPointer) & flagMask))
				return false;

			SizeType numObjects = objects.getSize();
			for (SizeType i = 0; i < numObjects; i++)
			{
				if (objects[i] == object)
				{
					((*object).*flagsPointer) &= ~flagMask;
					objects[i] = objects.getBack();
					objects.popBack();
					return true;
				}
			}

			return false;
		}

		/**
		 * Removes object from set by swapping it with the last one
		 */
		void removeWithSwap(SizeType i)
		{
			T *object = objects[i];
			((*object).*flagsPointer) &= ~flagMask;
			objects[i] = objects.getBack();
			objects.popBack();
		}

		bool find(T *object)
		{
			SizeType numObjects = objects.getSize();
			for (SizeType i = 0; i < numObjects; ++i)
			{
				if (objects[i] == object)
					return true;
			}
			return false;
		}

	private:
		typedef PodVector<T *> ObjectList;
		ObjectList objects;
		FlagsPointer flagsPointer;
		// forward declaration not allowed (VS2012 will generate classes with different sizes!)
		char dummy[sizeof(T) / sizeof(T)];
	};

FB_END_PACKAGE1()
