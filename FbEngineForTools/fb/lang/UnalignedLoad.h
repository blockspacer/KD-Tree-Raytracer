#ifndef FB_LANG_UNALIGNEDLOAD_H
#define FB_LANG_UNALIGNEDLOAD_H

FB_PACKAGE1(lang)

// no action by default
template<class T>
inline T loadUnaligned(const void *ptr)
{
	return *(const T*)ptr;
};

// floats and doubles need to be aligned on some platforms
#if 0

	#if (1)
		template<class FloatType>
		inline FloatType loadUnalignedTemplate(const void *ptr)
		{
			union __attribute__ ((packed)) U
			{
				FloatType v;
			};
			U *pu = (U*)ptr;
			return pu->v;
		}

	#elif (2)
		template<class FloatType>
		inline FloatType loadUnalignedTemplate(const void *ptr)
		{
			union __attribute__ ((packed)) U
			{
				FloatType v;
			};
			U *pu = (U*)ptr;
			return pu->v;
		}
	#endif

	template<>
	inline float loadUnaligned<float>(const void *ptr)
	{
		return loadUnalignedTemplate<float>(ptr);
	}

	template<>
	inline double loadUnaligned<double>(const void *ptr)
	{
		return loadUnalignedTemplate<double>(ptr);
	}

#endif

FB_END_PACKAGE1()

#endif
