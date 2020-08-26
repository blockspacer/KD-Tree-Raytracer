#ifndef FB_MATH_TRAITS_VEC2COMPARETRAITS_H
#define FB_MATH_TRAITS_VEC2COMPARETRAITS_H

#include "fb/algorithm/traits/CompareTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class Vec2CompareTraits
{
public:
	static const bool doesSupportCompare = true;
	static const bool doesSupportSortingCompare = false;
	static const bool hasSpecializedCompare = true;
	static const bool hasSpecializedSortingCompare = false;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	template<> class CompareTraits<fb::math::VC2> : public fb::math::traits::Vec2CompareTraits<float> { };
	template<> class CompareTraits<fb::math::DVC2> : public fb::math::traits::Vec2CompareTraits<double> { };
	template<> class CompareTraits<fb::math::VC2I> : public fb::math::traits::Vec2CompareTraits<int> { };
} } }

#endif
