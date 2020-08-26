#include "Precompiled.h"
#include "BezierCurve.h"

#include "fb/algorithm/Sort.h"

FB_PACKAGE1(math)

void BezierCurve::setKnots(const PodVector<BezierCurve::KnotType> &knotsParam)
{
	knots = knotsParam;
	algorithm::stableSort(knots.getBegin(), knots.getEnd());
}

FB_END_PACKAGE1();
