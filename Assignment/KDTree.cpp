#include "Precompiled.h"
#include "KDTree.h"

FB_PACKAGE1(assignment)

KDTree::KDTree(const PodVector<Triangle> *tris, const math::VC3 &minBounds,
               const math::VC3 &maxBounds)
  : totalTriangles(tris), minBounds(minBounds), maxBounds(maxBounds) {
}

void KDTree::init() {
  root = new KDNode(totalTriangles, 0, minBounds, maxBounds);
  root->expand();
}

bool KDTree::intersects(Ray &ray) {
  return root->intersects(ray);
}


const KDNode *KDTree::getRoot() const {
  return root;
}


FB_END_PACKAGE1()
