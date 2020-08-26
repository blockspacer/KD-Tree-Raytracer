#pragma once

#include "fb/container/PodVector.h"
#include "fb/math/Lerp.h"
#include "fb/math/curve/MultiCurveEnums.h"

FB_PACKAGE1(math)

struct MultiCurveKnot
{
	typedef float PointType;
	math::VC2 point;
	math::VC2 enterTangent = math::VC2(1, 0);
	math::VC2 leaveTangent = math::VC2(1, 0);
	MultiCurveTangentType enterType = MultiCurveTangentType::Manual;
	MultiCurveTangentType leaveType = MultiCurveTangentType::Manual;

	MultiCurveKnot()
	{
	}

	MultiCurveKnot(math::VC2 point, math::VC2 enterTangent, math::VC2 leaveTangent, uint8_t enterType, uint8_t leaveType)
		: point(point)
		, enterTangent(enterTangent)
		, leaveTangent(leaveTangent)
		, enterType(static_cast<MultiCurveTangentType>(enterType))
		, leaveType(static_cast<MultiCurveTangentType>(leaveType))
	{
		fb_expensive_assert(this->enterType <= MultiCurveTangentType::SteppedNext);
		fb_expensive_assert(this->leaveType <= MultiCurveTangentType::SteppedNext);
	}


	/* For sorting containers of Knots */
	bool operator < (const MultiCurveKnot &other) const
	{
		return this->point.x < other.point.x;
	}

	bool operator==(const MultiCurveKnot &other) const
	{
		return point == other.point && leaveTangent == other.leaveTangent && enterTangent == other.enterTangent;
	}

	bool operator!=(const MultiCurveKnot &other) const
	{
		return !(*this == other);
	}
};

struct MultiCurveEvaluator
{
	struct KnotType
	{
		float time;
		float point;
	};

	float evaluate(float t) const;
	const PodVector<KnotType> &getKnots() const
	{
		return knots;
	}
	
	PodVector<KnotType> knots;
};

struct MultiCurve
{
	typedef MultiCurveKnot KnotType;
	typedef KnotType::PointType PointType;

	const PodVector<KnotType> &getKnots() const
	{
		return knots;
	}
	PodVector<KnotType> &getMutableKnots()
	{
		return knots;
	}
	void setKnots(const PodVector<KnotType> &knotsParam);

	float evaluate(float t) const;

	static math::VC2 getEnterTangent(const PodVector<MultiCurveKnot> &knots, SizeType index);
	static math::VC2 getLeaveTangent(const PodVector<MultiCurveKnot> &knots, SizeType index);

	typedef MultiCurveEvaluator Evaluator; // TODO: Replace with a cached polyline aproximation
	Evaluator getEvaluator(float lossyCompression = 0.05f) const;

	bool operator==(const MultiCurve &other) const
	{
		if (getKnots().getSize() != other.getKnots().getSize())
			return false;

		for (SizeType i = 0; i < getKnots().getSize(); ++i)
		{
			if (getKnots()[i] != other.getKnots()[i])
				return false;
		}

		return true;
	}

	// For Lua, please don't use
	void addKnot(const KnotType &);
	void addKnotWithoutSorting(const KnotType &);
	void sortKnots();

private:
	PodVector<KnotType> knots;
};

FB_END_PACKAGE1()
