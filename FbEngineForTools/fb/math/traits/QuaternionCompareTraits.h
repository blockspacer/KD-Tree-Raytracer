#ifndef FB_MATH_TRAITS_QUATERNIONCOMPARETRAITS_H
#define FB_MATH_TRAITS_QUATERNIONCOMPARETRAITS_H

#include "fb/algorithm/traits/CompareTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class QuaternionCompareTraits
{
public:
	static const bool doesSupportCompare = true;
	static const bool doesSupportSortingCompare = false;
	static const bool hasSpecializedCompare = true;
	static const bool hasSpecializedSortingCompare = false;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	template<> class CompareTraits<fb::math::QUAT> : public fb::math::traits::QuaternionCompareTraits<float> { };
} } }

#endif
