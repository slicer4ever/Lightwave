#ifndef LWEGEOMETRY2D_H
#define LWEGEOMETRY2D_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>

class LWEGeometry2D{
public:
	static bool RayRayIntersect(const LWVector2f &aRayStart, const LWVector2f &aRayEnd, const LWVector2f &bRayStart, const LWVector2f &bRayEnd, LWVector2f *IntersectPoint);

	static bool RayAABBIntersect(const LWVector2f &RayStart, const LWVector2f &RayEnd, const LWVector2f &AABBMin, const LWVector2f &AABBMax, float *Min, float *Max);

	static bool RayCircleIntersect(const LWVector2f &RayStart, const LWVector2f &RayEnd, const LWVector2f &CenterPnt, float Radius, float *Min, float *Max);
	
	/*!< \brief tests a plane and ray intersection point.  a plane is represented by a normal(x,y) and distance(z) from 0 that plane sits at.  if the plane is offset from 0, then raystart should already have the offset applied. */
	static bool RayPlaneIntersect(const LWVector2f &RayStart, const LWVector2f &RayDir, const LWVector3f &Plane, float *Dis);

	static bool AABBIntersect(const LWVector2f &aAABBMin, const LWVector2f &aAABBMax, const LWVector2f &bAABBMin, const LWVector2f &bAABBMax, LWVector2f *IntersectNrm);

	static bool CircleIntersect(const LWVector2f &aCenterPoint, const LWVector2f &bCenterPoint, float aRadius, float bRadius, LWVector2f *IntersectNrm);

	static bool PlanePlaneIntersect(const LWVector3f &aPlane, const LWVector3f &bPlane, LWVector2f *IntersectPnt);

	static bool PointInsidePlanes(const LWVector2f &Point, const LWVector3f *Planes, uint32_t PlaneCnt);
	
	/*!< \brief the method of collision detection between two convex hulls is likely to be different in lightwave than in other engines.  a convex hull to lightwave is described by a set of intersecting planes, this implementation of collision detection for the convex hull requires that both hulls are perfectly symetrical along all axis's.  to ensure this is true the collision system tests both the positive and negative of each plane, in other words the passed in planes should only describe half of the convex hull and this algorithm will deduce the other half.  it is fine to pass in all sides of the convex hull, but planes which don't have a symetrical opposite will be duplicated and may cause incorrect collision behavior.
	*/
	static bool ConvexHullIntersect(const LWVector2f &aCenterPnt, const LWVector3f *aPlanes, uint32_t aPlaneCnt, const LWVector2f &bCenterPnt, const LWVector3f *bPlanes, uint32_t bPlaneCnt, LWVector2f *IntersectNrm);

	/*!< \brief the method for ray collision detection against a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
	static bool RayConvexHullIntersect(const LWVector2f &RayStart, const LWVector2f &RayDir, const LWVector3f *Planes, uint32_t PlaneCnt, float *Dis);
	
	/*!< \brief the method a point is inside a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
	static bool PointInsideConvexHull(const LWVector2f &Point, const LWVector3f *Planes, uint32_t PlaneCnt);

	/*!< \brief the method of collision detection between a convex hull and a circle.    in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
	static bool CircleConvexHullIntersect(const LWVector2f &CircCenterPnt, float Radius, const LWVector2f &ConvexHullCenterPoint, const LWVector3f *Planes, uint32_t PlaneCnt, LWVector2f *IntersectNrm);
};

#endif