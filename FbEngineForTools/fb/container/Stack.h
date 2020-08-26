#pragma once

#include "Vector.h"

FB_PACKAGE0()

template<typename T, typename ImpVecType = fb::Vector<T>>
struct Stack: private ImpVecType
{
	Stack() {}
	Stack(const Stack &other) { copy(other); }
	Stack(Stack &&other) { move(other); }
	template<typename OtherImpVecType>
	Stack(const Stack<T, OtherImpVecType> &other) { copy(other); }
	template<typename OtherImpVecType>
	Stack(Stack<T, OtherImpVecType> &&other) { move(other); }

	void operator = (const Stack &other) { copy(other); }
	void operator = (Stack &&other) { move(other); }
	template<typename OtherImpVecType>
	void operator = (const Stack<T, OtherImpVecType> &other) { copy(other); }
	template<typename OtherImpVecType>
	void operator = (Stack<T, OtherImpVecType> &&other) { move(other); }

	void swap(Stack &other) { ImpVecType::swap(other); }
	void move(Stack &other) { ImpVecType::swap(other); }
	void copy(const Stack &other) { ImpVecType::operator= (other); }
	template<typename OtherImpVecType>
	void swap(Stack<T, OtherImpVecType> &other) { ImpVecType::swap(other); }
	template<typename OtherImpVecType>
	void move(Stack<T, OtherImpVecType> &other) { ImpVecType::swap(other); }
	template<typename OtherImpVecType>
	void copy(const Stack<T, OtherImpVecType> &other) { ImpVecType::operator= (other); }

	using typename ImpVecType::ValueType;
	using ImpVecType::pushBack;
	using ImpVecType::popBack;
	using ImpVecType::getBack;
	using ImpVecType::getSize;
	using ImpVecType::getCapacity;
	using ImpVecType::isEmpty;
	void push(const T &value) { pushBack(value); }
	void push(T &&value) { pushBack(value); }
	T popBackWithValue() { T tmp = getBack(); popBack(); return tmp; }
	const T &operator[] (SizeType index) const { return ImpVecType::operator[] (index); }

	// For raw access
	const ImpVecType &getVectorImp() const { return *this; }
	ImpVecType &getVectorImp() { return *this; }
};

template<typename T, int LocalCapacity>
using CacheStack = Stack<T, CacheVector<T, LocalCapacity>>;
template<typename T, int LocalCapacity>
using StaticStack = Stack<T, StaticVector<T, LocalCapacity>>;

FB_END_PACKAGE0()
