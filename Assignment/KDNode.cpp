#include "Precompiled.h"
#include "KDNode.h"


#include "Triangle.h"

FB_PACKAGE1(assignment)


void KDNode::expand() {
  //this->triangles = tris;
  //this->leftNode = nullptr;
  //this->rightNode = nullptr;
  //this->bbox = BoundingBox(maxBounds, minBounds);

  if (triangles.getSize() < 3 || depth == 20) {
    return; //no more expansion
  }

  math::VC3 medianPoint{0, 0, 0};

  SizeType trisSize = triangles.getSize();

  for (SizeType i = 0; i < trisSize; i++) {
    medianPoint = medianPoint + triangles[i].getMidPoint() * (1.0f / trisSize);
  }

  PodVector<Triangle> leftTris;
  PodVector<Triangle> rightTris;

  int8_t axixClipIndex = depth % 3;
  int32_t commonTris = 0;

  for (SizeType i = 0; i < triangles.getSize(); i++) {
    int8_t side = triangles[i].getSideFromPoint(medianPoint, axixClipIndex);
    fb_assert(side == -1 || side == 0 || side == 1);

    switch (side) {
      // Left
    case -1:
      leftTris.pushBack(triangles[i]);
      break;
      // Both
    case 0:
      leftTris.pushBack(triangles[i]);
      rightTris.pushBack(triangles[i]);
      commonTris++;
      break;
      // Right
    case 1:
      rightTris.pushBack(triangles[i]);
      break;
    }
  }

  const math::VC3 rightChildMaxBound(
      (axixClipIndex == 0) ? medianPoint.x : maxBounds.x,
      (axixClipIndex == 1) ? medianPoint.y : maxBounds.y,
      (axixClipIndex == 2) ? medianPoint.z : maxBounds.z
      );
  const math::VC3 leftChildMinBound(
      (axixClipIndex == 0) ? medianPoint.x : minBounds.x,
      (axixClipIndex == 1) ? medianPoint.y : minBounds.y,
      (axixClipIndex == 2) ? medianPoint.z : minBounds.z
      );

  rightNode = new KDNode(rightTris, depth + 1, minBounds, rightChildMaxBound);
  leftNode = new KDNode(leftTris, depth + 1, leftChildMinBound, maxBounds);

  rightNode->expand();
  leftNode->expand();
}

bool KDNode::intersects(Ray &ray) {
  if (!bbox->intersects(ray)) {
    return false;
  }
  if (rightNode != nullptr && leftNode != nullptr) {
    return rightNode->intersects(ray) || leftNode->intersects(ray);
  }
  bool somethingFound = false;
  for (SizeType i = 0; i < triangles.getSize(); ++i) {
    const Triangle &tri = triangles[i];
    if (tri.intersects(ray) == 1) {
      somethingFound = true;
    }
  }
  return somethingFound;

}


FB_END_PACKAGE1()
