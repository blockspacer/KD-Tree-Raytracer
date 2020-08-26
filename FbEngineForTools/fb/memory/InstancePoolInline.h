	template<typename InstanceType, typename FixedSizeAllocatorType>
	InstancePool<InstanceType, FixedSizeAllocatorType>::InstancePool(SizeType instanceAmount, SizeType alignment)
		: allocator(instanceAmount, sizeof(InstanceType), 0, alignment)
	{
	}

	template<typename InstanceType, typename FixedSizeAllocatorType>	
	InstancePool<InstanceType, FixedSizeAllocatorType>::InstancePool(char *memoryBuffer, SizeType memorySize, SizeType alignment)
		: allocator(memoryBuffer, sizeof(InstanceType), alignment)
	{
	}

	template<typename InstanceType, typename FixedSizeAllocatorType>	
	InstancePool<InstanceType, FixedSizeAllocatorType>::~InstancePool()
	{
		fb_assert(isEmpty());
	}

	template<typename InstanceType, typename FixedSizeAllocatorType>	
	bool InstancePool<InstanceType, FixedSizeAllocatorType>::isEmpty() const
	{
		return allocator.isEmpty();
	}

	template<typename InstanceType, typename FixedSizeAllocatorType>	
	InstanceType *InstancePool<InstanceType, FixedSizeAllocatorType>::getPointer()
	{
		InstanceType *pointer = getPointerWithoutDefaultContructor();
		if (!pointer)
			return 0;

		// Default constructor with placement new
		return new (pointer) InstanceType();
	}

	template<typename InstanceType, typename FixedSizeAllocatorType>	
	InstanceType *InstancePool<InstanceType, FixedSizeAllocatorType>::getPointerWithoutDefaultContructor()
	{
		// may apparently return null (if out of memory?)
		//typedef InstanceType* PtrType;
		return (InstanceType *) allocator.allocate();
	}

	template<typename InstanceType, typename FixedSizeAllocatorType>	
	void InstancePool<InstanceType, FixedSizeAllocatorType>::freePointer(InstanceType *ptr)
	{
		if (ptr)
		{
			ptr->~InstanceType();
			allocator.deallocate((char*)ptr);
		}
	}

