#ifndef FB_MEMORY_MULTIINSTANCEPOOL_H
#define FB_MEMORY_MULTIINSTANCEPOOL_H

FB_PACKAGE1(memory)

	template<typename InstanceType, typename MultiFixedSizeAllocatorType>
	class MultiInstancePool
	{
	public:
		inline MultiInstancePool(int instanceAmount, int initialPoolAmount, int alignment);
		inline ~MultiInstancePool();

		inline InstanceType *getPointer();
		inline InstanceType *getPointerWithoutDefaultContructor();

		inline void freePointer(InstanceType *ptr);

	private:
		MultiFixedSizeAllocatorType allocator;
	};

FB_END_PACKAGE1()


#endif
