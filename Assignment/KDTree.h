#pragma once

FB_DECLARE(assignment, KDNode)
FB_DECLARE_STRUCT(assignment, Ray)

#include "KDNode.h"

FB_PACKAGE1(assignment)


class KDTree {
public:
  KDTree(const PodVector<Triangle> &tris, const math::VC3 &minBounds,
         const math::VC3 &maxBounds);
  void init();
  bool intersects(Ray& ray);
  const KDNode *getRoot() const;
private:
  KDNode *root;
  const PodVector<Triangle> &totalTriangles;
  const math::VC3 &minBounds, maxBounds;
};

FB_END_PACKAGE1()
