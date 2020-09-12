#pragma once

#include "BoundingBox.h"

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)

class Triangle;
struct Ray;

///<summary>KDNode represents each node in the K-D Tree.</summary>
/// <param name="tris">Gets the vector containing all the triangles the fit into this node.</param>
/// <param name="depth">Gets the depth level of tree that this node falls into.</param>
/// <param name="minBounds">Gets the coordination of the minimum boundary of this node.</param>
/// <param name="maxBounds">Gets the coordination of the maximum boundary of this node.</param>
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
	
  ///<summary>Traverses the K-D Tree to find the best node hitting the given ray.</summary>
  /// <param name="ray">Gets the ray to check the intersection upon. The intersection check continues throughout children recursively.</param>
  bool intersects(Ray& ray);

  ///<summary>Expand this node throughout its children recursively. This process stops when it reaches to some limit set in the code.</summary>
  void expand();

   ~KDNode();

private:
  const int32_t depth;
  const math::VC3 minBounds;
  const math::VC3 maxBounds;


};

FB_END_PACKAGE1()
