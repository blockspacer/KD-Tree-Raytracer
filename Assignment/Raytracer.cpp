#include "Precompiled.h"
#include "Raytracer.h"

#include "Assignment/Image.h"
#include "Assignment/Model.h"
#include "Assignment/Ray.h"

FB_PACKAGE1(assignment)

Raytracer::Raytracer(const Model &model)
	: model(model)
{
	/* Right value for "practically zero" depends on absolute dimensions of model. I'm not an
	 * expert on floating point, but if I understand the idea of epsilon correctly, scaling it by
	 * the model dimension should give us sensible result. */
	zeroDistanceThreshold = (model.getMaxBounds() - model.getMinBounds()).getLength() * std::numeric_limits<float>::epsilon();
}

void Raytracer::raytrace(Image &imageOut)
{
	SizeType totalTiles = resolution.x / tileSize + (resolution.x % tileSize != 0 ? 1 : 0);
	totalTiles *= resolution.y / tileSize + (resolution.y % tileSize != 0 ? 1 : 0);
	FB_LOG_INFO(FB_MSG("\nRaytracing ", resolution.x - 1, " x ", resolution.y - 1, " pixels.\n\tTile size ", tileSize, "x", tileSize, "\n\tTotal tiles ", totalTiles));

	TracerInfo info(*this, imageOut, 0, 0, 0);
	trace(info);
}

void Raytracer::setCulling(bool backfaceCullingEnabled, bool frontfaceCullingEnabled)
{
	backfaceCulling = backfaceCullingEnabled;
	frontfaceCulling = frontfaceCullingEnabled;
}

void Raytracer::addLights(const PodVector<Light> &newLights)
{
	lights.reserve(lights.getSize() + newLights.getSize());
	for (SizeType i = 0; i < newLights.getSize(); ++i)
		lights.pushBack(newLights[i]);
}

void Raytracer::clearLights()
{
	lights.clear();
}

void Raytracer::setShadows(bool enabled)
{
	shadowsEnabled = enabled;
}

void Raytracer::setRelativeCameraPosition(const math::VC3 &relativePosition)
{	
	cameraPosition = model.getBoundingBoxCenter();
	cameraDirection = relativePosition;
	cameraDirection.normalize();
	cameraDirection *= -1.0f;
	
	/* We make sure location is outside the model's bounding box */
	float farEnough = (model.getMaxBounds() - model.getMinBounds()).getLength() * 4.0f;
	cameraPosition -= cameraDirection * farEnough;
	math::VC3 steepestCorner = model.getMaxBounds();
	float steepestAngleCos = 1.0f;
	for (SizeType x = 0; x < 2; ++x) 
	{
		for (SizeType y = 0; y < 2; ++y) 
		{
			for (SizeType z = 0; z < 2; ++z) 
			{
				math::VC3 corner;
				corner.x = (x == 0 ? this->model.getMinBounds().x : this->model.getMaxBounds().x);
				corner.y = (y == 0 ? this->model.getMinBounds().y : this->model.getMaxBounds().y);
				corner.z = (z == 0 ? this->model.getMinBounds().z : this->model.getMaxBounds().z);
				math::VC3 cornerDirection = (corner - cameraPosition).getNormalized();
				float angleCos = cornerDirection.getDotWith(cameraDirection);
				
				/* Since we moved camera position outside the model, scalar product should be > 0 and < 1 */
				fb_assert(angleCos >= 0.0 && angleCos < 1.0 && "Automatic camera has invalid angleCos");
				if (angleCos < steepestAngleCos) 
				{
					steepestAngleCos = angleCos;
					steepestCorner = corner;
				}
			}
		}
	}
	
	math::VC3 scDirection = steepestCorner - cameraPosition;
	math::VC3 vpDirectionRight = cameraDirection.getCrossWith(cameraUpVector).getNormalized();
	math::VC3 vpDirectionUp = cameraDirection.getCrossWith(vpDirectionRight).getNormalized();

	/* If this scalar product is zero, we should actually do something about it. But we don't, since there are quite many other ways of user shooting himself in foot. */
	fb_assert(vpDirectionUp.getDotWith(cameraUpVector) != 0.0f && "Automatic camera has issues with up direction and camera up vector");
	float scVpUpComponent = std::fabs(scDirection.getDotWith(vpDirectionUp.getNormalized()));
	float scVpRightComponent = std::fabs(scDirection.getDotWith(vpDirectionRight.getNormalized()));
	float scCameraDirectionComponent = std::fabs(scDirection.getDotWith(cameraDirection.getNormalized()));
	float aspectRatio = 1.0f * resolution.x / resolution.y;
	float naturalAspectRatio = scVpRightComponent / scVpUpComponent;
	if (naturalAspectRatio > aspectRatio) 
	{
		/* Model is wider than screen */
		scVpUpComponent *= naturalAspectRatio / aspectRatio;
	}
	else 
	{
		/* Model is higher than screen */
		scVpRightComponent *= aspectRatio / naturalAspectRatio;
	}
	
	/* Zoom factor of 0.5 would fit model's bounding box just */
	float zoom = 0.51f;
	viewportCorners[ViewportCornersUpperLeft] = cameraPosition + (vpDirectionUp * scVpUpComponent + vpDirectionRight * -scVpRightComponent +	cameraDirection * scCameraDirectionComponent) * zoom;
	viewportCorners[ViewportCornersUpperRight] = cameraPosition + (vpDirectionUp * scVpUpComponent +	vpDirectionRight * scVpRightComponent +	cameraDirection * scCameraDirectionComponent) * zoom;
	viewportCorners[ViewportCornersLowerRight] = cameraPosition + (vpDirectionUp * -scVpUpComponent + vpDirectionRight * scVpRightComponent + cameraDirection * scCameraDirectionComponent) * zoom;
	viewportCorners[ViewportCornersLowerLeft] = cameraPosition + (vpDirectionUp * -scVpUpComponent +	vpDirectionRight * -scVpRightComponent + cameraDirection * scCameraDirectionComponent) * zoom;
}

