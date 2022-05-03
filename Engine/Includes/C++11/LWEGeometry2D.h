#ifndef LWEGEOMETRY2D_H
#define LWEGEOMETRY2D_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>

template<class Type>
bool LWERayRayIntersect(const LWVector2<Type> &aRayStart, const LWVector2<Type> &aRayEnd, const LWVector2<Type> &bRayStart, const LWVector2<Type> &bRayEnd, LWVector2<Type> *IntersectPoint) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//Use float epsilon instead of double.
	LWVector2<Type> ADir = aRayEnd - aRayStart;
	LWVector2<Type> BDir = bRayEnd - bRayStart;
	LWVector2<Type> Dir = aRayStart - bRayStart;
	Type d = BDir.y * ADir.x - BDir.x * ADir.y;
	Type na = BDir.x * Dir.y - BDir.y * Dir.x;
	Type nb = ADir.x * Dir.y - ADir.y * Dir.x;
	if ((Type)abs(na) < e && (Type)abs(nb) < e && (Type)abs(d) < e) {
		if (IntersectPoint) *IntersectPoint = ADir * 0.5f;
		return true;
	}
	if ((Type)abs(d) < e) return false;
	d = (Type)1 / d;
	Type u = na * d;
	Type v = nb * d;
	if (IntersectPoint) *IntersectPoint = aRayStart + ADir * u;
	return u >= (Type)0 && u <= (Type)1 && v >= (Type)0 && v <= (Type)1;
}

template<class Type>
bool LWERayAABBIntersect(const LWVector2<Type> &RayStart, const LWVector2<Type> &RayEnd, const LWVector2<Type> &AABBMin, const LWVector2<Type> &AABBMax, Type *Min, Type *Max) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//Use float epsilon instead of double.
	LWVector2<Type> Dir = RayEnd - RayStart;
	Dir = Dir.Abs().Blend_Less(e, e) * Dir.Sign();

	LWVector2<Type> invDir = (Type)1 / Dir;

	LWVector2<Type> MinBox = (AABBMin - RayStart) * invDir;
	LWVector2<Type> MaxBox = (AABBMax - RayStart) * invDir;
	Type tmin = MinBox.Min(MaxBox).Max();
	Type tmax = MaxBox.Max(MinBox).Min();
	if (Min) *Min = tmin;
	if (Max) *Max = tmax;
	if (tmax < (Type)0) return false;
	if (tmin > tmax) return false;
	return true;
}

template<class Type>
bool LWERayCircleIntersect(const LWVector2<Type> &RayStart, const LWVector2<Type> &RayEnd, const LWVector2<Type> &CenterPnt, Type Radius, Type *Min, Type *Max) {
	LWVector2<Type> RayDir = RayEnd - RayStart;
	Type RayLen = RayDir.Length();
	if (RayLen <= std::numeric_limits<float>::epsilon()) return false;
	Type iRayLen = 1.0f / RayLen;
	LWVector2<Type> RayDirN = RayDir * iRayLen;
	LWVector2<Type> CrcDir = CenterPnt - RayStart;
	Type cDot = CrcDir.Dot(RayDirN);
	if (cDot < 0.0f) return false;
	LWVector2<Type> CrcPnt = (RayStart + RayDirN * cDot - CenterPnt);
	Type d2 = CrcPnt.LengthSquared();
	if (d2 > Radius * Radius) return false;
	Type p = sqrtf(Radius * Radius - d2);
	Type t0 = cDot - p;
	Type t1 = cDot + p;
	if (t1 < t0) std::swap(t0, t1);
	if (t0 > RayLen) return false;
	if (t0 < (Type)0) {
		t0 = t1;
		if (t0 < (Type)0) return false;
	}
	if (Min) *Min = t0 * iRayLen;
	if (Max) *Max = t1 * iRayLen;
	return true;
}

