#include "LWEGeometry3D.h"
#include <algorithm>
#include <iostream>


bool LWEGeometry3D::RayRayIntersect(const LWVector3f &aRayStart, const LWVector3f &aRayEnd, const LWVector3f &bRayStart, const LWVector3f &bRayEnd, LWVector3f *IntersectPoint) {
	return false;
}

bool LWEGeometry3D::RayAABBIntersect(const LWVector3f &RayStart, const LWVector3f &RayEnd, const LWVector3f &AABBMin, const LWVector3f &AABBMax, float *Min, float *Max) {
	LWVector3f Dir = RayEnd - RayStart;
	const float e = std::numeric_limits<float>::epsilon();

	if (fabs(Dir.x) < e) Dir.x = e;
	if (fabs(Dir.y) < e) Dir.y = e;
	if (fabs(Dir.z) < e) Dir.z = e;

	LWVector3f invDir = 1.0f / Dir;

	LWVector3f MinBox = (AABBMin - RayStart)*invDir;
	LWVector3f MaxBox = (AABBMax - RayStart)*invDir;
	float tmin = std::max<float>(std::max<float>(std::min<float>(MinBox.x, MaxBox.x), std::min<float>(MinBox.y, MaxBox.y)), std::min<float>(MinBox.z, MaxBox.z));
	float tmax = std::min<float>(std::min<float>(std::max<float>(MinBox.x, MaxBox.x), std::max<float>(MinBox.y, MaxBox.y)), std::max<float>(MinBox.z, MaxBox.z));
	if (Min) *Min = tmin;
	if (Max) *Max = tmax;
	if (tmax < 0) return false;
	if (tmin > tmax) return false;
	return true;
}

bool LWEGeometry3D::SphereIntersect(const LWVector3f &aCenterPnt, const LWVector3f &bCenterPnt, float aRadius, float bRadius, LWVector3f *IntersectNrm) {
	LWVector3f Dir = bCenterPnt - aCenterPnt;
	float LenSq = Dir.LengthSquared();
	float r = aRadius + bRadius;
	if (IntersectNrm) *IntersectNrm = Dir.Normalize()*(Dir.Length() - r);
	return LenSq < r*r;
}

