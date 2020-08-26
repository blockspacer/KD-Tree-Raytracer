#ifndef FB_MATH_CURVE_SPLINECURVEFUNCTIONS_H
#define FB_MATH_CURVE_SPLINECURVEFUNCTIONS_H

// Based on source code from WildTangent4 by Geometric Tools, LLC.

// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#include "detail/SplineBasis.h"
#include "fb/lang/IntTypes.h"
#include "fb/lang/platform/CompilerOptimizations.h"
#include "fb/lang/FastCast.h"

FB_PACKAGE2(math, curve)

	inline void mul(math::VC3 *value, float basisValue)
	{
		*value *= basisValue;
	}

	inline void mul(math::QUAT *value, float basisValue)
	{
		value->x *= basisValue;
		value->y *= basisValue;
		value->z *= basisValue;
		value->w *= basisValue;
	}

	template<typename Trait, int degree>
	bool getSplineValue(float t, typename Trait::DataType *FB_RESTRICT value, void *storage, uint16_t controlPointAmount)
	{
		typedef typename Trait::CalculationType CalculationType;
		typedef typename Trait::StorageType StorageType;
		typedef typename Trait::DataType DataType;

		fb_expensive_assert(controlPointAmount > degree);
		StorageType *FB_RESTRICT controlData = (StorageType *) storage;

		int16_t min = 0;
		int16_t max = 0;

		detail::SplineBasis<CalculationType, degree> basis;
		basis.compute(t, min, max, controlPointAmount);
		fb_expensive_assert(min >= 0 && min < controlPointAmount);

		StorageType *FB_RESTRICT sourceStorage = &controlData[min * Trait::storageComponentAmount];
		CalculationType basisValue = basis.getValue(0);

		Trait::getValue(sourceStorage, value);
		sourceStorage += Trait::storageComponentAmount;
		mul(value, basisValue);

		typename Trait::DataType tmp;
		for (int i = min + 1, index = 1; i <= max; ++i, ++index)
		{
			basisValue = basis.getValue(uint32_t(index));
			
			Trait::getValue(sourceStorage, &tmp);
			sourceStorage += Trait::storageComponentAmount;

			mul(&tmp, basisValue);
			*value += tmp;
		}

		return true;
	}

	template<typename Trait>
	bool getLinearValue(float t, typename Trait::DataType *FB_RESTRICT value, void *storage, uint16_t controlPointAmount)
	{
		typedef typename Trait::CalculationType CalculationType;
		typedef typename Trait::StorageType StorageType;
		typedef typename Trait::DataType DataType;

		StorageType *FB_RESTRICT controlData = (StorageType *) storage;

		if (controlPointAmount > 1)
		{
			//float delta = 1.f / (float(controlPointAmount - 1));
			uint16_t controlAmountI = controlPointAmount - 1U;
			if (t < 1.0f)
			{
				float delta = 1.f / fb::lang::getFastU16Float(controlAmountI);
				float frame = (t / delta);
				//int i0 = (int) (frame);
				uint16_t i0 = fb::lang::getFastU16(frame);
				fb_expensive_assert(i0 >= 0 && i0 < controlPointAmount);

				StorageType *FB_RESTRICT s0 = &controlData[i0 * Trait::storageComponentAmount];
				StorageType *FB_RESTRICT s1 = s0 + Trait::storageComponentAmount;

				DataType tmp;
				Trait::getValue(s0, value);
				Trait::getValue(s1, &tmp);
				float interpolateValue = frame - (float)i0;

				*value = Trait::interpolate(value, &tmp, interpolateValue);
			}
			else
			{
				uint16_t i0 = controlAmountI;
				StorageType *FB_RESTRICT s0 = &controlData[i0 * Trait::storageComponentAmount];
				Trait::getValue(s0, value);
			}
		}
		else if (controlPointAmount == 1)
		{
			Trait::getValue(controlData, value);
		}
		else
			return false;

		return true;
	}

FB_END_PACKAGE2()

#endif
