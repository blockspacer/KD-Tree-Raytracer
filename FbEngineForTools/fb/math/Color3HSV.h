#ifndef FB_MATH_COLOR3HSV_H
#define FB_MATH_COLOR3HSV_H

FB_PACKAGE1(math)

/**
 * A simple structure for HSV color space colors. 
 * No operations are supported for these really.
 *
 * Assuming that you should never have any direct use for these, just use the RGB colors instead.
 */
class Color3HSV
{
public:
	float h;
	float s;
	float v;

	Color3HSV(float h, float s, float v) : h(h), s(s), v(v) { };
};

FB_END_PACKAGE1()

#endif
