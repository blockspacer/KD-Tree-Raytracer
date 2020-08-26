#pragma once

#include "fb/math/Vec3.h"
#include "fb/lang/FBAssert.h"

// no crap dependencies... make utils out of these if needed.
//#include "Quaternion.h"

FB_PACKAGE1(math)

template <class A> class TPlane
{
public:
	// Data (public for speed)
	Vec3<A> planeNormal;
	A rangeToOrigin;

	// going with this "no default constructor" available policy until needed.
	// (since the default plane will be quite a bogus plane anyway)
	/*
	TPlane() 
		: planeNormal(0,1,0)
		, rangeToOrigin(0) 
	{
		// nop
	}
	*/

	TPlane(const Vec3<A> &planeNormal, A rangeToOrigin)
		: planeNormal(planeNormal) 
		, rangeToOrigin(rangeToOrigin)
	{
	}


	static TPlane constructFromNormalAndPosition(const Vec3<A> &planeNormal, const Vec3<A> &planepos)
	{
		return TPlane(planeNormal, planeNormal.getDotWith(planepos));
	}


	static TPlane makeFromPoints(const Vec3<A> &point1, const Vec3<A> &point2, const Vec3<A> &point3, const Vec3<A> &nonPlanarFailsafePlaneNormal)
	{
		Vec3<A> edge1 = point2 - point1;
		Vec3<A> edge2 = point3 - point1;
		Vec3<A> planeNormal = edge1.getCrossWith(edge2);
		planeNormal.normalizeWithZeroFailsafe(nonPlanarFailsafePlaneNormal);
		return TPlane(planeNormal, planeNormal.getDotWith(point1));
	}

	/**
	 * @param nonPlanarFailsafePlaneNormal, Vec3<A> a failsafe value in case the points do not define a plane (they are the same 
	 * point or all of them are on the same line)
	 * the failsafe value determine the plane normal direction is such a case.
	 */
	TPlane(const Vec3<A> &point1, const Vec3<A> &point2, const Vec3<A> &point3, const Vec3<A> &nonPlanarFailsafePlaneNormal)
	{
		makeFromPoints(point1, point2, point3, nonPlanarFailsafePlaneNormal);
	}

	/**
	 * Note, returns negative values if point is behind plane.
	 */
	A getPointRange(const Vec3<A> &point) const
	{
		return planeNormal.getDotWith(point) - rangeToOrigin;
	}


	// Operators
	bool operator==(const TPlane& other) const
	{
		static const float PLANE_EPS = 0.00001f;

		if ((fabs(planeNormal.x-other.planeNormal.x)<PLANE_EPS)&&
			(fabs(planeNormal.y-other.planeNormal.y)<PLANE_EPS)&&
			(fabs(planeNormal.z-other.planeNormal.z)<PLANE_EPS)&&
			(fabs(rangeToOrigin-other.rangeToOrigin)<PLANE_EPS)) 
		{
			return true;
		} else {
			return false;
		}
	}

	// Vector modify...
	void mirrorVector(Vec3<A> &vectorInOut) const
	{
		A range=planeNormal.getDotWith(vectorInOut)-rangeToOrigin;
		vectorInOut-=planeNormal*(2*range);
	}

	void flattenVector(Vec3<A> &vectorInOut, float amount) const
	{
		A range=planeNormal.getDotWith(vectorInOut)-rangeToOrigin;
		vectorInOut-=planeNormal*(amount*range);
	}

	void projectVector(Vec3<A> &vectorInOut) const
	{	
		A range=planeNormal.getDotWith(vectorInOut)-rangeToOrigin;
		vectorInOut-=planeNormal*range;
	}

	const Vec3<A> getMirroredVector(const Vec3<A> &vector) const
	{
		Vec3<A> temp=vector;
		mirrorVector(temp);
		return temp;
	}

	const Vec3<A> getFlattenedVector(const Vec3<A> &vector,float amount) const
	{
		Vec3<A> temp=vector;
		flattenVector(temp);
		return temp;
	}

	const Vec3<A> getProjectedVector(const Vec3<A> &vector) const
	{	
		Vec3<A> temp=vector;
		projectVector(temp);
		return temp;
	}

	bool getClip ( const Vec3<A> p1, const Vec3<A> p2, Vec3<A> * out ) const
	{
		// Checks if line defined by points p1 and p2 intersects the plane.
		// If it does, and if out != nullptr, out will be the intersection point.
		// Otherwise returns false.

		float p1Range = getPointRange( p1 );
		float p2Range = getPointRange( p2 );

		if (p1Range == 0)
		{
			if (out)
			{
				*out = p1;
			}
			return true;
		}
		if (p2Range == 0)
		{
			if (out)
			{
				*out = p2;
			}
			return true;
		}


		bool p1Inside = (p1Range < 0);
		bool p2Inside = (p2Range < 0);
		if ( p1Inside == p2Inside )
			return false;

		const Vec3<A> q0= planeNormal * rangeToOrigin;
		const Vec3<A> q1= q0 - p1;
		const Vec3<A> q2= p2 - p1;
		const A w1 = q1.getDotWith(planeNormal);
		const A w2 = q2.getDotWith(planeNormal);

		fb_assert( w2 != 0 ); // w2 should be non-zero because we already checked if at least one of the points are on the plane.

		A w3 = w1 / w2;

		if (w3 < 0.0f) w3 = 0.0f;
		if (w3 > 1.0f) w3 = 1.0f;
		//fb_assert( w3 >= 0 && w3 <= 1 ); // w3 should be at range [0,1] since we already checked that intersection occurs for sure.

		if (out)
			*out = p1 + q2 * w3;

		return true;
	}


	// note, you are not allowed to make unnormalized planes at the first place.
	/*
	void normalize()
	{
		A len = planeNormal.GetLength();
		planeNormal.Normalize();
		rangeToOrigin = rangeToOrigin / len;
	}
	*/

/*
	void clipLine(const VC3 &p1, const VC3 &p2, VC3 &result)
	{
		float d1 = GetPointRange(p1);
		float d2 = GetPointRange(p2);
		if (d1 < 0 && d2 < 0)
			return;
		if (d1 > 0 && d2 > 0)
			return;

		float s = (d1 / (d1 - d2));
		result = p1 + ((p2 - p1) * s);
	}
*/

	// these oughta be utils.
	/*
	TPlane getTransformed(const TMatrix<A> &matrix) const
	{
		TPlane temp(*this);
		temp.Transform(matrix);
		return temp;
	}

	TPlane getRotated(const Quaternion<A> &rotation) const
	{
		TPlane temp(*this);
		temp.Rotate(rotation);
		return temp;
	}

	// Functions (these modify this plane)
	void Transform(const TMatrix<A> &matrix)
	{
		// Calculate position first
		Vec3<A> pos=planeNormal*rangeToOrigin;
		matrix.TransformVector(pos);
		//matrix.GetWithoutTranslation().GetInverse().GetTranspose().TransformVector(planeNormal);
		TMatrix<A> tmp = matrix;
		tmp.Inverse();
		tmp = tmp.GetTranspose();
		tmp.RotateVector(planeNormal);

		// Calculate new range to origin
		rangeToOrigin=planeNormal.GetDotWith(pos);
	}

	void Rotate(const Quaternion<A> &rotation)
	{
		// Calculate position first
		Vec3<A> pos=planeNormal*rangeToOrigin;

		// Convert the quaternion to matrix first
		// Optimized code... Messy but fast... (even a P4 should survive with these 9 mults;)
		A x2=rotation.x+rotation.x;
		A y2=rotation.y+rotation.y;
		A z2=rotation.z+rotation.z;

		A xx2=rotation.x*x2; 
		A yy2=rotation.y*y2;
		A zz2=rotation.z*z2;
		A xy2=rotation.x*y2;
		A xz2=rotation.x*z2;
		A yz2=rotation.y*z2;
		A wx2=rotation.w*x2;
		A wy2=rotation.w*y2;
		A wz2=rotation.w*z2;
    
		A mat[9];
		mat[0]=1.0f-yy2+zz2;
		mat[1]=xy2-wz2;
		mat[2]=xz2+wy2;

		mat[3]=xy2+wz2;
		mat[4]=1.0f-xx2+zz2;
		mat[5]=yz2-wx2;

		mat[6]=xz2-wy2;
		mat[7]=yz2+wx2;
		mat[8]=1.0f-xx2+yy2;

		// Transform as 3x3
		// Much faster than transforming a 4x4 matrix (only 9 mults, compared to 16 of 4x4)
		A tmp_x=pos.x*mat[0]+pos.y*mat[3]+pos.z*mat[6];
		A tmp_y=pos.x*mat[1]+pos.y*mat[4]+pos.z*mat[7];
		A tmp_z=pos.x*mat[2]+pos.y*mat[5]+pos.z*mat[8];
		pos.x=tmp_x;
		pos.y=tmp_y;
		pos.z=tmp_z;

		// Calc new normal vector
		planeNormal=pos/rangeToOrigin;
	}
	*/


};

typedef TPlane<float> Plane;

FB_END_PACKAGE1()
