#pragma once

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)
struct Ray;

///<summary>Bounding Box object that is used to check the intersection of the ray with each K-D Tree node.</summary>
/// <param name="cornerMin">Gets the coordination of the minimum boundary of this node.</param>
/// <param name="cornerMax">Gets the coordination of the maximum boundary of this node.</param>
class BoundingBox
{
public:
	BoundingBox(const math::VC3& cornerMin, const math::VC3& cornerMax):cornerMin(cornerMin), cornerMax(cornerMax) {};

	///<summary>Checks if the given node intersects with the bonding box.</summary>
	/// <param name="ray">Gets the ray to check the intersection upon.</param>
	bool intersects(Ray& ray) const;

	///<summary>Returns the longest axis of the bounding box.</summary>
	int8_t getLongestAxis() const;
	
private:
	const math::VC3 cornerMin;
	const math::VC3 cornerMax;
};


FB_END_PACKAGE1()


