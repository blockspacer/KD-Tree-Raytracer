#ifndef FB_MATH_COLOR4_H
#define FB_MATH_COLOR4_H

FB_PACKAGE1(math)

/**
 * A simple structure for RGBA colors. 
 * No operations are supported for these really at the moment.
 * TODO: Make this a full sensible data type class, similar to Color3
 */
template<typename T> class Color4
{
public:
	T r;
	T g;
	T b;
	T a;

	Color4(T r, T g, T b, T a) : r(r), g(g), b(b), a(a) { };
};

typedef Color4<float> COL4;

FB_END_PACKAGE1()

#endif
