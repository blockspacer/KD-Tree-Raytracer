#pragma once

#include "Vec3.h"
#include "Quaternion.h"

FB_PACKAGE1(math)

template<class T>
class TPose
{
public:
	TPose()
	{
	}

	explicit TPose(const math::Vec3<T> &position)
		: position(position)
	{
	}

	explicit TPose(const math::Quaternion<T> &rotation)
		: rotation(rotation)
	{
	}

	explicit TPose(const math::Vec3<T> &position, const math::Quaternion<T> &rotation)
		: position(position)
		, rotation(rotation)
	{
	}

	TPose operator*(const TPose &other) const
	{
		return TPose(other.position + other.rotation.getRotated(position), rotation * other.rotation);
	}

	TPose getInverse() const
	{
		TPose res;
		res.rotation = rotation.getInverse();
		res.position = res.rotation.getRotated(-position);
		return res;
	}

	math::Vec3<T> getTransformed(const math::Vec3<T> &pos) const
	{
		return rotation.getRotated(pos) + position;
	}

	math::Quaternion<T> getTransformed(const math::Quaternion<T> &rot) const
	{
		return rot * rotation;
	}

	bool isNan() const
	{
		return position.isNaN() || rotation.isNaN();
	}

	bool isInf() const
	{
		return position.isInf() || rotation.isInf();
	}

	bool isFinite() const
	{
		return position.isFinite() && rotation.isFinite();
	}

	bool isDenormal() const
	{
		return position.isDenormal() || rotation.isDenormal();
	}

	static ClassId getStaticClassId() { return FB_FOURCC('P', 'o', 's', 'e'); }
	template<class S>
	bool stream(S &strm);

	math::Vec3<T> position;
	math::Quaternion<T> rotation;
};

typedef TPose<float> Pose;

FB_END_PACKAGE1()
