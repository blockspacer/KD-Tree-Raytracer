#ifndef FB_MATH_CURVE_SPLINECURVECREATOR_H
#define FB_MATH_CURVE_SPLINECURVECREATOR_H

// Based on source code from WildTangent4 by Geometric Tools, LLC.

// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#include "BSplineCurve.h"
#include "detail/BandedMatrix.h"

FB_PACKAGE2(math, curve)

template<typename Trait, typename CompressTrait, typename IntermediateTrait, uint32_t degree>
void createBSplineCurveManual(SizeType inputAmount, const typename CompressTrait::InputType *FB_RESTRICT inputData, SizeType compressedAmount, BSplineCurve<Trait, degree, true> &result, typename CompressTrait::CalculationType *FB_RESTRICT optionalErrorMetric = 0)
{
	fb_assert(1 <= degree && degree < inputAmount);
	fb_assert(compressedAmount <= inputAmount);
	typedef typename CompressTrait::CalculationType CalculationType;
	typedef typename Trait::StorageType StorageType;
	typedef typename Trait::DataType DataType;

	detail::SplineBasis<CalculationType, degree> basis;
	CalculationType multiplier = CalculationType(1.0) / CalculationType(inputAmount - 1);

    // Fit the data points with a B-spline curve using a least-squares error
    // metric.  The problem is of the form A^T*A*Q = A^T*P, where A^T*A is a
    // banded matrix, P contains the sample data, and Q is the unknown vector
    // of control points.

	detail::BandedMatrix<CalculationType> *ataMatrix = new detail::BandedMatrix<CalculationType>(compressedAmount, degree + 1, degree + 1);
	for (SizeType i0 = 0; i0 < compressedAmount; ++i0)
	{
		for (SizeType i1 = 0; i1 < i0; ++i1)
			(*ataMatrix)(i0,i1) = (*ataMatrix)(i1,i0);

		SizeType max1 = i0 + degree;
		if (max1 >= compressedAmount)
			max1 = compressedAmount - 1;

		for (SizeType i1 = i0; i1 <= max1; ++i1)
		{
			CalculationType value = 0;
			for (SizeType i2 = 0; i2 < inputAmount; ++i2)
			{
				CalculationType t = multiplier * CalculationType(i2);
				t = FB_FCLAMP(float(t), 0.0001f, 0.9999f);

				int16_t min = 0;
				int16_t max = 0;
				basis.compute((float) t, min, max, compressedAmount);
				if (min <= int16_t(i0) && int16_t(i0) <= max && min <= int16_t(i1) && int16_t(i1) <= max)
				{
					CalculationType b0 = basis.getValue(uint32_t(i0 - min));
					CalculationType b1 = basis.getValue(uint32_t(i1 - min));
					value += b0 * b1;
				}
			}

			(*ataMatrix)(i0,i1) = value;
		}
	}

	// Construct the matrix A^T.
	CalculationType **atMatrix = new CalculationType *[compressedAmount];
	atMatrix[0] = new CalculationType[compressedAmount * inputAmount];
	for (SizeType row = 1; row < compressedAmount; ++row)
		atMatrix[row] = &atMatrix[0][inputAmount * row];

	lang::MemSet::set(atMatrix[0], 0, inputAmount * compressedAmount * sizeof(CalculationType));
	for (SizeType i0 = 0; i0 < compressedAmount; ++i0)
	{
		for (SizeType i1 = 0; i1 < inputAmount; ++i1)
		{
			CalculationType t = multiplier * CalculationType(i1);
			t = FB_FCLAMP(float(t), 0.0001f, 0.9999f);

			int16_t min = 0;
			int16_t max = 0;
			basis.compute((float) t, min, max, compressedAmount);
			if (min <= int16_t(i0) && int16_t(i0) <= max)
				atMatrix[i0][i1] = basis.getValue(uint32_t(i0 - min));
		}
	}

	// Compute X0 = (A^T*A)^{-1}*A^T by solving the linear system A^T*A*X = A^T.
	FB_UNUSED_NAMED_VAR(bool, solved) = ataMatrix->solveSystem(atMatrix, inputAmount);
	fb_assert(solved);
  
	// The control points for the fitted curve are stored in the vector
	// Q = X0*P, where P is the vector of sample data.
	CalculationType *controlData = new CalculationType[compressedAmount * CompressTrait::calculationTypeAmount];
	lang::MemSet::set(controlData, 0, compressedAmount * CompressTrait::calculationTypeAmount * sizeof(CalculationType));

	for (SizeType i0 = 0; i0 < compressedAmount; ++i0)
	{
		CalculationType *q = controlData + (i0 * CompressTrait::calculationTypeAmount);
		CalculationType p[CompressTrait::calculationTypeAmount];

		for (SizeType i1 = 0; i1 < inputAmount; ++i1)
		{
			CompressTrait::getInput(*(inputData + i1), p);

			CalculationType xvalue = CalculationType(atMatrix[i0][i1]);
			for (SizeType j = 0; j < CompressTrait::calculationTypeAmount; ++j)
				q[j] += xvalue * p[j];
		}
	}

	// Set the first and last output control points to match the first and last input samples.  
	// This supports the application of fitting keyframe data with B-spline curves.  The user expects that the curve passes
	// through the first and last positions in order to support matching two consecutive keyframe sequences.
	CalculationType *c0 = controlData;
	CalculationType *c1 = &controlData[CompressTrait::calculationTypeAmount * (compressedAmount - 1)];
	CalculationType i0[CompressTrait::calculationTypeAmount];
	CompressTrait::getInput(*inputData, i0);
	CalculationType i1[CompressTrait::calculationTypeAmount];
	CompressTrait::getInput(*(inputData + (inputAmount - 1)), i1);
	for (SizeType j = 0; j < CompressTrait::calculationTypeAmount; ++j)
	{
		*c0++ = i0[j];
		*c1++ = i1[j];
	}

	// Finally, save the results
	StorageType *finalBuffer = new typename Trait::StorageType[Trait::storageComponentAmount * compressedAmount];
	CalculationType *b1 = controlData;
	StorageType *b2 = finalBuffer;
	DataType tmp;

	for (SizeType i = 0; i < compressedAmount; ++i)
	{
		CompressTrait::getInput(b1, tmp);
		Trait::storeValue(tmp, b2);

		b1 += CompressTrait::calculationTypeAmount;
		b2 += Trait::storageComponentAmount;
	}

	result.initialize(compressedAmount, finalBuffer, true, true);

	// And calculate error metric
	if (optionalErrorMetric)
	{
		CalculationType avrError = 0;
		CalculationType avrErrorMax = 0;
		CalculationType rmsError = 0;
		CalculationType rmsErrorMax = 0;

		CalculationType localErrorMax = 0;
		static const SizeType localErrorSampleAmount = 4;
		CalculationType localErrorSamples[localErrorSampleAmount] = { 0 };

		// Generate intermediate buffer
		typename IntermediateTrait::StorageType *tempBuffer = new typename IntermediateTrait::StorageType[IntermediateTrait::storageComponentAmount * compressedAmount];
		CalculationType *b1Temp = controlData;
		typename IntermediateTrait::StorageType *b2Temp = tempBuffer;
		typename IntermediateTrait::DataType temp;

		for (SizeType i = 0; i < compressedAmount; ++i)
		{
			CompressTrait::getInput(b1Temp, temp);
			IntermediateTrait::storeValue(temp, b2Temp);

			b1Temp += CompressTrait::calculationTypeAmount;
			b2Temp += IntermediateTrait::storageComponentAmount;
		}

		BSplineCurve<IntermediateTrait, degree, true> tempResult;
		tempResult.initialize(compressedAmount, tempBuffer, true, true);

		typename Trait::DataType s1;
		for (SizeType i = 0; i < inputAmount; ++i)
		{
			const typename CompressTrait::InputType &s2 = inputData[i];
			
			float t = float(i) / float(inputAmount - 1);
			tempResult.getValue(t, s1);

			CalculationType squareLength = CompressTrait::dot(s1, s2);

			avrError += squareLength;
			avrErrorMax = (squareLength > avrErrorMax) ? squareLength : avrErrorMax;

			CalculationType length = CalculationType(sqrt(squareLength));
			rmsError += length;
			rmsErrorMax = (length > rmsErrorMax) ? length : rmsErrorMax;

			// Local error

			// Set to new values
			for (SizeType j = 0; j < localErrorSampleAmount - 1; ++j)
				localErrorSamples[j] = localErrorSamples[j + 1];
			localErrorSamples[localErrorSampleAmount - 1] = rmsError;

			CalculationType currentLocalError = 0;
			for (SizeType j = 0; j < localErrorSampleAmount; ++j)
				currentLocalError += localErrorSamples[j];

			localErrorMax = (currentLocalError > localErrorMax) ? currentLocalError : localErrorMax;
		}

		rmsError /= CalculationType(inputAmount);
		avrError /= CalculationType(inputAmount);
		localErrorMax /= CalculationType(localErrorSampleAmount);

		// Normally max values are not taken into account, should we take average between max and avg error or such?
		// For the time being, just return something
		//*optionalErrorMetric = (rmsError + avrError) * CalculationType(0.5);
		//*optionalErrorMetric = (rmsError + avrError + avrErrorMax + rmsErrorMax) * CalculationType(0.25);
		//*optionalErrorMetric = (rmsError + rmsErrorMax) * CalculationType(0.5);
		//*optionalErrorMetric = (rmsError * CalculationType(0.8)) + (rmsErrorMax * CalculationType(0.2));
		//*optionalErrorMetric = rmsError;

		*optionalErrorMetric = (localErrorMax + rmsErrorMax) * CalculationType(0.5);
		//*optionalErrorMetric = localErrorMax;
	}

	// And free up temporary buffers
	delete[] controlData;
	delete[] atMatrix[0];
	delete[] atMatrix;
	delete ataMatrix;
}

