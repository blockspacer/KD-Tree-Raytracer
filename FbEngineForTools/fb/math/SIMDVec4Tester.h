#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

#include "fb/lang/FBStaticAssert.h"
#include "fb/math/util/IsFinite.h"
#include "fb/string/HeapString.h"

#if FB_ASSERT_ENABLED == FB_FALSE
	#error SIMDVec4Tester uses asserts to report errors. No sense in running it without
#endif

FB_PACKAGE1(math)

/* Class to test SIMDVec4 implementation by redoing every calculation with reference implementation and comparing 
 * results. AllowedErrors are relative magnitudes. 1 means error of 1/2 (50 %) is allowed. 2 mean 1/4 (25 %) and so 
 * on. 
 *
 * 0: 0 % (no error allowed)
 * 1: 50 %
 * 2: 25 %
 * 3: 12.5 %
 * 4: 6.25 %
 * 5: 3.13 %
 * 6: 1.56 %
 * 7: 0.781 %
 * 8: 0.391 %
 * 9: 0.195 %
 * 10: 0.0977 %
 * 11: 0.0488 %
 * 12: 0.0244 %
 * 13: 0.0122 %
 */

template<typename VecToTest, typename ReferenceVec, uint32_t allowedErrorOnLength, uint32_t allowedErrorOnNormalize>
class SIMDVec4Tester
{
	FB_NOINLINE void validate() const
	{
		VC4 testeeAsVC4;
		testee.storeTo(&testeeAsVC4.x);
		VC4 refAsVC4;
		ref.storeTo(&refAsVC4.x);
		fb_assert(testeeAsVC4 == refAsVC4 && "Testee and reference don't compare right");
		fb_static_assert(VecToTest::alignment == ReferenceVec::alignment && "Vectors have different alignments. Needs workarounds");
		fb_static_assert(VecToTest::simdAlignmentMask == ReferenceVec::simdAlignmentMask && "Vectors have different alignments. Needs workarounds");
	}

	FB_NOINLINE void validate(uint32_t allowedErrorMagnitude, const StringRef &id) const
	{
		VC4 testeeResult;
		testee.storeTo(testeeResult);
		VC4 refResult;
		ref.storeTo(refResult);
		validateResult(testeeResult, refResult, allowedErrorMagnitude, id);
	}
	
	FB_NOINLINE static float getErrorAsFloat(uint32_t errorMagnitude)
	{
		return errorMagnitude > 0 ? 1.0f / (1 << errorMagnitude) : 0.0f;
	}

	FB_NOINLINE void validateResult(float testeeResult, float refResult, uint32_t allowedErrorMagnitude, const StringRef &id) const
	{
		const float nearZero = 1e-6f;
		const float allowedError = getErrorAsFloat(allowedErrorMagnitude);
		float error = lang::abs(refResult) > nearZero ? lang::abs((testeeResult - refResult) / refResult) : lang::abs(testeeResult - refResult);
		if (error > allowedError)
		{
			TempString msg;
			msg << "Too large error for " << id << ". Results:\n";
			msg << "\tTestee   : ";
			msg.appendAccurate(testeeResult) << " vs. \n";
			msg << "\tReference: ";
			msg.appendAccurate(refResult) << "\n";
			msg << "\tError    : ";
			msg.appendAccurate(error) << "\n";
			msg << "\tAllowed  : ";
			msg.appendAccurate(allowedError);
			fb_assertf(false, "%s", msg.getPointer());
		}
	}

