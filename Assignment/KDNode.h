#pragma once

#include "BoundingBox.h"

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)

class Triangle;
struct Ray;

class KDNode {
public:
  const BoundingBox* bbox;
  const PodVector<Triangle>* triangles;

  KDNode *leftNode;
  KDNode *rightNode;


  KDNode(const PodVector<Triangle>* tris, const int32_t depth, const math::VC3 &minBounds,
         const math::VC3 &maxBounds)
    : triangles(tris),
      depth(depth),
      minBounds(minBounds),
      maxBounds(maxBounds) {
	bbox = new BoundingBox(minBounds, maxBounds);
    leftNode = nullptr;
    rightNode = nullptr;

  }

  bool intersects(Ray& ray);
  void expand();

private:
  const int32_t depth;
  const math::VC3 minBounds;
  const math::VC3 maxBounds;


};

FB_END_PACKAGE1()
