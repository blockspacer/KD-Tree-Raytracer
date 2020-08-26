#include "Precompiled.h"
#include "Vec2.h"

#include "fb/math/util/IsDenormal.h"
#include "fb/math/util/IsFinite.h"
#include "fb/math/util/IsInf.h"
#include "fb/math/util/IsNaN.h"
#include "fb/string/HeapString.h"

FB_PACKAGE1(math)

HeapString &debugAppendToString(HeapString &result, VC2 val)
{
	return result << val;
}

HeapString &debugAppendToString(HeapString &result, VC2I val)
{
	return result << val;
}

HeapString &debugAppendToString(HeapString &result, const DVC2 &val)
{
	return result << val;
}

template<>
bool Vec2<float>::isDenormal() const
{
	return math::util::isDenormal(x) || math::util::isDenormal(y);
}

template<>
bool Vec2<double>::isDenormal() const
{
	return math::util::isDenormal(x) || math::util::isDenormal(y);
}

template<>
bool Vec2<int32_t>::isDenormal() const
{
	return false;
}

template<>
bool Vec2<float>::isNaN() const
{
	return math::util::isNaN(x) || math::util::isNaN(y);
}

template<>
bool Vec2<double>::isNaN() const
{
	return math::util::isNaN(x) || math::util::isNaN(y);
}

template<>
bool Vec2<int32_t>::isNaN() const
{
	return false;
}

template<>
bool Vec2<float>::isInf() const
{
	return math::util::isInf(x) || math::util::isInf(y);
}

template<>
bool Vec2<double>::isInf() const
{
	return math::util::isInf(x) || math::util::isInf(y);
}

template<>
bool Vec2<int32_t>::isInf() const
{
	return false;
}

template<>
bool Vec2<float>::isFinite() const
{
	return math::util::isFinite(x) || math::util::isFinite(y);
}

template<>
bool Vec2<double>::isFinite() const
{
	return math::util::isFinite(x) || math::util::isFinite(y);
}

template<>
bool Vec2<int32_t>::isFinite() const
{
	return true;
}

FB_END_PACKAGE1()
