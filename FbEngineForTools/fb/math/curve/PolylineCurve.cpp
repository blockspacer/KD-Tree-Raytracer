#include "Precompiled.h"
#include "PolylineCurve.h"

#include "fb/algorithm/Sort.h"

FB_PACKAGE1(math)

void PolylineCurve::setKnots(const PodVector<PolylineCurve::KnotType> &knotsParam)
{
	knots = knotsParam;
	algorithm::stableSort(knots.getBegin(), knots.getEnd());
}

FB_END_PACKAGE1();
