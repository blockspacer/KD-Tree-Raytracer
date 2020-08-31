#include "Precompiled.h"
#include "Triangle.h"

#include "Assignment/Ray.h"
#include "fb/lang/NumericLimits.h"

FB_PACKAGE1(assignment)
	Triangle::Triangle(math::VC3* a, math::VC3* b, math::VC3* c)
		: vertices{a, b, c}
	{
		fb_assert(a && b && c && "Passing a null vertex to a triangle");
		if (a->isInf() || a->isNaN() || b->isInf() || b->isNaN() || c->isInf() || c->
			isNaN())
		{
			FB_LOG_ERROR("Inf or NaN vertex passed to a triangle");

			/* Make normal invalid */
			normal = *a + *b + *c;
			return;
		}

		math::VC3 u = *b - *a;
		math::VC3 v = *c - *a;
		normal = u.getCrossWith(v);

		/* Our math::VC3::normalize assumes that the values are not close to zero and rounds small values to zero. 
		 * Calculate manually to get rid of the assumption */
		float len = normal.getLength();
		float iLen = 1.0f / len;
		normal.x *= iLen;
		normal.y *= iLen;
		normal.z *= iLen;

		if (isNormalInvalid())
		{
			FB_LOG_WARNING("Triangle with invalid normal created");
			return;
		}

		uDotV = u.getDotWith(v);
		fb_assert(
			uDotV > -lang::NumericLimits<float>::getMax() && uDotV < lang::
			NumericLimits<float>::getMax() && "Invalid uDotV in a triangle");
		uDotU = u.getSquareLength();
		vDotV = v.getSquareLength();
		denom = uDotV * uDotV - uDotU * vDotV;

		if (isDenomInvalid())
		{
			FB_LOG_WARNING("Invalid denom calculated for a triangle");
			return;
		}

		/* We expect all triangles to have area */
		if (normal.getSquareLength() > 0.0f && isInvalid())
		{
			FB_LOG_WARNING("Invalid triangle constructed");
			return;
		}
	}

	int Triangle::intersects(Ray& ray) const
	{
		/* This algorithm is from http://www.softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm .
		 * Algorithm presumes triangles have area. Since Triangle does too, this shouldn't cause any
		 * problems here. */
		/* First we check whether the ray intersects the plane of triangle */
		float rI = normal.getDotWith(ray.direction);
		/* If rI == 0, ray is at the plane of triangle. It may intersect or not. We don't care. */
		if (rI == 0)
			return 0;

		/* Xface culling */
		if (ray.backfaceCulling && rI > 0)
			return -4;

		if (ray.frontfaceCulling && rI < 0)
			return -4;

		rI = normal.getDotWith((*vertices[0] - ray.origin)) / rI;
		/* Now we actually have the rI that algorithm mentioned above talks about. rI < 0 => intersects
		 * nothing */
		if (rI < 0)
			return -3;

		/* rI > 0, so the ray intersects the plane. Let's first check if intersection point is near
		 * enough to be interesting. */
		math::VC3 rayVector = ray.direction * rI;
		float distSq = rayVector.getSquareLength();
		math::VC3 intPoint = ray.origin + rayVector;
		if (distSq > ray.bestDistanceSq)
			return -2;

		/* Buddha and dragon models have duplicate triangles and other errors. This is needed to tackle
		 * them in shadow tracing. This also actually triggers so often even without the errors that
		 * logging all occurrances has significant impact on performance. */
		if (distSq < ray.zeroDistanceThreshold)
			return -5;

		/* Close enough. Let's see where and whether that is inside the triangle. */
		/* Here's the original algorithm commented out. Below is optimized version, where we use
		 * pre-computed dot products and division. One might think this optimization is a great help,
		 * but actually (apparently) we very seldom reach even this point. Usually wrong triangles are
		 * dismissed earlier. Performance benefit was somewhere about around five percent, which isn't
		 * even that easy to measure, and of course depends on relative efficiency of rest of the
		 * program. */
		//Vector3 w = int_point - *vertices[0];
		//Vector3 u = *vertices[1] - *vertices[0];
		//Vector3 v = *vertices[2] - *vertices[0];
		//float u_dot_v = u.getDotWith(v);
		//float w_dot_v = w.getDotWith(v);
		//float w_dot_u = w.getDotWith(u);
		//float u_dot_u = u.getSquareLength();
		//float v_dot_v = v.getSquareLength();
		//float denom = u_dot_v * u_dot_v - u_dot_u * v_dot_v;
		//float sI = (u_dot_v * w_dot_v - v_dot_v * w_dot_u) / denom;
		//float tI = (u_dot_v * w_dot_u - u_dot_u * w_dot_v) / denom;
		math::VC3 w = intPoint - *vertices[0];
		math::VC3 u = *vertices[1] - *vertices[0];
		math::VC3 v = *vertices[2] - *vertices[0];
		float wDotV = w.getDotWith(v);
		float wDotU = w.getDotWith(u);
		fb_assert(
			wDotV > -lang::NumericLimits<float>::getMax() && wDotV < lang::
			NumericLimits<float>::getMax());
		fb_assert(
			wDotU > -lang::NumericLimits<float>::getMax() && wDotU < lang::
			NumericLimits<float>::getMax());
		float sI = (this->uDotV * wDotV - this->vDotV * wDotU) / this->denom;
		float tI = (this->uDotV * wDotU - this->uDotU * wDotV) / this->denom;
		fb_assert(
			sI > -lang::NumericLimits<float>::getMax() && sI < lang::NumericLimits<
			float>::getMax());
		fb_assert(
			tI > -lang::NumericLimits<float>::getMax() && tI < lang::NumericLimits<
			float>::getMax());
		/* Precalculating the division actually caused algorithm to fail, when intersection was very
		 * near or at the edge. Using doubles instead of floats corrected the problem, but would go
		 * against original assignment from Housemarque. */
		//float sI = this->uDotVPerDenom * wDotV - this->vDotVPerDenom * wDotU;
		//float tI = this->uDotVPerDenom * wDotU - this->uDotUPerDenom * wDotV;
		float sIPlusTI = sI + tI;
		fb_assert(
			sIPlusTI > -lang::NumericLimits<float>::getMax() && sIPlusTI < lang::
			NumericLimits<float>::getMax());
		/* First line (ors) corresponds to intersection point being at the edge of the triangle. */
		if (sI > 0.0f && tI > 0.0f && sIPlusTI < 1.0f)
		{
			ray.intersectionPoint = intPoint;
			ray.bestDistanceSq = distSq;
			ray.bestTriangle = this;
			return 1;
		}
		else
		{
			return -1;
		}
	}


	const math::VC3 Triangle::getMidPoint() const
	{
		return (*vertices[0] + *vertices[1] + *vertices[2]) / 3;
	}

	/*
		  -2  = Error
		  -1  = Left Side
		   0  = Both Side
		   1  = Right Side
	*/
	const ParallelPlaneSideResult Triangle::getParallelPlaneSide(const math::VC3& point, int8_t perpendicularTo) const
	{
		ParallelPlaneSideResult pResut = {
			-2,
			new PodVector<math::VC3>,
			new PodVector<math::VC3>,
			new PodVector<math::VC3>
		};
		for (math::VC3* const v : vertices)
		{
			auto vertex = *v;
			switch (perpendicularTo)
			{
			case 0:
				if (vertex.x > point.x)
					pResut.leftSide->pushBack(vertex);
				else if (vertex.x == point.x)
					pResut.onSide->pushBack(vertex);
				else
					pResut.rightSide->pushBack(vertex);
				break;
			case 1:
				if (vertex.y > point.y)
					pResut.leftSide->pushBack(vertex);
				else if (vertex.y == point.y)
					pResut.onSide->pushBack(vertex);
				else
					pResut.rightSide->pushBack(vertex);
				break;
			case 2:
				if (vertex.z > point.z)
					pResut.leftSide->pushBack(vertex);
				else if (vertex.z == point.z)
					pResut.onSide->pushBack(vertex);
				else
					pResut.rightSide->pushBack(vertex);
				break;
			}
		}
		return pResut;
	}

	bool Triangle::isInvalid() const
	{
		return isNormalInvalid() || isDenomInvalid();
	}

	const math::VC3& Triangle::getNormal() const
	{
		return normal;
	}

	bool Triangle::isNormalInvalid() const
	{
		return normal.isInf() || normal.isNaN();
	}

	bool Triangle::isDenomInvalid() const
	{
		return lang::abs<float>(denom) < lang::NumericLimits<float>::getMin();
	}

FB_END_PACKAGE1()
