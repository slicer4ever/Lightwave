#include "LWEGeometry2D.h"
#include <algorithm>

bool LWEGeometry2D::RayRayIntersect(const LWVector2f &aRayStart, const LWVector2f &aRayEnd, const LWVector2f &bRayStart, const LWVector2f &bRayEnd, LWVector2f *IntersectPoint){
	const float e = std::numeric_limits<float>::epsilon();
	LWVector2f ADir = aRayEnd - aRayStart;
	LWVector2f BDir = bRayEnd - bRayStart;
	LWVector2f Dir = aRayStart - bRayStart;
	float d = BDir.y*ADir.x - BDir.x*ADir.y;
	float na = BDir.x*Dir.y - BDir.y*Dir.x;
	float nb = ADir.x*Dir.y - ADir.y*Dir.x;
	if (fabs(na) < e && fabs(nb) < e && fabs(d) < e) {
		if (IntersectPoint) *IntersectPoint = ADir * 0.5f;
		return true;
	}
	if (fabs(d) < e) return false;
	d = 1.0f / d;
	float u = na * d;
	float v = nb * d;
	if (IntersectPoint) *IntersectPoint = aRayStart + ADir * u;
	return u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f;
}

bool LWEGeometry2D::RayAABBIntersect(const LWVector2f &RayStart, const LWVector2f &RayEnd, const LWVector2f &AABBMin, const LWVector2f &AABBMax, float *Min, float *Max){
	LWVector2f Dir = RayEnd - RayStart;
	const float e = std::numeric_limits<float>::epsilon();

	if (fabs(Dir.x) < e) Dir.x = e;
	if (fabs(Dir.y) < e) Dir.y = e;

	LWVector2f invDir = 1.0f / Dir;

	LWVector2f MinBox = (AABBMin - RayStart)*invDir;
	LWVector2f MaxBox = (AABBMax - RayStart)*invDir;
	float tmin = std::max<float>(std::min<float>(MinBox.x, MaxBox.x), std::min<float>(MinBox.y, MaxBox.y));
	float tmax = std::min<float>(std::max<float>(MinBox.x, MaxBox.x), std::max<float>(MinBox.y, MaxBox.y));
	if (Min) *Min = tmin;
	if (Max) *Max = tmax;
	if (tmax < 0) return false;
	if (tmin > tmax) return false;
	return true;
}

