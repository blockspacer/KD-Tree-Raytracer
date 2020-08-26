#pragma once

#include "fb/lang/Declaration.h"
#include "fb/lang/Package.h"

FB_DECLARE_TEMPLATED_CLASS(math, RectangleMinMax)

FB_PACKAGE1(math)
typedef RectangleMinMax<float> RectMinMax;
typedef RectangleMinMax<int> IRectMinMax;
FB_END_PACKAGE1()
