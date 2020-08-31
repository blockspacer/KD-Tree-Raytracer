#pragma once

FB_DECLARE(assignment, KDNode)
FB_DECLARE_STRUCT(assignment, Ray)

#include "KDNode.h"

FB_PACKAGE1(assignment)

///<summary>KDTree class is used to construct a K-D Tree object to accelerate the ray tracing process by using spatial subdivision.</summary>
/// <param name="tris">Gets the vector containing all the triangles in the scene.</param>
/// <param name="minBounds">Gets the coordination of the minimum boundary of the scene.</param>
/// <param name="maxBounds">Gets the coordination of the maximum boundary of the scene.</param>
class KDTree {
public:
	
  KDTree(const PodVector<Triangle> *tris, const math::VC3 &minBounds,
         const math::VC3 &maxBounds);

	///<summary>Initializes the K-D Tree. This process may take up some time depends on the number of triangles and some internal tree parameters.</summary>
  void init();

	///<summary>Traverses the K-D Tree to find if the given ray interacts with any triangle.</summary>
	/// <param name="ray">Gets the ray to check the intersection upon.</param>
  bool intersects(Ray& ray);

	///<summary>Returns the root of the K-D Tree.</summary>
  const KDNode *getRoot() const;
	
private:
  KDNode *root;
  const PodVector<Triangle>* totalTriangles;
  const math::VC3 &minBounds, maxBounds;
};

FB_END_PACKAGE1()
