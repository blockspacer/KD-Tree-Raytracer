#pragma once

#include "fb/lang/NumericLimits.h"
#include "fb/math/Vec3.h" // For getTransformed
#include "fb/math/Matrix.h" // For getTransformed

FB_PACKAGE1(math)

template<class FloatType>
class TBounds
{
public:
	TBounds()
		: boundsMin(lang::NumericLimits<FloatType>::getHighest(), lang::NumericLimits<FloatType>::getHighest(), lang::NumericLimits<FloatType>::getHighest())
		, boundsMax(lang::NumericLimits<FloatType>::getLowest(), lang::NumericLimits<FloatType>::getLowest(), lang::NumericLimits<FloatType>::getLowest())
	{
	}

	TBounds(const math::Vec3<FloatType> &boundsMin, const math::Vec3<FloatType> &boundsMax)
		: boundsMin(boundsMin)
		, boundsMax(boundsMax)
	{
	}

	bool isValid() const
	{
		return boundsMin.x <= boundsMax.x && boundsMin.y <= boundsMax.y && boundsMin.z <= boundsMax.z;
	}

	void setZero()
	{
		boundsMin.z = boundsMin.y = boundsMin.x = 0;
		boundsMax.z = boundsMax.y = boundsMax.x = 0;
	}

	void setEmpty()
	{
		boundsMin.z = boundsMin.y = boundsMin.x = lang::NumericLimits<FloatType>::getHighest();
		boundsMax.z = boundsMax.y = boundsMax.x = lang::NumericLimits<FloatType>::getLowest();
	}
	
	void include(const math::Vec3<FloatType> &other)
	{
		boundsMin.x = lang::min(other.x, boundsMin.x);
		boundsMin.y = lang::min(other.y, boundsMin.y);
		boundsMin.z = lang::min(other.z, boundsMin.z);
		
		boundsMax.x = lang::max(other.x, boundsMax.x);
		boundsMax.y = lang::max(other.y, boundsMax.y);
		boundsMax.z = lang::max(other.z, boundsMax.z);
	}

	void include(const TBounds &other)
	{
		boundsMin.x = lang::min(other.boundsMin.x, boundsMin.x);
		boundsMin.y = lang::min(other.boundsMin.y, boundsMin.y);
		boundsMin.z = lang::min(other.boundsMin.z, boundsMin.z);
		
		boundsMax.x = lang::max(other.boundsMax.x, boundsMax.x);
		boundsMax.y = lang::max(other.boundsMax.y, boundsMax.y);
		boundsMax.z = lang::max(other.boundsMax.z, boundsMax.z);
	}

	// note: if there's no overlap, will generate invalid bounds
	void intersect(const TBounds &other)
	{
		boundsMin.x = lang::max(other.boundsMin.x, boundsMin.x);
		boundsMin.y = lang::max(other.boundsMin.y, boundsMin.y);
		boundsMin.z = lang::max(other.boundsMin.z, boundsMin.z);
		
		boundsMax.x = lang::min(other.boundsMax.x, boundsMax.x);
		boundsMax.y = lang::min(other.boundsMax.y, boundsMax.y);
		boundsMax.z = lang::min(other.boundsMax.z, boundsMax.z);
	}

	bool testOverlap(const TBounds &other) const
	{
		fb_assert(isValid());
		fb_assert(other.isValid());
		return boundsMin.x < other.boundsMax.x && boundsMax.x > other.boundsMin.x
			&& boundsMin.y < other.boundsMax.y && boundsMax.y > other.boundsMin.y
			&& boundsMin.z < other.boundsMax.z && boundsMax.z > other.boundsMin.z;
	}

	bool testContains(const TBounds &other) const
	{
		// other is inside this
		fb_assert(isValid());
		fb_assert(other.isValid());
		return boundsMin.x <= other.boundsMin.x && boundsMax.x >= other.boundsMax.x
			&& boundsMin.y <= other.boundsMin.y && boundsMax.x >= other.boundsMax.y
			&& boundsMin.z <= other.boundsMin.z && boundsMax.x >= other.boundsMax.z;
	}

