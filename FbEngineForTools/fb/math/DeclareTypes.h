#pragma once

#include "fb/lang/Declaration.h"

FB_DECLARE_TEMPLATED_CLASS(math, Color3)
FB_DECLARE_TEMPLATED_CLASS(math, Color4)
FB_DECLARE_TEMPLATED_CLASS(math, Quaternion)
FB_DECLARE_TEMPLATED_CLASS(math, TBounds)
FB_DECLARE_TEMPLATED_CLASS(math, TMatrix)
FB_DECLARE_TEMPLATED_CLASS(math, TMatrix44)
FB_DECLARE_TEMPLATED_CLASS(math, Vec2)
FB_DECLARE_TEMPLATED_CLASS(math, Vec3)
FB_DECLARE_TEMPLATED_CLASS(math, Vec4)

FB_PACKAGE1(math)

typedef Color3<float> COL;
typedef Color4<float> COL4;

typedef Quaternion<float> QUAT;
typedef Quaternion<double> DQUAT;

typedef TBounds<float> Bounds;

typedef TMatrix<float> MAT;
typedef TMatrix44<float> MAT44;

typedef Vec2<double> DVC2;
typedef Vec2<float> VC2;
typedef Vec2<int32_t> VC2I;

typedef Vec3<double> DVC3;
typedef Vec3<float> VC3;
typedef Vec3<int32_t> VC3I;

typedef Vec4<double> DVC4;
typedef Vec4<float> VC4;
typedef Vec4<int32_t> VC4I;

FB_END_PACKAGE1()
