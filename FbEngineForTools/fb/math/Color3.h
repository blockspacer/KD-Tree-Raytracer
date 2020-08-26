#ifndef FB_MATH_COLOR3_H
#define FB_MATH_COLOR3_H

#include "fb/lang/FBAssert.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/platform/ForceInline.h"

FB_PACKAGE1(math)

#if (FB_BUILD != FB_FINAL_RELEASE)
	// (note, one might save some time on stupid bugs if we actually had sensible validated data types)
	//#define FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED FB_TRUE
	#define FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED FB_FALSE
#else
	#define FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED FB_FALSE
#endif


#if (FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED == FB_TRUE)
	template<typename ColorType> class ColorExtraValidator
	{
	public:
		static inline void validate(const ColorType &c)
		{
			// nothing by default, specialize for type.
		}
	};
#endif


template <class A>
class Color3
{
public:
	typedef A ComponentType;
	typedef A ValueType;

	// Note, these are unsafe for static initializations.
	// Go see ColorContants instead for safer use.
	static const Color3 black;
	static const Color3 white;

	union
	{
		A v[3];
		struct
		{
			A r,g,b;
		};
	};

#if (FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED == FB_TRUE)
	Color3() : r(1.0),g(1.0),b(1.0) { validate(); };
	Color3(A _r,A _g,A _b) : r(_r),g(_g),b(_b) { validate(); };
	Color3(A a[3]) : r(a[0]),g(a[1]),b(a[2]) { validate(); };
#elif (FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED == FB_FALSE)
	Color3() : r(1.0),g(1.0),b(1.0) {};
	Color3(A _r,A _g,A _b) : r(_r),g(_g),b(_b) {};
	Color3(const A a[3]) : r(a[0]),g(a[1]),b(a[2]) {};
#else
	#error "FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED define is missing or invalid."
#endif

	Color3<A> operator-() const
	{
		return Color3<A>(-r,-g,-b);
	}
	
	Color3<A> operator+(const Color3<A>& other) const
	{
		return Color3<A>(r+other.r,g+other.g,b+other.b);
	}

	Color3<A> operator-(const Color3<A>& other) const
	{
		return Color3<A>(r-other.r,g-other.g,b-other.b);
	}
	
	Color3<A> operator*(const Color3<A>& other) const
	{
		return Color3<A>(r*other.r,g*other.g,b*other.b);
	}
	
	Color3<A> operator/(const Color3<A>& other) const
	{
		return Color3<A>(r/other.r,g/other.g,b/other.b);
	}
	
	Color3<A> operator*(A num) const
	{
		return Color3<A>(r*num,g*num,b*num);
	}

	Color3<A> operator/(A num) const
	{
		A inum=(A)1/num;
		return Color3<A>(r*inum,g*inum,b*inum);
	}
	
	void operator+=(const Color3<A>& other)
	{
		r+=other.r;
		g+=other.g;
		b+=other.b;
	}

	void operator-=(const Color3<A>& other)
	{
		r-=other.r;
		g-=other.g;
		b-=other.b;
	}

	void operator*=(const Color3<A>& other)
	{
		r*=other.r;
		g*=other.g;
		b*=other.b;
	}

	void operator/=(const Color3<A>& other)
	{
		r/=other.r;
		g/=other.g;
		b/=other.b;
	}

	void operator*=(A num)
	{
		r*=num;
		g*=num;
		b*=num;
	}

	void operator/=(A num)
	{
		A inum=(A)1/num;
		r*=inum;
		g*=inum;
		b*=inum;
	}

	const A &operator[](SizeType index) const
	{
		fb_expensive_assert(index < 3 && "Invalid index for Color operator[]");
		return v[index];
	}

	A &operator[](SizeType index)
	{
		fb_expensive_assert(index < 3 && "Invalid index for Color operator[]");
		return v[index];
	}
	
	bool operator== (const Color3<A> &other) const
	{
		return (r == other.r && g == other.g && b == other.b);
	}

	bool operator!= (const Color3<A> &other) const
	{
		return (r != other.r || g != other.g || b != other.b);
	}

	void capTo(ComponentType min, ComponentType max)
	{
		r = lang::max(min, lang::min(max, r));
		g = lang::max(min, lang::min(max, g));
		b = lang::max(min, lang::min(max, b));
	}

	// (Supplied for scripting language binding)
	A getRed() const
	{
		return r;
	}
	A getGreen() const
	{
		return g;
	}
	A getBlue() const
	{
		return b;
	}

	void cap()
	{
		capTo(0.0f, 1.0f);
	}

#if (FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED == FB_TRUE)
	void validate()
	{
		ColorExtraValidator<Color3<A> >::validate(*this);
	}
#endif

};

template<typename A> const Color3<A> Color3<A>::black = Color3<A>(0, 0, 0);
template<typename A> const Color3<A> Color3<A>::white = Color3<A>(A(1), A(1), A(1));

typedef Color3<float> COL;

FB_FORCEINLINE math::COL lerp(const COL& a, const COL& b, float delta)
{
	return a + (b - a) * delta;
}

#if (FB_MATH_COLOR_EXTRA_VALIDATION_ENABLED == FB_TRUE)
template<>
void ColorExtraValidator<Color3<float> >::validate(const Color3<float> &c)
{
	// cannot do real proper validation at this point, that would probably break up absolutely everything, just detecting the "obvious?" cases.
	
	// guess if we have some light color intensities of 1000+ etc.? Yes we do. Thus, this just won't do
	//fb_assert_once(c.r < 128.0f && c.g < 128.0f && c.b < 128.0f && "Probably attempting to set 8 bit (0 - 255) RGB values to normalized floating point (0.0 - 1.0) color.");

	fb_assert_once(c.r != 128.0f && c.g != 128.0f && c.b != 128.0f && "Probably attempting to set 8 bit (0 - 255) RGB values to normalized floating point (0.0 - 1.0) color.");
	fb_assert_once(c.r != 255.0f && c.g != 255.0f && c.b != 255.0f && "Probably attempting to set 8 bit (0 - 255) RGB values to normalized floating point (0.0 - 1.0) color.");
}
#endif


FB_END_PACKAGE1()

#include "fb/math/traits/Color3Traits.h"

#endif
