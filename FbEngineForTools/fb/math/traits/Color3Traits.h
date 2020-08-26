#ifndef FB_MATH_TRAITS_COLOR3TRAITS_H
#define FB_MATH_TRAITS_COLOR3TRAITS_H

#include "fb/math/traits/Color3CompareTraits.h"
#include "fb/math/traits/Color3ColorTraits.h"
//#include "fb/math/traits/Color3VAlidationTraits.h"

FB_PACKAGE2(math, traits)

template<class T> class Color3Traits
{
public:
	static const Color3ColorTraits<T> colorTraits;
	static const Color3CompareTraits<T> compareTraits;
	//static const Color3MathValidationTraits<T> validationTraits;
};

FB_END_PACKAGE2()

namespace fb { namespace algorithm { namespace traits {
	class COLTraits : public fb::math::traits::Color3Traits<float> { };
} } }

#endif
