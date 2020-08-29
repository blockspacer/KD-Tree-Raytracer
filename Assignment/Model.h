#pragma once

#include "Triangle.h"

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)
class KDTree;
struct Ray;

class Model
{
public:
	void loadModel(const HeapString &fileName);
	void initializeKDTree();
	bool getIntersection(Ray &ray) const;

	bool isKDTreeInitialized() const;
	math::VC3 getBoundingBoxCenter() const;
	const math::VC3 &getMaxBounds() const;
	const math::VC3 &getMinBounds() const;

private:
	PodVector<math::VC3> vertices;
	PodVector<Triangle> polygons;
	math::VC3 maxBounds;
	math::VC3 minBounds;
	KDTree* kdTree;
};

FB_END_PACKAGE1()
