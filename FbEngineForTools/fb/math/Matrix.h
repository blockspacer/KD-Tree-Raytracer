#ifndef FB_MATH_MATRIX_H
#define FB_MATH_MATRIX_H

#include "Quaternion.h"
#include "Vec3.h"
#include "Vec4.h"
#include "fb/math/util/IsFinite.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/platform/ForceInline.h"
#include "fb/lang/platform/Sel.h"

FB_PACKAGE1(math)

//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class TMatrix;
template <class A> class TMatrix44;
typedef TMatrix<float> MAT;
typedef TMatrix44<float> MAT44;

//------------------------------------------------------------------
// Matrix
//------------------------------------------------------------------
template <class A> class TMatrix
{
	A mat[16];

public:

	static const TMatrix identity;

	// Create...

	// Creates ID-matrix
	FB_FORCEINLINE TMatrix()
	{ 
		// Leaving data uninitialized and not cheking those on GetTranslation() etc ..
		// That's BAD!
		//	-- psd

		mat[0] = 1;
		mat[1] = 0;
		mat[2] = 0;
		mat[3] = 0;

		mat[4] = 0;
		mat[5] = 1;
		mat[6] = 0;
		mat[7] = 0;
	
		mat[8] = 0;
		mat[9] = 0;
		mat[10] = 1;
		mat[11] = 0;
		
		mat[12] = 0;
		mat[13] = 0;
		mat[14] = 0;
		mat[15] = 1;
	}

	// Creates matrix from array
	FB_FORCEINLINE explicit TMatrix(const A *f16)
	{
		lang::MemCopy::copy(mat, f16, sizeof(A) * 16);
	}

	FB_FORCEINLINE TMatrix(const Vec3<A> &translation, const Quaternion<A> &rotation)
	{
		createRotationMatrix(rotation);
		mat[12] = translation.x;
		mat[13] = translation.y;
		mat[14] = translation.z;
	}

	FB_FORCEINLINE TMatrix(const Vec3<A> &translation, const Quaternion<A> &rotation, const Vec3<A> &scale)
	{
		math::VC3 axes[3];
		rotation.getAxes(axes);

		mat[0] = axes[0].x*scale.x;
		mat[1] = axes[0].y*scale.x;
		mat[2] = axes[0].z*scale.x;
		mat[3] = 0.0f;
		
		mat[4] = axes[1].x*scale.y;
		mat[5] = axes[1].y*scale.y;
		mat[6] = axes[1].z*scale.y;
		mat[7] = 0.0f;
		
		mat[8] = axes[2].x*scale.z;
		mat[9] = axes[2].y*scale.z;
		mat[10] = axes[2].z*scale.z;
		mat[11] = 0.0f;

		mat[12] = translation.x;
		mat[13] = translation.y;
		mat[14] = translation.z;
		mat[15] = 1.0f;
	}
	
	FB_FORCEINLINE TMatrix(const Vec3<A> &row0, const Vec3<A> &row1, const Vec3<A> &row2, const Vec3<A> &row3)
	{
		mat[0] = row0.x;
		mat[1] = row0.y;
		mat[2] = row0.z;
		mat[3] = 0.0f;
		
		mat[4] = row1.x;
		mat[5] = row1.y;
		mat[6] = row1.z;
		mat[7] = 0.0f;
		
		mat[8] = row2.x;
		mat[9] = row2.y;
		mat[10] = row2.z;
		mat[11] = 0.0f;

		mat[12] = row3.x;
		mat[13] = row3.y;
		mat[14] = row3.z;
		mat[15] = 1.0f;
	}
	
	FB_FORCEINLINE TMatrix(A _0, A _1, A _2, A _3, A _4, A _5, A _6, A _7, A _8, A _9, A _10, A _11, A _12, A _13, A _14, A _15)
	{
		mat[0] = _0;
		mat[1] = _1;
		mat[2] = _2;
		mat[3] = _3;
		
		mat[4] = _4;
		mat[5] = _5;
		mat[6] = _6;
		mat[7] = _7;
		
		mat[8] = _8;
		mat[9] = _9;
		mat[10] = _10;
		mat[11] = _11;

		mat[12] = _12;
		mat[13] = _13;
		mat[14] = _14;
		mat[15] = _15;
	}

	// Creates identity matrix
	void createIdentityMatrix()
	{
		mat[0] = 1;
		mat[1] = 0;
		mat[2] = 0;
		mat[3] = 0;

		mat[4] = 0;
		mat[5] = 1;
		mat[6] = 0;
		mat[7] = 0;
	
		mat[8] = 0;
		mat[9] = 0;
		mat[10] = 1;
		mat[11] = 0;
		
		mat[12] = 0;
		mat[13] = 0;
		mat[14] = 0;
		mat[15] = 1;
	}

	// Creates translation(move) matrix
	void createTranslationMatrix(const Vec3<A> &translation)
	{
		mat[1]=mat[2]=mat[3]=0;
		mat[4]=mat[6]=mat[7]=0;
		mat[8]=mat[9]=mat[11]=0;
		mat[0]=mat[5]=mat[10]=mat[15]=(A)1;

		mat[12]=translation.x;
		mat[13]=translation.y;
		mat[14]=translation.z;
	}

	// Creates scale matrix
	void createScaleMatrix(const Vec3<A> &scale)
	{
		mat[1]=mat[2]=mat[3]=0.0f;
		mat[4]=mat[6]=mat[7]=0.0f;
		mat[8]=mat[9]=mat[11]=0.0f;
		mat[12]=mat[13]=mat[14]=0.0f;
		mat[15]=1.0f;
		
		mat[0]=scale.x;
		mat[5]=scale.y;
		mat[10]=scale.z;
	}
	
	void setAxes(const Vec3<A> (&axes)[3])
	{
		mat[0] = axes[0].x;
		mat[1] = axes[0].y;
		mat[2] = axes[0].z;
		
		mat[4] = axes[1].x;
		mat[5] = axes[1].y;
		mat[6] = axes[1].z;
		
		mat[8] = axes[2].x;
		mat[9] = axes[2].y;
		mat[10] = axes[2].z;
	}

	void setAxes(const Quaternion<A> &rotation)
	{
		Vec3<A> axes[3];
		rotation.getAxes(axes);
		setAxes(axes);
	}
	
	// Creates rotation matrix
	void createRotationMatrix(const Quaternion<A> &rotation)
	{
		setAxes(rotation);
		mat[3]=mat[7]=mat[11]=0;
		mat[12]=mat[13]=mat[14]=0;
		mat[15]=(A)1;
	}
	
	// Creates shear matrix
	void createShearMatrix(const Vec3<A> &shearX, const Vec3<A> &shearY, const Vec3<A> &shearZ)
	{
		mat[0] = shearX.x;
		mat[1] = shearX.y;
		mat[2] = shearX.z;
		mat[3] = 0.0f;

		mat[4] = shearY.x;
		mat[5] = shearY.y;
		mat[6] = shearY.z;
		mat[7] = 0.0f;

		mat[8] = shearZ.x;
		mat[9] = shearZ.y;
		mat[10] = shearZ.z;
		mat[11] = 0.0f;

		mat[12] = mat[13] = mat[14] = 0.0f;
		mat[15] = 1.0f;
	}

	// Creates camera matrix
	void createCameraMatrix(const Vec3<A> &position,const Vec3<A> &target,const Vec3<A> &up)
	{
		// Calculate direction (Z) vector
		Vec3<A> z=target-position;
		// Normalize direction vector
		z.normalize();

		// Calculate real up vector (Y)
		A pdu=up.getDotWith(z);
		Vec3<A> y=up-z*pdu;
		// Normalize the y vector
		y.normalize();

		createCameraMatrixWithDirUp(position, z, y);
	}

	// Use direction and real up vectors directly
	void createCameraMatrixWithDirUp(const Vec3<A> &position,const Vec3<A> &dir,const Vec3<A> &up)
	{
		Vec3<A> z = dir;
		z.normalize();
		Vec3<A> y = up;
		y.normalize();

		// Calculate x vector with cross product
		Vec3<A> x=y.getCrossWith(z);

		// Build matrix (rotation part)
		mat[0]=x.x;	mat[1]=y.x;	mat[2]=z.x;
		mat[4]=x.y;	mat[5]=y.y;	mat[6]=z.y;
		mat[8]=x.z;	mat[9]=y.z;	mat[10]=z.z;

		// Build matrix (translation part)
		mat[12]=-position.getDotWith(x);
		mat[13]=-position.getDotWith(y);
		mat[14]=-position.getDotWith(z);

		// Last
		mat[3]=0;
		mat[7]=0;
		mat[11]=0;
		mat[15]=(A)1;
	}

	// Creates base change matrix
	void createBaseChangeMatrix(const Vec3<A> &base_x,const Vec3<A> &base_y,const Vec3<A> &base_z)
	{
		// Create the matrix
		mat[0]=base_x.x;
		mat[1]=base_y.x;
		mat[2]=base_z.x;
	
		mat[4]=base_x.y;
		mat[5]=base_y.y;
		mat[6]=base_z.y;
	
		mat[8]=base_x.z;
		mat[9]=base_y.z;
		mat[10]=base_z.z; 

		mat[3]=mat[7]=mat[11]=0;
		mat[12]=mat[13]=mat[14]=0;
		mat[15]=1;
	}

	// Functions (these do not modify this matrix)...

	TMatrix<A> getInverse() const
	{
			A mat_inv[16] = { 0 };

			A fDetInv = (A)1 / ( mat[0] * ( mat[5] * mat[10] - mat[6] * mat[9] ) -
								 mat[1] * ( mat[4] * mat[10] - mat[6] * mat[8] ) +
								 mat[2] * ( mat[4] * mat[9] - mat[5] * mat[8] ) );

			// possible division by zero here
			fb_assert(math::util::isFinite(fDetInv));

			mat_inv[0] =  fDetInv * ( mat[5] * mat[10] - mat[6] * mat[9] );
			mat_inv[1] = -fDetInv * ( mat[1] * mat[10] - mat[2] * mat[9] );
			mat_inv[2] =  fDetInv * ( mat[1] * mat[6] - mat[2] * mat[5] );
			mat_inv[3] = 0;

			mat_inv[4] = -fDetInv * ( mat[4] * mat[10] - mat[6] * mat[8] );
			mat_inv[5] =  fDetInv * ( mat[0] * mat[10] - mat[2] * mat[8] );
			mat_inv[6] = -fDetInv * ( mat[0] * mat[6] - mat[2] * mat[4] );
			mat_inv[7] = 0;

			mat_inv[8] =  fDetInv * ( mat[4] * mat[9] - mat[5] * mat[8] );
			mat_inv[9] = -fDetInv * ( mat[0] * mat[9] - mat[1] * mat[8] );
			mat_inv[10] =  fDetInv * ( mat[0] * mat[5] - mat[1] * mat[4] );
			mat_inv[11] = 0;

			mat_inv[12] = -( mat[12] * mat_inv[0] + mat[13] * mat_inv[4] + mat[14] * mat_inv[8] );
			mat_inv[13] = -( mat[12] * mat_inv[1] + mat[13] * mat_inv[5] + mat[14] * mat_inv[9] );
			mat_inv[14] = -( mat[12] * mat_inv[2] + mat[13] * mat_inv[6] + mat[14] * mat_inv[10] );
			mat_inv[15] = (A)1;

		return TMatrix<A>(mat_inv);
	}

	TMatrix<A> getTranspose() const
	{
		A mat_t[16];

		// Transpose it
		mat_t[0 + 0*4] = mat[0 + 0*4];
		mat_t[1 + 0*4] = mat[0 + 1*4];
		mat_t[2 + 0*4] = mat[0 + 2*4];
		mat_t[3 + 0*4] = mat[0 + 3*4];

		mat_t[0 + 1*4] = mat[1 + 0*4];
		mat_t[1 + 1*4] = mat[1 + 1*4];
		mat_t[2 + 1*4] = mat[1 + 2*4];
		mat_t[3 + 1*4] = mat[1 + 3*4];

		mat_t[0 + 2*4] = mat[2 + 0*4];
		mat_t[1 + 2*4] = mat[2 + 1*4];
		mat_t[2 + 2*4] = mat[2 + 2*4];
		mat_t[3 + 2*4] = mat[2 + 3*4];

		mat_t[0 + 3*4] = mat[3 + 0*4];
		mat_t[1 + 3*4] = mat[3 + 1*4];
		mat_t[2 + 3*4] = mat[3 + 2*4];
		mat_t[3 + 3*4] = mat[3 + 3*4];

		return TMatrix<A>(mat_t);
	}

	TMatrix<A> getWithoutTranslation() const
	{
		TMatrix m(mat);

		// Remove translation
		m.mat[12]=0;
		m.mat[13]=0;
		m.mat[14]=0;

		return m;
	}

	// Functions (these modify this matrix)...
	inline void inverse()
	{
		A mat_inv[16] = { 0 };

		// Copy matrix to inverse
		lang::MemCopy::copy(mat_inv,mat,sizeof(A)*16);

		// Make new matrix (inversed)
		A fDetInv = (A)1 / ( mat_inv[0] * ( mat_inv[5] * mat_inv[10] - mat_inv[6] * mat_inv[9] ) -
								 mat_inv[1] * ( mat_inv[4] * mat_inv[10] - mat_inv[6] * mat_inv[8] ) +
								 mat_inv[2] * ( mat_inv[4] * mat_inv[9] - mat_inv[5] * mat_inv[8] ) );

		// possible division by zero here
		fb_assert(math::util::isFinite(fDetInv));

		mat[0] =  fDetInv * ( mat_inv[5] * mat_inv[10] - mat_inv[6] * mat_inv[9] );
		mat[1] = -fDetInv * ( mat_inv[1] * mat_inv[10] - mat_inv[2] * mat_inv[9] );
		mat[2] =  fDetInv * ( mat_inv[1] * mat_inv[6] - mat_inv[2] * mat_inv[5] );
		mat[3] = 0;

		mat[4] = -fDetInv * ( mat_inv[4] * mat_inv[10] - mat_inv[6] * mat_inv[8] );
		mat[5] =  fDetInv * ( mat_inv[0] * mat_inv[10] - mat_inv[2] * mat_inv[8] );
		mat[6] = -fDetInv * ( mat_inv[0] * mat_inv[6] - mat_inv[2] * mat_inv[4] );
		mat[7] = 0;

		mat[8] =  fDetInv * ( mat_inv[4] * mat_inv[9] - mat_inv[5] * mat_inv[8] );
		mat[9] = -fDetInv * ( mat_inv[0] * mat_inv[9] - mat_inv[1] * mat_inv[8] );
		mat[10] =  fDetInv * ( mat_inv[0] * mat_inv[5] - mat_inv[1] * mat_inv[4] );
		mat[11] = 0;

		mat[12] = -( mat_inv[12] * mat[0] + mat_inv[13] * mat[4] + mat_inv[14] * mat[8] );
		mat[13] = -( mat_inv[12] * mat[1] + mat_inv[13] * mat[5] + mat_inv[14] * mat[9] );
		mat[14] = -( mat_inv[12] * mat[2] + mat_inv[13] * mat[6] + mat_inv[14] * mat[10] );
		mat[15] = (A)1;
	}

	void multiply(const TMatrix<A>& other)
	{
		A *tmat = this->mat;

		float tmpMat0 = mat[0];
		tmat[0] = mat[0] * other.mat[0] + mat[1] * other.mat[4] + mat[2] * other.mat[8];
		float tmpMat1 = mat[1];
		tmat[1] = tmpMat0 * other.mat[1] + mat[1] * other.mat[5] + mat[2] * other.mat[9];
		float tmpMat2 = mat[2];
		tmat[2] = tmpMat0 * other.mat[2] + tmpMat1 * other.mat[6] + tmpMat2 * other.mat[10];
		tmat[3] = 0;

		float tmpMat4 = mat[4];
		tmat[4] = mat[4] * other.mat[0] + mat[5] * other.mat[4] + mat[6] * other.mat[8];
		float tmpMat5 = mat[5];
		tmat[5] = tmpMat4 * other.mat[1] + mat[5] * other.mat[5] + mat[6] * other.mat[9];
		tmat[6] = tmpMat4 * other.mat[2] + tmpMat5 * other.mat[6] + mat[6] * other.mat[10];
		tmat[7] = 0;

		float tmpMat8 = mat[8];
		tmat[8] = mat[8] * other.mat[0] + mat[9] * other.mat[4] + mat[10] * other.mat[8];
		float tmpMat9 = mat[9];
		tmat[9] = tmpMat8 * other.mat[1] + mat[9] * other.mat[5] + mat[10] * other.mat[9];
		tmat[10] = tmpMat8 * other.mat[2] + tmpMat9 * other.mat[6] + mat[10] * other.mat[10];
		tmat[11] = 0;

		float tmpMat12 = mat[12];
		tmat[12] = mat[12] * other.mat[0] + mat[13] * other.mat[4] + mat[14] * other.mat[8] + other.mat[12];
		float tmpMat13 = mat[13];
		tmat[13] = tmpMat12 * other.mat[1] + mat[13] * other.mat[5] + mat[14] * other.mat[9] + other.mat[13];
		tmat[14] = tmpMat12 * other.mat[2] + tmpMat13 * other.mat[6] + mat[14] * other.mat[10] + other.mat[14];
		tmat[15] = 1;
	}


	// Operators...

	TMatrix<A> operator*(const TMatrix<A>& other) const
	{
		TMatrix<A> result;
		A *tmat = result.mat;

		tmat[0] = mat[0] * other.mat[0] + mat[1] * other.mat[4] + mat[2] * other.mat[8];
		tmat[1] = mat[0] * other.mat[1] + mat[1] * other.mat[5] + mat[2] * other.mat[9];
		tmat[2] = mat[0] * other.mat[2] + mat[1] * other.mat[6] + mat[2] * other.mat[10];
		tmat[3] = 0;

		tmat[4] = mat[4] * other.mat[0] + mat[5] * other.mat[4] + mat[6] * other.mat[8];
		tmat[5] = mat[4] * other.mat[1] + mat[5] * other.mat[5] + mat[6] * other.mat[9];
		tmat[6] = mat[4] * other.mat[2] + mat[5] * other.mat[6] + mat[6] * other.mat[10];
		tmat[7] = 0;

		tmat[8] = mat[8] * other.mat[0] + mat[9] * other.mat[4] + mat[10] * other.mat[8];
		tmat[9] = mat[8] * other.mat[1] + mat[9] * other.mat[5] + mat[10] * other.mat[9];
		tmat[10] = mat[8] * other.mat[2] + mat[9] * other.mat[6] + mat[10] * other.mat[10];
		tmat[11] = 0;

		tmat[12] = mat[12] * other.mat[0] + mat[13] * other.mat[4] + mat[14] * other.mat[8] + other.mat[12];
		tmat[13] = mat[12] * other.mat[1] + mat[13] * other.mat[5] + mat[14] * other.mat[9] + other.mat[13];
		tmat[14] = mat[12] * other.mat[2] + mat[13] * other.mat[6] + mat[14] * other.mat[10] + other.mat[14];
		tmat[15] = (A)1;

		return result;
	}

	// Vector modify...
	void transformVector(Vec3<A> &vec) const
	{
		// Transform as 4x3
		A tmp_x=vec.x*mat[0]+vec.y*mat[4]+vec.z*mat[8]+mat[12];
		A tmp_y=vec.x*mat[1]+vec.y*mat[5]+vec.z*mat[9]+mat[13];
		A tmp_z=vec.x*mat[2]+vec.y*mat[6]+vec.z*mat[10]+mat[14];

		vec.x=tmp_x;
		vec.y=tmp_y;
		vec.z=tmp_z;
	}

	// psd
	void rotateVector(Vec3<A> &vec) const
	{
		// Transform as 4x3
		A tmp_x=vec.x*mat[0]+vec.y*mat[4]+vec.z*mat[8];
		A tmp_y=vec.x*mat[1]+vec.y*mat[5]+vec.z*mat[9];
		A tmp_z=vec.x*mat[2]+vec.y*mat[6]+vec.z*mat[10];

		vec.x=tmp_x;
		vec.y=tmp_y;
		vec.z=tmp_z;
	}

	Vec3<A> getTransformedVector(const Vec3<A> &vec) const
	{
		Vec3<A> temp=vec;
		transformVector(temp);
		return temp;
	}

	Vec3<A> getTransformed(const Vec3<A> &vec) const
	{
		return getTransformedVector(vec);
	}

	Vec3<A> getRotated(const Vec3<A> &vec) const
	{
		Vec3<A> temp=vec;
		rotateVector(temp);
		return temp;
	}

	void getAsD3DCompatible4x4(A *dest) const
	{
		for (int i = 0; i < 16; ++i)
			dest[i] = mat[i];
	}

	// Needed for exporters
	// -- psd
	A get(int index) const
	{
		fb_assert((index >= 0) && (index < 16));
		return mat[index];
	}
	
	void set(int index, A value)
	{
		fb_expensive_assert((index >= 0) && (index < 16));
		mat[index] = value;
	}
	
	void setTranslation(const Vec3<A> &t)
	{
		mat[12] = t.x;
		mat[13] = t.y;
		mat[14] = t.z;
	}

	Quaternion<A> getRotation() const
	{
		A sqScaleX = A(mat[0]*mat[0] + mat[1]*mat[1] + mat[2]*mat[2]);
		A sqScaleY = A(mat[4]*mat[4] + mat[5]*mat[5] + mat[6]*mat[6]);
		A sqScaleZ = A(mat[8]*mat[8] + mat[9]*mat[9] + mat[10]*mat[10]);

		A epsilon = (A)0.00001;
		// invScaleX = sqrScaleX >= epsilon ? 1.0 / sqrt(sqrScaleX) : 1.0
		A invScaleX = (A)FB_FSEL(sqScaleX - epsilon, (A)1.0 / sqrt(sqScaleX), (A)1.0);
		A invScaleY = (A)FB_FSEL(sqScaleY - epsilon, (A)1.0 / sqrt(sqScaleY), (A)1.0);
		A invScaleZ = (A)FB_FSEL(sqScaleZ - epsilon, (A)1.0 / sqrt(sqScaleZ), (A)1.0);

		A mat2[16] = 
		{
			mat[0] * invScaleX, mat[1] * invScaleX, mat[2] * invScaleX, mat[3],
			mat[4] * invScaleY, mat[5] * invScaleY, mat[6] * invScaleY, mat[7],
			mat[8] * invScaleZ, mat[9] * invScaleZ, mat[10] * invScaleZ, mat[11],
			mat[12], mat[13], mat[14], mat[15],
		};

		TMatrix m2(mat2);
		return m2.getRotationFromNonScaledMatrix();
	}

	Quaternion<A> getRotationFromNonScaledMatrix() const
	{
		Quaternion<A> result;

		// check the diagonal
		A tr = mat[0] + mat[5] + mat[10];
		if (tr > 0.0) 
		{
			A s = A(sqrt(tr + 1.0f));
			result.w = s / 2.0f;
			s = 0.5f / s;
			
			result.x = (mat[9] - mat[6]) * s;
			result.y = (mat[2] - mat[8]) * s;
			result.z = (mat[4] - mat[1]) * s;
		} 
		else 
		{
			// diagonal is negative
			A q[4] = { 0 };
			int nxt[3] = { 1, 2, 0 };
			int i = 0;
			
			if (mat[5] > mat[0])
				i = 1;
			if (mat[10] > mat[i*4 + i])
				i = 2;
			
			int j = nxt[i];
			int k = nxt[j];

			A s = A(sqrt((mat[i*4+i] - (mat[j*4+j] + mat[k*4+k])) + A(1.0)));
			q[i] = s * 0.5f;
			
			if (s != 0) 
				s = 0.5f / s;

			q[3] = (mat[k*4+j] - mat[j*4+k]) * s;
			q[j] = (mat[j*4+i] + mat[i*4+j]) * s;
			q[k] = (mat[k*4+i] + mat[i*4+k]) * s;

			result.x = q[0];
			result.y = q[1];
			result.z = q[2];
			result.w = q[3];
		}

		result.normalizeWithZeroFailsafe(Quaternion<A>::identity);
		return result;
	}

	Vec3<A> getTranslation() const
	{
		return Vec3<A> (mat[12], mat[13], mat[14]);
	}

	Vec3<A> getScale() const
	{
		A x = A(sqrt(mat[0]*mat[0] + mat[1]*mat[1] + mat[2]*mat[2]));
		A y = A(sqrt(mat[4]*mat[4] + mat[5]*mat[5] + mat[6]*mat[6]));
		A z = A(sqrt(mat[8]*mat[8] + mat[9]*mat[9] + mat[10]*mat[10]));

		return Vec3<A>(x,y,z);
	}

	const A *getAsFloat() const
	{
		return mat;
	}

	void getAxes(Vec3<A> (&axes)[3]) const
	{
		axes[0] = Vec3<A>(mat[0], mat[1], mat[2]);
		axes[1] = Vec3<A>(mat[4], mat[5], mat[6]);
		axes[2] = Vec3<A>(mat[8], mat[9], mat[10]);
	}
};