	bool testSphereOverlap(const math::Vec3<FloatType> &pos, FloatType radius) const
	{
		fb_assert(isValid());
		const math::Vec3<FloatType> nearestPoint(lang::min(lang::max(pos.x, boundsMin.x), boundsMax.x),
			lang::min(lang::max(pos.y, boundsMin.y), boundsMax.y),
			lang::min(lang::max(pos.z, boundsMin.z), boundsMax.z));
		return (pos - nearestPoint).getSquareLength() <= radius*radius;
	}

	FloatType getSquareDistanceToPoint(const math::Vec3<FloatType> &pos) const
	{
		fb_assert(isValid());
		const math::Vec3<FloatType> nearestPoint(lang::min(lang::max(pos.x, boundsMin.x), boundsMax.x),
			lang::min(lang::max(pos.y, boundsMin.y), boundsMax.y),
			lang::min(lang::max(pos.z, boundsMin.z), boundsMax.z));
		return (pos - nearestPoint).getSquareLength();
	}
	
	bool testPoint(const math::Vec3<FloatType> &pos) const
	{
		fb_assert(isValid());
		return pos.x >= boundsMin.x && pos.x <= boundsMax.x
			&& pos.y >= boundsMin.y && pos.y <= boundsMax.y
			&& pos.z >= boundsMin.z && pos.z <= boundsMax.z;
	}

	bool testPoint(const math::Vec3<FloatType> &pos, float bias) const
	{
		fb_assert(isValid());
		return pos.x + bias >= boundsMin.x && pos.x <= boundsMax.x + bias
			&& pos.y + bias >= boundsMin.y && pos.y <= boundsMax.y + bias
			&& pos.z + bias >= boundsMin.z && pos.z <= boundsMax.z + bias;
	}

	void move(const math::Vec3<FloatType> &shift)
	{
		boundsMin.x += shift.x;
		boundsMin.y += shift.y;
		boundsMin.z += shift.z;

		boundsMax.x += shift.x;
		boundsMax.y += shift.y;
		boundsMax.z += shift.z;
	}

	math::Vec3<FloatType> getCenter() const
	{
		fb_assert(isValid());
		return (boundsMin + boundsMax) * 0.5f;
	}
	
	math::Vec3<FloatType> getHalfSize() const
	{
		fb_assert(isValid());
		return (boundsMax - boundsMin) * 0.5f;
	}

	FloatType getBoundingSphereRadius() const
	{
		return getHalfSize().getLength();
	}

	FloatType getLargestDimension() const
	{
		fb_assert(isValid());
		math::Vec3<FloatType> size(boundsMax - boundsMin);
		return lang::max(lang::max(size.x, size.y), size.z);
	}
	
	math::Vec3<FloatType> getSize() const
	{
		fb_assert(isValid());
		return (boundsMax - boundsMin);
	}
	
	const math::Vec3<FloatType> &getMin() const
	{
		return boundsMin;
	}
	
	const math::Vec3<FloatType> &getMax() const
	{
		return boundsMax;
	}

	TBounds operator*(FloatType scale) const
	{
		return TBounds(boundsMin * scale, boundsMax * scale);
	}

	TBounds getTransformed(const math::Vec3<FloatType> &pos, const math::Quaternion<FloatType> &rot) const
	{
		fb_assert(isValid());
		math::Vec3<FloatType> axes[3];
		rot.getAxes(axes);
		math::Vec3<FloatType> localCenter = (boundsMin + boundsMax) * 0.5f;
		math::Vec3<FloatType> localHalfSize = (boundsMax - boundsMin) * 0.5f;
		math::Vec3<FloatType> center(
			axes[0].x * localCenter.x + axes[1].x * localCenter.y + axes[2].x * localCenter.z,
			axes[0].y * localCenter.x + axes[1].y * localCenter.y + axes[2].y * localCenter.z,
			axes[0].z * localCenter.x + axes[1].z * localCenter.y + axes[2].z * localCenter.z);
		center += pos;
		math::Vec3<FloatType> halfSize(
			abs(axes[0].x) * localHalfSize.x + abs(axes[1].x) * localHalfSize.y + abs(axes[2].x) * localHalfSize.z,
			abs(axes[0].y) * localHalfSize.x + abs(axes[1].y) * localHalfSize.y + abs(axes[2].y) * localHalfSize.z,
			abs(axes[0].z) * localHalfSize.x + abs(axes[1].z) * localHalfSize.y + abs(axes[2].z) * localHalfSize.z);
		return TBounds(center - halfSize, center + halfSize);
	}
	
