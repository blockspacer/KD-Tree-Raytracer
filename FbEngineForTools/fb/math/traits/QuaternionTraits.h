#ifndef FB_MATH_TRAITS_QUATERNIONTRAITS_H
#define FB_MATH_TRAITS_QUATERNIONTRAITS_H

#include "fb/math/traits/QuaternionCompareTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class QuaternionTraits
{
public:
	static const QuaternionCompareTraits<T> compareTraits;
	//static const QuaternionMathValidationTraits<T> validationTraits;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	class QUATTraits : public fb::math::traits::QuaternionTraits<float> { };
} } }

#endif
