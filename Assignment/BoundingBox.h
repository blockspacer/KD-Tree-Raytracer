#pragma once

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)
struct Ray;

class BoundingBox
{
public:
	BoundingBox(math::VC3& cornerMax, math::VC3& cornerMin);
	bool intersects(Ray& ray) const;
	
private:
	math::VC3 cornerMin, cornerMax;
};

FB_END_PACKAGE1()


