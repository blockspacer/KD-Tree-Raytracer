#ifndef FB_MATH_TRAITS_VEC3COMPARETRAITS_H
#define FB_MATH_TRAITS_VEC3COMPARETRAITS_H

#include "fb/algorithm/traits/CompareTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class Vec3CompareTraits
{
public:
	static const bool doesSupportCompare = true;
	static const bool doesSupportSortingCompare = false;
	static const bool hasSpecializedCompare = true;
	static const bool hasSpecializedSortingCompare = false;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	template<> class CompareTraits<fb::math::VC3> : public fb::math::traits::Vec3CompareTraits<float> { };
	template<> class CompareTraits<fb::math::DVC3> : public fb::math::traits::Vec3CompareTraits<double> { };
	template<> class CompareTraits<fb::math::VC3I> : public fb::math::traits::Vec3CompareTraits<int> { };
} } }

#endif
