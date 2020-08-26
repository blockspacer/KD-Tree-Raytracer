#pragma once

#include "fb/container/PodVector.h"
#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE1(util)

	/**
	* A container similar to std::set<T*> where every unique object exists only once.
	*
	* Inserting and removing is a fast operation because the object stores its own
	* index in the set.
	*
	* Example usage:
	*
	* class MyObject
	* {
	* public:
	*   MyObject() : indexInTestSet(ObjectIndexSet<MyObject>::invalidIndex) {}
	*   int indexInTestSet;
	* };
	* ObjectIndexSet<MyObject> testSet(&MyObject::indexInTestSet);
	* MyObject myObject;
	* testSet.insert(&myObject);
	* // now testSet[0] == &myObject
	*
	*/
	template<class T, bool UseMultithreadedAllocator = false>
	class ObjectIndexSet
	{
	public:
		typedef fb::SizeType SizeType;
		typedef int IndexType;
		static const IndexType invalidIndex = -1;
		typedef IndexType T::*IndexPointer;
		ObjectIndexSet(IndexPointer indexPointer)
			: indexPointer(indexPointer)
		{
		}

		~ObjectIndexSet()
		{
			clearWithoutReset();
		}

		/**
		 * Adds object to set.
		 */
		void insert(T *object)
		{
			// already exists?
			fb_expensive_assert(!isObjectInSet(object) && "Refactored regression assert triggered.");
			insertIfNecessary(object);
		}

		void insertIfNecessary(T *object)
		{
			if (((*object).*indexPointer) == invalidIndex)
			{
				((*object).*indexPointer) = (IndexType)objects.getSize();
				objects.pushBack(object);
			}
		}

		bool isObjectInSet(const T *object) const
		{
			IndexType index = ((*object).*indexPointer);
			if (index != invalidIndex)
			{
				fb_expensive_assert(index >= 0 && SizeType(index) < objects.getSize());
				fb_expensive_assert(objects[SizeType(index)] == object);
				return true;
			}
			else
			{
				return false;
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

		T *operator[](IndexType index) const
		{
			fb_expensive_assert(index >= 0 && SizeType(index) < objects.getSize());
			fb_expensive_assert(((*objects[SizeType(index)]).*indexPointer) == index);
			return objects[SizeType(index)];
		}

		/* FIXME: Loose IndexType, just use SizeType */
		/* Gah, it's either this or adding IndexType to Vectors */
		T *operator[](SizeType index) const
		{
			return (*this)[IndexType(index)];
		}

		void clear()
		{
			if (!objects.isEmpty())
			{
				T **objs = objects.getPointer();
				SizeType numObjects = objects.getSize();
				for (SizeType i = 0; i < numObjects; i++)
				{
					T *object = objs[i];
					fb_expensive_assert(((*object).*indexPointer) == int(i));
					((*object).*indexPointer) = invalidIndex;
				}
				objects.clear();
			}
		}

		/**
		 * Clears set without resetting indices (useful when object pointers are no longer valid)
		 */
		void clearWithoutReset()
		{
			objects.clear();
		}

		/**
		 * Removes object from set
		 */
		void remove(T *object)
		{
			fb_expensive_assert(isObjectInSet(object) && "Refactored regression assert triggered.");
			removeIfNecessary(object);
		}

		bool removeIfNecessary(T *object)
		{
			IndexType index = ((*object).*indexPointer);
			if (index == invalidIndex)
				return false;

			fb_assert(index >= 0 && SizeType(index) < objects.getSize());
#if FB_BUILD != FB_FINAL_RELEASE
			if (SizeType(index) >= objects.getSize()) return false;
#endif
			fb_expensive_assert(objects[SizeType(index)] == object);

			// switch with last
			T *lastObject = objects.getBack();
			if (lastObject != object)
			{
				fb_expensive_assert(((*lastObject).*indexPointer) >= 0 && SizeType((*lastObject).*indexPointer) < objects.getSize());
				fb_expensive_assert(objects[ SizeType((*lastObject).*indexPointer) ] == lastObject);
				((*lastObject).*indexPointer) = index;
				objects[SizeType(index)] = lastObject;
			}
			((*object).*indexPointer) = invalidIndex;
			objects.popBack();
			return true;
		}

		/**
		 * Returns last object in set
		 */
		T *getBack()
		{
			return objects.getBack();
		}

		/**
		 * Pops object from set, must not be empty!
		 */
		void popBack()
		{
			T *lastObject = objects.getBack();
			fb_expensive_assert((SizeType)((*lastObject).*indexPointer) < objects.getSize());
			fb_expensive_assert(objects[ ((*lastObject).*indexPointer) ] == lastObject);
			((*lastObject).*indexPointer) = invalidIndex;
			objects.popBack();
		}

		/**
		 * Makes a fast copy of objects into array
		 */
		void copyToArray(T **arrray)
		{
			fb_expensive_assert(validate());
			lang::MemCopy::copy(arrray, objects.getPointer(), sizeof(T*)*objects.getSize());
		}

		bool validate()
		{
			T **objs = objects.getPointer();
			SizeType numObjects = objects.getSize();
			for (SizeType i = 0; i < numObjects; i++)
			{
				T *object = objs[i];
				if (((*object).*indexPointer) != int(i))
					return false;
			}
			return true;
		}

		typedef PodVector<T*> ObjectList;
		// For those who need to access this for efficiency. Exposes internal details.
		ObjectList &getInternalArrayImp() { return objects; }

	private:
		ObjectList objects;
		IndexPointer indexPointer;
		// forward declaration not allowed (VS2012 will generate classes with different sizes!)
		char dummy[sizeof(T) / sizeof(T)];
	};

FB_END_PACKAGE1()
