#ifndef FB_MATH_CURVE_DETAIL_BANDEDMATRIX_H
#define FB_MATH_CURVE_DETAIL_BANDEDMATRIX_H

// Based on source code from WildTangent4 by Geometric Tools, LLC.

// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

FB_PACKAGE3(math, curve, detail)

template<typename T>
class BandedMatrix
{
public:
    BandedMatrix(SizeType size_, SizeType lbands_, SizeType ubands_)
	:	size(size_)
	,	lbands(lbands_)
	,	ubands(ubands_)
    ,	dband(0)
    ,	lband(0)
    ,	uband(0)
	{
		fb_assert(size > 0 && lbands >= 0 && ubands >= 0);
		fb_assert(lbands < size && ubands < size);

		allocate();
	}

    BandedMatrix(const BandedMatrix &other)
	:	size(0)
	,	lbands(0)
	,	ubands(0)
    ,	dband(0)
    ,	lband(0)
    ,	uband(0)
	{
		*this = other;
	}

    ~BandedMatrix ()
	{
		deallocate();
	}

    BandedMatrix &operator= (const BandedMatrix &other)
	{
		deallocate();
		size = other.size;
		lbands = other.lbands;
		ubands = other.ubands;
		allocate();

		SizeType uiSize = size * sizeof(T);
		memcpy(dband, other.dband, uiSize);

		for (SizeType i = 0; i < lbands; i++)
		{
			uiSize = (size - 1 - i) * sizeof(T);
			memcpy(lband[i], other.lband[i], uiSize);
		}

		for (SizeType i = 0; i < ubands; i++)
		{
			uiSize = (size - 1 - i) * sizeof(T);
			memcpy(uband[i], other.uband[i], uiSize);
		}

	    return *this;
	}

	SizeType getSize() const { return size; }
	SizeType getLBands() const { return lbands; }
	SizeType getUBands() const { return ubands; }

	T *getDBand() { return dband; }
    const T* getDBand () const { return dband; }

    int getLBandMax(SizeType i) const
	{
		fb_assert(i < lbands);
		return size - 1 - i;
	}
    T *getLBand(SizeType i)
	{
		if (lband)
		{
			fb_assert(i < lbands);
			return lband[i];
		}
		return 0;
	}
    const T *getLBand (SizeType i) const
	{
		if (lband)
		{
			fb_assert(i < lbands);
			return lband[i];
		}
		return 0;
	}

    int getUBandMax (SizeType i) const  // UBand(i):  0 <= index < UBandMax
	{
		fb_assert(i < ubands);
		return size - 1 - i;
	}
	T *getUBand (SizeType i)
	{
		if (uband)
		{
			fb_assert(i < ubands);
			return uband[i];
		}
		return 0;
	}
    const T *getUBand (SizeType i) const
	{
		if (lband)
		{
			fb_assert(i < ubands);
			return uband[i];
		}
		return 0;
	}

    T &operator() (SizeType row, SizeType col)
	{
		fb_assert(row < size && col < size);

		if (col > row)
		{
			SizeType band = col - row;
			if (--band < ubands && row < size - 1 - band)
				return uband[band][row];
		}
		else if (col < row)
		{
			SizeType band = row - col;
			if (--band < lbands && col < size - 1 - band)
				return lband[band][col];
		}
		else
		{
			return dband[row];
		}

		static T dummy = 0;
		return dummy;
	}
    T operator() (SizeType row, SizeType col) const
	{
		fb_assert(row < size && col < size);

		if (col > row)
		{
			SizeType band = col - row;
			if (--band < ubands && row < size - 1 - band)
				return uband[band][row];
		}
		else if (col < row)
		{
			SizeType band = row - col;
			if (--band < lbands && col < size - 1 - band)
				return lband[band][col];
		}
		else
		{
			return dband[row];
		}

		return 0;
	}

    void setZero()
	{
		lang::MemSet::set(dband, 0, size * sizeof(T));

		for (SizeType i = 0; i < lbands; ++i)
			lang::MemSet::set(lband[i], 0, (size-1-i) * sizeof(T));

		for (SizeType i = 0; i < ubands; ++i)
			lang::MemSet::set(uband[i], 0, (size-1-i) * sizeof(T));
	}
    void setIdentity()
	{
		setZero();

		for (SizeType i = 0; i < size; ++i)
			dband[i] = T(1);
	}

    // Factor the square banded matrix A into A = L*L^T, where L is a
    // lower-triangular matrix (L^T is an upper-triangular matrix).
    // This is an LU decomposition that allows for stable inversion
    // of A to solve A*X = B.  The return value is 'true' iff the
    // factorizing is successful (L is invertible).  If successful, A
    // contains the Cholesky factorization (L in the lower-triangular part
    // of A and/ L^T in the upper-triangular part of A).
    bool choleskyFactor()
	{
		fb_assert(lbands == ubands);
		if (lbands != ubands)
			return false;

		SizeType sizeM1 = size - 1;

		SizeType kMax = 0;
		for (SizeType i = 0; i < size; ++i)
		{
			SizeType jMin = lbands < i ? i - lbands : 0;

			SizeType j = 0;
			for (j = jMin; j < i; ++j)
			{
				kMax = j + lbands;
				if (kMax > sizeM1)
					kMax = sizeM1;

				for (SizeType k = i; k <= kMax; ++k)
					(*this)(k,i) -= (*this)(i,j) * (*this)(k,j);
			}

			kMax = j + lbands;
			if (kMax > sizeM1)
				kMax = sizeM1;

			for (SizeType k = 0; k < i; ++k)
				(*this)(k,i) = (*this)(i,k);

			T diagonal = (*this)(i,i);
			if (diagonal <= 0)
				return false;

			/*
			T fInvSqrt = Math<Real>::InvSqrt(fDiagonal);
			for (k = i; k <= kMax; k++)
			{
				(*this)(k,i) *= fInvSqrt;
			}
			*/

			T root = T(sqrt(diagonal));
			T invSqrt = T(1) / root;
			for (SizeType k = i; k <= kMax; ++k)
			{
				(*this)(k,i) *= invSqrt;
			}
		}

		return true;

	}

