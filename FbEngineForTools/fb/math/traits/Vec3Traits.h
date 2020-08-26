#ifndef FB_MATH_TRAITS_VEC3TRAITS_H
#define FB_MATH_TRAITS_VEC3TRAITS_H

#include "fb/math/traits/Vec3CompareTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class Vec3Traits
{
public:
	static const Vec3CompareTraits<T> compareTraits;
	//static const Vec3MathValidationTraits<T> validationTraits;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	class VC3Traits : public fb::math::traits::Vec3Traits<float> { };
	class DVC3Traits : public fb::math::traits::Vec3Traits<double> { };
	class VC3ITraits : public fb::math::traits::Vec3Traits<int> { };
} } }

#endif
