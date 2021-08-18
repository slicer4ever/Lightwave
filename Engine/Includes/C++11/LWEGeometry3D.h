#ifndef LWEGEOMETRY3D_H
#define LWEGEOMETRY3D_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWMatrix.h>

template<class Type>
bool LWERayRayIntersect(const LWVector3<Type> &aRayStart, const LWVector3<Type> &aRayEnd, const LWVector3<Type> &bRayStart, const LWVector3<Type> &bRayEnd, LWVector3<Type> *IntersectPoint) {
	return false;
	//TODO: Implement.
}

template<class Type>
bool LWERayAABBIntersect(const LWVector3<Type> &RayStart, const LWVector3<Type> &RayEnd, const LWVector3<Type> &AABBMin, const LWVector3<Type> &AABBMax, Type *Min, Type *Max) {
	LWVector3<Type> e = LWVector3<Type>((Type)std::numeric_limits<float>::epsilon());
	//Use float's e instead of double as double is too precise.
	LWVector3<Type> Dir = RayEnd - RayStart;
	Dir = Dir.Abs().Blend_Less(e, e)*Dir.Sign();
	LWVector3<Type> iDir = (Type)1 / Dir;
	LWVector3<Type> MinBox = (AABBMin - RayStart) * iDir;
	LWVector3<Type> MaxBox = (AABBMax - RayStart) * iDir;

	Type tmin = (MinBox.Min(MaxBox)).Max();
	Type tmax = (MinBox.Max(MaxBox)).Min();
	if (Min) *Min = tmin;
	if (Max) *Max = tmax;
	if (tmax < 0) return false;
	if (tmin > tmax) return false;
	return true;
}

template<class Type>
bool LWEPointInsideAABB(const LWVector3<Type> &MinBounds, const LWVector3<Type> &MaxBounds, const LWVector3<Type> &Pnt) {
	return Pnt.x >= MinBounds.x && Pnt.x <= MaxBounds.x && Pnt.y >= MinBounds.y && Pnt.y <= MaxBounds.y && Pnt.z >= MinBounds.z && Pnt.z <= MaxBounds.z;
}
	
template<class Type>
bool LWESphereIntersect(const LWVector3<Type> &aCenterPnt, const LWVector3<Type> &bCenterPnt, Type aRadius, Type bRadius, LWVector3<Type> *IntersectNrm) {
	LWVector3<Type> Dir = bCenterPnt - aCenterPnt;
	Type LenSq = Dir.LengthSquared();
	Type r = aRadius + bRadius;
	if (IntersectNrm) *IntersectNrm = Dir.Normalize() * (Dir.Length() - r);
	return LenSq < r*r;
}

template<class Type>
bool LWERaySphereIntersect(const LWVector3<Type> &RayStart, const LWVector3<Type> &RayEnd, const LWVector3<Type> &CenterPnt, Type Radius, Type *Min, Type *Max) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//use float epislon value instead of double.
	LWVector3<Type> RayDir = RayEnd - RayStart;
	Type RayLen = RayDir.Length();
	if (RayLen <= e) return false;
	Type iRayLen = 1.0f / RayLen;
	LWVector3<Type> RayDirN = RayDir * iRayLen;
	LWVector3<Type> CrcDir = CenterPnt - RayStart;
	Type cDot = CrcDir.Dot(RayDirN);
	if (cDot < (Type)0) return false;
	LWVector3<Type> CrcPnt = (RayStart + RayDirN * cDot - CenterPnt);
	Type d2 = CrcPnt.LengthSquared();
	if (d2 > Radius * Radius) return false;
	Type p = (Type)sqrt(Radius * Radius - d2);
	Type t0 = cDot - p;
	Type t1 = cDot + p;
	if (t1 < t0) std::swap(t0, t1);
	if (t0 > RayLen) return false;
	if (t0 < 0.0f) {
		t0 = t1;
		if (t0 < 0.0f) return false;
	}
	if (Min) *Min = t0 * iRayLen;
	if (Max) *Max = t1 * iRayLen;
	return true;
}

