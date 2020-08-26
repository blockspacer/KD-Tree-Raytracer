#pragma once

#include "PodVector.h"
#include "LinearSet.h"

FB_PACKAGE0()

template<typename T, typename SortFunctor = container::DefaulLinearSetSorter>
using LinearPodSet = LinearSet<T, PodVector<T>, SortFunctor>;
template<typename T, int LocalCapacity, typename SortFunctor = container::DefaulLinearSetSorter>
using CacheLinearPodSet = LinearSet<T, CachePodVector<T, LocalCapacity>, SortFunctor>;
template<typename T, int LocalCapacity, typename SortFunctor = container::DefaulLinearSetSorter>
using StaticLinearPodSet = LinearSet<T, StaticPodVector<T, LocalCapacity>, SortFunctor>;

FB_END_PACKAGE0()
