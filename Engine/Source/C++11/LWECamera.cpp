#include "LWECamera.h"
#include <LWCore/LWMath.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWSMatrix.h>
#include <LWESGeometry3D.h>
#include <iostream>
#include <cstdarg>

LWSMatrix4f LWECameraPoint::MakeMatrix(void) const {
	float iRadi = 1.0f / m_Radius;
	return LWSMatrix4f(iRadi, iRadi, iRadi, 1.0f);
}

const LWECameraPoint &LWECameraPoint::BuildFrustrum(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const {
	LWSVector4f R = LWSVector4f(m_Radius);
	Result[0] = Fwrd.AAAB(R);
	Result[1] = (-Fwrd).AAAB(R);
	Result[2] = Right.AAAB(R);
	Result[3] = (-Right).AAAB(R);
	Result[4] = (-Up).AAAB(R);
	Result[5] = Up.AAAB(R);
	return *this;
}

const LWECameraPoint &LWECameraPoint::BuildFrustrumPoints(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const {
	LWSVector4f R = LWSVector4f(m_Radius);
	LWSVector4f NC = -Fwrd * R;
	LWSVector4f FC = Fwrd * R;

	Result[0] = NC - Right * R - Up * R;//LWVector4f(NC - Right * m_Radius - Up * m_Radius, -m_Radius);
	Result[1] = NC + Right * R - Up * R;// LWVector4f(NC + Right * m_Radius - Up * m_Radius, m_Radius);
	Result[2] = NC + Right * m_Radius + Up * R;
	Result[3] = FC - Right * m_Radius - Up * R;
	Result[4] = FC + Right * m_Radius - Up * R;
	Result[5] = FC + Right * m_Radius + Up * R;
	Result[0] = Result[0].AAAB(-R);
	Result[1] = Result[1].AAAB(R);
	return *this;
}

LWECameraPoint::LWECameraPoint(float Radius) : m_Radius(Radius) {}


LWSMatrix4f LWECameraPerspective::MakeMatrix(void) const {
	return LWSMatrix4f::Perspective(m_FOV, m_Aspect, m_Near, m_Far);
}

const LWECameraPerspective &LWECameraPerspective::BuildFrustrum(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const {
	float t = tanf(m_FOV*0.5f);
	float nh = m_Near * t;
	float nw = nh * m_Aspect;

	LWSVector4f NC = Fwrd * m_Near;

	LWSVector4f NT = (NC - (Up*nh)).Normalize();
	LWSVector4f NB = (NC + (Up*nh)).Normalize();
	LWSVector4f NR = (NC - (Right*nw)).Normalize();
	LWSVector4f NL = (NC + (Right*nw)).Normalize();

	Result[0] = Fwrd;// LWVector4f(Fwrd, m_Near);
	Result[1] = -Fwrd;// LWVector4f(-Fwrd, m_Far);
	Result[2] = NR.Cross3(Up);// LWVector4f(NR.Cross(Up), 0.0f);
	Result[3] = (-NL).Cross3(Up);// LWVector4f(-NL.Cross(Up), 0.0f);
	Result[4] = (-NT).Cross3(Right); // LWVector4f(-NT.Cross(Right), 0.0f);
	Result[5] = NB.Cross3(Right);// LWVector4f(NB.Cross(Right), 0.0f);
	Result[0] = Result[0].AAAB(LWSVector4f(m_Near));
	Result[1] = Result[1].AAAB(LWSVector4f(m_Far));
	return *this;
}

const LWECameraPerspective &LWECameraPerspective::BuildFrustrumPoints(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const {
	float t = tanf(m_FOV*0.5f);
	float nh = m_Near * t;
	float nw = nh * m_Aspect;

	float fh = m_Far * t;
	float fw = fh * m_Aspect;

	LWSVector4f NC = Fwrd * m_Near;
	LWSVector4f FC = Fwrd * m_Far;

	Result[0] = NC - Right * nw + Up * nh;//LWVector4f(NC - Right * nw + Up * nh, m_Near);
	Result[1] = NC + Right * nw + Up * nh;// LWVector4f(NC + Right * nw + Up * nh, m_Far);
	Result[2] = NC - Right * nw - Up * nh;// , 0.0f);

	Result[3] = FC - Right * fw + Up * fh;// , 0.0f);
	Result[4] = FC + Right * fw + Up * fh;// , 0.0f);
	Result[5] = FC - Right * fw - Up * fh;// , 0.0f);
	Result[0] = Result[0].AAAB(LWSVector4f(m_Near));
	Result[1] = Result[1].AAAB(LWSVector4f(m_Far));
	return *this;
}

LWECameraPerspective::LWECameraPerspective(float FOV, float Aspect, float Near, float Far) : m_FOV(FOV), m_Aspect(Aspect), m_Near(Near), m_Far(Far) {}



LWSMatrix4f LWECameraOrtho::MakeMatrix(void) const {
	return LWSMatrix4f::Ortho(m_Left, m_Right, m_Bottom, m_Top, m_Near, m_Far);
}

const LWECameraOrtho &LWECameraOrtho::BuildFrustrum(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const {
	Result[0] = Fwrd.AAAB(LWSVector4f(m_Near));// LWVector4f(Fwrd, m_Near);
	Result[1] = (-Fwrd).AAAB(LWSVector4f(m_Far));// (-Fwrd, m_Far);
	Result[2] = (Right).AAAB(LWSVector4f(-m_Left));// LWVector4f(Right, -m_Left);
	Result[3] = (-Right).AAAB(LWSVector4f(m_Right));// LWVector4f(-Right, m_Right);
	Result[4] = -Up.AAAB(LWSVector4f(-m_Top));// LWVector4f(-Up, m_Top);
	Result[5] = (Up).AAAB(LWSVector4f(-m_Bottom)); // LWVector4f(Up, -m_Bottom);
	//for (uint32_t i = 0; i < 6; i++) std::cout << i << ": " << Result[i] << std::endl;
	return *this;
}

const LWECameraOrtho &LWECameraOrtho::BuildFrustrumPoints(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const {
	LWSVector4f NC = Fwrd * m_Near;
	LWSVector4f FC = Fwrd * m_Far;
	Result[0] = NC + Right * m_Left + Up * m_Bottom;// , m_Near);
	Result[1] = NC + Right * m_Right + Up * m_Bottom;// , m_Far);
	Result[2] = NC + Right * m_Right + Up * m_Top;// , 0.0f);

	Result[3] = FC + Right * m_Left + Up * m_Bottom;// , 0.0f);
	Result[4] = FC + Right * m_Right + Up * m_Bottom;// , 0.0f);
	Result[5] = FC + Right * m_Right + Up * m_Top;// , 0.0f);
	Result[0] = Result[0].AAAB(LWSVector4f(m_Near));
	Result[1] = Result[1].AAAB(LWSVector4f(m_Far));
	return *this;
}

LWECameraOrtho::LWECameraOrtho(float Left, float Right, float Near, float Far, float Top, float Bottom) : m_Left(Left), m_Right(Right), m_Near(Near), m_Far(Far), m_Top(Top), m_Bottom(Bottom) {}

uint32_t LWECamera::MakeCascadeCameraViews(const LWSVector4f &LightDir, const LWSVector4f &ViewPosition, const LWSVector4f *ViewFrustumPoints, const LWSMatrix4f &ProjViewMatrix, LWECamera *CamBuffer, uint32_t CascadeCnt, const LWSVector4f &SceneAABBMin, const LWSVector4f &SceneAABBMax) {
	LWSVector4f U = LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f);
	if (fabs(U.Dot3(LightDir)) >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f);
	LWSVector4f R = LightDir.Cross3(U).Normalize3();
	U = R.Cross3(LightDir);
	LWSVector4f NTL = ViewFrustumPoints[0];
	LWSVector4f NTR = ViewFrustumPoints[1];
	LWSVector4f NBL = ViewFrustumPoints[2];

	LWSVector4f FTL = ViewFrustumPoints[3];
	LWSVector4f FTR = ViewFrustumPoints[4];
	LWSVector4f FBL = ViewFrustumPoints[5];

	LWSVector4f NX = NTR - NTL;
	LWSVector4f NY = NBL - NTL;

	LWSVector4f FX = FTR - FTL;
	LWSVector4f FY = FBL - FTL;

	LWSVector4f NBR = NTL + NX + NY;
	LWSVector4f FBR = FTL + FX + FY;

	LWSVector4f TL = FTL - NTL;
	LWSVector4f TR = FTR - NTR;
	LWSVector4f BL = FBL - NBL;
	LWSVector4f BR = FBR - NBR;

	//Max of 4 cascades.
	CascadeCnt = std::min<uint32_t>(CascadeCnt, 4);
	float Far = ViewFrustumPoints[1].w;
	float MinDistance = std::min<float>(75.0f * 3.0f, Far * 0.6f);
	//Manually adjusted cascaded distances, depending on CasecadeCnt, and minimum distance
	float SDistances[5] = { 0.0f, (MinDistance * 0.33f) / Far, MinDistance / Far, 0.6f, 1.0f };
	SDistances[CascadeCnt] = 1.0f;
	float zDistance = 0.0f;
	LWSVector4f AABBPnts[8] = { SceneAABBMin,
								SceneAABBMin.BAAA(SceneAABBMax),
								SceneAABBMin.AABA(SceneAABBMax),
								SceneAABBMin.BABA(SceneAABBMax),
								SceneAABBMin.ABAA(SceneAABBMax),
								SceneAABBMin.BBAA(SceneAABBMax),
								SceneAABBMin.ABBA(SceneAABBMax),
								SceneAABBMax };
	for (uint32_t i = 0; i < 8; i++) {
		AABBPnts[i] = AABBPnts[i] * ProjViewMatrix;
		zDistance = std::min<float>(zDistance, LightDir.Dot3(AABBPnts[i]));
	}
	for (uint32_t i = 0; i < CascadeCnt; i++) {
		float iL = SDistances[i];
		float nL = SDistances[i + 1];
		LWSVector4f P[8];
		P[0] = NTL + iL * TL;
		P[1] = NTR + iL * TR;
		P[2] = NBL + iL * BL;
		P[3] = NBR + iL * BR;

		P[4] = NTL + nL * TL;
		P[5] = NTR + nL * TR;
		P[6] = NBL + nL * BL;
		P[7] = NBR + nL * BR;

		LWSVector4f Min = LWSVector4f();
		LWSVector4f Max = LWSVector4f();
		for (uint32_t n = 1; n < 8; n++) {
			LWSVector4f C = P[n] - P[0];
			LWSVector4f Pnt = LWSVector4f(R.Dot3(C), U.Dot3(C), LightDir.Dot3(C), 1.0f);
			Min = Min.Min(Pnt);
			Max = Max.Max(Pnt);
		}
		Min.z = std::min<float>(Min.z, zDistance * SDistances[i + 1]);
		P[0] += ViewPosition + LightDir * Min.z;
		CamBuffer[i] = LWECamera(P[0].AAAB(LWSVector4f(1.0f)), LightDir, U, Min.x, Max.x, Min.y, Max.y, 0.0f, (Max.z - Min.z), ShadowCaster);
		CamBuffer[i].BuildFrustrum();
	}
	return CascadeCnt;
}

LWVector2f LWECamera::MakeSphereDirection(const LWSVector4f &Direction) {
	LWVector4f D = Direction.AsVec4();
	return LWVector2f(atan2f(D.z, D.x), asinf(D.y));
}

LWSVector4f LWECamera::MakeDirection(const LWVector2f &SphereDir) {
	float c = cosf(SphereDir.y);
	return LWSVector4f(cosf(SphereDir.x)*c, sinf(SphereDir.y), sinf(SphereDir.x)*c, 0.0f);
}

LWECamera &LWECamera::SetCameraControlled(bool Control) {
	m_CameraControlled = Control;
	m_ControlToggled = false;
	return *this;
}

LWECamera &LWECamera::SetPosition(const LWSVector4f &Position) {
	m_Position = Position;
	return *this;
}

LWECamera &LWECamera::SetDirection(const LWSVector4f &Direction) {
	m_Direction = Direction;
	return *this;
}

LWECamera &LWECamera::SetUp(const LWSVector4f &Up) {
	m_Up = Up;
	return *this;
}

LWECamera &LWECamera::SetAspect(float Aspect) {
	if (IsPointCamera()) return *this;
	if (IsOrthoCamera()) return *this;
	m_Perspective.m_Aspect = Aspect;
	return *this;
}

LWECamera &LWECamera::SetOrthoPropertys(float Left, float Right, float Bottom, float Top, float Near, float Far) {
	if(IsOrthoCamera())	m_Ortho = LWECameraOrtho(Left, Right, Near, Far, Top, Bottom);
	return *this;
}

LWECamera &LWECamera::ProcessDirectionInputThird(const LWSVector4f &Center, float Radius, const LWVector2f &MouseDis, float HorizontalSens, float VerticalSens, const LWVector4f &MinMaxXY, bool Controlling){
	if (!Controlling || !m_CameraControlled) {
		m_PrevCameraControlled = false;
		return *this;
	}
	if (m_PrevCameraControlled) {
		LWVector2f CamDir = GetSphericalDirection();
		CamDir.x += MouseDis.x*HorizontalSens;
		CamDir.y += MouseDis.y*VerticalSens;
		if (CamDir.x > LW_PI) CamDir.x -= LW_2PI;
		if (CamDir.x < -LW_PI) CamDir.x += LW_2PI;
		CamDir.x = std::min<float>(std::max<float>(CamDir.x, MinMaxXY.x), MinMaxXY.y);
		CamDir.y = std::min<float>(std::max<float>(CamDir.y, MinMaxXY.z), MinMaxXY.w);
		SetSphericalDirection(CamDir);
		m_Position = Center - GetDirection() * Radius;
	}
	m_PrevCameraControlled = true;
	return *this;
}

LWECamera &LWECamera::ProcessDirectionInputFirst(const LWVector2f &MouseDis, float HorizontalSens, float VerticalSens, const LWVector4f &MinMaxXY, bool Controlling) {
	if (!Controlling || !m_CameraControlled) {
		m_PrevCameraControlled = false;
		return *this;
	}
	if (m_PrevCameraControlled) {
		LWVector2f CamDir = GetSphericalDirection();
		CamDir.x += MouseDis.x*HorizontalSens;
		CamDir.y += MouseDis.y*VerticalSens;
		if (CamDir.x > LW_PI) CamDir.x -= LW_2PI;
		if (CamDir.x < -LW_PI) CamDir.x += LW_2PI;
		CamDir.x = std::min<float>(std::max<float>(CamDir.x, MinMaxXY.x), MinMaxXY.y);
		CamDir.y = std::min<float>(std::max<float>(CamDir.y, MinMaxXY.z), MinMaxXY.w);
		SetSphericalDirection(CamDir);
	}
	m_PrevCameraControlled = true;
	return *this;
}

LWECamera &LWECamera::SetSphericalDirection(const LWVector2f &SphereCoordinates) {
	m_Direction = MakeDirection(SphereCoordinates);
	return *this;
}

LWECamera &LWECamera::ToggleCameraControl(void) {
	if (m_CameraControlled) {
		m_CameraControlled = false;
		m_ControlToggled = false;
	} else m_ControlToggled = true;
	return *this;
}

LWSVector4f LWECamera::UnProject(const LWVector2f &ScreenPnt, float Depth, const LWVector2f &WndSize) const {
	LWSVector4f Pnt = LWSVector4f(ScreenPnt / WndSize * 2.0f - 1.0f, Depth*2.0f - 1.0f, 1.0f);
	Pnt = Pnt * (GetViewMatrix() * GetProjMatrix()).Inverse();
	float w = Pnt.w;
	if (fabs(w) <= std::numeric_limits<float>::epsilon()) return m_Position;
	w = 1.0f / w;
	return LWSVector4f(Pnt * w);
}

LWSVector4f LWECamera::Project(const LWSVector4f &Pnt, const LWVector2f &WndSize) const {
	LWSVector4f P = Pnt * GetViewMatrix()*GetProjMatrix();
	float w = P.w;
	if (fabs(w) <= std::numeric_limits<float>::epsilon()) return LWSVector4f(-1.0f);
	w = 1.0f / w;
	P *= w;
	return (P * LWSVector4f(0.5f, 0.5f, 1.0f, 1.0f) + LWSVector4f(0.5f, 0.5f, 1.0f, 0.0f)) * LWSVector4f(WndSize.x, WndSize.y, 0.5f, 1.0f);
	//return LWVector3f((P.x*0.5f + 0.5f)*WndSize.x, (P.y*0.5f + 0.5f)*WndSize.y, (1.0f+P.z)*0.5f);
}


bool LWECamera::Project(const LWSVector4f &Pnt, const LWVector2f &WndSize, LWSVector4f &ScreenPnt) const {
	ScreenPnt = Project(Pnt, WndSize);
	return ScreenPnt.x >= 0.0f && ScreenPnt.x < WndSize.x && ScreenPnt.y >= 0.0f && ScreenPnt.y <= WndSize.y && ScreenPnt.z >= 0.0f && ScreenPnt.z <= 1.0f;
}

bool LWECamera::UnProjectAgainstPlane(const LWVector2f &ScreenPnt, const LWVector2f &WndSize, const LWSVector4f &Plane, LWSVector4f &Pnt) const {
	LWSMatrix4f Matrix = (GetViewMatrix()*GetProjMatrix()).Inverse();
	LWSVector4f NearPnt = LWSVector4f(ScreenPnt / WndSize * 2.0f - 1.0f, -1.0f, 1.0f);
	LWSVector4f FarPnt = NearPnt.AABB(LWVector4f(1.0f));
	LWSVector4f Near = NearPnt * Matrix;
	LWSVector4f Far = FarPnt * Matrix;
	float nw = Near.w;
	float fw = Far.w;
	if (fabs(nw) < std::numeric_limits<float>::epsilon()) Near = m_Position;// LWVector4f(m_Position, 1.0f);
	else Near = Near * (1.0f / nw);
	if (fabs(fw) < std::numeric_limits<float>::epsilon()) Far = m_Position;
	else Far = Far * (1.0f / fw);

	LWSVector4f Dir = Far - Near;
	float Dis = 1.0f;
	if (!LWERayPlaneIntersect(Near, Dir, Plane, &Dis)) return false;
	Pnt = Near + Dir * Dis;
	return true;
}


LWECamera &LWECamera::SetOrtho(bool isOrtho) {
	m_Flag = (m_Flag&~OrthoSource) | (isOrtho ? OrthoSource : 0);
	return *this;
}

LWECamera &LWECamera::SetPointSource(bool isPointSource) {
	m_Flag = (m_Flag&~PointSource) | (isPointSource ? PointSource : 0);
	return *this;
}

LWECamera &LWECamera::SetShadowCaster(bool isShadowCaster) {
	m_Flag = (m_Flag&~ShadowCaster) | (isShadowCaster ? ShadowCaster : 0);
	return *this;
}

LWECamera &LWECamera::BuildFrustrum(void) {
	LWSVector4f U = m_Up;
	if (fabs(m_Direction.Dot3(U)) >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f);
	LWSVector4f R = m_Direction.Cross3(U).Normalize3();
	U = R.Cross3(m_Direction);

	if (IsPointCamera()) m_Point.BuildFrustrum(m_Direction, U, R, m_ViewFrustrum);
	else if (IsOrthoCamera()) m_Ortho.BuildFrustrum(m_Direction, U, R, m_ViewFrustrum);
	else m_Perspective.BuildFrustrum(m_Direction, U, R, m_ViewFrustrum);
	return *this;
}

const LWECamera &LWECamera::BuildFrustrumPoints(LWSVector4f *Result) const {
	LWSVector4f U = m_Up;
	if (fabs(m_Direction.Dot3(U)) >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f);
	LWSVector4f R = m_Direction.Cross3(U).Normalize3();
	U = R.Cross3(m_Direction);

	if (IsPointCamera()) m_Point.BuildFrustrumPoints(m_Direction, U, R, Result);
	else if (IsOrthoCamera()) m_Ortho.BuildFrustrumPoints(m_Direction, U, R, Result);
	else m_Perspective.BuildFrustrumPoints(m_Direction, U, R, Result);
	return *this;
}

LWSMatrix4f LWECamera::GetViewMatrix(void) const {
	LWSVector4f Fwrd, Up, Rght;
	MakeViewDirections(Fwrd, Rght, Up);
	return LWSMatrix4f(Rght, Up, -Fwrd, m_Position).TransformInverse();
}

LWSMatrix4f LWECamera::GetDirectionMatrix(void) const {
	LWSVector4f Fwrd, Up, Rght;
	MakeViewDirections(Fwrd, Rght, Up);
	return LWSMatrix4f(Rght, Up, Fwrd, m_Position);
}

LWSMatrix4f LWECamera::GetProjMatrix(void) const {
	if (IsPointCamera()) return m_Point.MakeMatrix();
	else if (IsOrthoCamera()) return m_Ortho.MakeMatrix();

	return m_Perspective.MakeMatrix();
}

LWSMatrix4f LWECamera::GetProjViewMatrix(void) const {
	return GetViewMatrix()*GetProjMatrix();
}

LWSVector4f LWECamera::GetPosition(void) const {
	return m_Position;
}

LWSVector4f LWECamera::GetDirection(void) const {
	return m_Direction;
}

LWSVector4f LWECamera::GetFlatDirection(void) const {
	LWSVector4f Z = LWSVector4f();
	LWSVector4f Dir = m_Direction.ABAA(Z);
	return Dir.Normalize3();
}

LWSVector4f LWECamera::GetUp(void) const {
	return m_Up;
}

LWECameraOrtho &LWECamera::GetOrthoPropertys(void) {
	return m_Ortho;
}

LWECameraPerspective &LWECamera::GetPerspectivePropertys(void) {
	return m_Perspective;
}

LWECameraPoint &LWECamera::GetPointPropertys(void) {
	return m_Point;
}

const LWECamera &LWECamera::MakeViewDirections(LWSVector4f &Forward, LWSVector4f &Right, LWSVector4f &Up) const {
	LWSVector4f U = m_Up;
	float D = fabs(U.Dot3(m_Direction));
	if (D >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f);
	Forward = m_Direction;
	Right = m_Direction.Cross3(U).Normalize3();
	Up = Right.Cross3(Forward);
	return *this;
}

LWVector2f LWECamera::GetSphericalDirection(void) const {
	return MakeSphereDirection(m_Direction);
}

const LWSVector4f *LWECamera::GetViewFrustrum(void) const {
	return m_ViewFrustrum;
}

bool LWECamera::SphereInFrustrum(const LWSVector4f &Position, float Radius) {
	return LWESphereInFrustum(Position, Radius, m_Position, m_ViewFrustrum);
}

bool LWECamera::AABBInFrustrum(const LWSVector4f &AAMin, const LWSVector4f &AAMax) {
	return LWEAABBInFrustum(AAMin, AAMax, m_Position, m_ViewFrustrum);
}

bool LWECamera::ConeInFrustrum(const LWSVector4f &Position, const LWSVector4f &Direction, float Length, float Theta) {
	return LWEConeInFrustum(Position, Direction, Theta, Length, m_Position, m_ViewFrustrum);
}

bool LWECamera::isCameraControlled(void) const {
	return m_CameraControlled;
}

bool LWECamera::isControlToggled(void) const {
	return m_ControlToggled;
}

bool LWECamera::IsOrthoCamera(void) const {
	return (m_Flag&OrthoSource)!=0;
}

bool LWECamera::IsPointCamera(void) const {
	return (m_Flag&PointSource)!=0;
}

bool LWECamera::IsShadowCaster(void) const {
	return (m_Flag&ShadowCaster) != 0;
}

bool LWECamera::IsReflection(void) const {
	return (m_Flag & Reflection) != 0;
}

LWECamera::LWECamera(const LWSVector4f &Position, const LWSVector4f &ViewDirection, const LWSVector4f &Up, float Aspect, float Fov, float Near, float Far, uint32_t Flag) : m_Position(Position), m_Direction(ViewDirection), m_Up(Up), m_Flag(Flag) {
	m_Perspective = LWECameraPerspective(Fov, Aspect, Near, Far);
	BuildFrustrum();
}

LWECamera::LWECamera(const LWSVector4f &Position, const LWSVector4f &ViewDirection, const LWSVector4f &Up, float Left, float Right, float Bottom, float Top, float Near, float Far, uint32_t Flag) : m_Position(Position), m_Direction(ViewDirection), m_Up(Up), m_Flag(Flag | OrthoSource) {
	m_Ortho = LWECameraOrtho(Left, Right, Near, Far, Top, Bottom);
	BuildFrustrum();
}

LWECamera::LWECamera(const LWSVector4f &Position, float Radius, uint32_t Flag) : m_Position(Position), m_Flag(Flag|PointSource) {
	m_Point = LWECameraPoint(Radius);
	BuildFrustrum();
}

LWECamera::LWECamera() {
	m_Perspective = LWECameraPerspective(LW_PI_4, 1.0f, 0.1f, 10000.0f);
}