template<class Type>
bool LWERayPlaneIntersect(const LWVector2<Type> &RayStart, const LWVector2<Type> &RayDir, const LWVector3<Type> &Plane, Type *Dis) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//use float epsilon instead of double.
	LWVector2<Type> Nrm = Plane.xy();
	Type d = RayDir.Dot(Nrm);
	if ((Type)abs(d) < e) return false;
	if (Dis) *Dis = -(RayStart.Dot(Nrm) - Plane.z) / d;
	return true;
}

template<class Type>
bool LWEAABBIntersect(const LWVector2<Type> &aAABBMin, const LWVector2<Type> &aAABBMax, const LWVector2<Type> &bAABBMin, const LWVector2<Type> &bAABBMax, LWVector2<Type> *IntersectNrm) {
	if (aAABBMin.x > bAABBMax.x || aAABBMin.y > bAABBMax.y || aAABBMax.x < bAABBMin.x || aAABBMax.y < bAABBMin.y) return false;
	if (IntersectNrm) {
		LWVector2<Type> Nrms[4] = { LWVector2<Type>(-1, 0), LWVector2<Type>(-1, 0), LWVector2<Type>(0, -1), LWVector2<Type>(0, -1) };
		Type d[4] = { aAABBMin.x - bAABBMax.x, aAABBMax.x - bAABBMin.x, aAABBMin.y - bAABBMax.y, aAABBMax.y - bAABBMin.y };
		uint32_t low = 0;
		for (uint32_t i = 1; i < 4; i++) {
			if (abs(d[i]) < abs(d[low])) low = i;
		}
		*IntersectNrm = Nrms[low] * d[low];
	}
	return true;
}

template<class Type>
bool LWECircleIntersect(const LWVector2<Type> &aCenterPoint, const LWVector2<Type> &bCenterPoint, Type aRadius, Type bRadius, LWVector2<Type> *IntersectNrm) {
	LWVector2<Type> Dir = bCenterPoint - aCenterPoint;
	Type LenSq = Dir.LengthSquared();
	Type r = aRadius + bRadius;
	if (IntersectNrm) *IntersectNrm = Dir.Normalize() * (Dir.Length() - r);
	return LenSq < r*r;
}

template<class Type>
bool LWEPlanePlaneIntersect(const LWVector3<Type> &aPlane, const LWVector3<Type> &bPlane, LWVector2<Type> *IntersectPnt) {
	LWVector2<Type> aNrm = aPlane.xy();
	LWVector2<Type> bNrm = bPlane.xy();
	LWVector2<Type> aPnt = aNrm * aPlane.z;
	LWVector2<Type> paNrm = aNrm.Perpindicular();
	Type t;
	bool Result = LWERayPlaneIntersect(aPnt, paNrm, bPlane, &t);
	if (!Result) return false;
	if (IntersectPnt) *IntersectPnt = aPnt + paNrm * t;
	return true;
}

template<class Type>
bool LWEPointInsideAABB(const LWVector2<Type> &Point, const LWVector2<Type> &AABBMin, const LWVector2<Type> &AABBMax) {
	return Point.x >= AABBMin.x && Point.x <= AABBMax.x && Point.y >= AABBMin.y && Point.y <= AABBMax.y;
}

template<class Type>
bool LWEPointInsidePlanes(const LWVector2<Type> &Point, const LWVector3<Type> *Planes, uint32_t PlaneCnt) {
	const Type e = (Type)0.0001;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2<Type> Nrm = Planes[i].xy();
		Type d = Nrm.Dot(Point) - Planes[i].z;
		if (d > e) return false;
	}
	return true;
}

