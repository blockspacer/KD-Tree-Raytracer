#ifndef FB_MATH_CURVE_DETAIL_SPLINEBASIS_H
#define FB_MATH_CURVE_DETAIL_SPLINEBASIS_H

// Based on source code from WildTangent4 by Geometric Tools, LLC.

// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#include "fb/lang/FastCast.h"

#if FB_BUILD == FB_FINAL_RELEASE
	#define FB_OPTIMIZED_SPLINECURVE_BASE
#else
	#define FB_SPLINECURVE_ASSERTS
#endif

FB_PACKAGE3(math, curve, detail)

	template<typename T, uint32_t degree>
	class SplineBasis
	{
		T values[degree + 1];
		T knots[2 * degree];

	public:

		/* TODO: If someone actually knows whether min and max may be less than zero, please simplify code by making 
		 * them unsigned. */
		void compute(float time, int16_t &min, int16_t &max, SizeType controlPoints)
		{
#ifdef FB_SPLINECURVE_ASSERTS
			fb_assert(controlPoints <= 65535);
			fb_assert(0.0 <= time && time <= 1.0f);
			fb_assert(degree < controlPoints);
#endif

			//#if !defined(FB_OPTIMIZED_SPLINECURVE_BASE)
				time = FB_FCLAMP(time, 0.0001f, 0.9999f);
			//#endif

			// Assume we are already within normalized time
			#if defined(FB_OPTIMIZED_SPLINECURVE_BASE)
				T qmD = fb::lang::getFastU16Float(uint16_t(controlPoints - degree));
			#else
				T qmD = T(controlPoints - degree);
			#endif

			T t = qmD * time;
			#if defined(FB_OPTIMIZED_SPLINECURVE_BASE)
				min = fb::lang::getFastS16(float(t));
			#else
				min = (int16_t)t;
			#endif

			max = int16_t(min + degree);

			// Precompute the knots.
			/* I don't know if signed i1 makes sense or not */
			for (int16_t i0 = 0, i1 = int16_t(max + 1 - degree); i0 < 2 * degree; ++i0, ++i1)
			{
				if (i1 <= degree)
					knots[i0] = 0;
				else if (i1 >= int(controlPoints))
					knots[i0] = qmD;
				else
				{
					//knots[i0] = T(i1 - degree);
					#if defined(FB_OPTIMIZED_SPLINECURVE_BASE)
						knots[i0] = fb::lang::getFastS16Float(int16_t(i1 - degree));
					#else
						knots[i0] = T(i1 - degree);
					#endif
				}
			}

			// Initialize the basis function evaluation table.  The first degree-1
			// entries are zero, but they do not have to be set explicitly.
			values[degree] = T(1.0);

			// Update the basis function evaluation table, each iteration overwriting
			// the results from the previous iteration.
			for (int row = degree - 1; row >= 0; --row)
			{
				int k0 = degree;
				int k1 = row;
				T knot0 = knots[k0];
				T knot1 = knots[k1];
				T invDenom = T(1.0)/(knot0 - knot1);
				T c1 = (knot0 - t) * invDenom, c0;
				values[row] = c1 * values[row + 1];

				for (int col = row + 1; col < degree; ++col)
				{
					c0 = (t - knot1) * invDenom;
					values[col] *= c0;

					knot0 = knots[++k0];
					knot1 = knots[++k1];
					invDenom = T(1.0) / (knot0 - knot1);
					c1 = (knot0 - t) * invDenom;
					values[col] += c1 * values[col + 1];
				}

				c0 = (t - knot1) * invDenom;
				values[degree] *= c0;
			}
		}

		T getValue(uint32_t i) const
		{
#ifdef FB_SPLINECURVE_ASSERTS
			fb_assert(i <= degree);
#endif
			return values[i];
		}
	};

FB_END_PACKAGE3()

#endif
