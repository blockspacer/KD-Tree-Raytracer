#ifndef FB_MEMORY_DETAIL_SIMPLESTACK_H
#define FB_MEMORY_DETAIL_SIMPLESTACK_H

#include "fb/lang/MemoryFunctions.h"

FB_PACKAGE2(memory, detail)

	// Internal implementation class for memory namespace. Hands off from elsewhere.
	// Only works for POD's !!

	template<typename T>
	class SimpleStack
	{
	public:
		SimpleStack(SizeType initialAmount);
		~SimpleStack();

		bool isEmpty() const;
		SizeType getSize() const { return instanceAmount; }
		SizeType getCapacity() const { return arraySize; }

		void reserve(SizeType amount);

		void push(const T &value);
		T pop();
		T get(SizeType index);
		
		void clear();

	private:
		T *data;
		SizeType arraySize;
		SizeType instanceAmount;
	};

	#include "SimpleStackInline.h"

FB_END_PACKAGE2()


#endif
