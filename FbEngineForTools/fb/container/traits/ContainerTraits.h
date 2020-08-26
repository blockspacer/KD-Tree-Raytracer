#ifndef FB_CONTAINER_TRAITS_CONTAINERTRAITS_H
#define FB_CONTAINER_TRAITS_CONTAINERTRAITS_H

// Template classes for handling container traits. This file collects all the specific container traits 
// together.
// 
// This is specifically NOT a part of the container itself, to allow all the traits from bloating up the
// the containers (which are used extensively). This means that the use of container traits normally happens by
// creating an instance of the traits class for that specific container. 
//
// And any container that wishes to extends it's behaviour by some specific traits needs to have 
// specialized versions of the traits and utils somewhere.
//
// (Default trait values / utils has been omitted in order to prevent any accidental use of the default container 
// trait/util by not having included the file specifying the specialized version - as those files are not automatically 
// included by the container classes, since most of source is expected not to be interested in them)

#include "fb/lang/platform/FourCC.h"

FB_PACKAGE2(container, traits)

template<class T> class ContainerTraits
{
public:
	static const int classId = 0; // Specialize this please, consider 0 to be invalid.
};

FB_END_PACKAGE2()

#endif