	FB_NOINLINE void validateResult(const VC4 &testeeResult, const VC4 &refResult, uint32_t allowedErrorMagnitude, const StringRef &id) const
	{
		const float nearZero = 1e-6f;
		const float allowedError = getErrorAsFloat(allowedErrorMagnitude);
		VC4 error(
			lang::abs(refResult.x) > nearZero ? lang::abs((testeeResult.x - refResult.x) / refResult.x) : lang::abs(testeeResult.x - refResult.x),
			lang::abs(refResult.y) > nearZero ? lang::abs((testeeResult.y - refResult.y) / refResult.y) : lang::abs(testeeResult.y - refResult.y),
			lang::abs(refResult.z) > nearZero ? lang::abs((testeeResult.z - refResult.z) / refResult.z) : lang::abs(testeeResult.z - refResult.z),
			lang::abs(refResult.w) > nearZero ? lang::abs((testeeResult.w - refResult.w) / refResult.w) : lang::abs(testeeResult.w - refResult.w)
		);
		if (error.x > allowedError || error.y > allowedError || error.z > allowedError || error.w > allowedError)
		{
			TempString msg;
			msg << "Too large error for " << id << ". Results:\n";
			msg << "\tTestee   : ";
			msg.appendAccurate(testeeResult) << " vs. \n";
			msg << "\tReference: ";
			msg.appendAccurate(refResult) << "\n";
			msg << "\tError    : ";
			msg.appendAccurate(error) << "\n";
			msg << "\tAllowed  : ";
			msg.appendAccurate(allowedError);
			fb_assertf(false, "%s", msg.getPointer());
		}
	}

	SIMDVec4Tester(const VecToTest &testee, const ReferenceVec &ref)
		: testee(testee)
		, ref(ref)
	{
		validate();
	}

public:
	FB_NOINLINE SIMDVec4Tester()
	{
		validate();
	}

	FB_NOINLINE SIMDVec4Tester(float x, float y, float z, float w)
		: testee(x, y, z, w)
		, ref(x, y, z, w)
	{
		validate();
	}

	FB_FORCEINLINE SIMDVec4Tester(const VC3 &vec)
		: testee(vec)
		, ref(vec)
	{
		validate();
	}

	FB_NOINLINE SIMDVec4Tester(const math::VC4 &vec)
		: testee(vec)
		, ref(vec)
	{
		validate();
	}