/*!< \brief tests a plane and ray intersection point.  a plane is represented by a normal(x,y,z) and distance(w) from 0 that plane sits at.  if the plane is Transformed, then raystart and RayDir should already have the transform applied. */
template<class Type>
bool LWERayPlaneIntersect(const LWVector3<Type> &RayStart, const LWVector3<Type> &RayDir, const LWVector4<Type> &Plane, Type *Dis) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//use float epsilon instead of double's.
	Type d = RayDir.Dot(Plane.xyz());
	if ((Type)abs(d) < e) return false;
	if (Dis) *Dis = -(RayStart.Dot(Plane.xyz()) - Plane.w) / d;
	return true;
}

template<class Type>
bool LWEAABBIntersect(const LWVector3<Type> &aAABBMin, const LWVector3<Type> &aAABBMax, const LWVector3<Type> &bAABBMin, const LWVector3<Type> &bAABBMax, LWVector3<Type> *IntersectNrm) {
	if (aAABBMin.x > bAABBMax.x || aAABBMin.y > bAABBMax.y || aAABBMin.z > bAABBMax.z || aAABBMax.x < bAABBMin.x || aAABBMax.y < bAABBMin.y || aAABBMax.z < bAABBMin.z) return false;
	if (IntersectNrm) {
		LWVector3<Type> Nrms[6] = { LWVector3<Type>(-1, 0, 0), LWVector3<Type>(-1, 0, 0), LWVector3<Type>(0, -1, 0), LWVector3<Type>(0, -1, 0), LWVector3<Type>(0, 0, -1), LWVector3<Type>(0, 0, -1) };
		Type d[6] = { aAABBMin.x - bAABBMax.x, aAABBMax.x - bAABBMin.x, aAABBMin.y - bAABBMax.y, aAABBMax.y - bAABBMin.y, aAABBMin.z - bAABBMax.z, aAABBMax.z - bAABBMin.z };
		uint32_t low = 0;
		for (uint32_t i = 1; i < 6; i++) {
			if ((Type)abs(d[i]) < (Type)abs(d[low])) low = i;
		}
		*IntersectNrm = Nrms[low] * d[low];
	}
	return true;
}

template<class Type>
bool LWEPlanePlaneIntersect(const LWVector4<Type> &aPlane, const LWVector4<Type> &bPlane, LWVector3<Type> *IntersectPnt, LWVector3<Type> *IntersectDir) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	LWVector3<Type> aNrm = aPlane.xyz();//LWVector3f(aPlane.x, aPlane.y, aPlane.z);
	LWVector3<Type> bNrm = bPlane.xyz();//LWVector3f(bPlane.x, bPlane.y, bPlane.z);
	Type d = aNrm.Dot(bNrm);
	if (1 - (Type)abs(d) < e) return false;
	if (IntersectPnt || IntersectDir) {
		LWVector3<Type> Dir = aNrm.Cross(bNrm);
		if (IntersectDir) *IntersectDir = Dir;
		if (IntersectPnt) {
			uint32_t BestDir = abs(Dir.x) > abs(Dir.y) ? (abs(Dir.x) > abs(Dir.z) ? 0 : 3) : (abs(Dir.y) > abs(Dir.z) ? 1 : 2);
			if (BestDir == 0) {
				Type iX = 1.0f / Dir.x;
				*IntersectPnt = LWVector3<Type>(0.0f, (aPlane.w * bNrm.z - bPlane.w * aNrm.z) * iX, (bPlane.w * aPlane.y - aPlane.w * bNrm.y) * iX);
			} else if (BestDir == 1) {
				Type iY = 1.0f / Dir.y;
				*IntersectPnt = LWVector3<Type>((bPlane.w * aNrm.z - aPlane.w * bNrm.z) * iY, 0.0f, (aPlane.w * bNrm.x - bPlane.w * aNrm.x) * iY);
			} else {
				Type iZ = 1.0f / Dir.z;
				*IntersectPnt = LWVector3<Type>((aPlane.w * bNrm.y - bPlane.w * aNrm.y) * iZ, (bPlane.w * aNrm.x - aPlane.w * bNrm.x) * iZ, 0.0f);
			}
		}
	}
	return true;
}