// Expensive function, ment for preprocessing
template<typename Trait, typename CompressTrait, typename IntermediateTrait, int degree>
bool createBSplineCurve(SizeType inputAmount, const typename CompressTrait::InputType *input, BSplineCurve<Trait, degree, true> &result, typename CompressTrait::CalculationType allowedError)
{
	typedef typename CompressTrait::CalculationType CalculationType;

	bool hasResult = false;
	SizeType maxRange = inputAmount / 2;

	// Binary search for optimal amount of control points
	SizeType limit = (degree + 2) * 2;
	if (limit < 4)
		limit = 4;

	SizeType minRange = limit;
	if (maxRange < limit)
		maxRange = limit;

	CalculationType lastError = 0;
	while (maxRange - minRange > limit)
	{
		BSplineCurve<Trait, degree, true> tmp;
		SizeType controlPoints = (maxRange + minRange) / 2;

		CalculationType currentError = 0;
		createBSplineCurveManual<Trait, CompressTrait, IntermediateTrait, degree>(inputAmount, input, controlPoints, tmp, &currentError);

		SizeType delta = (maxRange - minRange) / 2;
		fb_assert(maxRange - delta >= minRange);
		fb_assert(minRange + delta <= maxRange);

		lastError = currentError;
		if (currentError > allowedError)
			minRange += delta;
		else
		{
			maxRange -= delta;
			hasResult = true;
			result = tmp;
		}
	}

	// Failsafe
	if (!hasResult)
	{
		//fb_assert(0 && "Curve generation failed?!");
		createBSplineCurveManual<Trait, CompressTrait, IntermediateTrait, degree>(inputAmount, input, maxRange, result);
	}

	//return hasResult;
	return true;
}

template<typename Trait, typename CompressTrait, int degree>
void createLinearCurve(SizeType inputAmount, const typename CompressTrait::InputType *inputData, BSplineCurve<Trait, degree, true> &result)
{
	typename Trait::StorageType *finalBuffer = new typename Trait::StorageType[Trait::storageComponentAmount * inputAmount];
	typename Trait::StorageType *b = finalBuffer;
	
	typename Trait::DataType tmp;
	for (SizeType i = 0; i < inputAmount; ++i)
	{
		Trait::storeValue(inputData[i], b);
		b += Trait::storageComponentAmount;
	}

	result.initialize(inputAmount, finalBuffer, true, true);
}

FB_END_PACKAGE2()

#endif
