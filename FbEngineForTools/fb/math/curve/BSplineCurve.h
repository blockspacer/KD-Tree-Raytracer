#pragma once

// Based on source code from WildTangent4 by Geometric Tools, LLC.

// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#include "detail/SplineBasis.h"
#include "BSplineCurveFunctions.h"
#include "fb/lang/IntTypes.h"

FB_PACKAGE2(math, curve)

template<typename Trait, uint32_t degree, bool deleteArray>
class BSplineCurve
{
	typedef typename Trait::CalculationType CalculationType;
	typedef typename Trait::StorageType StorageType;
	typedef typename Trait::DataType DataType;

	StorageType *controlData;
	uint16_t controlPointAmount;
	//bool deleteArray;

public:
	BSplineCurve()
	:	controlData(0)
	,	controlPointAmount(0)
	{
	}

	BSplineCurve(const BSplineCurve &other)
	:	controlData(0)
	,	controlPointAmount(0)
	{
		*this = other;
	}

	~BSplineCurve()
	{
		if (deleteArray)
			delete[] controlData;
	}

	const BSplineCurve &operator = (const BSplineCurve &other)
	{
		initialize(other.controlPointAmount, other.controlData, false, false);
		return *this;
	}

	void initialize(SizeType controlPointAmount_, typename Trait::StorageType *data, bool useGivenBuffer, bool deleteGivenBuffer)
	{
		fb_assert(controlPointAmount_ <= 65535);
		controlPointAmount = uint16_t(controlPointAmount_);
		if (useGivenBuffer)
		{
			controlData = data;
		}
		else
		{
			if (deleteArray)
				delete[] controlData;

			controlData = new typename Trait::StorageType[controlPointAmount * Trait::storageComponentAmount];
			lang::MemCopy::copy(controlData, data, controlPointAmount * Trait::storageComponentAmount * sizeof(typename Trait::StorageType));
		}
	}

	bool getValue(float t, DataType &value)
	{
		return getSplineValue<Trait, degree> (t, &value, controlData, controlPointAmount);
		/*
		fb_assert(controlPointAmount > degree);

		int min = 0;
		int max = 0;

		detail::SplineBasis<CalculationType, degree> basis;
		basis.compute(t, min, max, controlPointAmount);
		fb_expensive_assert(min >= 0 && min < controlPointAmount);

		StorageType *sourceStorage = &controlData[min * Trait::storageComponentAmount];
		CalculationType basisValue = basis.getValue(0);

		Trait::getValue(sourceStorage, value);
		sourceStorage += Trait::storageComponentAmount;
		value *= basisValue;

		typename Trait::DataType tmp;
		for (int i = min + 1, index = 1; i <= max; ++i, ++index)
		{
			basisValue = basis.getValue(index);
			
			Trait::getValue(sourceStorage, tmp);
			sourceStorage += Trait::storageComponentAmount;

			tmp *= basisValue;
			value += tmp;
		}

		return true;
		*/
	}

	bool getLinearValue(float t, DataType &value)
	{
		return math::curve::getLinearValue<Trait> (t, &value, controlData, controlPointAmount);

		/*
		if (controlPointAmount > 1)
		{
			float delta = 1.f / (float(controlPointAmount - 1));
			float frame = (t / delta);
			int i0 = (int) (frame);
			fb_expensive_assert(i0 >= 0 && i0 + 1 < controlPointAmount);

			StorageType *s0 = &controlData[i0 * Trait::storageComponentAmount];
			StorageType *s1 = s0 + Trait::storageComponentAmount;

			DataType tmp;
			Trait::getValue(s0, value);
			Trait::getValue(s1, tmp);
			float interpolateValue = fmodf(t, delta) / delta;

			value = Trait::interpolate(value, tmp, interpolateValue);
		}
		else if (controlPointAmount == 1)
		{
			Trait::getValue(controlData, value);
		}
		else
			return false;

		return true;
		*/
	}

	SizeType getControlPointAmount() const
	{
		return controlPointAmount;
	}

	const StorageType *getControlPoints() const
	{
		return controlData;
	}
};

FB_END_PACKAGE2()
