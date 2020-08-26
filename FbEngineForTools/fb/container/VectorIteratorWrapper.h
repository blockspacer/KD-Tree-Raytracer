#pragma once

FB_PACKAGE1(container)

// this is a dummy class, used by the VectorIteratorWrapper, indicating it being used as the "no-interface" version.
class INoVectorIteratorWrapperInterface
{
public:
	// nothing here.
};

/**
 * Vector iterator wrapper helper classes
 *
 * Usage example: 
 * <pre>
 *   using namespace container;
 * 
 *   static const int MY_INT_NONE = 0;
 *
 *   Vector<int> myIntVector;
 *   Vector<MyClass *> myPointerVectorWithInterface;
 *
 *   class IMyInterface; 
 *
 *   typedef util::VectorIteratorWrapper< Vector<int>, int > MyIntIterator;
 *   typedef util::VectorIteratorWrapperImplementingInterface< Vector<MyClass *>, MyClass *, IMyInterface > MyPointerIterator;
 *
 *   MyIntIterator getMyIntIterator() { return MyIntIterator(myIntVector, MY_INT_NONE); }
 *   MyPointerIterator getMyPointerIterator() { return MyPointerIterator(myPointerVector, nullptr); }
 * </pre>
 */

template<class VectorT, class ElementT, class InterfaceT > 
class VectorIteratorWrapperImplementingInterface : public InterfaceT
{
public:
	VectorIteratorWrapperImplementingInterface(VectorT &vec, ElementT end) : vector(vec), endValue(end), index(0) {}
	ElementT next()
	{
		if (index < vector.getSize())
			return vector[index++];
		return endValue;
	}

public:
	VectorT &vector;
	ElementT endValue;
	typename VectorT::SizeType index;
};


template<class VectorT, class ElementT, class InterfaceT > 
class ConstVectorIteratorWrapperImplementingInterface : public InterfaceT
{
public:
	ConstVectorIteratorWrapperImplementingInterface(const VectorT &vec, ElementT end) : vector(vec), endValue(end), index(0) {}
	ElementT next()
	{
		if (index < vector.getSize())
			return vector[SizeType(index++)];
		return endValue;
	}

public:
	const VectorT &vector;
	ElementT endValue;
	typename VectorT::SizeType index;
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<class VectorT, class ElementT > 
class VectorIteratorWrapper : public container::VectorIteratorWrapperImplementingInterface<VectorT, ElementT, container::INoVectorIteratorWrapperInterface >
{
public:
	VectorIteratorWrapper(VectorT &vec, ElementT end)
		: container::VectorIteratorWrapperImplementingInterface<VectorT, ElementT, container::INoVectorIteratorWrapperInterface >(vec, end)
	{
	}
};

template<class VectorT, class ElementT > 
class ConstVectorIteratorWrapper : public container::ConstVectorIteratorWrapperImplementingInterface<VectorT, ElementT, container::INoVectorIteratorWrapperInterface >
{
public:
	ConstVectorIteratorWrapper(const VectorT &vec, ElementT end)
		: container::ConstVectorIteratorWrapperImplementingInterface<VectorT, ElementT, container::INoVectorIteratorWrapperInterface >(vec, end)
	{
	}
};

FB_END_PACKAGE0()