    // Solve the linear system A*X = B, where A is an NxN banded matrix and
    // B is an Nx1 vector.  The unknown X is also Nx1.  The input to this
    // function is B.  The output X is computed and stored in B.  The return
    // value is 'true' iff the system has a solution.  The matrix A and the
    // vector B are both modified by this function.  If successful, A contains
    // the Cholesky factorization (L in the lower-triangular part of A and
    // L^T in the upper-triangular part of A).
    bool solveSystem(T *b)
	{
		return choleskyFactor() && solveLower(b) && solveUpper(b);
	}

    // Solve the linear system A*X = B, where A is an NxN banded matrix and
    // B is an NxM matrix.  The unknown X is also NxM.  The input to this
    // function is B.  The output X is computed and stored in B.  The return
    // value is 'true' iff the system has a solution.  The matrix A and the
    // vector B are both modified by this function.  If successful, A contains
    // the Cholesky factorization (L in the lower-triangular part of A and
    // L^T in the upper-triangular part of A).
    bool solveSystem(T **b, SizeType numColumns)
	{
		return choleskyFactor() && solveLower(b, numColumns) && solveUpper(b, numColumns);
	}

private:
    // The linear system is L*U*X = B, where A = L*U and U = L^T,  Reduce this
    // to U*X = L^{-1}*B.  The return value is 'true' iff the operation is
    // successful.
    bool solveLower(T *data) const
	{
		for (SizeType row = 0; row < size; ++row)
		{
			T lowerRR = (*this)(row,row);
			if (abs(lowerRR) < T(0.00001))
				return false;

			for (SizeType col = 0; col < row; ++col)
			{
				T lowerRC = (*this)(row, col);
				data[row] -= lowerRC * data[col];
			}

			data[row] /= lowerRR;
		}

		return true;
	}

    // The linear system is U*X = L^{-1}*B.  Reduce this to
    // X = U^{-1}*L^{-1}*B.  The return value is 'true' iff the operation is
    // successful.
    bool solveUpper (T *data) const
	{
		for (SizeType row = size - 1; row < size; --row)
		{
			T upperRR = (*this)(row, row);
			if (abs(upperRR) < T(0.00001))
				return false;

			for (SizeType col = row + 1; col < size; ++col)
			{
				T upperRC = (*this)(row, col);
				data[row] -= upperRC * data[col];
			}

			data[row] /= upperRR;
		}

	    return true;
	}

    // The linear system is L*U*X = B, where A = L*U and U = L^T,  Reduce this
    // to U*X = L^{-1}*B.  The return value is 'true' iff the operation is
    // successful.
    bool solveLower(T **data, SizeType numColumns) const
	{
		for (SizeType row = 0; row < size; ++row)
		{
			T lowerRR = (*this)(row, row);
			if (fabs(lowerRR) < T(0.00001))
				return false;

			for (SizeType col = 0; col < row; ++col)
			{
				T lowerRC = (*this)(row, col);
				for (SizeType bcol = 0; bcol < numColumns; ++bcol)
					data[row][bcol] -= lowerRC * data[col][bcol];
			}

			T inverse = T(1) / lowerRR;
			for (SizeType bcol = 0; bcol < numColumns; ++bcol)
				data[row][bcol] *= inverse;
		}

		return true;
	}

    // The linear system is U*X = L^{-1}*B.  Reduce this to
    // X = U^{-1}*L^{-1}*B.  The return value is 'true' iff the operation is
    // successful.
    bool solveUpper(T **data, SizeType numColumns) const
	{
		for (SizeType row = size - 1; row < size; --row)
		{
			T upperRR = (*this)(row, row);
			if (fabs(upperRR) < T(0.00001))
				return false;

			for (SizeType col = row + 1; col < size; ++col)
			{
				T upperRC = (*this)(row,col);
				for (SizeType bcol = 0; bcol < numColumns; bcol++)
					data[row][bcol] -= upperRC * data[col][bcol];
			}

			T inverse = T(1) / upperRR;
			for (SizeType bcol = 0; bcol < numColumns; ++bcol)
				data[row][bcol] *= inverse;
		}

		return true;
	}

    void allocate()
	{
		dband = new T[size];
		lang::MemSet::set(dband, 0, size * sizeof(T));

		if (lbands > 0)
			lband = new T* [lbands];
		else
			lband = nullptr;

		if (ubands > 0)
			uband = new T* [ubands];
		else
			uband = nullptr;

		for (SizeType i = 0; i < lbands; ++i)
		{
			lband[i] = new T[size - 1 - i];
			lang::MemSet::set(lband[i], 0, (size-1-i) * sizeof(T));
		}

		for (SizeType i = 0; i < ubands; ++i)
		{
			uband[i] = new T[size - 1 - i];
			lang::MemSet::set(uband[i], 0, (size-1-i) * sizeof(T));
		}
	}

    void deallocate()
	{
		delete[] dband;

		if (lband)
		{
			for (SizeType i = 0; i < lbands; ++i)
				delete[] lband[i];

			delete[] lband;
			lband = 0;
		}

		if (uband)
		{
			for (SizeType i = 0; i < ubands; ++i)
				delete[] uband[i];

			delete[] uband;
			uband = 0;
		}
	}

    SizeType size;
	SizeType lbands;
	SizeType ubands;
    T *dband;
    T **lband;
    T **uband;
};

FB_END_PACKAGE3()

#endif
