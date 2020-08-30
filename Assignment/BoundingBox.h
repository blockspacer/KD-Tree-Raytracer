#pragma once

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)
struct Ray;

class BoundingBox
{
public:
	BoundingBox(const math::VC3& cornerMin, const math::VC3& cornerMax):cornerMin(cornerMin), cornerMax(cornerMax) {};
	bool intersects(Ray& ray) const;
	int8_t getLongestAxis() const;
	
private:
	const math::VC3 cornerMin;
	const math::VC3 cornerMax;
};


FB_END_PACKAGE1()


