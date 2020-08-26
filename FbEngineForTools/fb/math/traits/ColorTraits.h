#ifndef FB_MATH_TRAITS_COLORTRAITS_H
#define FB_MATH_TRAITS_COLORTRAITS_H

FB_PACKAGE2(math, traits)

template<class T> class ColorTraits
{
public:
	static typename T::ComponentType getColorComponentMinValue();
	static typename T::ComponentType getColorComponentMaxValue();
};

FB_END_PACKAGE2()

#endif