bool LWEGeometry2D::RayCircleIntersect(const LWVector2f &RayStart, const LWVector2f &RayEnd, const LWVector2f &CenterPnt, float Radius, float *Min, float *Max) {
	LWVector2f RayDir = RayEnd - RayStart;
	float RayLen = RayDir.Length();
	if (RayLen <= std::numeric_limits<float>::epsilon()) return false;
	float iRayLen = 1.0f / RayLen;
	LWVector2f RayDirN = RayDir * iRayLen;
	LWVector2f CrcDir = CenterPnt - RayStart;
	float cDot = CrcDir.Dot(RayDirN);
	if (cDot < 0.0f) return false;
	LWVector2f CrcPnt = (RayStart + RayDirN * cDot - CenterPnt);
	float d2 = CrcPnt.LengthSquared();
	if (d2 > Radius*Radius) return false;
	float p = sqrtf(Radius*Radius - d2);
	float t0 = cDot - p;
	float t1 = cDot + p;
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

bool LWEGeometry2D::RayPlaneIntersect(const LWVector2f &RayStart, const LWVector2f &RayDir, const LWVector3f &Plane, float *Dis) {
	LWVector2f Nrm = LWVector2f(Plane.x, Plane.y);
	float d = RayDir.Dot(Nrm);
	if (fabs(d) < std::numeric_limits<float>::epsilon()) return false;
	if (Dis) *Dis = -(RayStart.Dot(Nrm) - Plane.z) / d;
	return true;
}

bool LWEGeometry2D::CircleIntersect(const LWVector2f &aCenterPoint, const LWVector2f &bCenterPoint, float aRadius, float bRadius, LWVector2f *IntersectNrm) {
	LWVector2f Dir = bCenterPoint - aCenterPoint;
	float LenSq = Dir.LengthSquared();
	float r = aRadius + bRadius;
	if (IntersectNrm) *IntersectNrm = Dir.Normalize()*(Dir.Length() - r);
	return LenSq < r*r;
}

bool LWEGeometry2D::PlanePlaneIntersect(const LWVector3f &aPlane, const LWVector3f &bPlane, LWVector2f *IntersectPnt) {
	LWVector2f aNrm = LWVector2f(aPlane.x, aPlane.y);
	LWVector2f bNrm = LWVector2f(bPlane.x, bPlane.y);
	LWVector2f aPnt = aNrm*aPlane.z;
	LWVector2f paNrm = aNrm.Perpindicular();
	float t;
	bool Result = RayPlaneIntersect(aPnt, paNrm, bPlane, &t);
	if (!Result) return false;
	if (IntersectPnt) *IntersectPnt = aPnt - paNrm*t;
	return true;
}

bool LWEGeometry2D::AABBIntersect(const LWVector2f &aAABBMin, const LWVector2f &aAABBMax, const LWVector2f &bAABBMin, const LWVector2f &bAABBMax, LWVector2f *IntersectNrm) {
	if (aAABBMin.x > bAABBMax.x || aAABBMin.y > bAABBMax.y || aAABBMax.x < bAABBMin.x || aAABBMax.y < bAABBMin.y) return false;
	if (IntersectNrm) {
		LWVector2f Nrms[4] = { LWVector2f(-1.0f, 0.0f), LWVector2f(-1.0f, 0.0f), LWVector2f(0.0f, -1.0f), LWVector2f(0.0f, -1.0f) };
		float d[4] = { aAABBMin.x - bAABBMax.x, aAABBMax.x - bAABBMin.x, aAABBMin.y - bAABBMax.y, aAABBMax.y - bAABBMin.y };
		uint32_t low = 0;
		for (uint32_t i = 1; i < 4; i++){
			if (fabs(d[i]) < fabs(d[low])) low = i;
		}
		*IntersectNrm = Nrms[low] * d[low];
	}
	return true;
}


bool LWEGeometry2D::PointInsidePlanes(const LWVector2f &Point, const LWVector3f *Planes, uint32_t PlaneCnt) {
	const float e = 0.0001f;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2f Nrm = LWVector2f(Planes[i].x, Planes[i].y);
		float d = Nrm.Dot(Point) - Planes[i].z;
		if (d > e) return false;
	}
	return true;
}

bool LWEGeometry2D::ConvexHullIntersect(const LWVector2f &aCenterPnt, const LWVector3f *aPlanes, uint32_t aPlaneCnt, const LWVector2f &bCenterPnt, const LWVector3f *bPlanes, uint32_t bPlaneCnt, LWVector2f *IntersectNrm) {
	const float e = 0.0001f;
	const uint32_t PlaneCnt = 32;
	LWVector2f Dir = bCenterPnt - aCenterPnt;
	LWVector3f Planes[PlaneCnt];
	float OrigDis[PlaneCnt];
	uint32_t APlaneCnt = aPlaneCnt * 2;
	uint32_t BPlaneCnt = bPlaneCnt * 2;
	float Lowest = -10000.0f;
	LWVector2f LowNrm;
	if (APlaneCnt + BPlaneCnt >= PlaneCnt) return false;
	for (uint32_t i = 0; i < aPlaneCnt; i++) {
		Planes[i * 2] = aPlanes[i];
		Planes[i * 2 + 1] = LWVector3f(-aPlanes[i].x, -aPlanes[i].y, aPlanes[i].z);
		OrigDis[i * 2] = OrigDis[i * 2 + 1] = aPlanes[i].z;
	}
	for (uint32_t i = 0; i < bPlaneCnt; i++) {
		Planes[APlaneCnt+i * 2] = bPlanes[i];
		Planes[APlaneCnt + i * 2 + 1] = LWVector3f(-bPlanes[i].x, -bPlanes[i].y, bPlanes[i].z);
		OrigDis[APlaneCnt+i * 2] = OrigDis[APlaneCnt+i * 2 + 1] = bPlanes[i].z;
	}
	for(uint32_t i=0;i<APlaneCnt;i++){
		LWVector2f ANrm = LWVector2f(Planes[i].x, Planes[i].y);
		for (uint32_t n = APlaneCnt; n < APlaneCnt+BPlaneCnt; n++) {
			LWVector2f BNrm = LWVector2f(Planes[n].x, Planes[n].y);
			float d = ANrm.Dot(BNrm);
			if (d < e) continue;
			Planes[i].z += OrigDis[n]*d;
			Planes[n].z += OrigDis[i]*d;
		}
	}
	for (uint32_t i = 0; i < APlaneCnt+BPlaneCnt; i++) {
		LWVector2f Nrm = LWVector2f(Planes[i].x, Planes[i].y);
		float Dot = Nrm.Dot(Dir) - Planes[i].z;
		if (Dot < e) {
			if (Dot > Lowest) {
				Lowest = Dot;
				LowNrm = Nrm;
			}
		} else return false;
	}
	if (IntersectNrm) *IntersectNrm = LowNrm*Lowest;
	return true;
}

