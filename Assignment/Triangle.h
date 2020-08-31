#pragma once

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)

struct Ray;

struct ParallelPlaneSideResult
{
	int8_t result;
	PodVector<math::VC3>* rightSide;
	PodVector<math::VC3>* leftSide;
	PodVector<math::VC3>* onSide;
};

class Triangle
{
public:
	Triangle(math::VC3 *a, math::VC3 *b, math::VC3 *c);

	int intersects(Ray &ray) const;
	bool isInvalid() const;

	///<summary>Returns the mid point of the triangle.</summary>
	const math::VC3 getMidPoint() const;

	///<summary>Returns a struct containing some vectors tha include vertices of this triangle based on if they fall on either side of the parallel plane.</summary>
	/// <param name="point">Gets a point on the plane.</param>
	/// <param name="perpendicularTo">Gets the axis the plane is perpendicular to.</param>
	const ParallelPlaneSideResult getParallelPlaneSide(const math::VC3& point, int8_t perpendicularTo) const;

	
	const math::VC3& getNormal() const;

private:
	bool isNormalInvalid() const;
	bool isDenomInvalid() const;

	/* We do not own these */
	math::VC3 *vertices[3] = { nullptr, nullptr, nullptr };
	math::VC3 normal;
	float uDotV;
	float uDotU;
	float vDotV;
	float denom;

};

FB_END_PACKAGE1()