template<class Type>
bool LWEPlanePlanePlaneIntersect(const LWVector4<Type> &aPlane, const LWVector4<Type> &bPlane, const LWVector4<Type> &cPlane, LWVector3<Type> *IntersectPoint) {
	LWVector3<Type> tPnt;
	LWVector3<Type> tDir;
	Type t;
	if (!LWEPlanePlaneIntersect(aPlane, bPlane, &tPnt, &tDir)) return false;
	if (!LWERayPlaneIntersect(tPnt, tDir, cPlane, &t)) return false;
	if (IntersectPoint) *IntersectPoint = tPnt + tDir * t;
	return true;
}

template<class Type>
bool LWEPointInsidePlanes(const LWVector3<Type> &Point, const LWVector4<Type> *Planes, uint32_t PlaneCnt) {
	const Type e = (Type)0.0001;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3<Type> Nrm = Planes[i].xyz();
		Type d = Nrm.Dot(Point) - Planes[i].w;
		if (d > e) return false;
	}
	return true;
}
	
template<class Type>
bool LWEConvexHullIntersect(const LWVector3<Type> &aCenterPnt, const LWVector4<Type> *aPlanes, uint32_t aPlaneCnt, const LWVector3<Type> &bCenterPnt, const LWVector4<Type> *bPlanes, uint32_t bPlaneCnt, LWVector3<Type> *IntersectNrm) {
	const Type e = (Type)0.0001;
	const uint32_t PlaneCnt = 32;
	LWVector3<Type> Dir = bCenterPnt - aCenterPnt;
	LWVector4<Type> Planes[PlaneCnt];
	Type OrigDis[PlaneCnt];
	uint32_t APlaneCnt = aPlaneCnt * 2;
	uint32_t BPlaneCnt = bPlaneCnt * 2;
	Type Lowest = (Type)-10000.0;
	LWVector3<Type> LowNrm;
	if (APlaneCnt + BPlaneCnt >= PlaneCnt) return false;
	for (uint32_t i = 0; i < aPlaneCnt; i++) {
		Planes[i * 2] = aPlanes[i];
		Planes[i * 2 + 1] = LWVector4<Type>(-aPlanes[i].x, -aPlanes[i].y, -aPlanes[i].z, aPlanes[i].w);
		OrigDis[i * 2] = OrigDis[i * 2 + 1] = aPlanes[i].w;
	}
	for (uint32_t i = 0; i < bPlaneCnt; i++) {
		Planes[APlaneCnt + i * 2] = bPlanes[i];
		Planes[APlaneCnt + i * 2 + 1] = LWVector4<Type>(-bPlanes[i].x, -bPlanes[i].y, -bPlanes[i].z, bPlanes[i].w);
		OrigDis[APlaneCnt + i * 2] = OrigDis[APlaneCnt + i * 2 + 1] = bPlanes[i].w;
	}
	for (uint32_t i = 0; i < APlaneCnt; i++) {
		LWVector3<Type> ANrm = Planes[i].xyz();
		for (uint32_t n = APlaneCnt; n < APlaneCnt + BPlaneCnt; n++) {
			LWVector3<Type> BNrm = Planes[n].xyz();
			Type d = ANrm.Dot(BNrm);
			if (d < e) continue;
			Planes[i].w += OrigDis[n] * d;
			Planes[n].w += OrigDis[i] * d;
		}
	}
	for (uint32_t i = 0; i < APlaneCnt + BPlaneCnt; i++) {
		LWVector3<Type> Nrm = Planes[i].xyz();//LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		Type Dot = Nrm.Dot(Dir) - Planes[i].w;
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
bool LWERayConvexHullIntersect(const LWVector3<Type> &RayStart, const LWVector3<Type> &RayDir, const LWVector4<Type> *Planes, uint32_t PlaneCnt, Type *Dis) {
	Type Lowest = (Type)10000.0;
	bool Result = false;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3<Type> aNrm = Planes[i].xyz();
		LWVector3<Type> Nrms[2] = { aNrm, -aNrm };
		for (uint32_t n = 0; n < 2; n++) {
			LWVector4<Type> Plane = LWVector4<Type>(Nrms[n].x, Nrms[n].y, Nrms[n].z, Planes[i].w);
			Type t;
			if (!LWERayPlaneIntersect(RayStart, RayDir, Plane, &t)) continue;
			if (t < 0) continue;
			LWVector3<Type> Pnt = RayStart + RayDir * t;
			if (!LWEPointInsideConvexHull(Pnt, Planes, PlaneCnt)) continue;
			if (t < Lowest) Lowest = t;
			Result = true;
		}
	}
	if (Dis) *Dis = Lowest;
	return Result;
}

/*!< \brief the method a point is inside a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWEPointInsideConvexHull(const LWVector3<Type> &Point, const LWVector4<Type> *Planes, uint32_t PlaneCnt) {
	const Type e = (Type)0.0001;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3<Type> aNrm = Planes[i].xyz();
		Type dA = aNrm.Dot(Point) - Planes[i].w;
		Type dB = (-aNrm).Dot(Point) - Planes[i].w;
		if (dA > e || dB > e) return false;
	}
	return true;
}
	
/*!< \brief the method of collision detection between a convex hull and a sphere.    in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWESphereConvexHullIntersect(const LWVector3<Type> &CircCenterPnt, Type Radius, const LWVector3<Type> &ConvexHullCenterPoint, const LWVector4<Type> *Planes, uint32_t PlaneCnt, LWVector3<Type> *IntersectNrm) {
	const Type e = (Type)0.0001;
	LWVector3<Type> Dir = ConvexHullCenterPoint - CircCenterPnt;
	Type Lowest = (Type)10000.0;
	LWVector3<Type> LowNrm;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3<Type> aNrm = Planes[i].xyz();
		LWVector3<Type> Nrms[2] = { aNrm, -aNrm };
		for (uint32_t k = 0; k < 2; k++) {
			Type Dis = Planes[i].w + Radius;
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
static void LWETransformAABB(const LWVector3<Type> &AAMin, const LWVector3<Type> &AAMax, const LWMatrix4<Type> &TransformMatrix, LWVector3<Type> &AAMinResult, LWVector3<Type> &AAMaxResult) {
	LWVector3<Type> xAxisA = TransformMatrix.m_Rows[0].xyz() * AAMin.x;
	LWVector3<Type> xAxisB = TransformMatrix.m_Rows[0].xyz() * AAMax.x;

	LWVector3<Type> yAxisA = TransformMatrix.m_Rows[1].xyz() * AAMin.y;
	LWVector3<Type> yAxisB = TransformMatrix.m_Rows[1].xyz() * AAMax.y;

	LWVector3<Type> zAxisA = TransformMatrix.m_Rows[2].xyz() * AAMin.z;
	LWVector3<Type> zAxisB = TransformMatrix.m_Rows[2].xyz() * AAMax.z;

	AAMinResult = xAxisA.Min(xAxisB) + yAxisA.Min(yAxisB) + zAxisA.Min(zAxisB) + TransformMatrix.m_Rows[3].xyz();
	AAMaxResult = xAxisA.Max(xAxisB) + yAxisA.Max(yAxisB) + zAxisA.Max(zAxisB) + TransformMatrix.m_Rows[3].xyz();
	return;
}

/*!< \brief returns true if a sphere is inside the frustum(a frustum is 6 vec4 planes.) */
template<class Type>
bool LWESphereInFrustum(const LWVector3<Type> &Position, Type Radius, const LWVector3<Type> &FrustumPosition, const LWVector4<Type> *Frustum) {
	LWVector4<Type> P = LWVector4<Type>(Position - FrustumPosition, 1.0f);
	Type d0 = Frustum[0].Dot(P);
	Type d1 = Frustum[1].Dot(P);
	Type d2 = Frustum[2].Dot(P);
	Type d3 = Frustum[3].Dot(P);
	Type d4 = Frustum[4].Dot(P);
	Type d5 = Frustum[5].Dot(P);
	Type m = std::min<Type>(std::min<Type>(std::min<Type>(d0, d1), std::min<Type>(d2, d3)), std::min<Type>(d4, d5));
	return m >= -Radius;
}


/*!< \brief returns true if the aabb is inside/intersecting the frustum. */
template<class Type>
bool LWEAABBInFrustum(const LWVector4<Type> &AABBMin, const LWVector4<Type> &AABBMax, const LWVector4<Type> &FrustumPosition, const LWVector4<Type> *Frustum) {
	LWVector4<Type> Min = (AABBMin - FrustumPosition).AAAB(LWVector4<Type>((Type)1));
	LWVector4<Type> Max = (AABBMax - FrustumPosition).AAAB(LWVector4<Type>((Type)1));
	LWSVector4<Type> Table[8] = { //LUT of all possible Min/Max mix's.
		Min,
		Min.AABB(Max),
		Min.ABAB(Max),
		Min.ABBB(Max),
		Min.BAAB(Max),
		Min.BABB(Max),
		Min.BBAB(Max),
		Max };
	for (uint32_t i = 0; i < 6; i++) {
		uint32_t LUTIdx = (Frustum[i].x > (Type)0) << 2 | (Frustum[i].y > (Type)0) << 1 | (Frustum[i].z > (Type)0);
		if (Frustum[i].Dot(Table[LUTIdx]) < (Type)0) return false; //Outside
		if (Frustum[i].Dot(Table[(~LUTIdx) & 0x7]) <= (Type)0) return true; //Intersection
	}
	return true; //All inside.
}

/*!< \brief returns true if the aabb is inside the frustum, this function turns the aabb into the largest sphere, which will produce some incorrect results. */
template<class Type>
bool LWEAABBInFrustum(const LWVector3<Type> &AABBMin, const LWVector3<Type> &AABBMax, const LWVector3<Type> &FrustumPosition, const LWVector4<Type> *Frustum) {
	//SquareRoot3
	const Type SR3 = (Type)1.732050807568877;
	LWVector3<Type> hLen = (AABBMax - AABBMin) * 0.5f;
	LWVector3<Type> Pos = AABBMin + hLen;
	return LWESphereInFrustum(Pos, hLen.Max() * SR3, FrustumPosition, Frustum);
}

/*!< \brief returns true if a cone is inside the frustum, the cone is defined as a point, direction that expands out at theta upto length size. */
template<class Type>
bool LWEConeInFrustum(const LWVector3<Type> &Position, const LWVector3<Type> &Direction, Type Theta, Type Length, const LWVector3<Type> &FrustumPosition, const LWVector4<Type> *Frustum) {
	auto ConeInPlane = [](const LWVector4<Type> &Plane, const LWVector3<Type> &Pos, const LWVector3<Type> &Dir, Type Len, Type Radius) {
		LWVector3<Type> M = Plane.xyz().Cross(Dir).Cross(Dir).Normalize();
		LWVector3<Type> Q = Pos + Dir * Len - M * Radius;
		Type md = LWVector4<Type>(Pos, 1).Dot(Plane);
		Type mq = LWVector4<Type>(Q, 1).Dot(Plane);
		return mq >= 0 || md >= 0;
	};
	LWVector3<Type> P = Position - FrustumPosition;
	Type Radi = (Type)tan(Theta) * Length;
	for (uint32_t i = 0; i < 6; i++) {
		if (!ConeInPlane(Frustum[i], P, Direction, Length, Radi)) return false;
	}
	return true;
} 

#endif