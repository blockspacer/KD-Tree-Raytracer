
	class lang::DefaultScopedPointerDeleter
	{
	public:
		template<class T>
		void operator()(T *pointer)
		{
			/* Type completeness check. From boost's checked delete. Deleting incomplete type is undefined behaviour. */
			// intentionally complex - simplification causes regressions
			typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
			(void) sizeof(type_must_be_complete);
			delete pointer;
		}
	};

	template<typename T, typename D>
	ScopedPointer<T, D>::ScopedPointer(T *pointer_)
	{
		pointer = pointer_;
	}

	template<typename T, typename D>
	ScopedPointer<T, D>::~ScopedPointer()
	{
		D()((T*) pointer);
	}

	template<typename T, typename D>
	void ScopedPointer<T, D>::dismiss()
	{
		pointer = 0;
	}

	template<typename T, typename D>
	void ScopedPointer<T, D>::reset(T *pointer_)
	{
		D()((T*) pointer);
		pointer = pointer_;
	}

	template<typename T, typename D>
	T *ScopedPointer<T, D>::get() const
	{
		return (T*) pointer;
	}

	template<typename T, typename D>
	bool ScopedPointer<T, D>::operator !() const
	{
		return (pointer == 0) ? true : false;
	}
	
	template<typename T, typename D>
	T *ScopedPointer<T, D>::operator -> () const
	{
		fb_assert(pointer);
		return (T*) pointer;
	}