void Raytracer::setResolution(SizeType width, SizeType height)
{
	/* These are off by one for some simple aa */
	resolution = math::VC2I((int32_t)width + 1, (int32_t)height + 1);
}

void Raytracer::setViewportCorners(const PodVector<math::VC3> &corners)
{
	if (corners.getSize() != ViewportCornersCount)
	{
		FB_LOG_ERROR("Couldn't set viewport corners. Invalid number of values");
		return;
	}

	for (SizeType i = 0; i < corners.getSize(); ++i)
		viewportCorners[i] = corners[i];
}

void Raytracer::setCameraPosition(const math::VC3 &position)
{
	cameraPosition = position.getNormalized();
}

void Raytracer::setCameraDirection(const math::VC3 &direction)
{
	cameraDirection = direction.getNormalized();
}

void Raytracer::setCameraUpVector(const math::VC3 &upVector)
{
	cameraUpVector = upVector.getNormalized();
}

SizeType count = 0;
int Raytracer::trace(TracerInfo &tracerInfo)
{
	FB_PRINTF("\t000%%");

	const SizeType width = (SizeType)tracerInfo.owner.resolution.x;
	const SizeType height = (SizeType)tracerInfo.owner.resolution.y;
	const SizeType tileSize = tracerInfo.owner.tileSize;
	SizeType tileX = 0; 
	SizeType tileY = 0;
	
	while (tracerInfo.owner.getNextTileCoord(tracerInfo, tileX, tileY))
	{
		int hundreth = int((width*height / 100));
		for (SizeType y = tileY; y < tileY + tileSize && y < height; ++y) 
		{
			for (SizeType x = tileX; x < tileX + tileSize && x < width; ++x) 
			{
				if (count++ % hundreth == 0)
				{
					SizeType percentage = (count / hundreth);
					FB_PRINTF("\b\b\b\b");
					if (percentage < 10)
						FB_PRINTF("00%d%%", percentage);
					else
						FB_PRINTF("0%d%%", percentage);
				}

				tracerInfo.image.setPixelColor(x, y, tracerInfo.owner.raytracePixel(x, y));
			}
		}
	}
	
	FB_PRINTF("\b\b\b\b100%%\n");
	return 0;
}

