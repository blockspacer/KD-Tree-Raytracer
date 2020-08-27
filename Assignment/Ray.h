#pragma once

#include "Assignment/IntersectionType.h"
#include "fb/lang/NumericLimits.h"
#include "Triangle.h"

FB_PACKAGE1(assignment)

struct Ray
{
	math::VC3 origin;
	math::VC3 direction;
	math::VC3 intersectionPoint;
	Triangle *startingPoint = nullptr;
	const Triangle *bestTriangle = nullptr;
	IntersectionType intersectionType = IntersectionTypeNearest;
	float bestDistanceSq = lang::NumericLimits<float>::getMax();
	float zeroDistanceThreshold = 0.0001f;
	bool backfaceCulling = true;
	bool frontfaceCulling = false;
};

FB_END_PACKAGE1()
