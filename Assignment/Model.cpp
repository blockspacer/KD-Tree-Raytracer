#include "Precompiled.h"
#include "Model.h"
#include "KDNode.h"

#include "Assignment/Ray.h"
#include "external/plyreader/ply_reader.h"

FB_PACKAGE1(assignment)

void Model::loadModel(const HeapString &fileName)
{
	FB_LOG_INFO(FB_MSG("Reading model from ", fileName));
	std::auto_ptr<PLY_Model> plyModel = Read_PLY_Model(fileName.getPointer());
	
	FB_LOG_INFO("Converting model data...");
	std::vector<float> vertexData = plyModel->m_vertex_data;
	vertices.reserve((SizeType)vertexData.size() / 3);
	for (SizeType i = 0; i < vertexData.size(); i += 3) 
		vertices.pushBack(math::VC3(vertexData[i], vertexData[i + 1], vertexData[i + 2]));

	std::vector<int> indices = plyModel->m_face_data;
	SizeType polysRejected = 0;
	for (SizeType i = 0; i < indices.size(); i += 3)
	{
		polygons.pushBack(Triangle(&(vertices[(SizeType)indices[i]]), &(vertices[(SizeType)indices[i + 1]]), &(vertices[(SizeType)indices[i + 2]])));
		if (polygons.getBack().isInvalid())
		{
			++polysRejected;
			this->polygons.popBack();
		}
	}

	FB_LOG_INFO(FB_MSG(vertices.getSize(), " vertices, ", polygons.getSize(), " polys created, ", polysRejected, " polys rejected"));
	maxBounds.x = plyModel->Get_Bound_Max(0);
	maxBounds.y = plyModel->Get_Bound_Max(1);
	maxBounds.z = plyModel->Get_Bound_Max(2);
	minBounds.x = plyModel->Get_Bound_Min(0);
	minBounds.y = plyModel->Get_Bound_Min(1);
	minBounds.z = plyModel->Get_Bound_Min(2);
	
	HeapString maxBoundsStr;
	HeapString minBoundsStr;
	debugAppendToString(maxBoundsStr, maxBounds);
	debugAppendToString(minBoundsStr, minBounds);
	FB_LOG_INFO(FB_MSG("\nModel ", fileName, "\n\t", polygons.getSize(), " polys, ", vertices.getSize(), " vertices\n\tMin bounds: ", minBoundsStr, "\n\tMax bounds: ", maxBoundsStr, "\n"));
}

void Model::initializeKDTree()
{
	/* TODO FOR ASSIGNMENT: Create nodes for kd tree */
	FB_LOG_ERROR("KDTree not implemented");
}

bool Model::getIntersection(Ray &ray) const 
{
	if (isKDTreeInitialized()) 
	{
		/* TODO FOR ASSIGNMENT: Get intersection using KD Tree */
	}

	bool somethingFound = false;
	for (SizeType i = 0; i < polygons.getSize(); ++i) 
	{
		const Triangle &tri = polygons[i];
		if (ray.startingPoint != &tri && tri.intersects(ray) == 1) 
		{
			/* We have a hit */
			somethingFound = true;
			if (ray.intersectionType == IntersectionTypeAny) 
				break;
		}
	}

	/* Maybe we found something, maybe not */
	return somethingFound;
}

bool Model::isKDTreeInitialized() const
{
	/* TODO FOR ASSIGNMENT: Return correct value */
	return false;
}

math::VC3 Model::getBoundingBoxCenter() const
{
	return (minBounds + maxBounds) / 2.0f;
}

const math::VC3 &Model::getMaxBounds() const
{
	return maxBounds;
}

const math::VC3 &Model::getMinBounds() const
{
	return minBounds;
}

FB_END_PACKAGE1()