/*!< \brief the method of collision detection between two convex hulls is likely to be different in lightwave than in other engines.  a convex hull to lightwave is described by a set of intersecting planes, this implementation of collision detection for the convex hull requires that both hulls are perfectly symetrical along all axis's.  to ensure this is true the collision system tests both the positive and negative of each plane, in other words the passed in planes should only describe half of the convex hull and this algorithm will deduce the other half.  it is fine to pass in all sides of the convex hull, but planes which don't have a symetrical opposite will be duplicated and may cause incorrect collision behavior.*/
template<class Type>
bool LWEConvexHullIntersect(const LWVector2<Type> &aCenterPnt, const LWVector3<Type> *aPlanes, uint32_t aPlaneCnt, const LWVector2<Type> &bCenterPnt, const LWVector3<Type> *bPlanes, uint32_t bPlaneCnt, LWVector2<Type> *IntersectNrm) {
	const Type e = (Type)0.0001;
	const uint32_t PlaneCnt = 32;
	LWVector2<Type> Dir = bCenterPnt - aCenterPnt;
	LWVector3<Type> Planes[PlaneCnt];
	Type OrigDis[PlaneCnt];
	uint32_t APlaneCnt = aPlaneCnt * 2;
	uint32_t BPlaneCnt = bPlaneCnt * 2;
	Type Lowest = (Type)-10000.0;
	LWVector2<Type> LowNrm;
	if (APlaneCnt + BPlaneCnt >= PlaneCnt) return false;
	for (uint32_t i = 0; i < aPlaneCnt; i++) {
		Planes[i * 2] = aPlanes[i];
		Planes[i * 2 + 1] = LWVector3<Type>(-aPlanes[i].x, -aPlanes[i].y, aPlanes[i].z);
		OrigDis[i * 2] = OrigDis[i * 2 + 1] = aPlanes[i].z;
	}
	for (uint32_t i = 0; i < bPlaneCnt; i++) {
		Planes[APlaneCnt + i * 2] = bPlanes[i];
		Planes[APlaneCnt + i * 2 + 1] = LWVector3<Type>(-bPlanes[i].x, -bPlanes[i].y, bPlanes[i].z);
		OrigDis[APlaneCnt + i * 2] = OrigDis[APlaneCnt + i * 2 + 1] = bPlanes[i].z;
	}
	for (uint32_t i = 0; i < APlaneCnt; i++) {
		LWVector2<Type> ANrm = Planes[i].xy();
		for (uint32_t n = APlaneCnt; n < APlaneCnt + BPlaneCnt; n++) {
			LWVector2f BNrm = Planes[n].xy();
			Type d = ANrm.Dot(BNrm);
			if (d < e) continue;
			Planes[i].z += OrigDis[n] * d;
			Planes[n].z += OrigDis[i] * d;
		}
	}
	for (uint32_t i = 0; i < APlaneCnt + BPlaneCnt; i++) {
		LWVector2<Type> Nrm = Planes[i].xy();
		Type Dot = Nrm.Dot(Dir) - Planes[i].z;
		if (Dot < e) {
			if (Dot > Lowest) {
				Lowest = Dot;
				LowNrm = Nrm;
			}
		} else return false;
	}
	if (IntersectNrm) *IntersectNrm = LowNrm * Lowest;
	return true;
}