//------------------------------------------------------------------
// 4 x 4 Matrix
//------------------------------------------------------------------
template <class A> class TMatrix44
{
public:

	union {
		struct {
			A _11, _12, _13, _14;
			A _21, _22, _23, _24;
			A _31, _32, _33, _34;
			A _41, _42, _43, _44;
		};
		A m[4][4];
		A raw[16];
	};

//public:

	//! Default constructor
	FB_FORCEINLINE TMatrix44() :
		_11(1), _12(0), _13(0), _14(0),
		_21(0), _22(1), _23(0), _24(0),
		_31(0), _32(0), _33(1), _34(0),
		_41(0), _42(0), _43(0), _44(1)
	{}

	//! Constructor
	FB_FORCEINLINE TMatrix44(A __11, A __12, A __13, A __14,
			  A __21, A __22, A __23, A __24,
			  A __31, A __32, A __33, A __34,
			  A __41, A __42, A __43, A __44)
	{
		_11 = __11; _12 = __12; _13 = __13; _14 = __14;
		_21 = __21; _22 = __22; _23 = __23; _24 = __24;
		_31 = __31; _32 = __32; _33 = __33; _34 = __34;
		_41 = __41; _42 = __42; _43 = __43; _44 = __44;
	}

	//! Constructor
	FB_FORCEINLINE TMatrix44(const MAT &mat)
	{
		// FFS .. who the hell decided to transpose matrix to different convention,
		// even though MAT and MAT44 are supposed to be on ON SAME ONE!!!
		// - j

		/*
		_11 = mat.Get(0); _12 = mat.Get(4); _13 = mat.Get(8);  _14 = mat.Get(12);
		_21 = mat.Get(1); _22 = mat.Get(5); _23 = mat.Get(9);  _24 = mat.Get(13);
		_31 = mat.Get(2); _32 = mat.Get(6); _33 = mat.Get(10); _34 = mat.Get(14);
		_41 = mat.Get(3); _42 = mat.Get(7); _43 = mat.Get(11); _44 = mat.Get(15);
		*/

		_11 = mat.get(0); _12 = mat.get(1); _13 = mat.get(2);  _14 = mat.get(3);
		_21 = mat.get(4); _22 = mat.get(5); _23 = mat.get(6);  _24 = mat.get(7);
		_31 = mat.get(8); _32 = mat.get(9); _33 = mat.get(10); _34 = mat.get(11);
		_41 = mat.get(12); _42 = mat.get(13); _43 = mat.get(14); _44 = mat.get(15);

	}

	//! Constructor
	FB_FORCEINLINE TMatrix44(const A *f16)
	{
		lang::MemCopy::copy(raw, f16, sizeof(A)*16);
	}

	//! Operator []
	A operator[](int i) const
	{
		int row = i / 4;
		int col = i % 4;
		return this->m[row][col];
	}

	//! Operator ()
	A &operator()(int row, int col)
	{
		return m[row][col];
	}

	//! Get matrix contents as an array
	/*!
		\return matrix
	*/
	const A* getAsFloat() const
	{
		return raw;
	}

	//! Operator *
	TMatrix44<A> operator*(const TMatrix44<A> &other) const {
		TMatrix44<A> n;

		n._11 = _11 * other._11 + _12 * other._21 + _13 * other._31 + _14 * other._41;
		n._12 = _11 * other._12 + _12 * other._22 + _13 * other._32 + _14 * other._42;
		n._13 = _11 * other._13 + _12 * other._23 + _13 * other._33 + _14 * other._43;
		n._14 = _11 * other._14 + _12 * other._24 + _13 * other._34 + _14 * other._44;

		n._21 = _21 * other._11 + _22 * other._21 + _23 * other._31 + _24 * other._41;
		n._22 = _21 * other._12 + _22 * other._22 + _23 * other._32 + _24 * other._42;
		n._23 = _21 * other._13 + _22 * other._23 + _23 * other._33 + _24 * other._43;
		n._24 = _21 * other._14 + _22 * other._24 + _23 * other._34 + _24 * other._44;

		n._31 = _31 * other._11 + _32 * other._21 + _33 * other._31 + _34 * other._41;
		n._32 = _31 * other._12 + _32 * other._22 + _33 * other._32 + _34 * other._42;
		n._33 = _31 * other._13 + _32 * other._23 + _33 * other._33 + _34 * other._43;
		n._34 = _31 * other._14 + _32 * other._24 + _33 * other._34 + _34 * other._44;

		n._41 = _41 * other._11 + _42 * other._21 + _43 * other._31 + _44 * other._41;
		n._42 = _41 * other._12 + _42 * other._22 + _43 * other._32 + _44 * other._42;
		n._43 = _41 * other._13 + _42 * other._23 + _43 * other._33 + _44 * other._43;
		n._44 = _41 * other._14 + _42 * other._24 + _43 * other._34 + _44 * other._44;

		return n;
	}

	//! Creates identity matrix
	void createIdentity()
	{
		_11 = 1;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = 1;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = 1;
		_34 = 0;

		_41 = 0;
		_42 = 0;
		_43 = 0;
		_44 = 1;
	}

	// FFS!
	// GetVec3Transform() and GetVec4Transform() are assuming CM instead of RM 
	// which all other functions are using. /facepalm to whoever made these.
	// Left the retarded defaults named as FooTransposed().
	// - j

	//! Transform vector (x, y, z, 1) assuming a transposed matrix
	/*!
		\param in three-dimensional vector to transform
		\return four-dimensional transformed vector
	*/
	Vec4<A> getVec3TransformTransposed(const Vec3<A> &in) const
	{
		Vec4<A> out;
		out.x = _11 * in.x + _12 * in.y + _13 * in.z + _14 * 1.0f;
		out.y = _21 * in.x + _22 * in.y + _23 * in.z + _24 * 1.0f;
		out.z = _31 * in.x + _32 * in.y + _33 * in.z + _34 * 1.0f;
		out.w = _41 * in.x + _42 * in.y + _43 * in.z + _44 * 1.0f;
		return out;
	}

	//! Transform vector (x, y, z, w) assuming a transposed matrix
	/*!
		\param in four-dimensional vector to transform
		\return four-dimensional transformed vector
	*/
	Vec4<A> getVec4TransformTransposed(const Vec4<A> &in) const
	{
		Vec4<A> out;
		out.x = _11 * in.x + _12 * in.y + _13 * in.z + _14 * in.w;
		out.y = _21 * in.x + _22 * in.y + _23 * in.z + _24 * in.w;
		out.z = _31 * in.x + _32 * in.y + _33 * in.z + _34 * in.w;
		out.w = _41 * in.x + _42 * in.y + _43 * in.z + _44 * in.w;
		return out;
	}

	//! Transform vector (x, y, z, 1)
	/*!
		\param in three-dimensional vector to transform
		\return four-dimensional transformed vector
	*/
	Vec4<A> getVec3Transform(const Vec3<A> &in) const
	{
		Vec4<A> out;
		out.x = _11 * in.x + _21 * in.y + _31 * in.z + _41 * 1.0f;
		out.y = _12 * in.x + _22 * in.y + _32 * in.z + _42 * 1.0f;
		out.z = _13 * in.x + _23 * in.y + _33 * in.z + _43 * 1.0f;
		out.w = _14 * in.x + _24 * in.y + _34 * in.z + _44 * 1.0f;

		return out;
	}

	//! Transform vector (x, y, z, w)
	/*!
		\param in four-dimensional vector to transform
		\return four-dimensional transformed vector
	*/
	Vec4<A> getVec4Transform(const Vec4<A> &in) const
	{
		Vec4<A> out;
		out.x = _11 * in.x + _12 * in.y + _13 * in.z + _14 * in.w;
		out.y = _21 * in.x + _22 * in.y + _23 * in.z + _24 * in.w;
		out.z = _31 * in.x + _32 * in.y + _33 * in.z + _34 * in.w;
		out.w = _41 * in.x + _42 * in.y + _43 * in.z + _44 * in.w;

		out.x = _11 * in.x + _21 * in.y + _31 * in.z + _41 * in.w;
		out.y = _12 * in.x + _22 * in.y + _32 * in.z + _42 * in.w;
		out.z = _13 * in.x + _23 * in.y + _33 * in.z + _43 * in.w;
		out.w = _14 * in.x + _24 * in.y + _34 * in.z + _44 * in.w;

		return out;
	}

	//! Transform vector and project the result back into w = 1
	/*!
		\param in three-dimensional vector to transform
		\return three-dimensional transformed vector
	*/
	Vec3<A> getVec3TransformCoord(const Vec3<A> &in) const
	{
		Vec4<A> temp;
		Vec3<A> out;
		temp.x = _11 * in.x + _21 * in.y + _31 * in.z + _41 * 1.0f;
		temp.y = _12 * in.x + _22 * in.y + _32 * in.z + _42 * 1.0f;
		temp.z = _13 * in.x + _23 * in.y + _33 * in.z + _43 * 1.0f;
		temp.w = _14 * in.x + _24 * in.y + _34 * in.z + _44 * 1.0f;
		out.x = temp.x / temp.w;
		out.y = temp.y / temp.w;
		out.z = temp.z / temp.w;
		return out;
	}

	//! Transform vector normal
	/*!
		\param in three-dimensional vector to transform
		\return three-dimensional transformed vector
	*/
	Vec3<A> getVec3TransformNormal(const Vec3<A> &in) const
	{
		Vec3<A> out;
		out.x = _11 * in.x + _21 * in.y + _31 * in.z;
		out.y = _12 * in.x + _22 * in.y + _32 * in.z;
		out.z = _13 * in.x + _23 * in.y + _33 * in.z;
		return out;
	}

	//! Gets the transpose of the matrix
	/*!
		\return matrix transpose
	*/
	TMatrix44<A> getTranspose() const
	{
		TMatrix44<A> out;

		out._11 = _11;
		out._12 = _21;
		out._13 = _31;
		out._14 = _41;

		out._21 = _12;
		out._22 = _22;
		out._23 = _32;
		out._24 = _42;

		out._31 = _13;
		out._32 = _23;
		out._33 = _33;
		out._34 = _43;

		out._41 = _14;
		out._42 = _24;
		out._43 = _34;
		out._44 = _44;

		return out;
	}

	//! Gets the determinant of the matrix
	/*!
		\return matrix determinant
	*/
	A getDeterminant() const
	{
		A fA0 = _11 * _22 - _12 * _21;
		A fA1 = _11 * _23 - _13 * _21;
		A fA2 = _11 * _24 - _14 * _21;
		A fA3 = _12 * _23 - _13 * _22;
		A fA4 = _12 * _24 - _14 * _22;
		A fA5 = _13 * _24 - _14 * _23;
		A fB0 = _31 * _42 - _32 * _41;
		A fB1 = _31 * _43 - _33 * _41;
		A fB2 = _31 * _44 - _34 * _41;
		A fB3 = _32 * _43 - _33 * _42;
		A fB4 = _32 * _44 - _34 * _42;
		A fB5 = _33 * _44 - _34 * _43;

		return fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
	}

	//! Gets the inverse of the matrix
	/*!
		\return matrix inverse
	*/
	TMatrix44<A> getInverse() const
	{
		TMatrix44<A> out;

		A fA0 = _11 * _22 - _12 * _21;
		A fA1 = _11 * _23 - _13 * _21;
		A fA2 = _11 * _24 - _14 * _21;
		A fA3 = _12 * _23 - _13 * _22;
		A fA4 = _12 * _24 - _14 * _22;
		A fA5 = _13 * _24 - _14 * _23;
		A fB0 = _31 * _42 - _32 * _41;
		A fB1 = _31 * _43 - _33 * _41;
		A fB2 = _31 * _44 - _34 * _41;
		A fB3 = _32 * _43 - _33 * _42;
		A fB4 = _32 * _44 - _34 * _42;
		A fB5 = _33 * _44 - _34 * _43;

		A fDet = fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
		A fInvDet = ((A)1.0)/fDet;

		out._11 = (+ _22*fB5 - _23*fB4 + _24*fB3) * fInvDet;
		out._21 = (- _21*fB5 + _23*fB2 - _24*fB1) * fInvDet;
		out._31 = (+ _21*fB4 - _22*fB2 + _24*fB0) * fInvDet;
		out._41 = (- _21*fB3 + _22*fB1 - _23*fB0) * fInvDet;
		out._12 = (- _12*fB5 + _13*fB4 - _14*fB3) * fInvDet;
		out._22 = (+ _11*fB5 - _13*fB2 + _14*fB1) * fInvDet;
		out._32 = (- _11*fB4 + _12*fB2 - _14*fB0) * fInvDet;
		out._42 = (+ _11*fB3 - _12*fB1 + _13*fB0) * fInvDet;
		out._13 = (+ _42*fA5 - _43*fA4 + _44*fA3) * fInvDet;
		out._23 = (- _41*fA5 + _43*fA2 - _44*fA1) * fInvDet;
		out._33 = (+ _41*fA4 - _42*fA2 + _44*fA0) * fInvDet;
		out._43 = (- _41*fA3 + _42*fA1 - _43*fA0) * fInvDet;
		out._14 = (- _32*fA5 + _33*fA4 - _34*fA3) * fInvDet;
		out._24 = (+ _31*fA5 - _33*fA2 + _34*fA1) * fInvDet;
		out._34 = (- _31*fA4 + _32*fA2 - _34*fA0) * fInvDet;
		out._44 = (+ _31*fA3 - _32*fA1 + _33*fA0) * fInvDet;

		return out;
	}

	//! Creates a left-handed look-at matrix
	/*!
		\param eye eye position
		\param at camera target
		\param up up direction
	*/
	void createLookAtLH(const Vec3<A> &eye, const Vec3<A> &at, const Vec3<A> &up)
	{
		Vec3<A> zaxis = at - eye;
		zaxis.normalize();

		Vec3<A> xaxis = up.getCrossWith(zaxis);
		xaxis.normalize();
		Vec3<A> yaxis = zaxis.getCrossWith(xaxis);

		_11 = xaxis.x;
		_12 = yaxis.x;
		_13 = zaxis.x;
		_14 = 0;

		_21 = xaxis.y;
		_22 = yaxis.y;
		_23 = zaxis.y;
		_24 = 0;

		_31 = xaxis.z;
		_32 = yaxis.z;
		_33 = zaxis.z;
		_34 = 0;

		_41 = -xaxis.getDotWith(eye);
		_42 = -yaxis.getDotWith(eye);
		_43 = -zaxis.getDotWith(eye);
		_44 = 1;
	}

	//! Creates a left-handed perspective projection matrix
	/*!
		\param fov Y-direction field of view, radians
		\param aspect aspect ratio
		\param zNear near-plane Z-value
		\param zFar far-plane Z-value
	*/
	void createPerspectiveFovLH(A fov, A aspect, A zNear, A zFar)
	{
		A yscale = 1.0f / (tan(fov/2));
		A xscale = yscale / aspect;

		_11 = xscale;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = yscale;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = zFar / (zFar - zNear);
		_34 = 1;

		_41 = 0;
		_42 = 0;
		_43 = -zNear * zFar / (zFar - zNear);
		_44 = 0;
	}

	// Perspective projection matrix for lim(zNear->Infinity)
	void createPerspectiveFovLHInfinite(A fov, A aspect, A zNear)
	{
		A yscale = 1.0f / (tan(fov / 2));
		A xscale = yscale / aspect;

		_11 = xscale;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = yscale;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = 0;
		_34 = 1;

		_41 = 0;
		_42 = 0;
		_43 = zNear;
		_44 = 0;
	}

	//! Creates a left-handed perspective projection matrix
	/*!
		\param l minimum x-value of the view volume
		\param r maximum x-value of the view volume
		\param b minimum y-value of the view volume
		\param t maximum x-value of the view volume
		\param zn minimum z-value of the view volume
		\param zf maximum x-value of the view volume
	*/
	void createPerspectiveOffCenterLH(A l, A r, A b, A t, A zn, A zf)
	{
		_11 = 2 * zn / (r - l);
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = 2 * zn / (t - b);
		_23 = 0;
		_24 = 0;

		_31 = (l + r) / (l - r);
		_32 = (t + b) / (b - t);
		_33 = zf / (zf - zn);
		_34 = 1;

		_41 = 0;
		_42 = 0;
		_43 = zn * zf / (zn - zf);
		_44 = 0;
	}

	//! Creates a left-handed orthographic projection matrix
	/*!
		\param l minimum x-value of the view volume
		\param r maximum x-value of the view volume
		\param b minimum y-value of the view volume
		\param t maximum x-value of the view volume
		\param zn minimum z-value of the view volume
		\param zf maximum x-value of the view volume
	*/
	void createOrthoOffCenterLH(A l, A r, A b, A t, A zn, A zf)
	{
		_11 = 2 / (r - l);
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = 2 / (t - b);
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = 1.0f / (zf - zn);
		_34 = 0;

		_41 = (l + r)/(l - r);
		_42 = (t + b) / (b - t);
		_43 = zn / (zn - zf);
		_44 = 1.f;
	}

	void createOrthoLH(float width, float height, float minZ, float maxZ)
	{
		_11 = 2.f / width;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = 2.f / height;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = 1.f / (maxZ - minZ);
		_34 = 0;

		_41 = 0;
		_42 = 0;
		_43 = minZ / (minZ - maxZ);
		_44 = 1.f;
	}

	//! Creates a Y-axis rotation matrix
	/*!
		\param angle rotation angle, radians
	*/
	void createRotationY(A angle)
	{
		_11 = cos(angle);
		_12 = 0;
		_13 = sin(angle);
		_14 = 0;

		_21 = 0;
		_22 = 1;
		_23 = 0;
		_24 = 0;

		_31 = -sin(angle);
		_32 = 0;
		_33 = cos(angle);
		_34 = 0;

		_41 = 0;
		_42 = 0;
		_43 = 0;
		_44 = 1;
	}

	//! Creates a 3D affine transformation matrix
	/*!
		\param scale scaling factor
		\param rotCenter center of rotation
		\param rot rotation
		\param trans translation
	*/
	void createAffineTransformation(A scale, const Vec3<A> &rotCenter, const QUAT &rot, const Vec3<A> &trans)
	{
		TMatrix44<A> s;
		s._11 = scale;
		s._22 = scale;
		s._33 = scale;

		TMatrix44<A> rc;
		rc._41 = rotCenter.x;
		rc._42 = rotCenter.y;
		rc._43 = rotCenter.z;

		TMatrix44<A> r;
		A xx = rot.x * rot.x;
		A xy = rot.x * rot.y;
		A xz = rot.x * rot.z;
		A xw = rot.x * rot.w;

		A yy = rot.y * rot.y;
		A yz = rot.y * rot.z;
		A yw = rot.y * rot.w;

		A zz = rot.z * rot.z;
		A zw = rot.z * rot.w;

		r._11 = 1 - 2 * (yy + zz);
		r._12 = 2 * (xy - zw);
		r._13 = 2 * (xz + yw);

		r._21 = 2 * (xy + zw);
		r._22 = 1 - 2 * (xx + zz);
		r._23 = 2 * (yz - xw);

		r._31 = 2 * (xz - yw);
		r._32 = 2 * (yz + xw);
		r._33 = 1 - 2 * (xx + yy);

		TMatrix44<A> t;
		t._41 = trans.x;
		t._42 = trans.y;
		t._43 = trans.z;

		TMatrix44<A> rci;
		rci = rc.getInverse();

		TMatrix44<A> out;
		out = s * rci * r * rc * t;

		lang::MemCopy::copy(raw, out.raw, sizeof(A)*16);
	}

	/**
	 * Returns 3x3 rotation matrix
	 */
	void getAxes(Vec3<A> (&axes)[3]) const
	{
		axes[0] = Vec3<A>(_11, _12, _13);
		axes[1] = Vec3<A>(_21, _22, _23);
		axes[2] = Vec3<A>(_31, _32, _33);
	}
};

FB_END_PACKAGE1()

#endif
