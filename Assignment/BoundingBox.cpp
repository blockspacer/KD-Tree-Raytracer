#include "Precompiled.h"

#include "BoundingBox.h"
#include "Assignment/Ray.h"
#include "minmax.h"
#include "fb/lang/Swap.h"


FB_PACKAGE1(assignment)


bool BoundingBox::intersects(Ray &ray) const {

  //test if parallel

  if (
    ray.direction.x == 0 && (ray.origin.x < cornerMin.x || ray.origin.x >
                             cornerMax.x) ||
    ray.direction.y == 0 && (ray.origin.y < cornerMin.y || ray.origin.y >
                             cornerMax.y) ||
    ray.direction.z == 0 && (ray.origin.z < cornerMin.z || ray.origin.z >
                             cornerMax.z)) {
    return false;
  }
  

  // X
  //fb_assert(ray.direction.x != 0);
  float txMin = (cornerMin.x - ray.origin.x) / ray.direction.x;
  float txMax = (cornerMax.x - ray.origin.x) / ray.direction.x;
  if (txMin > txMax)
    lang::swap(txMin, txMax);

  // Y
  //fb_assert(ray.direction.y != 0);
  float tyMin = (cornerMin.y - ray.origin.y) / ray.direction.y;
  float tyMax = (cornerMax.y - ray.origin.y) / ray.direction.y;
  if (tyMin > tyMax)
    lang::swap(tyMin, tyMax);

  // Z
  //fb_assert(ray.direction.z != 0);
  float tzMin = (cornerMin.z - ray.origin.z) / ray.direction.z;
  float tzMax = (cornerMax.z - ray.origin.z) / ray.direction.z;
  if (tzMin > tzMax)
    lang::swap(tzMin, tzMax);

  float tStart = max(txMin, max(tyMin, tzMin));
  float tEnd = min(txMax, min(tyMax, tzMax));

  if (tStart > tEnd || tEnd < 0.0001f) //box is missed or behind
    return false;
  return true;
}

FB_END_PACKAGE1()
