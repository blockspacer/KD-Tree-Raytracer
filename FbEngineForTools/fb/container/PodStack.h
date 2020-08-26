#pragma once

#include "Stack.h"
#include "PodVector.h"

FB_PACKAGE0()

template<typename T>
using PodStack = Stack<T, PodVector<T>>;
template<typename T, int LocalCapacity>
using CachePodStack = Stack<T, CachePodVector<T, LocalCapacity>>;
template<typename T, int LocalCapacity>
using StaticPodStack = Stack<T, StaticPodVector<T, LocalCapacity>>;

FB_END_PACKAGE0()
