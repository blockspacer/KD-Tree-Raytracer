#ifndef FB_MATH_TRAITS_COLOR3COLORTRAITS_H
#define FB_MATH_TRAITS_COLOR3COLORTRAITS_H

#include "fb/math/traits/ColorTraits.h"
#include "fb/lang/FBStaticAssert.h"

// usage example, checking that COL has a max component value of 1.0:
// 
//   #include "fb/math/traits/Color3ColorTraits.h"
//   ...
//   fb_assert(math::traits::ColorTraits<COL>::getColorComponentMaxValue() == 1.0f);

FB_PACKAGE2(math, traits)

template<class ColorT> class Color3ColorTraits
{
public:
	static typename ColorT::ComponentType getColorComponentMinValue() { fb_template_specialization_assert(0 && "Color3ColorTraits specialization issue."); }
	static typename ColorT::ComponentType getColorComponentMaxValue() { fb_template_specialization_assert(0 && "Color3ColorTraits specialization issue."); }
};

// specialized for known Color3 types...
// color with float component, 0.0f - 1.0f
template<> class Color3ColorTraits<float>
{
public:
	static float getColorComponentMinValue() { return 0.0f; }
	static float getColorComponentMaxValue() { return 1.0f; }
};

// color with unsigned char components, 0 - 255
template<> class Color3ColorTraits<unsigned char>
{
public:
	static unsigned char getColorComponentMinValue() { return 0; }
	static unsigned char getColorComponentMaxValue() { return 255; }
};


// specializing color traits for Color3...
template<class T> class ColorTraits<math::Color3<T> >
{
public:
	static typename Color3<T>::ComponentType getColorComponentMinValue() { return Color3ColorTraits<T>::getColorComponentMinValue(); }
	static typename Color3<T>::ComponentType getColorComponentMaxValue() { return Color3ColorTraits<T>::getColorComponentMaxValue(); }
};


FB_END_PACKAGE2()

#endif
