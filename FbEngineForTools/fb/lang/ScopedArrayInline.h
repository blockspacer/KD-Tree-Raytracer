
	class lang::DefaultScopedArrayDeleter
	{
	public:
		template<class T>
		void operator()(T *pointer)
		{
			/* Type completeness check. From boost's checked delete. Deleting incomplete type is undefined behaviour. */
			// intentionally complex - simplification causes regressions
			typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
			(void) sizeof(type_must_be_complete);
			delete[] pointer;
		}
	};

	template<typename T, typename D>
	ScopedArray<T, D>::ScopedArray(T *pointer_)
	:	pointer(pointer_)
	{
	}

	template<typename T, typename D>
	ScopedArray<T, D>::~ScopedArray()
	{
		D()(pointer);
	}

	template<typename T, typename D>
	void ScopedArray<T, D>::dismiss()
	{
		pointer = 0;
	}

	template<typename T, typename D>
	void ScopedArray<T, D>::reset(T *pointer_)
	{
		D()(pointer);
		pointer = pointer_;
	}

	template<typename T, typename D>
	T *ScopedArray<T, D>::get() const
	{
		return pointer;
	}

	template<typename T, typename D>
	bool ScopedArray<T, D>::operator !() const
	{
		return (pointer == 0) ? true : false;
	}
	
	template<typename T, typename D>
	T *ScopedArray<T, D>::operator -> () const
	{
		fb_assert(pointer);
		return pointer;
	}
