#ifndef FB_MATH_CURVE_VALUEWITHNUMCOMPONENTS_H
#define FB_MATH_CURVE_VALUEWITHNUMCOMPONENTS_H

FB_PACKAGE2(math, curve)

/**
 * Just a generic container for multi-float-component vector like (VC3, VC4, COL, etc.) types.
 * (This exists just to make it easier to wrap all of these types for the single AnyCurve class)
 *
 * Because the current implementation avoids doing loads of heap allocations, there is a max amount for the supported component.
 *
 * Also notice, that this value type may appear a bit "cryptic" as far as the MathTypeConverter goes, etc. - because
 * it has a variable number of components actually used.
 */
template<int NumMaxSupportedComponents> class ValueWithNumComponents
{
public:
	typedef float ValueType;

	ValueWithNumComponents(int numComponents)
	{ 
		fb_expensive_assert(numComponents < NumMaxSupportedComponents);
		this->numComponents = numComponents;
		initToZero();
	}

	ValueWithNumComponents(int numComponents, float *values)
	{ 
		fb_expensive_assert(numComponents < NumMaxSupportedComponents);
		this->numComponents = numComponents;
		initFromArray(values);
	}

	void initToZero()
	{
		for (int i = 0; i < numComponents; ++i) 
		{
			componentValues[i] = 0;
		}
	}

	void initFromArray(float *values)
	{
		for (int i = 0; i < numComponents; ++i) 
		{
			componentValues[i] = values[i];
		}
	}

	int getNumComponents()
	{
		return numComponents;
	}

	float getComponentValue(int num)
	{
		fb_expensive_assert(num < numComponents);
		return values[num];
	}

	void setComponentValue(int num, float value)
	{
		fb_expensive_assert(num < numComponents);
		values[num] = value;
	}

	const float *getComponentValues()
	{
		return values;
	}

protected:
	float componentValues[NumMaxSupportedComponents];
	int numComponents;
};

// specialized for the most common cases
template<> void ValueWithNumComponents<1>::initToZero() { componentValues[0] = 0; }
template<> void ValueWithNumComponents<1>::initFromArray(float *values) { componentValues[0] = values[0]; }
template<> void ValueWithNumComponents<3>::initToZero() { componentValues[0] = 0; componentValues[1] = 0; componentValues[2] = 0; }
template<> void ValueWithNumComponents<3>::initFromArray(float *values) { componentValues[0] = values[0]; componentValues[1] = values[1]; componentValues[2] = values[2]; }
template<> void ValueWithNumComponents<4>::initToZero() { componentValues[0] = 0; componentValues[1] = 0; componentValues[2] = 0; componentValues[3] = 0; }
template<> void ValueWithNumComponents<4>::initFromArray(float *values) { componentValues[0] = values[0]; componentValues[1] = values[1]; componentValues[2] = values[2]; componentValues[3] = values[3];}

typedef ValueWithNumComponents<4> FloatMax4Components;

FB_END_PACKAGE2()

#endif