	TBounds getTransformed(const math::Vec3<FloatType> &pos, const math::Quaternion<FloatType> &rot, FloatType scale) const
	{
		fb_assert(isValid());
		math::Vec3<FloatType> axes[3];
		rot.getAxes(axes);
		FloatType halfScale = scale * 0.5f;
		math::Vec3<FloatType> localCenter = (boundsMin + boundsMax) * halfScale;
		math::Vec3<FloatType> localHalfSize = (boundsMax - boundsMin) * halfScale;
		math::Vec3<FloatType> center(
			axes[0].x * localCenter.x + axes[1].x * localCenter.y + axes[2].x * localCenter.z,
			axes[0].y * localCenter.x + axes[1].y * localCenter.y + axes[2].y * localCenter.z,
			axes[0].z * localCenter.x + axes[1].z * localCenter.y + axes[2].z * localCenter.z);
		center += pos;
		math::Vec3<FloatType> halfSize(
			abs(axes[0].x) * localHalfSize.x + abs(axes[1].x) * localHalfSize.y + abs(axes[2].x) * localHalfSize.z,
			abs(axes[0].y) * localHalfSize.x + abs(axes[1].y) * localHalfSize.y + abs(axes[2].y) * localHalfSize.z,
			abs(axes[0].z) * localHalfSize.x + abs(axes[1].z) * localHalfSize.y + abs(axes[2].z) * localHalfSize.z);
		return TBounds(center - halfSize, center + halfSize);
	}

	TBounds getTransformed(const math::TMatrix<FloatType> &transform) const
	{
		fb_assert(isValid());
		const FloatType *m = transform.getAsFloat();
		math::Vec3<FloatType> localCenter = (boundsMin + boundsMax) * 0.5f;
		math::Vec3<FloatType> localHalfSize = (boundsMax - boundsMin) * 0.5f;
		math::Vec3<FloatType> center(transform.getTransformed(localCenter));
		math::Vec3<FloatType> halfSize(
			abs(m[0*4+0]) * localHalfSize.x + abs(m[1*4+0]) * localHalfSize.y + abs(m[2*4+0]) * localHalfSize.z,
			abs(m[0*4+1]) * localHalfSize.x + abs(m[1*4+1]) * localHalfSize.y + abs(m[2*4+1]) * localHalfSize.z,
			abs(m[0*4+2]) * localHalfSize.x + abs(m[1*4+2]) * localHalfSize.y + abs(m[2*4+2]) * localHalfSize.z);
		return TBounds(center - halfSize, center + halfSize);
	}

	template<class T>
	void readFromStream(T &strm)
	{
		strm.read(boundsMin.x);
		strm.read(boundsMin.y);
		strm.read(boundsMin.z);
		strm.read(boundsMax.x);
		strm.read(boundsMax.y);
		strm.read(boundsMax.z);
	}

	template<class T>
	void writeToStream(T &strm) const
	{
		strm.write(boundsMin.x);
		strm.write(boundsMin.y);
		strm.write(boundsMin.z);
		strm.write(boundsMax.x);
		strm.write(boundsMax.y);
		strm.write(boundsMax.z);
	}

	math::Vec3<FloatType> boundsMin, boundsMax;
};

typedef TBounds<float> Bounds;

FB_END_PACKAGE1()
