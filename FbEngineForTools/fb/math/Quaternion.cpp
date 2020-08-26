#include "Precompiled.h"
#include "Quaternion.h"

#include "fb/math/util/IsDenormal.h"
#include "fb/math/util/IsFinite.h"
#include "fb/math/util/IsInf.h"
#include "fb/math/util/IsNaN.h"
#include "fb/string/HeapString.h"

FB_PACKAGE1(math)

HeapString &debugAppendToString(HeapString &result, const QUAT &val)
{
	return result << val;
}

HeapString &debugAppendToString(HeapString &result, const DQUAT &val)
{
	return result << val;
}

template<>
bool Quaternion<float>::isDenormal() const
{
	return math::util::isDenormal(x) || math::util::isDenormal(y) || math::util::isDenormal(z) || math::util::isDenormal(w);
}

template<>
bool Quaternion<double>::isDenormal() const
{
	return math::util::isDenormal(x) || math::util::isDenormal(y) || math::util::isDenormal(z) || math::util::isDenormal(w);
}

template<>
bool Quaternion<float>::isNaN() const
{
	return math::util::isNaN(x) || math::util::isNaN(y) || math::util::isNaN(z) || math::util::isNaN(w);
}

template<>
bool Quaternion<double>::isNaN() const
{
	return math::util::isNaN(x) || math::util::isNaN(y) || math::util::isNaN(z) || math::util::isNaN(w);
}

template<>
bool Quaternion<float>::isInf() const
{
	return math::util::isInf(x) || math::util::isInf(y) || math::util::isInf(z) || math::util::isInf(w);
}

template<>
bool Quaternion<double>::isInf() const
{
	return math::util::isInf(x) || math::util::isInf(y) || math::util::isInf(z) || math::util::isInf(w);
}

template<>
bool Quaternion<float>::isFinite() const
{
	return math::util::isFinite(x) || math::util::isFinite(y) || math::util::isFinite(z) || math::util::isFinite(w);
}

template<>
bool Quaternion<double>::isFinite() const
{
	return math::util::isFinite(x) || math::util::isFinite(y) || math::util::isFinite(z) || math::util::isFinite(w);
}

FB_END_PACKAGE1()
