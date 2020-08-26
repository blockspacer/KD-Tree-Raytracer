#ifndef FB_MATH_TRAITS_VEC2TRAITS_H
#define FB_MATH_TRAITS_VEC2TRAITS_H

#include "fb/math/traits/Vec2CompareTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class Vec2Traits
{
public:
	static const Vec2CompareTraits<T> compareTraits;
	//static const Vec2MathValidationTraits<T> validationTraits;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	class VC2Traits : public fb::math::traits::Vec2Traits<float> { };
	class DVC2Traits : public fb::math::traits::Vec2Traits<double> { };
	class VC2ITraits : public fb::math::traits::Vec2Traits<int> { };
} } }

#endif