bool Raytracer::getNextTileCoord(TracerInfo &tracerInfo, SizeType &x, SizeType &y) const
{
	const SizeType width = (SizeType)resolution.x;
	const SizeType height = (SizeType)resolution.y;

	tracerInfo.nextTileX += tileSize;
	if (tracerInfo.nextTileX >= width)
	{
		tracerInfo.nextTileY += tileSize;
		if (tracerInfo.nextTileY >= height)
		{
			tracerInfo.nextTileX = width;
			tracerInfo.nextTileY = height;
			return false;
		}
		else 
		{
			tracerInfo.nextTileX = 0;
		}
	}
	x = tracerInfo.nextTileX;
	y = tracerInfo.nextTileY;
	++tracerInfo.tilesServed;
	return true;
}

math::VC3 Raytracer::raytracePixel(SizeType x, SizeType y) 
{
	const SizeType width = (SizeType)resolution.x;
	const SizeType height = (SizeType)resolution.y;
	math::VC3 vpDirectionRight = viewportCorners[ViewportCornersUpperRight] - viewportCorners[ViewportCornersUpperLeft];
	math::VC3 vpDirectionDown = viewportCorners[ViewportCornersLowerLeft] - viewportCorners[ViewportCornersUpperLeft];
	math::VC3 vpIntersectionPoint = viewportCorners[ViewportCornersUpperLeft] + vpDirectionRight * float(x) / float(width) + vpDirectionDown * float(y) / float(height);
	Ray ray;
	ray.origin = cameraPosition;
	ray.direction = vpIntersectionPoint - cameraPosition;
	ray.intersectionType = IntersectionTypeNearest;
	ray.backfaceCulling = backfaceCulling;
	ray.frontfaceCulling = frontfaceCulling;
	model.getIntersection(ray);	
	return calculatePixelColor(ray);
}

math::VC3 Raytracer::calculatePixelColor(Ray &ray) const 
{
	/* No hit, no color */
	if (!ray.bestTriangle) 		
		return math::VC3::zero;
	
	math::VC3 accumulatedLighting;
	for (SizeType i = 0; i < lights.getSize(); ++i) 
	{
		const Light& light = this->lights[i];
		if (light.getType() == LightTypeAmbient) 
		{
			accumulatedLighting += light.getColor();
		}
		else 
		{
			math::VC3 lightDir;
			if (light.getType() == LightTypePoint) 
				lightDir = (light.getPosition() - ray.intersectionPoint).getNormalized();
			else if (light.getType() == LightTypeDiffuse) 
				lightDir = (light.getPosition() * -1.0f).getNormalized();
			else 
				fb_assert(0 && "Unknown light type");
			
			/* To both front and backface cull makes no sense, so if user is doing that, result is undefined. */
			fb_assert(!(ray.backfaceCulling && ray.frontfaceCulling) && "Both backface and frontface culling are enabled, this can't work");

			float intensity = lightDir.getDotWith(ray.bestTriangle->getNormal());
			bool notCulling = !(ray.backfaceCulling || ray.frontfaceCulling);
			if (ray.frontfaceCulling) 
			{
				intensity = -intensity;
			}
			if (notCulling || intensity > 0) 
			{
				Ray shadowRay;
				shadowRay.origin = ray.intersectionPoint;
				shadowRay.direction = lightDir;
				shadowRay.intersectionType = IntersectionTypeAny;
				shadowRay.backfaceCulling = false;
				shadowRay.frontfaceCulling = false;
				shadowRay.zeroDistanceThreshold = zeroDistanceThreshold;
				shadowRay.bestTriangle = ray.bestTriangle;

				if (!shadowsEnabled || !model.getIntersection(shadowRay))
					accumulatedLighting += light.getColor() * FB_FABS(intensity);
			}
		}
	}
	return accumulatedLighting;
}

FB_END_PACKAGE1()