bool LWEGeometry3D::RaySphereIntersect(const LWVector3f &RayStart, const LWVector3f &RayEnd, const LWVector3f &CenterPnt, float Radius, float *Min, float *Max) {
	LWVector3f RayDir = RayEnd - RayStart;
	float RayLen = RayDir.Length();
	if (RayLen <= std::numeric_limits<float>::epsilon()) return false;
	float iRayLen = 1.0f / RayLen;
	LWVector3f RayDirN = RayDir * iRayLen;
	LWVector3f CrcDir = CenterPnt - RayStart;
	float cDot = CrcDir.Dot(RayDirN);
	if (cDot < 0.0f) return false;
	LWVector3f CrcPnt = (RayStart + RayDirN * cDot - CenterPnt);
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

bool LWEGeometry3D::AABBIntersect(const LWVector3f &aAABBMin, const LWVector3f &aAABBMax, const LWVector3f &bAABBMin, const LWVector3f &bAABBMax, LWVector3f *IntersectNrm) {
	if (aAABBMin.x > bAABBMax.x || aAABBMin.y > bAABBMax.y || aAABBMin.z>bAABBMax.z || aAABBMax.x < bAABBMin.x || aAABBMax.y < bAABBMin.y || aAABBMax.z<bAABBMin.z ) return false;
	if (IntersectNrm) {
		LWVector3f Nrms[6] = { LWVector3f(-1.0f, 0.0f, 0.0f), LWVector3f(-1.0f, 0.0f, 0.0f), LWVector3f(0.0f, -1.0f, 0.0f), LWVector3f(0.0f, -1.0f, 0.0f), LWVector3f(0.0f, 0.0f, -1.0f), LWVector3f(0.0f, 0.0f, -1.0f) };
		float d[6] = { aAABBMin.x - bAABBMax.x, aAABBMax.x - bAABBMin.x, aAABBMin.y - bAABBMax.y, aAABBMax.y - bAABBMin.y, aAABBMin.z-bAABBMax.z, aAABBMax.z-bAABBMin.z };
		uint32_t low = 0;
		for (uint32_t i = 1; i < 6; i++) {
			if (fabs(d[i]) < fabs(d[low])) low = i;
		}
		*IntersectNrm = Nrms[low] * d[low];
	}
	return true;
}

bool LWEGeometry3D::RayPlaneIntersect(const LWVector3f &RayStart, const LWVector3f &RayDir, const LWVector4f &Plane, float *Dis) {
	LWVector3f Nrm = LWVector3f(Plane.x, Plane.y, Plane.z);
	float d = RayDir.Dot(Nrm);
	if (fabs(d) < std::numeric_limits<float>::epsilon()) return false;
	if (Dis) *Dis = -(RayStart.Dot(Nrm) - Plane.w) / d;
	return true;
}

bool LWEGeometry3D::PlanePlaneIntersect(const LWVector4f &aPlane, const LWVector4f &bPlane, LWVector3f *IntersectPnt, LWVector3f *IntersectDir) {
	LWVector3f aNrm = LWVector3f(aPlane.x, aPlane.y, aPlane.z);
	LWVector3f bNrm = LWVector3f(bPlane.x, bPlane.y, bPlane.z);
	float d = aNrm.Dot(bNrm);
	if (1.0f - fabs(d) < std::numeric_limits<float>::epsilon()) return false;
	if (IntersectPnt || IntersectDir) {
		LWVector3f Dir = aNrm.Cross(bNrm);
		if (IntersectDir) *IntersectDir = Dir;
		if (IntersectPnt) {
			uint32_t BestDir = fabs(Dir.x) > fabs(Dir.y) ? (fabs(Dir.x) > fabs(Dir.z) ? 0 : 3) : (fabs(Dir.y) > fabs(Dir.z) ? 1 : 2);
			if (BestDir == 0) {
				float iX = 1.0f / Dir.x;
				*IntersectPnt = LWVector3f(0.0f, (aPlane.w*bNrm.z - bPlane.w*aNrm.z) * iX, (bPlane.w*aPlane.y - aPlane.w*bNrm.y) * iX);
			} else if (BestDir == 1) {
				float iY = 1.0f / Dir.y;
				*IntersectPnt = LWVector3f((bPlane.w*aNrm.z - aPlane.w*bNrm.z)*iY, 0.0f, (aPlane.w*bNrm.x - bPlane.w*aNrm.x)*iY);
			} else {
				float iZ = 1.0f / Dir.z;
				*IntersectPnt = LWVector3f((aPlane.w*bNrm.y - bPlane.w*aNrm.y)*iZ, (bPlane.w*aNrm.x - aPlane.w*bNrm.x)*iZ, 0.0f);
			}
		}
	}
	return true;
}

bool LWEGeometry3D::PlanePlanePlaneIntersect(const LWVector4f &aPlane, const LWVector4f &bPlane, const LWVector4f &cPlane, LWVector3f *IntersectPoint) {
	LWVector3f tPnt;
	LWVector3f tDir;
	float t;
	if (!PlanePlaneIntersect(aPlane, bPlane, &tPnt, &tDir)) return false;
	if (!RayPlaneIntersect(tPnt, tDir, cPlane, &t)) return false;
	if (IntersectPoint) *IntersectPoint = tPnt - tDir*t;
	return true;
}

bool LWEGeometry3D::PointInsidePlanes(const LWVector3f &Point, const LWVector4f *Planes, uint32_t PlaneCnt) {
	const float e = 0.0001f;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3f Nrm = LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		float d = Nrm.Dot(Point) - Planes[i].w;
		if (d > e) {
			return false;
		}
	}
	return true;
}


