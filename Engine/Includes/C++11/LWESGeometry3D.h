#ifndef LWESGEOMETRY3D_H
#define LWESGEOMETRY3D_H
#include <LWCore/LWSVector.h>
#include <LWCore/LWSMatrix.h>

template<class Type>
bool LWERayRayIntersect(const LWSVector4<Type> &aRayStart, const LWSVector4<Type> &aRayEnd, const LWSVector4<Type> &bRayStart, const LWSVector4<Type> &bRayEnd, LWSVector4<Type> *IntersectPoint) {
	return false;
	//TODO: Implement.
}

template<class Type>
bool LWERayAABBIntersect(const LWSVector4<Type> &RayStart, const LWSVector4<Type> &RayEnd, const LWSVector4<Type> &AABBMin, const LWSVector4<Type> &AABBMax, Type *Min, Type *Max) {
	const LWSVector4<Type> e = LWSVector4<Type>((Type)std::numeric_limits<float>::epsilon());
	//Use float's e instead of double as double is too precise.
	LWSVector4<Type> Dir = RayEnd - RayStart;
	Dir = Dir.Abs().Blend_LessEqual(e, e)*Dir.Sign();
	LWSVector4<Type> iDir = (Type)1 / Dir;
	LWSVector4<Type> MinBox = (AABBMin - RayStart) * iDir;
	LWSVector4<Type> MaxBox = (AABBMax - RayStart) * iDir;
	Type tmin = MinBox.Min(MaxBox).Max3();
	Type tmax = MaxBox.Max(MinBox).Min3();
	if (Min) *Min = tmin;
	if (Max) *Max = tmax;
	if (tmax < 0) return false;
	if (tmin > tmax) return false;
	return true;
}

template<class Type>
bool LWESphereIntersect(const LWSVector4<Type> &aCenterPnt, const LWSVector4<Type> &bCenterPnt, Type aRadius, Type bRadius, LWSVector4<Type> *IntersectNrm) {
	LWSVector4<Type> Dir = bCenterPnt - aCenterPnt;
	Type LenSq = Dir.LengthSquared3();
	Type r = aRadius + bRadius;
	if (IntersectNrm) *IntersectNrm = Dir.Normalize3() * (Dir.Length3() - r);
	return LenSq < r*r;
}

template<class Type>
bool LWERaySphereIntersect(const LWSVector4<Type> &RayStart, const LWSVector4<Type> &RayEnd, const LWSVector4<Type> &CenterPnt, Type Radius, Type *Min, Type *Max) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//use float epislon value instead of double.
	LWSVector4<Type> RayDir = RayEnd - RayStart;
	Type RayLen = RayDir.Length3();
	if (RayLen <= e) return false;
	Type iRayLen = 1.0f / RayLen;
	LWSVector4<Type> RayDirN = RayDir * iRayLen;
	LWSVector4<Type> CrcDir = CenterPnt - RayStart;
	Type cDot = CrcDir.Dot3(RayDirN);
	if (cDot < (Type)0) return false;
	LWSVector4<Type> CrcPnt = (RayStart + RayDirN * cDot - CenterPnt);
	Type d2 = CrcPnt.LengthSquared3();
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
bool LWERayPlaneIntersect(const LWSVector4<Type> &RayStart, const LWSVector4<Type> &RayDir, const LWSVector4<Type> &Plane, Type *Dis) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	//use float epsilon instead of double's.
	Type d = RayDir.Dot3(Plane);
	if ((Type)abs(d) < e) return false;
	if (Dis) *Dis = -(RayStart.Dot3(Plane) - Plane.w) / d;
	return true;
}

template<class Type>
bool LWEPointInsideAABB(const LWSVector4<Type> &MinBounds, const LWSVector4<Type> &MaxBounds, const LWSVector4<Type> &Pnt) {
	return Pnt.x >= MinBounds.x && Pnt.x <= MaxBounds.x && Pnt.y >= MinBounds.y && Pnt.y <= MaxBounds.y && Pnt.z >= MinBounds.z && Pnt.z <= MaxBounds.z;
}