bool LWEGeometry2D::RayConvexHullIntersect(const LWVector2f &RayStart, const LWVector2f &RayDir, const LWVector3f *Planes, uint32_t PlaneCnt, float *Dis){
	float Lowest = 10000.0f;
	bool Result = false;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2f aNrm = LWVector2f(Planes[i].x, Planes[i].y);
		LWVector2f Nrms[2] = { aNrm, -aNrm };
		for (uint32_t n = 0; n < 2; n++) {
			LWVector3f Plane = LWVector3f(Nrms[n].x, Nrms[n].y, Planes[i].z);
			float t;
			if (LWEGeometry2D::RayPlaneIntersect(RayStart, RayDir, Plane, &t)) {
				LWVector2f Pnt = RayStart + RayDir*t;
				if (LWEGeometry2D::PointInsideConvexHull(Pnt, Planes, PlaneCnt)) {
					if(fabs(t) < fabs(Lowest)) Lowest = t;
					Result = true;
				}
			}
		}
	}
	if (Dis) *Dis = Lowest;
	return Result;
}

bool LWEGeometry2D::PointInsideConvexHull(const LWVector2f &Point, const LWVector3f *Planes, uint32_t PlaneCnt) {
	const float e = 0.0001f;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2f aNrm = LWVector2f(Planes[i].x, Planes[i].y);
		LWVector2f Nrms[2] = { aNrm, -aNrm };
		for (uint32_t n = 0; n < 2; n++) {
			float d = Nrms[n].Dot(Point) - Planes[i].z;
			if (d > e) return false;
		}
	}
	return true;
}

bool LWEGeometry2D::CircleConvexHullIntersect(const LWVector2f &CircCenterPnt, float Radius, const LWVector2f &ConvexHullCenterPoint, const LWVector3f *Planes, uint32_t PlaneCnt, LWVector2f *IntersectNrm) {	
	const float e = 0.0001f;
	LWVector2f Dir = ConvexHullCenterPoint - CircCenterPnt;
	float Lowest = 10000.0f;
	LWVector2f LowNrm;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector2f aNrm = LWVector2f(Planes[i].x, Planes[i].y);
		LWVector2f Nrms[2] = { aNrm, -aNrm };
		for (uint32_t k = 0; k < 2; k++) {
			float Dis = Planes[i].z + Radius;
			float Dot = Nrms[k].Dot(Dir) + Dis;
			if (Dot > -e) {
				if (Dot < Lowest) {
					Lowest = Dot;
					LowNrm = Nrms[k];
				}
			} else return false;
		}
	}
	if (IntersectNrm) *IntersectNrm = LowNrm*Lowest;
	return true;
}