bool LWEGeometry3D::ConvexHullIntersect(const LWVector3f &aCenterPnt, const LWVector4f *aPlanes, uint32_t aPlaneCnt, const LWVector3f &bCenterPnt, const LWVector4f *bPlanes, uint32_t bPlaneCnt, LWVector3f *IntersectNrm){
	const float e = 0.0001f;
	const uint32_t PlaneCnt = 32;
	LWVector3f Dir = bCenterPnt - aCenterPnt;
	LWVector4f Planes[PlaneCnt];
	float OrigDis[PlaneCnt];
	uint32_t APlaneCnt = aPlaneCnt * 2;
	uint32_t BPlaneCnt = bPlaneCnt * 2;
	float Lowest = -10000.0f;
	LWVector3f LowNrm;
	if (APlaneCnt + BPlaneCnt >= PlaneCnt) return false;
	for (uint32_t i = 0; i < aPlaneCnt; i++) {
		Planes[i * 2] = aPlanes[i];
		Planes[i * 2 + 1] = LWVector4f(-aPlanes[i].x, -aPlanes[i].y, -aPlanes[i].z, aPlanes[i].w);
		OrigDis[i * 2] = OrigDis[i * 2 + 1] = aPlanes[i].w;
	}
	for (uint32_t i = 0; i < bPlaneCnt; i++) {
		Planes[APlaneCnt + i * 2] = bPlanes[i];
		Planes[APlaneCnt + i * 2 + 1] = LWVector4f(-bPlanes[i].x, -bPlanes[i].y, -bPlanes[i].z, bPlanes[i].w);
		OrigDis[APlaneCnt + i * 2] = OrigDis[APlaneCnt + i * 2 + 1] = bPlanes[i].w;
	}
	for (uint32_t i = 0; i < APlaneCnt; i++) {
		LWVector3f ANrm = LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		for (uint32_t n = APlaneCnt; n < APlaneCnt + BPlaneCnt; n++) {
			LWVector3f BNrm = LWVector3f(Planes[n].x, Planes[n].y, Planes[n].z);
			float d = ANrm.Dot(BNrm);
			if (d < e) continue;
			Planes[i].w += OrigDis[n] * d;
			Planes[n].w += OrigDis[i] * d;
		}
	}
	for (uint32_t i = 0; i < APlaneCnt + BPlaneCnt; i++) {
		LWVector3f Nrm = LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		float Dot = Nrm.Dot(Dir) - Planes[i].w;
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

bool LWEGeometry3D::RayConvexHullIntersect(const LWVector3f &RayStart, const LWVector3f &RayDir, const LWVector4f *Planes, uint32_t PlaneCnt, float *Dis) {
	float Lowest = 10000.0f;
	bool Result = false;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3f aNrm = LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		LWVector3f Nrms[2] = { aNrm, -aNrm };
		for (uint32_t n = 0; n < 2; n++) {
			LWVector4f Plane = LWVector4f(Nrms[n].x, Nrms[n].y, Nrms[n].z, Planes[i].w);
			float t;
			if (!LWEGeometry3D::RayPlaneIntersect(RayStart, RayDir, Plane, &t)) continue;
			if(t<0.0f) continue;
			LWVector3f Pnt = RayStart + RayDir*t;
			if (!LWEGeometry3D::PointInsideConvexHull(Pnt, Planes, PlaneCnt)) continue;
			if (t < Lowest) Lowest = t;
			Result = true;
		}
	}
	if (Dis) *Dis = Lowest;
	return Result;
}

bool LWEGeometry3D::PointInsideConvexHull(const LWVector3f &Point, const LWVector4f *Planes, uint32_t PlaneCnt) {
	const float e = 0.0001f;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3f aNrm = LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		float dA = aNrm.Dot(Point) - Planes[i].w;
		float dB = (-aNrm).Dot(Point) - Planes[i].w;
		if (dA > e || dB > e) return false;
	}
	return true;
}

