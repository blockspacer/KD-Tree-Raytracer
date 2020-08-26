#pragma once

#include "fb/math/SIMDVec4.h"
#include "fb/math/Matrix.h"

FB_PACKAGE1(math)
	
class SIMDMat3x4
{
public:
	SIMDVec4 row[3];
};

// TODO: only some of the stuff is SSE-specific, so it might make sense to write a paired singles version of the broadcast & pointwise mul functions. should benchmark though
#if FB_SIMD_VEC4_SSE_IN_USE == FB_TRUE

#define FB_SIMD_VEC4_BROADCAST_0(v) math::SIMDVec4(_mm_shuffle_ps((v).vec, (v).vec, 0x00))
#define FB_SIMD_VEC4_BROADCAST_1(v) math::SIMDVec4(_mm_shuffle_ps((v).vec, (v).vec, 0x55))
#define FB_SIMD_VEC4_BROADCAST_2(v) math::SIMDVec4(_mm_shuffle_ps((v).vec, (v).vec, 0xAA))
#define FB_SIMD_VEC4_BROADCAST_3(v) math::SIMDVec4(_mm_shuffle_ps((v).vec, (v).vec, 0xFF))

FB_FORCEINLINE SIMDVec4 combine(const SIMDVec4 &a, const SIMDMat3x4 &B)
{
	SIMDVec4 ret = FB_SIMD_VEC4_BROADCAST_0(a) * B.row[0];
	ret = ret + FB_SIMD_VEC4_BROADCAST_1(a) * B.row[1];
	ret = ret + FB_SIMD_VEC4_BROADCAST_2(a) * B.row[2];
	ret = ret + SIMDVec4(_mm_and_ps(a.vec, _mm_castsi128_ps(_mm_set_epi32(0xFFFFFFFF, 0, 0, 0))));
	return ret;
}

FB_FORCEINLINE void multiplyInPlace(SIMDMat3x4 * FB_RESTRICT A, const SIMDMat3x4 * FB_RESTRICT B)
{
	SIMDVec4 out0 = combine(B->row[0], *A);
	SIMDVec4 out1 = combine(B->row[1], *A);
	SIMDVec4 out2 = combine(B->row[2], *A);
	A->row[0] = out0;
	A->row[1] = out1;
	A->row[2] = out2;
}

FB_FORCEINLINE void multiplyTo(const SIMDMat3x4 * FB_RESTRICT A, const SIMDMat3x4 * FB_RESTRICT B, SIMDMat3x4 * FB_RESTRICT R)
{
	SIMDVec4 out0 = combine(B->row[0], *A);
	SIMDVec4 out1 = combine(B->row[1], *A);
	SIMDVec4 out2 = combine(B->row[2], *A);
	R->row[0] = out0;
	R->row[1] = out1;
	R->row[2] = out2;
}

#else

FB_FORCEINLINE void multiplyInPlace(SIMDMat3x4 * FB_RESTRICT A, const SIMDMat3x4 * FB_RESTRICT B)
{
	math::SIMDMat3x4 temp;

	for (int row = 0; row != 3; ++row)
	{
		float cols[4];
		cols[0] = B->row[row][0] * A->row[0][0]
			+ B->row[row][1] * A->row[1][0]
			+ B->row[row][2] * A->row[2][0];
		cols[1] = B->row[row][0] * A->row[0][1]
			+ B->row[row][1] * A->row[1][1]
			+ B->row[row][2] * A->row[2][1];
		cols[2] = B->row[row][0] * A->row[0][2]
			+ B->row[row][1] * A->row[1][2]
			+ B->row[row][2] * A->row[2][2];
		cols[3] = B->row[row][0] * A->row[0][3]
			+ B->row[row][1] * A->row[1][3]
			+ B->row[row][2] * A->row[2][3]
			+ B->row[row][3];
		temp.row[row] = SIMDVec4(cols[0], cols[1], cols[2], cols[3]);
	}

	*A = temp;
}

FB_FORCEINLINE void multiplyTo(const SIMDMat3x4 * FB_RESTRICT A, const SIMDMat3x4 * FB_RESTRICT B, SIMDMat3x4 * FB_RESTRICT R)
{
	for (int row = 0; row != 3; ++row)
	{
		float cols[4];
		cols[0] = B->row[row][0] * A->row[0][0]
			+ B->row[row][1] * A->row[1][0]
			+ B->row[row][2] * A->row[2][0];
		cols[1] = B->row[row][0] * A->row[0][1]
			+ B->row[row][1] * A->row[1][1]
			+ B->row[row][2] * A->row[2][1];
		cols[2] = B->row[row][0] * A->row[0][2]
			+ B->row[row][1] * A->row[1][2]
			+ B->row[row][2] * A->row[2][2];
		cols[3] = B->row[row][0] * A->row[0][3]
			+ B->row[row][1] * A->row[1][3]
			+ B->row[row][2] * A->row[2][3]
			+ B->row[row][3];
		R->row[row] = SIMDVec4(cols[0], cols[1], cols[2], cols[3]);
	}
}


#endif

FB_FORCEINLINE MAT fromSIMDMat3x4(const SIMDMat3x4 &mat)
{
	float values[16] = {
		 mat.row[0][0]
		,mat.row[1][0]
		,mat.row[2][0]
		,0.0f
		,mat.row[0][1]
		,mat.row[1][1]
		,mat.row[2][1]
		,0.0f
		,mat.row[0][2]
		,mat.row[1][2]
		,mat.row[2][2]
		,0.0f
		,mat.row[0][3]
		,mat.row[1][3]
		,mat.row[2][3]
		,1.0f
	};

	return MAT(values);

}

FB_FORCEINLINE SIMDMat3x4 toSIMDMat3x4(const MAT &mat)
{
	SIMDMat3x4 ret;
	ret.row[0] = SIMDVec4(mat.get(0), mat.get(4), mat.get(8), mat.get(12));
	ret.row[1] = SIMDVec4(mat.get(1), mat.get(5), mat.get(9), mat.get(13));
	ret.row[2] = SIMDVec4(mat.get(2), mat.get(6), mat.get(10), mat.get(14));
	return ret;
}

FB_FORCEINLINE void multiplyInPlace(SIMDMat3x4 &A, const SIMDMat3x4 &B)
{
	multiplyInPlace(&A, &B);
}

FB_END_PACKAGE1()
