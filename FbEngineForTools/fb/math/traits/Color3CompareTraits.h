#ifndef FB_MATH_TRAITS_COLOR3COMPARETRAITS_H
#define FB_MATH_TRAITS_COLOR3COMPARETRAITS_H

#include "fb/algorithm/traits/CompareTraits.h"
#include "fb/math/Color3.h"

FB_PACKAGE2(math, traits)

template<class T> class Color3CompareTraits
{
public:
	static const bool doesSupportCompare = true;
	static const bool doesSupportSortingCompare = false;
	static const bool hasSpecializedCompare = true;
	static const bool hasSpecializedSortingCompare = false;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	template<> class CompareTraits<fb::math::COL> : public fb::math::traits::Color3CompareTraits<float> { };
} } }

#endif
