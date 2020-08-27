#pragma once

#include "BoundingBox.h"

FB_PACKAGE1(assignment)

class KDNode
{
public:
	BoundingBox bbox;
	KDNode* leftNode;
	KDNode* rightNode;
	PodVector<Triangle*> triangles;

	KDNode();

	KDNode* Generate(PodVector<Triangle*>& triangles, int32_t depth) const;
	
};

FB_END_PACKAGE1()