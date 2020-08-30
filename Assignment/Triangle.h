#pragma once

FB_DECLARE_STRUCT(assignment, Ray)

FB_PACKAGE1(assignment)

struct Ray;

struct ParallelPlaneSideResult
{
	int8_t result;
	PodVector<math::VC3>* rightSide;
	PodVector<math::VC3>* leftSide;
};

class Triangle
{
public:
	Triangle(math::VC3 *a, math::VC3 *b, math::VC3 *c);

	int intersects(Ray &ray) const;
	bool isInvalid() const;
	const math::VC3 getMidPoint() const;
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
