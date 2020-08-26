#pragma once

#undef max
#undef min

#include "Vec3.h"
#include "Constants.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/platform/FBMinMax.h"
#include "fb/lang/platform/ForceInline.h"
#include "fb/lang/platform/Likely.h"

FB_DECLARE0(HeapString)

FB_PACKAGE1(math)

//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class Quaternion;
typedef Quaternion<float> QUAT;
typedef Quaternion<double> DQUAT;

// if the squared sum of components is this or below, normalize will make the quaternion jump to failsafe value
static const float quatNormalizeThreshold = 0.00001f;

//------------------------------------------------------------------
// Quaternion
//------------------------------------------------------------------
template <class A> class Quaternion
{
public:
	typedef A ValueType;

	static const Quaternion zero;
	static const Quaternion identity;

	// Data (public for speed)
	union
	{
		A v[4];
		struct
		{
			A x,y,z,w;
		};
	};

	// Creation...

	// I-Quaternion (0,0,0,1)
	FB_FORCEINLINE Quaternion()
		: x(0)
		, y(0)
		, z(0)
		, w(A(1))
	{};

	// Euler angles
	FB_FORCEINLINE Quaternion(A xang, A yang, A zang)
	{
		makeFromAngles(xang,yang,zang);
	}

	// Axis rotation
	FB_FORCEINLINE Quaternion(const Vec3<A> &axis, A angle)
	{
		makeFromAxisRotation(axis,angle);
	}
	
	// Quaternion
	FB_FORCEINLINE Quaternion(A x, A y, A z, A w)
		: x(x)
		, y(y)
		, z(z)
		, w(w)
	{}

	// Quaternion
	FB_FORCEINLINE Quaternion(const A (&f)[4])
		: x(f[0])
		, y(f[1])
		, z(f[2])
		, w(f[3])
	{}

	// Quaternion
	FB_FORCEINLINE Quaternion(const math::Vec3<A> (&axes)[3])
	{
		// check the diagonal
		A tr = axes[0].x + axes[1].y + axes[2].z;
		if (tr > 0.0) 
		{
			A s = A(sqrt(tr + 1.0f));
			w = s / 2.0f;
			s = 0.5f / s;
			
			x = (axes[2].y - axes[1].z) * s;
			y = (axes[0].z - axes[2].x) * s;
			z = (axes[1].x - axes[0].y) * s;
		} 
		else 
		{
			// diagonal is negative
			x = y = z = w = A(0);
			int nxt[3] = { 1, 2, 0 };
			int i = 0;
			
			if (axes[1].y > axes[0].x)
				i = 1;
			if (axes[2].z > axes[i].v[i])
				i = 2;
			
			int j = nxt[i];
			int k = nxt[j];

			A s = A(sqrt((axes[i].v[i] - (axes[j].v[j] + axes[k].v[k])) + A(1.0)));
			v[i] = s * 0.5f;
            
			if (s != 0) 
				s = 0.5f / s;

			v[3] = (axes[k].v[j] - axes[j].v[k]) * s;
			v[j] = (axes[j].v[i] + axes[i].v[j]) * s;
			v[k] = (axes[k].v[i] + axes[i].v[k]) * s;
		}
	}

	A getLength() const
	{
		return A(sqrt(x*x+y*y+z*z+w*w));
	}

	A getSquareLength() const
	{
		return x*x+y*y+z*z+w*w;
	}

	// Make from (conversion)
	void makeFromAngles(A xang=0,A yang=0,A zang=0)
	{
#if (FB_LEGACY_QUATERNION_ANGLES == FB_FALSE)
		zang = -zang; // consistency bug fix
#endif
		Quaternion qx,qy,qz,qf;

		bool xrot,yrot,zrot;
		static const float EPSILON_ = 0.0001f;
		if (fabs(xang)>EPSILON_) xrot=true; else xrot=false;
		if (fabs(yang)>EPSILON_) yrot=true; else yrot=false;
		if (fabs(zang)>EPSILON_) zrot=true; else zrot=false;

		if (xrot)
		{
			A deg = -xang/(A)2;
			qx.x=(A)sin(deg);
			qx.y=0; 
			qx.z=0; 
			qx.w=(A)cos(deg);
		}

		if (yrot)
		{
			A deg=-yang/(A)2;
			qy.x=0; 
			qy.y=(A)sin(deg);
			qy.z=0;
			qy.w=(A)cos(deg);
		}

		if (zrot)
		{
			A deg=zang/(A)2;
			qz.x=0;
			qz.y=0;
			qz.z=(A)sin(deg);
			qz.w=(A)cos(deg);
		}

		if (xrot)
		{
			if (yrot)
			{
				if (zrot)
				{
					qf=qx*qz;
					(*this)=qf*qy;
				}
				else
				{
					(*this)=qx*qy;
				}
			}
			else
			{
				if (zrot)
				{
					(*this)=qx*qz;
				}
				else
				{
					x=qx.x;
					y=qx.y;
					z=qx.z;
					w=qx.w;
				}
			}
		}
		else
		{
			if (yrot)
			{
				if (zrot)
				{
					(*this)=qz*qy;
				}
				else
				{
					x=qy.x;
					y=qy.y;
					z=qy.z;
					w=qy.w;
				}
			}
			else
			{
				if (zrot)
				{
					x=qz.x;
					y=qz.y;
					z=qz.z;
					w=qz.w;
				}
				else
				{
					x=0;
					y=0;
					z=0;
					w=(A)1;
				}
			}
		}
	}

	void makeFromAxisRotation(const Vec3<A> &axis=Vec3<A>(0,1,0),A ang=0)
	{
		fb_assert((axis.getSquareLength() - 1.0f) < 0.01f);
#if (FB_LEGACY_QUATERNION_ANGLES == FB_FALSE)
		const A deg=-ang/(A)2;
#else
		const A deg=ang/(A)2;
#endif
		const A cs=(A)sin(deg);
		w=(A)cos(deg);
		x=cs*axis.x;
		y=cs*axis.y;
		z=cs*axis.z;
	}
	
	void makeFromQuaternion(A _x=0,A _y=0,A _z=0,A _w=1)
	{
		x=_x;
		y=_y;
		z=_z;
		w=_w;
	}

	// Conversion to
	void convertToAxisRotation(Vec3<A> &axis,A &ang) const
	{
		const A tw=(A)acos(FB_FCLAMP(w, -1.0f, 1.0f));
		if (FB_LIKELY(fabsf(tw) > 0.001f))
		{
			A scale=(A)((A)1/(A)sin(tw));
#if (FB_LEGACY_QUATERNION_ANGLES == FB_FALSE)
			scale = -scale;
#endif
			ang=tw*(A)2;
			axis.x=x*scale;
			axis.y=y*scale;
			axis.z=z*scale;
		}
		else
		{
			ang = (A)0;
			axis.x = (A)1;
			axis.y = (A)0;
			axis.z = (A)0;
		}
	}

	// Functions (these do not modify this quaternion)
	Quaternion getNormalizedWithZeroFailsafe(const Quaternion &zeroFailsafeValue) const
	{
		Quaternion temp = *this;
		temp.normalizeWithZeroFailsafe(zeroFailsafeValue);
		return temp;
	}

	Quaternion getUnsafeNormalized() const
	{
		const A inv_length=(A)1/getLength();
		return Quaternion(x*inv_length,y*inv_length,z*inv_length,w*inv_length);
	}

	Quaternion getInverse() const
	{
		return Quaternion(-x,-y,-z,w);
	}

	bool getIsIdentity() const
	{
		// Test if (0,0,0,1)
		if ((fabs(x)<0.001)&&
			(fabs(y)<0.001)&&
			(fabs(z)<0.001)&&
			(fabs(w-(A)1)<0.001)) return true;

		return false;
	}

	Quaternion getSLInterpolationWith(const Quaternion &other,A interpolation) const	// interpolation value range: [0,1]
	{
		// Temp variables
		A ox=other.x;
		A oy=other.y;
		A oz=other.z;
		A ow=other.w;

		// Compute dot product (equal to cosine of the angle between quaternions)
		A fCosTheta=x*ox+y*oy+z*oz+w*ow;

		// Check angle to see if quaternions are in opposite hemispheres
		if (fCosTheta<0) 
		{
			// If so, flip one of the quaterions
			fCosTheta=-fCosTheta;
			ox=-ox;
			oy=-oy;
			oz=-oz;
			ow=-ow;
		}

		// Set factors to do linear interpolation, as a special case where the
		// quaternions are close together.
		A fBeta=(A)1-interpolation;
    
		// If the quaternions aren't close, proceed with spherical interpolation
		if ((A)1-fCosTheta>(A)0.001) 
		{   
	        A fTheta=(A)acos(fCosTheta);
		    fBeta=(A)sin(fTheta*fBeta)/(A)sin(fTheta);
			interpolation=(A)sin(fTheta*interpolation)/(A)sin(fTheta);
		}

		// Do the interpolation
		return Quaternion(fBeta*x+interpolation*ox,fBeta*y+interpolation*oy,
			fBeta*z+interpolation*oz,fBeta*w+interpolation*ow);
	}

	// Functions (these modify this quaternion)
	void unsafeNormalize()
	{
		A sqLen = getLength();
		fb_expensive_assert((A)fabs(sqLen) > (A)quatNormalizeThreshold && "Refactored regression assert triggered.");
		const A inv_length=(A)1/sqLen;
		x*=inv_length;
		y*=inv_length;
		z*=inv_length;
		w*=inv_length;
	}

	// Functions (these modify this quaternion)
	void normalizeWithZeroFailsafe(const Quaternion<A> &zeroFailsafeValue)
	{
		A sqLen = getSquareLength();
		if ((A)fabs(sqLen) > (A)quatNormalizeThreshold)
		{
			A sqrtLen = sqrt(sqLen);
			const A inv_length=(A)1/sqrtLen;
			x*=inv_length;
			y*=inv_length;
			z*=inv_length;
			w*=inv_length;
		} else {
			x = zeroFailsafeValue.x;
			y = zeroFailsafeValue.y;
			z = zeroFailsafeValue.z;
			w = zeroFailsafeValue.w;
		}
	}

	void inverse()
	{
		x=-x;
		y=-y;
		z=-z;
	}

	// Operators
	Quaternion operator*(const Quaternion& other) const
	{
		A Dx =  x*other.w + y*other.z - z*other.y + w*other.x;
		A Dy = -x*other.z + y*other.w + z*other.x + w*other.y;
		A Dz =  x*other.y - y*other.x + z*other.w + w*other.z;
		A Dw = -x*other.x - y*other.y - z*other.z + w*other.w;
		return Quaternion(Dx,Dy,Dz,Dw);
	}

	Quaternion operator*(A num) const
	{
		return Quaternion(x*num, y*num, z*num, w*num);
	}

	Quaternion operator+(const Quaternion& other) const
	{
		return Quaternion(x+other.x, y+other.y, z+other.z, w+other.w);
	}
	
	void operator+=(const Quaternion& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		this->w += other.w;
	}

	void operator-=(const Quaternion& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
		this->w -= other.w;
	}

	Quaternion operator-(const Quaternion& other) const
	{
		return Quaternion(x-other.x, y-other.y, z-other.z, w-other.w);
	}

	bool operator==(const Quaternion& other) const
	{
		static const float EPSILON_ = 0.0001f;

		if ((w < 0.0f) != (other.w < 0.0f))
		{
			if ((fabs(x+other.x)<EPSILON_)&&
				(fabs(y+other.y)<EPSILON_)&&
				(fabs(z+other.z)<EPSILON_)&&
				(fabs(w+other.w)<EPSILON_)) return true;
		}
		else
		{
			if ((fabs(x-other.x)<EPSILON_)&&
				(fabs(y-other.y)<EPSILON_)&&
				(fabs(z-other.z)<EPSILON_)&&
				(fabs(w-other.w)<EPSILON_)) return true;
		}
		return false;
	}

	const A &operator[](SizeType index) const
	{
		fb_expensive_assert(index < 4 && "Invalid index for quaternion operator[]");
		return v[index];
	}

	A &operator[](SizeType index)
	{
		fb_expensive_assert(index < 4 && "Invalid index for quaternion operator[]");
		return v[index];
	}

	bool isDenormal() const;
	bool isNaN() const;
	bool isInf() const;
	bool isFinite() const;

	// Vector modify...
	void rotateVector(Vec3<A> &vector) const
	{
		// Optimized even further... no need for complex matrix stuff.
		A tx = (vector.y * z - vector.z * y) * A(2);
		A ty = (vector.z * x - vector.x * z) * A(2);
		A tz = (vector.x * y - vector.y * x) * A(2);

		vector.x += w * tx + (ty * z - tz * y);
		vector.y += w * ty + (tz * x - tx * z);
		vector.z += w * tz + (tx * y - ty * x);
	}

	Vec3<A> getRotated(const Vec3<A> &vector) const
	{
		Vec3<A> temp(vector);
		rotateVector(temp);
		return temp;
	}

	Vec3<A> getEulerAngles() const
	{
		Quaternion rotation = getInverse();
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

		A heading = (A)0;
		A attitude = (A)0;
		A bank = (A)0;

		A m00 = (A)1-yy2-zz2;
		A m02 = xz2+wy2;
		A m10 = xy2+wz2;
		A m11 = (A)1-xx2-zz2;
		A m12 = yz2-wx2;
		A m20 = xz2-wy2;
		A m22 = (A)1-xx2-yy2;

		if (m10 > (A)0.999998)
		{
			heading = (A)atan2((A)m02, (A)m22);
			attitude = (A)-Pi/2;
			bank = (A)0;
		}
		else if (m10 < (A)-0.999998)
		{
			heading = (A)atan2((A)m02, (A)m22);
			attitude = (A)Pi/2;
			bank = (A)0;
		}
		else
		{
			heading = (A)atan2((A)-m20, (A)m00);
			attitude = -(A)asin((A)m10);
			bank = (A)atan2((A)-m12, (A)m11);
		}

#if (FB_LEGACY_QUATERNION_ANGLES == FB_FALSE)
		attitude = -attitude; // consistency bug fix
#endif
		return Vec3<A>(bank, heading, attitude);
	}

	void rotateTowards(const Vec3<A> &direction, const Vec3<A> &unrotatedDirection)
	{
		Vec3<A> axis = direction.getCrossWith(unrotatedDirection);
		A dot = direction.getDotWith(unrotatedDirection);

		if (dot < -A(0.999998)) // direction roughly opposite to unrotatedDirection (used to return identity here; now pick a solution that turns unrotatedDirection to its opposite direction)
		{
			if (lang::abs(unrotatedDirection.x) > A(0.7071067))
				axis = unrotatedDirection.getCrossWith(Vec3<A>(0, 1, 0));
			else
				axis = unrotatedDirection.getCrossWith(Vec3<A>(1, 0, 0));
			w = 0;
			x = axis.x; // (FB_LEGACY_QUATERNION_ANGLES not considered here as we've pulled the axis out of a hat anyway)
			y = axis.y;
			z = axis.z;
			normalizeWithZeroFailsafe(Quaternion(identity));
			return;
		}

		x = axis.x;
		y = axis.y;
		z = axis.z;
		w = A(dot + 1.0);
		normalizeWithZeroFailsafe(Quaternion(identity));
	}

	bool operator != (const Quaternion &other) const
	{
		return !((*this) == other);
	}

	/**
	 * Returns 3x3 rotation matrix
	 */
	FB_FORCEINLINE void getAxes(Vec3<A> (&axes)[3]) const
	{
		A x2=x+x;
		A y2=y+y;
		A z2=z+z;

		A xx2=x*x2;
		A yy2=y*y2;
		A zz2=z*z2;
		A xy2=x*y2;
		A xz2=x*z2;
		A yz2=y*z2;
		A wx2=w*x2;
		A wy2=w*y2;
		A wz2=w*z2;
    
		axes[0].x = (A)1-yy2-zz2;
		axes[0].y = xy2-wz2;
		axes[0].z = xz2+wy2;

		axes[1].x = xy2+wz2;
		axes[1].y = (A)1-xx2-zz2;
		axes[1].z = yz2-wx2;

		axes[2].x = xz2-wy2;
		axes[2].y = yz2+wx2;
		axes[2].z = (A)1-xx2-yy2;
	}

};

