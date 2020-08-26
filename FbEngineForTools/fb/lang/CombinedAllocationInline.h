	template<typename T>
	void CombinedAllocation::addPointer(SizeType amount, size_t alignment)
	{
		for (SizeType i = 0; i < amount; ++i)
			addPointerImp(sizeof(T), 1, alignment);
	}

	template<typename T>
	void CombinedAllocation::addArray(SizeType arraySize, size_t alignment)
	{
		addArrayImp(sizeof(T), arraySize, alignment);
	}

	template<typename T>
	T *CombinedAllocation::getNextPointer(size_t alignment)
	{
		T *ptr = getNextPointerNoDefaultConstructor<T> (alignment);
		return new (ptr) T();
	}

	template<typename T>
	T *CombinedAllocation::getNextPointerNoDefaultConstructor(size_t alignment)
	{
		char *ptr = getPointerImp(sizeof(T), alignment);
		return reinterpret_cast<T *> (ptr);
	}

	template<typename T>
	T *CombinedAllocation::getNextArray(SizeType arraySize, size_t alignment)
	{
		char *pointerRaw = getArrayImp(sizeof(T), arraySize, alignment);
		T *ptr = reinterpret_cast<T *> (pointerRaw);

		for (SizeType i = 0; i < arraySize; ++i)
			new (&ptr[i]) T();

		return ptr;
	}

	template<typename T>
	T *CombinedAllocation::getNextArrayNoDefaultConstructor(SizeType arraySize, size_t alignment)
	{
		char *ptr = getArrayImp(sizeof(T), arraySize, alignment);
		return reinterpret_cast<T *> (ptr);
	}

	template<typename T>
	void CombinedAllocation::freePointer(T *ptr)
	{
		if (ptr)
			ptr->~T();
	}

	template<typename T>
	void CombinedAllocation::freeArray(T *ptr, SizeType arraySize)
	{
		if (!ptr)
			return;

		for (SizeType i = 0; i < arraySize; ++i)
			freePointer(ptr++);
	}
