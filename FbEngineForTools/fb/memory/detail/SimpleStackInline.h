	template<typename T>
	SimpleStack<T>::SimpleStack(SizeType initialAmount)
		: data(0)
		, arraySize(0)
		, instanceAmount(0)
	{
		if (initialAmount)
			reserve(initialAmount);
	}

	template<typename T>
	SimpleStack<T>::~SimpleStack()
	{
		lang::freeMemory(data);
		data = 0;
		arraySize = 0xFACFACFA; // I've been deleted magic number!
	}

	template<typename T>
	bool SimpleStack<T>::isEmpty() const
	{
		return instanceAmount == 0;
	}
	
	template<typename T>
	void SimpleStack<T>::clear()
	{
		instanceAmount = 0;
	}

	template<typename T>
	void SimpleStack<T>::reserve(SizeType amount)
	{
		if (amount > arraySize)
		{
			data = static_cast<T *> (lang::reallocateMemory(data, amount * sizeof(T)));
			fb_expensive_assert(data != 0);

			// Huh? This never actually set the capacity?
			arraySize = amount;
		}
	}

	template<typename T>
	void SimpleStack<T>::push(const T &value)
	{
		if (instanceAmount >= arraySize)
		{
			SizeType newArraySize = arraySize * 2 + 1;
			reserve(arraySize * 2 + 1);
			arraySize = newArraySize;
		}

		fb_expensive_assert(arraySize > instanceAmount);
		data[instanceAmount] = value;
		++instanceAmount;
		fb_expensive_assert(arraySize >= instanceAmount);
	}

	template<typename T>
	T SimpleStack<T>::pop()
	{
		fb_assert(!isEmpty());
		
		--instanceAmount;
		fb_expensive_assert(instanceAmount >= 0);
		return data[instanceAmount];
	}

	template<typename T>
	T SimpleStack<T>::get(SizeType index)
	{
		fb_assert(index < instanceAmount);
		return data[index];
	}
