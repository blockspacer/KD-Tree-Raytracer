#include "Precompiled.h"
#include "KDNode.h"


#include "Triangle.h"

FB_PACKAGE1(assignment)
	/**
	 * \brief 
	 */
	void KDNode::expand()
	{
		if (triangles->getSize() < 3 || depth == 20)
		{
			return; //no more expansion
		}

		math::VC3 medianPoint{0, 0, 0};

		SizeType trisSize = triangles->getSize();

		for (SizeType i = 0; i < trisSize; i++)
		{
			medianPoint = medianPoint + (*triangles)[i].getMidPoint() * (1.0f / trisSize);
		}

		int8_t axisClipIndex = bbox->getLongestAxis();
		int32_t commonTris = 0;
		PodVector<Triangle>* leftTris = new PodVector<Triangle>;
		PodVector<Triangle>* rightTris = new PodVector<Triangle>;

		for (SizeType i = 0; i < triangles->getSize(); i++)
		{
			const ParallelPlaneSideResult result = (*triangles)[i].getParallelPlaneSide(medianPoint, axisClipIndex);
			const SizeType numRightVertices = result.rightSide->getSize();
			const SizeType numLeftVertices = result.leftSide->getSize();
			const SizeType numOnVertices = result.onSide->getSize();

			if (numRightVertices == 3 ||
				numRightVertices == 2 && numOnVertices == 1 ||
				numRightVertices == 1 && numOnVertices == 2
			)
			{
				rightTris->pushBack((*triangles)[i]);
			}
			else if (numLeftVertices == 3 ||
				numLeftVertices == 2 && numOnVertices == 1 ||
				numLeftVertices == 1 && numOnVertices == 2
			)
			{
				leftTris->pushBack((*triangles)[i]);
			}
			else if (numOnVertices == 0)
			{
				leftTris->pushBack((*triangles)[i]);
				rightTris->pushBack((*triangles)[i]);
			}
		}

		const math::VC3 rightChildMaxBound(
			axisClipIndex == 0 ? medianPoint.x : maxBounds.x,
			axisClipIndex == 1 ? medianPoint.y : maxBounds.y,
			axisClipIndex == 2 ? medianPoint.z : maxBounds.z
		);
		const math::VC3 leftChildMinBound(
			axisClipIndex == 0 ? medianPoint.x : minBounds.x,
			axisClipIndex == 1 ? medianPoint.y : minBounds.y,
			axisClipIndex == 2 ? medianPoint.z : minBounds.z
		);

		rightNode = new KDNode(rightTris, depth + 1, minBounds, rightChildMaxBound);
		leftNode = new KDNode(leftTris, depth + 1, leftChildMinBound, maxBounds);

		rightNode->expand();
		leftNode->expand();
	}

	bool KDNode::intersects(Ray& ray)
	{
		if (!bbox->intersects(ray))
		{
			return false;
		}
		if (rightNode != nullptr && leftNode != nullptr)
		{
			bool hitRight = rightNode->intersects(ray);
			bool hitLeft = leftNode->intersects(ray);
			return  hitRight || hitLeft;
		}
		bool somethingFound = false;
		for (SizeType i = 0; i < triangles->getSize(); i++)
		{
			const Triangle& tri = (*triangles)[i];
			if (tri.intersects(ray) == 1)
			{
				somethingFound = true;
			}
		}
	
		return somethingFound;
	}


FB_END_PACKAGE1()