template<class Type>
bool LWEAABBIntersect(const LWSVector4<Type> &aAABBMin, const LWSVector4<Type> &aAABBMax, const LWSVector4<Type> &bAABBMin, const LWSVector4<Type> &bAABBMax, LWSVector4<Type> *IntersectNrm) {
	if (aAABBMin.Greater3(bAABBMax) || aAABBMax.Less3(bAABBMin)) return false;
	if (IntersectNrm) {
		LWSVector4<Type> Nrms[6] = { LWSVector4<Type>(-1, 0, 0, 0), LWSVector4<Type>(-1, 0, 0, 0), LWSVector4<Type>(0, -1, 0, 0), LWSVector4<Type>(0, -1, 0, 0), LWVector4<Type>(0, 0, -1, 0), LWVector4<Type>(0, 0, -1, 0) };
		LWSVector4<Type> A = (aAABBMin - bAABBMax);
		LWSVector4<Type> B = (aAABBMax - bAABBMin);
		Type d[6] = { A.x, B.x, A.y, B.y, A.z, B.z };
		uint32_t Low = 0;
		for (uint32_t i = 1; i < 6; i++) {
			if ((Type)abs(d[i]) < (Type)(d[Low])) Low = i;
		}
		*IntersectNrm = Nrms[Low] * d[Low];
	}
	return true;
}

template<class Type>
bool LWEPlanePlaneIntersect(const LWSVector4<Type> &aPlane, const LWSVector4<Type> &bPlane, LWSVector4<Type> *IntersectPnt, LWSVector4<Type> *IntersectDir) {
	const Type e = (Type)std::numeric_limits<float>::epsilon();
	
	Type d = aPlane.Dot3(bPlane);
	if (1 - (Type)abs(d) < e) return false;

	if (IntersectPnt || IntersectDir) {
		LWSVector4<Type> Dir = aPlane.Cross3(bPlane).AAAB(LWSVector4<Type>(0));
		if (IntersectDir) *IntersectDir = Dir;
		if (IntersectPnt) {
			LWSVector4<Type> iDir = 1.0f / Dir;
			LWSVector4<Type> aDir = Dir.Abs();
			uint32_t BestDir = aDir.x > aDir.y ? (aDir.x > aDir.z ? 0 : 2) : (aDir.y > aDir.z ? 1 : 2);

			LWSVector4<Type> baW = bPlane * aPlane.wwww();
			LWSVector4<Type> abW = aPlane * bPlane.wwww();
			LWSVector4<Type> ab = abW - baW;
			LWSVector4<Type> ba = baW - abW;
			if (BestDir == 0) {
				ab *= iDir.x;
				ba *= iDir.x;
				*IntersectPnt = LWSVector4<Type>(0, ba.z, ab.y, 1);
			} else if (BestDir == 1) {
				ab *= iDir.y;
				ba *= iDir.y;
				*IntersectPnt = LWSVector4<Type>(ab.z, 0, ba.x, 1);
			} else {
				ab *= iDir.z;
				ba *= iDir.z;
				*IntersectPnt = LWSVector4<Type>(ba.y, ab.x, 0, 1);
			}
		}
	}
	return true;
}

template<class Type>
bool LWEPlanePlanePlaneIntersect(const LWSVector4<Type> &aPlane, const LWSVector4<Type> &bPlane, const LWSVector4<Type> &cPlane, LWSVector4<Type> *IntersectPoint) {
	LWSVector4<Type> tPnt;
	LWSVector4<Type> tDir;
	Type t;
	if (!LWEPlanePlaneIntersect(aPlane, bPlane, &tPnt, &tDir)) return false;
	if (!LWERayPlaneIntersect(tPnt, tDir, cPlane, &t)) return false;
	if (IntersectPoint) *IntersectPoint = tPnt + tDir * t;
	return true;
}

template<class Type>
bool LWEPointInsidePlanes(const LWSVector4<Type> &Point, const LWSVector4<Type> *Planes, uint32_t PlaneCnt) {
	const Type e = (Type)0.0001;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		Type d = Planes[i].Dot3(Point) - Planes[i].w();
		if (d > e) return false;
	}
	return true;
}

