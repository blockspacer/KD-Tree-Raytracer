#include "Precompiled.h"
#include "FiendishAllocator.h"

#include "fb/lang/Atomics.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/FBPrintf.h"

FB_PACKAGE1(lang)

class FiendishAllocator::Impl
{
public:
	Impl()
	{
		InitializeCriticalSectionAndSpinCount(&cs, 1000);
	}

	~Impl()
	{
		DeleteCriticalSection(&cs);
	}

	void reallyFreeAddressSpace()
	{
		/* Really free some address space */
		uint32_t freedListFirst = (freedListNextFree - freedListSize) % freedListMaxSize;
		freedListBytes -= lang::max(freedAllocations[freedListFirst].size, pageSize);
		if (VirtualFree(freedAllocations[freedListFirst].pointer, 0, MEM_RELEASE) == 0)
		{
			fb_assert(0 && "VirtualFree MEM_RELEASE failed");
		}
		--freedListSize;
	}

	uint64_t getRoundedToPageSize(uint64_t size)
	{
		/* Presuming pageSize is power of two */
		uint64_t result = (size + pageSize - 1) & ~(uint64_t(pageSize) - 1);
		fb_assert(result >= size && "Something wrong with math");
		fb_assert(result - size < pageSize && "Something wrong with math (2)");
		fb_assert(result % pageSize == 0 && "Something wrong with math (3)");
		fb_assert(size % pageSize != 0 || result == size && "Something wrong with math (4)");
		return result;
	}


	enum Constants : uint64_t
	{
		/* Index of requested size of allocation (without overhead) */
		sizeIndex = 0,
		/* Very simple check mostly to detect someone messing with saved size */
		crapIndex = 1,
		crapData = 0xCACCAAE7CACCE11A,
		/* Fiendish allocator allocates a page at a time anyway, so just make sure this is large enough */
		allocationExtraBytes = 2 * sizeof(uint64_t),
		/* Max number of old allocations. They aren't released right away, only protected and set aside */
		freedListMaxSize = 128 * 1024,
		/* In gigabytes */
		freedListMaxSizeGigaBytes = 4
	};

	struct FiendishLockHolder;
	struct Entry
	{
		uint64_t size;
		void *pointer;
	};

	CRITICAL_SECTION cs;

	Entry freedAllocations[freedListMaxSize];

	uint32_t freedListNextFree = 0;
	uint32_t freedListSize = 0;
	uint64_t freedListBytes = 0;
	uint64_t pageSize = 0;
	lang::AtomicUInt64 liveAllocationBytes;
};


struct FiendishAllocator::Impl::FiendishLockHolder
{
	FiendishLockHolder(FiendishAllocator *fiendishAllocator)
		: fiendishAllocator(fiendishAllocator)
	{
		EnterCriticalSection(&fiendishAllocator->impl.cs);
	}

	~FiendishLockHolder() { LeaveCriticalSection(&fiendishAllocator->impl.cs); }

	FiendishAllocator *fiendishAllocator;
};


FiendishAllocator::Impl &FiendishAllocator::getImplInstance()
{
	static Impl impl;
	return impl;
}


FiendishAllocator::FiendishAllocator()
	: impl(getImplInstance())
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	impl.pageSize = info.dwPageSize;
}

FiendishAllocator::~FiendishAllocator()
{
	while (impl.freedListSize > 0)
		impl.reallyFreeAddressSpace();
}

FiendishAllocator &FiendishAllocator::getInstance()
{
	static FiendishAllocator allocator;
	return allocator;
}

void *FiendishAllocator::allocate(uint64_t sizeInBytes)
{
	uint64_t sizeWithOverhead = impl.getRoundedToPageSize(sizeInBytes + Impl::allocationExtraBytes);
	uint64_t *base = reinterpret_cast<uint64_t *>(VirtualAlloc(NULL, sizeWithOverhead, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
	base[Impl::sizeIndex] = sizeInBytes;
	base[Impl::crapIndex] = Impl::crapData;
	atomicAddRelaxed(impl.liveAllocationBytes, sizeWithOverhead);
	return reinterpret_cast<char *>(base) + Impl::allocationExtraBytes;
}

void *FiendishAllocator::reallocate(void *oldPointer, uint64_t newSizeInBytes)
{
	uint64_t *oldBase = reinterpret_cast<uint64_t *>(reinterpret_cast<char *>(oldPointer) - Impl::allocationExtraBytes);
	void *newPointer = allocate(newSizeInBytes);
	uint64_t bytesToCopy = lang::min(oldBase[Impl::sizeIndex], newSizeInBytes);
	memcpy(newPointer, oldPointer, bytesToCopy);
	free(oldPointer);
	return newPointer;
}

void FiendishAllocator::free(void *pointer)
{
	if (pointer == nullptr)
		return;

	uint64_t *base = reinterpret_cast<uint64_t *>(reinterpret_cast<char *>(pointer) - Impl::allocationExtraBytes);
	fb_assert(base[Impl::crapIndex] == Impl::crapData && "Someone's in fiendish crap");
	uint64_t size = impl.getRoundedToPageSize(base[Impl::sizeIndex] + Impl::allocationExtraBytes);

	DWORD oldProtect = 0;
	VirtualProtect(base, size, PAGE_NOACCESS, &oldProtect);
	if (VirtualFree(base, 0, MEM_DECOMMIT) == 0)
	{
		fb_assert(0 && "VirtualFree MEM_DECOMMIT failed");
	}
	atomicSubRelaxed(impl.liveAllocationBytes, size);

	/* Check if we should actually free something */
	Impl::FiendishLockHolder guard(this);
	impl.freedListBytes += size;
	impl.freedAllocations[impl.freedListNextFree].pointer = base;
	impl.freedAllocations[impl.freedListNextFree].size = size;
	impl.freedListNextFree = (impl.freedListNextFree + 1) % Impl::freedListMaxSize;
	++impl.freedListSize;
	while (impl.freedListBytes >> 30 > Impl::freedListMaxSizeGigaBytes || impl.freedListSize > Impl::freedListMaxSize - 16)
		impl.reallyFreeAddressSpace();
}

uint64_t FiendishAllocator::getCurrentAllocationBytes()
{
	return atomicLoadRelaxed(impl.liveAllocationBytes);
}

FB_END_PACKAGE1()
