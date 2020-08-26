#if !defined(FB_CONTAINERIMP_CONTAINER_TYPE) || !defined(FB_CONTAINERIMP_TEMPLATE_PARAMS)
#error "Define these before including this file."
#endif


template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
typename FB_CONTAINERIMP_CONTAINER_TYPE ::Iterator begin(FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return vec.getBegin();
}

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
typename FB_CONTAINERIMP_CONTAINER_TYPE ::Iterator end(FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return vec.getEnd();
}

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
typename FB_CONTAINERIMP_CONTAINER_TYPE ::ConstIterator begin(const FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return vec.getBegin();
}

template<FB_CONTAINERIMP_TEMPLATE_PARAMS>
typename FB_CONTAINERIMP_CONTAINER_TYPE ::ConstIterator end(const FB_CONTAINERIMP_CONTAINER_TYPE &vec)
{
	return vec.getEnd();
}

#undef FB_CONTAINERIMP_CONTAINER_TYPE
#undef FB_CONTAINERIMP_TEMPLATE_PARAMS
