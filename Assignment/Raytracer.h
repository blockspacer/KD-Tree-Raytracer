#pragma once

FB_DECLARE(assignment, Image)
FB_DECLARE(assignment, Model)
FB_DECLARE_STRUCT(assignment, Ray)

#include "Assignment/Light.h"

FB_PACKAGE1(assignment)

class Raytracer
{
public:
	Raytracer() = delete;
	Raytracer(const Model &model);

	void raytrace(Image &imageOut);

	void setCulling(bool backfaceCullingEnabled, bool frontfaceCullingEnabled);
	void addLights(const PodVector<Light> &newLights);
	void clearLights();
	void setShadows(bool enabled);
	void setRelativeCameraPosition(const math::VC3 &relativePosition);
	void setResolution(SizeType width, SizeType height);
	void setViewportCorners(const PodVector<math::VC3> &corners);
	void setCameraPosition(const math::VC3 &position);
	void setCameraDirection(const math::VC3 &direction);
	void setCameraUpVector(const math::VC3 &upVector);

private:
	class TracerInfo 
	{
	public:
		TracerInfo(Raytracer &owner, Image &imageOut, SizeType nextTileX, SizeType nextTileY, SizeType tilesServed)
			: owner(owner)
			, image(imageOut)
			, nextTileX(nextTileX)
			, nextTileY(nextTileY)
			, tilesServed(tilesServed)
		{

		}

		Raytracer &owner;
		Image &image;
		SizeType nextTileX;
		SizeType nextTileY;
		SizeType tilesServed;
	};

	enum ViewportCorners
	{
		ViewportCornersUpperLeft = 0,
		ViewportCornersUpperRight,
		ViewportCornersLowerRight,
		ViewportCornersLowerLeft,
		ViewportCornersCount,
	};

	/* Raytracer starting point. Basically just asks for one tile at a time to raytrace and
	 * calls raytracePixel on every pixel in it. Param is pointer to TracerInfo struct which
	 * includes some parameters and "this". Can (and should) be started by as many times as there
	 * are threads. */
	static int trace(TracerInfo &tracerInfo);
	bool getNextTileCoord(TracerInfo &tracerInfo, SizeType &x, SizeType &y) const;
	math::VC3 raytracePixel(SizeType x, SizeType y);
	math::VC3 calculatePixelColor(Ray &ray) const;

	const Model &model;
	PodVector<Light> lights;
	math::VC3 viewportCorners[ViewportCornersCount];
	math::VC3 cameraPosition;
	math::VC3 cameraDirection;
	math::VC3 cameraUpVector = math::VC3(0.0f, 1.0f, 0.0f);
	math::VC2I resolution;
	SizeType tileSize = 16;
	float zeroDistanceThreshold = 0.0f;
	bool backfaceCulling = false;
	bool frontfaceCulling = false;
	bool shadowsEnabled = false;
	
};

FB_END_PACKAGE1()