template<class Type>
bool LWEConvexHullIntersect(const LWSVector4<Type> &aCenterPnt, const LWSVector4<Type> *aPlanes, uint32_t aPlaneCnt, const LWSVector4<Type> &bCenterPnt, const LWSVector4f *bPlanes, uint32_t bPlaneCnt, LWSVector4<Type> *IntersectNrm) {
	const Type e = (Type)0.0001;
	const uint32_t PlaneCnt = 32;
	LWSVector4<Type> Dir = bCenterPnt - aCenterPnt;
	LWSVector4f Planes[PlaneCnt];
	Type OrigDis[PlaneCnt];
	uint32_t APlaneCnt = aPlaneCnt * 2;
	uint32_t BPlaneCnt = bPlaneCnt * 2;
	Type Lowest = (Type)-10000.0;
	LWSVector4<Type> LowNrm;
	if (APlaneCnt + BPlaneCnt >= PlaneCnt) return false;
	for (uint32_t i = 0; i < aPlaneCnt; i++) {
		Planes[i * 2] = aPlanes[i];
		Planes[i * 2 + 1] = aPlanes[i] * LWSVector4<Type>(-1, -1, -1, 1);
		OrigDis[i * 2] = OrigDis[i * 2 + 1] = aPlanes[i].w();
	}
	for (uint32_t i = 0; i < bPlaneCnt; i++) {
		Planes[APlaneCnt + i * 2] = bPlanes[i];
		Planes[APlaneCnt + i * 2 + 1] = bPlanes[i] * LWSVector4<Type>(-1, -1, -1, 1);
		OrigDis[APlaneCnt + i * 2] = OrigDis[APlaneCnt + i * 2 + 1] = bPlanes[i].w();
	}
	for (uint32_t i = 0; i < APlaneCnt; i++) {
		for (uint32_t n = APlaneCnt; n < APlaneCnt + BPlaneCnt; n++) {
			Type d = Planes[i].Dot3(Planes[n]);
			if (d < e) continue;
			Planes[i] += LWSVector4<Type>(0, 0, 0, OrigDis[n] * d);
			Planes[n] += LWSVector4<Type>(0, 0, 0, OrigDis[i] * d);
		}
	}
	for (uint32_t i = 0; i < APlaneCnt + BPlaneCnt; i++) {
		Type Dot = Planes[i].Dot3(Dir) - Planes[i].w();
		if (Dot < e) {
			if (Dot > Lowest) {
				Lowest = Dot;
				LowNrm = Planes[i];
			}
		} else return false;
	}
	if (IntersectNrm) *IntersectNrm = (LowNrm * Lowest).AAAB(LWSVector4<Type>(0,0,0,1));
	return true;
}