bool LWEGeometry3D::SphereConvexHullIntersect(const LWVector3f &CircCenterPnt, float Radius, const LWVector3f &ConvexHullCenterPoint, const LWVector4f *Planes, uint32_t PlaneCnt, LWVector3f *IntersectNrm){
	const float e = 0.0001f;
	LWVector3f Dir = ConvexHullCenterPoint - CircCenterPnt;
	float Lowest = 10000.0f;
	LWVector3f LowNrm;
	for (uint32_t i = 0; i < PlaneCnt; i++) {
		LWVector3f aNrm = LWVector3f(Planes[i].x, Planes[i].y, Planes[i].z);
		LWVector3f Nrms[2] = { aNrm, -aNrm };
		for (uint32_t k = 0; k < 2; k++) {
			float Dis = Planes[i].w + Radius;
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

void LWEGeometry3D::TransformAABB(const LWVector3f &AAMin, const LWVector3f &AAMax, const LWMatrix4f &TransformMatrix, LWVector3f &AAMinResult, LWVector3f &AAMaxResult) {
	LWVector3f xAxisA = TransformMatrix.m_Rows[0].xyz()*AAMin.x;
	LWVector3f xAxisB = TransformMatrix.m_Rows[0].xyz()*AAMax.x;

	LWVector3f yAxisA = TransformMatrix.m_Rows[1].xyz()*AAMin.y;
	LWVector3f yAxisB = TransformMatrix.m_Rows[1].xyz()*AAMax.y;

	LWVector3f zAxisA = TransformMatrix.m_Rows[2].xyz()*AAMin.z;
	LWVector3f zAxisB = TransformMatrix.m_Rows[2].xyz()*AAMax.z;

	AAMinResult = xAxisA.Min(xAxisB) + yAxisA.Min(yAxisB) + zAxisA.Min(zAxisB) + TransformMatrix.m_Rows[3].xyz();
	AAMaxResult = xAxisA.Max(xAxisB) + yAxisA.Max(yAxisB) + zAxisA.Max(zAxisB) + TransformMatrix.m_Rows[3].xyz();
	return;
}

bool LWEGeometry3D::SphereInFrustum(const LWVector3f &Position, float Radius, const LWVector3f &FrustumPosition, const LWVector4f *Frustum) {
	LWVector4f P = LWVector4f(Position - FrustumPosition, 1.0f);
	float d0 = Frustum[0].Dot(P);
	float d1 = Frustum[1].Dot(P);
	float d2 = Frustum[2].Dot(P);
	float d3 = Frustum[3].Dot(P);
	float d4 = Frustum[4].Dot(P);
	float d5 = Frustum[5].Dot(P);
	float m = std::min<float>(std::min<float>(std::min<float>(d0, d1), std::min<float>(d2, d3)), std::min<float>(d4, d5));
	return m >= -Radius;
}

bool LWEGeometry3D::AABBInFrustum(const LWVector3f &AABBMin, const LWVector3f &AABBMax, const LWVector3f &FrustumPosition, const LWVector4f *Frustum) {
	LWVector3f hLen = (AABBMax - AABBMin)*0.5f;
	LWVector3f Pos = AABBMin + hLen;
	return SphereInFrustum(Pos, hLen.Max()*1.5f, FrustumPosition, Frustum);
}

bool LWEGeometry3D::ConeInFrustum(const LWVector3f &Position, const LWVector3f &Direction, float Theta, float Length, const LWVector3f &FrustumPosition, const LWVector4f *Frustum) {
	auto ConeInPlane = [](const LWVector4f &Plane, const LWVector3f &Pos, const LWVector3f &Dir, float Len, float Radius) {
		LWVector3f M = LWVector3f(Plane.x, Plane.y, Plane.z).Cross(Dir).Cross(Dir).Normalize();
		LWVector3f Q = Pos + Dir * Len - M * Radius;
		float md = LWVector4f(Pos, 1.0f).Dot(Plane);
		float mq = LWVector4f(Q, 1.0f).Dot(Plane);
		return mq >= 0.0f || md >= 0.0f;
	};
	LWVector3f P = Position - FrustumPosition;
	float Radi = tanf(Theta)*Length;
	for (uint32_t i = 0; i < 6; i++) {
		if (!ConeInPlane(Frustum[i], P, Direction, Length, Radi)) return false;
	}
	return true;
}

