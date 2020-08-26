#include "Precompiled.h"
#include "Vec3.h"

#include "fb/math/util/IsDenormal.h"
#include "fb/math/util/IsFinite.h"
#include "fb/math/util/IsInf.h"
#include "fb/math/util/IsNaN.h"
#include "fb/string/HeapString.h"

FB_PACKAGE1(math)

HeapString &debugAppendToString(HeapString &result, const VC3 &val)
{
	return result << val;
}

HeapString &debugAppendToString(HeapString &result, const VC3I &val)
{
	return result << val;
}

HeapString &debugAppendToString(HeapString &result, const DVC3 &val)
{
	return result << val;
}

template<>
bool Vec3<float>::isDenormal() const
{
	return math::util::isDenormal(x) || math::util::isDenormal(y) || math::util::isDenormal(z);
}

template<>
bool Vec3<double>::isDenormal() const
{
	return math::util::isDenormal(x) || math::util::isDenormal(y) || math::util::isDenormal(z);
}

template<>
bool Vec3<int32_t>::isDenormal() const
{
	return false;
}

template<>
bool Vec3<float>::isNaN() const
{
	return math::util::isNaN(x) || math::util::isNaN(y) || math::util::isNaN(z);
}

template<>
bool Vec3<double>::isNaN() const
{
	return math::util::isNaN(x) || math::util::isNaN(y) || math::util::isNaN(z);
}

template<>
bool Vec3<int32_t>::isNaN() const
{
	return false;
}

template<>
bool Vec3<float>::isInf() const
{
	return math::util::isInf(x) || math::util::isInf(y) || math::util::isInf(z);
}

template<>
bool Vec3<double>::isInf() const
{
	return math::util::isInf(x) || math::util::isInf(y) || math::util::isInf(z);
}

template<>
bool Vec3<int32_t>::isInf() const
{
	return false;
}

template<>
bool Vec3<float>::isFinite() const
{
	return math::util::isFinite(x) || math::util::isFinite(y) || math::util::isFinite(z);
}

template<>
bool Vec3<double>::isFinite() const
{
	return math::util::isFinite(x) || math::util::isFinite(y) || math::util::isFinite(z);
}

template<>
bool Vec3<int32_t>::isFinite() const
{
	return true;
}

FB_END_PACKAGE1()
