#include "Precompiled.h"

#include "BoundingBox.h"
#include "Assignment/Ray.h"
#include "minmax.h"
#include "fb/lang/Swap.h"



FB_PACKAGE1(assignment)

BoundingBox::BoundingBox(math::VC3& cornerMax, math::VC3& cornerMin):cornerMin(cornerMin), cornerMax(cornerMax) {}

bool BoundingBox::intersects(Ray& ray) const
{

	//test if parallel
	
	if(
		ray.direction.x == 0 && (ray.origin.x < this->cornerMin.x || ray.origin.x > this->cornerMax.x) ||
		ray.direction.y == 0 && (ray.origin.y < this->cornerMin.y || ray.origin.y > this->cornerMax.y) ||
		ray.direction.z == 0 && (ray.origin.z < this->cornerMin.z || ray.origin.z > this->cornerMax.z))
	{
		return false;
	}
		// X
		float txMin = (this->cornerMin.x - ray.origin.x) / ray.direction.x;
		float txMax = (this->cornerMax.x - ray.origin.x) / ray.direction.x;
		if (txMin > txMax) lang::swap(txMin, txMax);

		// Y
		float tyMin = (this->cornerMin.y - ray.origin.y) / ray.direction.y;
		float tyMax = (this->cornerMax.y - ray.origin.y) / ray.direction.y;
		if (tyMin > tyMax) lang::swap(tyMin, tyMax);

		// Z
		float tzMin = (this->cornerMin.z - ray.origin.z) / ray.direction.z;
		float tzMax = (this->cornerMax.z - ray.origin.z) / ray.direction.z;
		if (tzMin > tzMax) lang::swap(tzMin, tzMax);

		float tStart = max(txMin, max(tyMin, tzMin));
		float tEnd = min(txMax, min(tyMax, tzMax));

		if (tStart > tEnd || tEnd <0) //box is missed or behind
			return false;
	
	return true;
}

FB_END_PACKAGE1()