template<typename A>
A quatDot(const Quaternion<A> &first, const Quaternion<A> &second)
{
	// quaternion dot product, not named simply "dot" since this is rarely useful
	return first.x*second.x + first.y*second.y + first.z*second.z + first.w*second.w;
}

template<typename A>
bool isEqualWithinEpsilon(const Quaternion<A> &first, const Quaternion<A> &second, A epsilon = 0.0000005f)
{
	A dot = quatDot(first, second);
	A angleApprox = 2.0f * (1.0f - FB_FABS(dot));
	return FB_FABS(angleApprox) < epsilon;
}

template<typename A>
static Quaternion<A> slerp(const Quaternion<A> &a, const Quaternion<A> &b, A f)
{
	return a.getSLInterpolationWith(b, f);
}

template<typename A> const Quaternion<A> Quaternion<A>::zero(0, 0, 0, 0);
template<typename A> const Quaternion<A> Quaternion<A>::identity(0, 0, 0, A(1));

static math::Quaternion<double> toDouble(const math::QUAT &v)
{
	return math::Quaternion<double>(v.x, v.y, v.z, v.w);
}

static math::QUAT fromDouble(const math::Quaternion<double> &v)
{
	return math::QUAT((float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

HeapString &debugAppendToString(HeapString &result, const QUAT &val);
HeapString &debugAppendToString(HeapString &result, const DQUAT &val);

FB_END_PACKAGE1()

#include "fb/math/traits/QuaternionTraits.h"