/*!< \brief the method for ray collision detection against a convex hull.  in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWERayConvexHullIntersect(const LWSVector4<Type> &RayStart, const LWSVector4<Type> &RayDir, const LWSVector4<Type> *Planes, uint32_t PlaneCnt, Type *Dis) {
	Type Lowest = (Type)10000.0;
	bool Result = false;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWSVector4<Type> Nrms[2] = { Planes[i], Planes[i] * LWSVector4<Type>(-1,-1,-1,1) };
		for (uint32_t n = 0; n < 2; n++) {
			Type t;
			if (!LWERayPlaneIntersect(RayStart, RayDir, Nrms[n], &t)) continue;
			if (t < 0) continue;
			LWSVector4<Type> Pnt = RayStart + RayDir * t;
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
bool LWEPointInsideConvexHull(const LWSVector4<Type> &Point, const LWSVector4<Type> *Planes, uint32_t PlaneCnt) {
	const Type e = (Type)0.0001;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWSVector4<Type> nPlane = Planes[i] * LWSVector4<Type>(-1, -1, -1, 1);
		Type w = Planes[i].w();
		Type dA = Planes[i].Dot3(Point) - w;
		Type dB = nPlane.Dot3(Point) - w;
		if (dA > e || dB > e) return false;
	}
	return true;
}

/*!< \brief the method of collision detection between a convex hull and a sphere.    in order to conform with other convex hulls tests, the convex hull is represented in the same manner as the intersection function(half symmetrical planes). */
template<class Type>
bool LWESphereConvexHullIntersect(const LWSVector4<Type> &CircCenterPnt, Type Radius, const LWSVector4<Type> &ConvexHullCenterPoint, const LWSVector4<Type> *Planes, uint32_t PlaneCnt, LWSVector4<Type> *IntersectNrm) {
	const Type e = (Type)0.0001;
	LWSVector4<Type> Dir = ConvexHullCenterPoint - CircCenterPnt;
	Type Lowest = (Type)10000.0;
	LWSVector4<Type> LowNrm;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWSVector4<Type> Nrms[2] = { Planes[i], Planes[i] * LWSVector4<Type>(-1,-1,-1,1) };
		for (uint32_t k = 0; k < 2; k++) {
			Type Dis = Planes[i].w() + Radius;
			Type Dot = Nrms[k].Dot3(Dir) + Dis;
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
static void LWETransformAABB(const LWSVector4<Type> &AAMin, const LWSVector4<Type> &AAMax, const LWSMatrix4<Type> &TransformMatrix, LWSVector4<Type> &AAMinResult, LWSVector4<Type> &AAMaxResult) {
	LWSVector4<Type> R0 = TransformMatrix[0];
	LWSVector4<Type> R1 = TransformMatrix[1];
	LWSVector4<Type> R2 = TransformMatrix[2];
	LWSVector4<Type> R3 = TransformMatrix[3];

	LWSVector4<Type> xAxisA = R0 * AAMin.xxxx();
	LWSVector4<Type> xAxisB = R0 * AAMax.xxxx();

	LWSVector4<Type> yAxisA = R1 * AAMin.yyyy();
	LWSVector4<Type> yAxisB = R1 * AAMax.yyyy();

	LWSVector4<Type> zAxisA = R2 * AAMin.zzzz();
	LWSVector4<Type> zAxisB = R2 * AAMax.zzzz();

	AAMinResult = xAxisA.Min(xAxisB) + yAxisA.Min(yAxisB) + zAxisA.Min(zAxisB) + R3;
	AAMaxResult = xAxisA.Max(xAxisB) + yAxisA.Max(yAxisB) + zAxisA.Max(zAxisB) + R3;
	return;
}

/*!< \brief returns true if a sphere is inside the frustum(a frustum is 6 vec4 planes.) */
template<class Type>
bool LWESphereInFrustum(const LWSVector4<Type> &Position, Type Radius, const LWSVector4<Type> &FrustumPosition, const LWSVector4<Type> *Frustum) {
	LWSVector4<Type> P = (Position - FrustumPosition).AAAB(LWSVector4<Type>(0, 0, 0, 1));
	Type d0 = Frustum[0].Dot(P);
	Type d1 = Frustum[1].Dot(P);
	Type d2 = Frustum[2].Dot(P);
	Type d3 = Frustum[3].Dot(P);
	Type d4 = Frustum[4].Dot(P);
	Type d5 = Frustum[5].Dot(P);
	Type m = std::min<Type>(std::min<Type>(std::min<Type>(d0, d1), std::min<Type>(d2, d3)), std::min<Type>(d4, d5));
	return m >= -Radius;
}

/*!< \brief returns true if the aabb is inside the frustum, this function turns the aabb into the largest sphere, which will produce some incorrect results. */
template<class Type>
bool LWEAABBInFrustum(const LWSVector4<Type> &AABBMin, const LWSVector4<Type> &AABBMax, const LWSVector4<Type> &FrustumPosition, const LWSVector4<Type> *Frustum) {
	//SquareRoot3
	const Type SR3 = (Type)1.732050807568877;
	LWSVector4f hLen = (AABBMax - AABBMin) * 0.5f;
	LWSVector4f Pos = AABBMin + hLen;
	return LWESphereInFrustum(Pos, hLen.Max3() * SR3, FrustumPosition, Frustum);
}

/*!< \brief returns true if a cone is inside the frustum, the cone is defined as a point, direction that expands out at theta upto length size. */
template<class Type>
bool LWEConeInFrustum(const LWSVector4<Type> &Position, const LWSVector4<Type> &Direction, Type Theta, Type Length, const LWSVector4<Type> &FrustumPosition, const LWSVector4<Type> *Frustum) {
	auto ConeInPlane = [](const LWSVector4<Type> &Plane, const LWSVector4<Type> &Pos, const LWSVector4<Type> &Dir, Type Len, Type Radius) {
		LWSVector4<Type> M = Plane.Cross3(Dir).Cross3(Dir).Normalize3();
		LWSVector4<Type> Q = Pos + Dir * Len - M * Radius;
		Type md = Pos.Dot(Plane);
		Type mq = Q.AAAB(LWSVector4<Type>(1)).Dot(Plane);
		return mq >= 0 || md >= 0;
	};
	LWSVector4<Type> P = (Position - FrustumPosition).xyz1();
	Type Radi = (Type)tan(Theta) * Length;
	for (uint32_t i = 0; i < 6; i++) {
		if (!ConeInPlane(Frustum[i], P, Direction, Length, Radi)) return false;
	}
	return true;
}

#endif