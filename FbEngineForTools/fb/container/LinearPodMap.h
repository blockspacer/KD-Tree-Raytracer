#pragma once

#include "PodVector.h"
#include "LinearMap.h"

FB_PACKAGE1(container)

template<typename Key, typename Value>
struct PodMapPair
{
	typedef Key KeyType;
	typedef Value ValueType;
	Key key;
	Value value;

	PodMapPair(const Key &key_, const Value &value_) : key(key_), value(value_) {}
	PodMapPair(Key &&key_, const Value &value_) : key((Key&&)key_), value(value_) {}
	PodMapPair(const Key &key_, Value &&value_) : key(key_), value((Value&&)value_) {}
	PodMapPair(Key &&key_, Value &&value_) : key((Key&&)key_), value((Value&&)value_) {}

	bool operator == (const PodMapPair &other) const { return key == other.key; }
	bool operator == (const Key &otherKey) const { return key == otherKey; }
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<typename Key, typename T, typename SortFunctor = container::DefaultLinearMapSorter>
using LinearPodMap = LinearMap< Key, T, PodVector<container::PodMapPair<Key, T>>, SortFunctor>;
template<typename Key, typename T, int LocalCapacity, typename SortFunctor = container::DefaultLinearMapSorter>
using CacheLinearPodMap = LinearMap< Key, T, CachePodVector<container::PodMapPair<Key, T>, LocalCapacity>, SortFunctor>;
template<typename Key, typename T, int LocalCapacity, typename SortFunctor = container::DefaultLinearMapSorter>
using StaticLinearPodMap = LinearMap< Key, T, StaticPodVector<container::PodMapPair<Key, T>, LocalCapacity>, SortFunctor>;

FB_END_PACKAGE0()
