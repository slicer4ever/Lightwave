#ifndef LWEGEOMETRY3D_H
#define LWEGEOMETRY3D_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWMatrix.h>

class LWEGeometry3D{
public:
	static bool RayRayIntersect(const LWVector3f &aRayStart, const LWVector3f &aRayEnd, const LWVector3f &bRayStart, const LWVector3f &bRayEnd, LWVector3f *IntersectPoint);

	static bool RayAABBIntersect(const LWVector3f &RayStart, const LWVector3f &RayEnd, const LWVector3f &AABBMin, const LWVector3f &AABBMax, float *Min, float *Max);
	
	static bool SphereIntersect(const LWVector3f &aCenterPnt, const LWVector3f &bCenterPnt, float aRadius, float bRadius, LWVector3f *IntersectNrm);

	static bool RaySphereIntersect(const LWVector3f &RayStart, const LWVector3f &RayEnd, const LWVector3f &CenterPnt, float Radius, float *Min, float *Max);

	/*!< \brief tests a plane and ray intersection point.  a plane is represented by a normal(x,y,z) and distance(w) from 0 that plane sits at.  if the plane is offset from 0, then raystart should already have the offset applied. */
	static bool RayPlaneIntersect(const LWVector3f &RayStart, const LWVector3f &RayDir, const LWVector4f &Plane, float *Dis);
	
	static bool AABBIntersect(const LWVector3f &aAABBMin, const LWVector3f &aAABBMax, const LWVector3f &bAABBMin, const LWVector3f &bAABBMax, LWVector3f *IntersectNrm);

	static bool PlanePlaneIntersect(const LWVector4f &aPlane, const LWVector4f &bPlane, LWVector3f *IntersectPnt, LWVector3f *IntersectDir);

	static bool PlanePlanePlaneIntersect(const LWVector4f &aPlane, const LWVector4f &bPlane, const LWVector4f &cPlane, LWVector3f *IntersectPnt);

	static bool PointInsidePlanes(const LWVector3f &Point, const LWVector4f *Planes, uint32_t PlaneCnt);
	
	/*!< \brief the method of collision detection between two convex hulls is likely to be different in lightwave than in other engines.  a convex hull to lightwave is described by a set of intersecting planes, this implementation of collision detection for the convex hull requires that both hulls are perfectly symetrical along all axis's.  to ensure this is true the collision system tests both the positive and negative of each plane, in other words the passed in planes should only describe half of the convex hull and this algorithm will deduce the other half.  it is fine to pass in all sides of the convex hull, but planes which don't have a symetrical opposite will be duplicated and may cause incorrect collision behavior.
	*/
	static bool ConvexHullIntersect(const LWVector3f &aCenterPnt, const LWVector4f *aPlanes, uint32_t aPlaneCnt, const LWVector3f &bCenterPnt, const LWVector4f *bPlanes, uint32_t bPlaneCnt, LWVector3f *IntersectNrm);

	/*!< \brief the method fo ray collision detection against a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
	static bool RayConvexHullIntersect(const LWVector3f &RayStart, const LWVector3f &RayDir, const LWVector4f *Planes, uint32_t PlaneCnt, float *Dis);

	/*!< \brief the method a point is inside a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
	static bool PointInsideConvexHull(const LWVector3f &Point, const LWVector4f *Planes, uint32_t PlaneCnt);
	
	/*!< \brief the method of collision detection between a convex hull and a sphere.    in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
	static bool SphereConvexHullIntersect(const LWVector3f &CircCenterPnt, float Radius, const LWVector3f &ConvexHullCenterPoint, const LWVector4f *Planes, uint32_t PlaneCnt, LWVector3f *IntersectNrm);

	/*!< \brief Constructs a new AABB from the transform matrix applied to the passed in aabb's. (If TransformMatrix is not a standard scale/rotation/translation matrix then this method will produce incorrect results.) */
	static void TransformAABB(const LWVector3f &AAMin, const LWVector3f &AAMax, const LWMatrix4f &TransformMatrix, LWVector3f &AAMinResult, LWVector3f &AAMaxResult);

	/*!< \brief returns true if a sphere is inside the frustum(a frustum is 6 vec4 planes.) */
	static bool SphereInFrustum(const LWVector3f &Position, float Radius, const LWVector3f &FrustumPosition, const LWVector4f *Frustum);

	/*!< \brief returns true if the aabb is inside the frustum, this function turns the aabb into the largest sphere, which will produce some incorrect results. */
	static bool AABBInFrustum(const LWVector3f &AABBMin, const LWVector3f &AABBMax, const LWVector3f &FrustumPosition, const LWVector4f *Frustum);

	/*!< \brief returns true if a cone is inside the frustum, the cone is defined as a point, direction that expands out at theta upto length size. */
	static bool ConeInFrustum(const LWVector3f &Position, const LWVector3f &Direction, float Theta, float Length, const LWVector3f &FrustumPosition, const LWVector4f *Frustum);
};


#endif