	FB_NOINLINE SIMDVec4Tester getNormalized() const
	{
		VC4 testeeResult;
		testee.getNormalized().storeTo(testeeResult);
		VC4 refResult;
		ref.getNormalized().storeTo(refResult);
		validateResult(testeeResult, refResult, allowedErrorOnNormalize, __FUNCTION__);
		SIMDVec4Tester result(testeeResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester getNormalizedWithZeroFailsafe(const SIMDVec4Tester &failsafeValue) const
	{
		VC4 testeeResult;
		testee.getNormalizedWithZeroFailsafe(failsafeValue.testee).storeTo(testeeResult);
		VC4 refResult;
		ref.getNormalizedWithZeroFailsafe(failsafeValue.ref).storeTo(testeeResult);
		validateResult(testeeResult, refResult, allowedErrorOnNormalize, __FUNCTION__);
		SIMDVec4Tester result(testeeResult);
		return result;
	}

	FB_NOINLINE void normalize()
	{
		testee.normalize();
		ref.normalize();
		validate(allowedErrorOnNormalize, __FUNCTION__);
		/* As some error is expected, store testee to ref to avoid getting false positives */
		VC4 testeeResult;
		testee.storeTo(testeeResult);
		ref = ReferenceVec(testeeResult);
	}

	FB_NOINLINE void normalizeWithZeroFailsafe(const SIMDVec4Tester &failsafeValue)
	{
		testee.normalizeWithZeroFailsafe(failsafeValue.testee);
		ref.normalizeWithZeroFailsafe(failsafeValue.ref);
		validate(allowedErrorOnNormalize, __FUNCTION__);
		/* As some error is expected, store testee to ref to avoid getting false positives */
		VC4 testeeResult;
		testee.storeTo(testeeResult);
		ref = ReferenceVec(testeeResult);
	}

	FB_FORCEINLINE SIMDVec4Tester getAbs() const
	{
		VecToTest testeeResult = testee.getAbs();
		ReferenceVec refResult = ref.getAbs();
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator-() const
	{
		VecToTest testeeResult = -testee;
		ReferenceVec refResult = -ref;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator+(const SIMDVec4Tester &other) const
	{
		VecToTest testeeResult = testee + other.testee;
		ReferenceVec refResult = ref + other.ref;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator-(const SIMDVec4Tester &other) const
	{
		VecToTest testeeResult = testee - other.testee;
		ReferenceVec refResult = ref - other.ref;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator*(const SIMDVec4Tester &other) const
	{
		VecToTest testeeResult = testee * other.testee;
		ReferenceVec refResult = ref * other.ref;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator/(const SIMDVec4Tester &other) const
	{
		VecToTest testeeResult = testee / other.testee;
		ReferenceVec refResult = ref / other.ref;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator*(float value) const
	{
		VecToTest testeeResult = testee * value;
		ReferenceVec refResult = ref * value;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester operator/(float value) const
	{
		VecToTest testeeResult = testee / value;
		ReferenceVec refResult = ref / value;
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE SIMDVec4Tester& operator+=(const SIMDVec4Tester &other)
	{
		testee += other.testee;
		ref += other.ref;
		validate();
		return *this;
	}

	FB_NOINLINE SIMDVec4Tester& operator-=(const SIMDVec4Tester &other)
	{
		testee -= other.testee;
		ref -= other.ref;
		validate();
		return *this;
	}

	FB_NOINLINE SIMDVec4Tester& operator*=(const SIMDVec4Tester &other)
	{
		testee *= other.testee;
		ref *= other.ref;
		validate();
		return *this;
	}

	FB_NOINLINE SIMDVec4Tester& operator/=(const SIMDVec4Tester &other)
	{
		testee /= other.testee;
		ref /= other.ref;
		validate();
		return *this;
	}

	FB_NOINLINE SIMDVec4Tester& operator*=(float value)
	{
		testee *= value;
		ref *= value;
		validate();
		return *this;
	}

	FB_NOINLINE SIMDVec4Tester& operator/=(float value)
	{
		testee /= value;
		ref /= value;
		validate();
		return *this;
	}

	FB_NOINLINE float getLength() const
	{
		float testeeResult = testee.getLength();
		float refResult = ref.getLength();
		validateResult(testeeResult, refResult, allowedErrorOnLength, __FUNCTION__);
		return testeeResult;
	}

	FB_NOINLINE float getSquareLength() const
	{
		return getDotWith(*this);
	}

	FB_NOINLINE float getDotWith(const SIMDVec4Tester &other) const
	{
		float testeeResult = testee.getDotWith(other.testee);
		float refResult = ref.getDotWith(other.ref);
		fb_assert(testeeResult == refResult && "Testee and reference calculation differs");
		return testeeResult;
	}

	FB_NOINLINE SIMDVec4Tester getVectorDotWith(const SIMDVec4Tester &other) const
	{

		VecToTest testeeResult = testee.getVectorDotWith(other.testee);
		ReferenceVec refResult = ref.getVectorDotWith(other.ref);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE void storeTo(float *p) const
	{
		ref.storeTo(p);
		VC4 refResult(p[0], p[1], p[2], p[3]);
		testee.storeTo(p);
		VC4 testeeResult(p[0], p[1], p[2], p[3]);
		fb_assert(testeeResult == refResult && "Testee and reference differ");
	}

	FB_NOINLINE void storeTo(math::VC4 &result) const
	{
		VC4 refResult;
		ref.storeTo(&refResult.x);
		testee.storeTo(&result.x);
		fb_assert(result == refResult && "Testee and reference differ");
	}

	FB_NOINLINE bool isFinite() const
	{
		return util::isFinite((*this)[0]) && util::isFinite((*this)[1]) && util::isFinite((*this)[2]) && util::isFinite((*this)[3]);
	}

	FB_FORCEINLINE bool anyLessThanZero() const
	{
		bool testeeResult = testee.anyLessThanZero();
		bool refResult = ref.anyLessThanZero();
		fb_assert(testeeResult == refResult);
		return testeeResult;
	}

	FB_NOINLINE float operator[](SizeType i) const
	{
		float testeeResult = testee[i];
		float refResult = ref[i];
		fb_assert(testeeResult == refResult && "Testee and reference differ");
		return testeeResult;
	}

	FB_NOINLINE VC4 toVC4()
	{
		VC4 testeeResult = testee.toVC4();
		VC4 refResult = ref.toVC4();
		fb_assert(testeeResult == refResult && "Testee and reference differ");
		return testeeResult;

	}

	FB_NOINLINE VC3 toVC3()
	{
		VC3 testeeResult = testee.toVC3();
		VC3 refResult = ref.toVC3();
		fb_assert(testeeResult == refResult && "Testee and reference differ");
		return testeeResult;
	}

	FB_NOINLINE static SIMDVec4Tester zero()
	{
		return SIMDVec4Tester();
	}

	FB_NOINLINE static SIMDVec4Tester setXYZW(float value)
	{
		return SIMDVec4Tester(value, value, value, value);
	}

	FB_NOINLINE static SIMDVec4Tester loadAligned(const float *alignedPointer)
	{
		VecToTest testeeResult = VecToTest::loadAligned(alignedPointer);
		ReferenceVec refResult = ReferenceVec::loadAligned(alignedPointer);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static SIMDVec4Tester loadUnaligned(const float *unalignedPointer)
	{
		VecToTest testeeResult = VecToTest::loadUnaligned(unalignedPointer);
		ReferenceVec refResult = ReferenceVec::loadUnaligned(unalignedPointer);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static void storeAligned(float *alignedPointer, const SIMDVec4Tester& a)
	{
		ReferenceVec::storeAligned(alignedPointer, a.ref);
		VC4 refResult(alignedPointer[0], alignedPointer[1], alignedPointer[2], alignedPointer[3]);
		VecToTest::storeAligned(alignedPointer, a.testee);
		VC4 testeeResult(alignedPointer[0], alignedPointer[1], alignedPointer[2], alignedPointer[3]);
		fb_assert(testeeResult == refResult && "Testee and reference differ");
	}

	FB_NOINLINE static SIMDVec4Tester loadXYXY(const float *alignedPointer)
	{
		VecToTest testeeResult = VecToTest::loadXYXY(alignedPointer);
		ReferenceVec refResult = ReferenceVec::loadXYXY(alignedPointer);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static SIMDVec4Tester setXYXY(float x, float y)
	{
		VecToTest testeeResult = VecToTest::setXYXY(x, y);
		ReferenceVec refResult = ReferenceVec::setXYXY(x, y);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static void storeXY(float *alignedPointer, const SIMDVec4Tester& a)
	{
		ReferenceVec::storeXY(alignedPointer, a.ref);
		VC2 refResult(alignedPointer[0], alignedPointer[1]);
		VecToTest::storeXY(alignedPointer, a.testee);
		VC2 testeeResult(alignedPointer[0], alignedPointer[1]);
		fb_assert(testeeResult == refResult && "Testee and reference differ");
	}

	FB_NOINLINE static SIMDVec4Tester swapToYXWZ(const SIMDVec4Tester &a)
	{
		VecToTest testeeResult = VecToTest::swapToYXWZ(a.testee);
		ReferenceVec refResult = ReferenceVec::swapToYXWZ(a.ref);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static SIMDVec4Tester swapToZWXY(const SIMDVec4Tester &a)
	{
		VecToTest testeeResult = VecToTest::swapToZWXY(a.testee);
		ReferenceVec refResult = ReferenceVec::swapToZWXY(a.ref);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static SIMDVec4Tester combineXYAndZW(const SIMDVec4Tester &xy, const SIMDVec4Tester &zw)
	{
		VecToTest testeeResult = VecToTest::combineXYAndZW(xy.testee, zw.testee);
		ReferenceVec refResult = ReferenceVec::combineXYAndZW(xy.ref, zw.ref);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static SIMDVec4Tester min(const SIMDVec4Tester &a, const SIMDVec4Tester &b)
	{
		VecToTest testeeResult = VecToTest::min(a.testee, b.testee);
		ReferenceVec refResult = ReferenceVec::min(a.ref, b.ref);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	FB_NOINLINE static SIMDVec4Tester max(const SIMDVec4Tester &a, const SIMDVec4Tester& b)
	{
		VecToTest testeeResult = VecToTest::max(a.testee, b.testee);
		ReferenceVec refResult = ReferenceVec::max(a.ref, b.ref);
		SIMDVec4Tester result(testeeResult, refResult);
		return result;
	}

	static const uint32_t alignment = 16;
	static const uint32_t simdAlignmentMask = alignment - 1;

private:
	VecToTest testee;
	ReferenceVec ref;
};


FB_END_PACKAGE1()
