FB_PACKAGE1(lang)
	
class DefaultSharedPointerDeleter
{
public:
	template<class T>
	void operator()(T *pointer)
	{
		delete pointer;
	}
};

class SharedPointerImpl
{
public:
	lang::AtomicInt32 refCount;
	void (*destructor)(void *) = nullptr;
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<class T>
SharedPointer<T>::SharedPointer()
{
}

template<class T>
SharedPointer<T>::SharedPointer(T *ptr)
{
	reset(ptr);
}
template<class T> template<class D>
SharedPointer<T>::SharedPointer(T *ptr, const D &d)
{
	reset(ptr, d);
}

template<class T>
SharedPointer<T>::SharedPointer(const SharedPointer &other)
{
	*this = other;
}

template<class T> template<class T2>
SharedPointer<T>::SharedPointer(const SharedPointer<T2> &other)
{
	*this = other;
}

template<class T>
SharedPointer<T>::~SharedPointer()
{
	decreaseReferenceCount();
	rawPointer = 0;
	impl = 0;
}

template<class T>
SharedPointer<T> &SharedPointer<T>::operator=(const SharedPointer &other)
{
	if (this == &other)
		return *this;

	decreaseReferenceCount();
	this->rawPointer = other.rawPointer;
	this->impl = other.impl;
	increaseReferenceCount();

	return *this;
}

template<class T> template<class T2>
SharedPointer<T> &SharedPointer<T>::operator=(const SharedPointer<T2> &other)
{
	fb_assert(this->impl != other.getImpl());

	decreaseReferenceCount();
	this->rawPointer = other.get();
	this->impl = other.getImpl();
	increaseReferenceCount();

	return *this;
}
	
template<class T>
void SharedPointer<T>::reset(T *ptr)
{
	reset(ptr, lang::DefaultSharedPointerDeleter());
}

template<class T> template<class D>
void SharedPointer<T>::reset(T *ptr, D d)
{
	decreaseReferenceCount();
	this->impl = 0;
	this->rawPointer = ptr;
	if (ptr)
	{
		struct Destructor
		{
			static void destroy(void *ptr)
			{
				D()((T*)ptr);
			}
		};
		impl = new lang::SharedPointerImpl();
		lang::atomicIncRelease(impl->refCount);
		impl->destructor = &Destructor::destroy;
	}
}

FB_END_PACKAGE0()