/*!< \brief the method for ray collision detection against a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWERayConvexHullIntersect(const LWVector2<Type> &RayStart, const LWVector2<Type> &RayDir, const LWVector3<Type> *Planes, uint32_t PlaneCnt, Type *Dis) {
	Type Lowest = (Type)10000.0;
	bool Result = false;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2<Type> aNrm = Planes[i].xy();
		LWVector2<Type> Nrms[2] = { aNrm, -aNrm };
		for (uint32_t n = 0; n < 2; n++) {
			LWVector3<Type> Plane = LWVector3f(Nrms[n].x, Nrms[n].y, Planes[i].z);
			Type t;
			if(LWERayPlaneIntersect(RayStart, RayDir, Plane, &t)){
				LWVector2<Type> Pnt = RayStart + RayDir * t;
				if (LWEPointInsideConvexHull(Pnt, Planes, PlaneCnt)) {
					if ((Type)abs(t) < (Type)abs(Lowest)) Lowest = t;
					Result = true;
				}
			}
		}
	}
	if (Dis) *Dis = Lowest;
	return Result;
}

/*!< \brief the method a point is inside a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWEPointInsideConvexHull(const LWVector2<Type> &Point, const LWVector3<Type> *Planes, uint32_t PlaneCnt) {
	const Type e = (Type)0.0001;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2<Type> aNrm = Planes[i].xy();
		LWVector2<Type> Nrms[2] = { aNrm, -aNrm };
		for (uint32_t n = 0; n < 2; n++) {
			Type d = Nrms[n].Dot(Point) - Planes[i].z;
			if (d > e) return false;
		}
	}
	return true;
}

/*!< \brief the method of collision detection between a convex hull and a circle.    in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWECircleConvexHullIntersect(const LWVector2<Type> &CircCenterPnt, Type Radius, const LWVector2<Type> &ConvexHullCenterPoint, const LWVector3<Type> *Planes, uint32_t PlaneCnt, LWVector2<Type> *IntersectNrm) {
	const Type e = (Type)0.0001;
	LWVector2<Type> Dir = ConvexHullCenterPoint - CircCenterPnt;
	Type Lowest = 10000.0f;
	LWVector2<Type> LowNrm;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2<Type> aNrm = Planes[i].xy();
		LWVector2<Type> Nrms[2] = { aNrm, -aNrm };
		for (uint32_t k = 0; k < 2; k++) {
			Type Dis = Planes[i].z + Radius;
			Type Dot = Nrms[k].Dot(Dir) + Dis;
			if (Dot > -e) {
				if (Dot < Lowest) {
					Lowest = Dot;
					LowNrm = Nrms[k];
				}
			} else return false;
		}
	}
	if (IntersectNrm) *IntersectNrm = LowNrm * Lowest;
	return true;
}


/*!< \brief Constructs a new AABB from the transform matrix applied to the passed in aabb's. (If TransformMatrix is not a standard scale/rotation/translation matrix then this method will produce incorrect results.) */
template<class Type>
static void LWETransformAABB(const LWVector2<Type> &AAMin, const LWVector2<Type> &AAMax, const LWMatrix3<Type> &TransformMatrix, LWVector2<Type> &AAMinResult, LWVector2<Type> &AAMaxResult) {
	LWVector2<Type> xAxisA = TransformMatrix.m_Rows[0].xy() * AAMin.x;
	LWVector2<Type> xAxisB = TransformMatrix.m_Rows[0].xy() * AAMax.x;

	LWVector2<Type> yAxisA = TransformMatrix.m_Rows[1].xy() * AAMin.y;
	LWVector2<Type> yAxisB = TransformMatrix.m_Rows[1].xy() * AAMax.y;

	AAMinResult = xAxisA.Min(xAxisB) + yAxisA.Min(yAxisB) + TransformMatrix.m_Rows[3].xy();
	AAMaxResult = xAxisA.Max(xAxisB) + yAxisA.Max(yAxisB) + TransformMatrix.m_Rows[3].xy();
	return;
}


/*!< \brief Constructs a new AABB from the transform matrix applied to the passed in aabb's. (If TransformMatrix is not a standard scale/rotation matrix then this method will produce incorrect results.) */
template<class Type>
static void LWETransformAABB(const LWVector2<Type> &AAMin, const LWVector2<Type> &AAMax, const LWMatrix2<Type> &TransformMatrix, LWVector2<Type> &AAMinResult, LWVector2<Type> &AAMaxResult) {
	LWVector2<Type> xAxisA = TransformMatrix.m_Rows[0] * AAMin.x;
	LWVector2<Type> xAxisB = TransformMatrix.m_Rows[0] * AAMax.x;

	LWVector2<Type> yAxisA = TransformMatrix.m_Rows[1] * AAMin.y;
	LWVector2<Type> yAxisB = TransformMatrix.m_Rows[1] * AAMax.y;

	AAMinResult = xAxisA.Min(xAxisB) + yAxisA.Min(yAxisB);
	AAMaxResult = xAxisA.Max(xAxisB) + yAxisA.Max(yAxisB);
	return;
}

#endif