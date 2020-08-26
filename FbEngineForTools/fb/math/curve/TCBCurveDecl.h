#pragma once

FB_DECLARE_TEMPLATED_CLASS(math, curve, Knot)
FB_DECLARE_TEMPLATED_CLASS(math, curve, TCBCurveBase)

FB_PACKAGE1(math)
typedef math::curve::Knot<float> TCBCurveKnot;
typedef math::curve::TCBCurveBase<TCBCurveKnot> TCBCurve;
FB_END_PACKAGE1()
