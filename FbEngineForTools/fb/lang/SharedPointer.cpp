#include "Precompiled.h"
#include "SharedPointer.h"
#include "MemoryFunctions.h"

FB_PACKAGE1(lang)

bool SharedPointerBaseImpl::isUnique() const
{ 
	return impl != nullptr && lang::atomicLoadRelaxed(impl->refCount) == 1;
}

void SharedPointerBaseImpl::reset()
{
	decreaseReferenceCount();
	impl = nullptr;
	rawPointer = nullptr;
}

void SharedPointerBaseImpl::swap(SharedPointerBaseImpl &other)
{
	lang::swap(other.rawPointer, rawPointer);
	lang::swap(other.impl, impl);
}

void SharedPointerBaseImpl::increaseReferenceCount()
{
	if (impl)
	{
		fb_assert(rawPointer);
		lang::atomicIncRelaxed(impl->refCount);
	}
	else
	{
		fb_assert(!rawPointer);
	}
}

void SharedPointerBaseImpl::decreaseReferenceCount()
{
	if (impl)
	{
		// release/acquire semantics based on boost
		const int oldRefCount = lang::atomicDecRelease(impl->refCount);
		fb_assert(oldRefCount >= 1);
		const bool isZero = (oldRefCount == 1);
		if (isZero)
		{
			lang::atomicThreadFenceAcquire();

			impl->destructor(rawPointer);
			rawPointer = nullptr;
			delete impl;
			impl = nullptr;
		}
	}
	else
	{
		fb_assert(!rawPointer);
	}
}

FB_END_PACKAGE1()