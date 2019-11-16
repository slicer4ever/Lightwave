#include "Camera.h"
#include "Model.h"
#include "Renderer.h"
#include <LWCore/LWMath.h>
#include <iostream>
#include <cstdarg>



LWMatrix4f CameraPoint::MakeMatrix(void) const {
	float iRadi = 1.0f / m_Radius;
	return LWMatrix4f(iRadi, iRadi, iRadi, 1.0f);
}

CameraPoint &CameraPoint::BuildFrustrum(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result) {
	Result[0] = LWVector4f(Fwrd, m_Radius);
	Result[1] = LWVector4f(-Fwrd, m_Radius);
	Result[2] = LWVector4f(Right, m_Radius);
	Result[3] = LWVector4f(-Right, m_Radius);
	Result[4] = LWVector4f(-Up, m_Radius);
	Result[5] = LWVector4f(Up, m_Radius);
	return *this;
}

CameraPoint &CameraPoint::BuildFrustrumPoints(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result) {
	LWVector3f NC = -Fwrd * m_Radius;
	LWVector3f FC = Fwrd * m_Radius;
	Result[0] = LWVector4f(NC - Right * m_Radius - Up * m_Radius, -m_Radius);
	Result[1] = LWVector4f(NC + Right * m_Radius - Up * m_Radius, m_Radius);
	Result[2] = LWVector4f(NC + Right * m_Radius + Up * m_Radius, 0.0f);
	Result[3] = LWVector4f(FC - Right * m_Radius - Up * m_Radius, 0.0f);
	Result[4] = LWVector4f(FC + Right * m_Radius - Up * m_Radius, 0.0f);
	Result[5] = LWVector4f(FC + Right * m_Radius + Up * m_Radius, 0.0f);
	return *this;
}

CameraPoint::CameraPoint(float Radius) : m_Radius(Radius) {}


LWMatrix4f CameraPerspective::MakeMatrix(void) const {
	return LWMatrix4f::Perspective(m_FOV, m_Aspect, m_Near, m_Far);
}

CameraPerspective &CameraPerspective::BuildFrustrum(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result) {
	float t = tanf(m_FOV*0.5f);
	float nh = m_Near * t;
	float nw = nh * m_Aspect;

	LWVector3f NC = Fwrd * m_Near;

	LWVector3f NT = (NC - (Up*nh)).Normalize();
	LWVector3f NB = (NC + (Up*nh)).Normalize();
	LWVector3f NR = (NC - (Right*nw)).Normalize();
	LWVector3f NL = (NC + (Right*nw)).Normalize();

	Result[0] = LWVector4f(Fwrd, m_Near);
	Result[1] = LWVector4f(-Fwrd, m_Far);
	Result[2] = LWVector4f(NR.Cross(Up), 0.0f);
	Result[3] = LWVector4f(-NL.Cross(Up), 0.0f);
	Result[4] = LWVector4f(-NT.Cross(Right), 0.0f);
	Result[5] = LWVector4f(NB.Cross(Right), 0.0f);
	return *this;
}

CameraPerspective &CameraPerspective::BuildFrustrumPoints(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result) {
	float t = tanf(m_FOV*0.5f);
	float nh = m_Near * t;
	float nw = nh * m_Aspect;

	float fh = m_Far * t;
	float fw = fh * m_Aspect;

	LWVector3f NC = Fwrd * m_Near;
	LWVector3f FC = Fwrd * m_Far;

	Result[0] = LWVector4f(NC - Right * nw + Up * nh, m_Near);
	Result[1] = LWVector4f(NC + Right * nw + Up * nh, m_Far);
	Result[2] = LWVector4f(NC - Right * nw - Up * nh, 0.0f);

	Result[3] = LWVector4f(FC - Right * fw + Up * fh, 0.0f);
	Result[4] = LWVector4f(FC + Right * fw + Up * fh, 0.0f);
	Result[5] = LWVector4f(FC - Right * fw - Up * fh, 0.0f);
	return *this;
}

CameraPerspective::CameraPerspective(float FOV, float Aspect, float Near, float Far) : m_FOV(FOV), m_Aspect(Aspect), m_Near(Near), m_Far(Far) {}



LWMatrix4f CameraOrtho::MakeMatrix(void) const {
	return LWMatrix4f::Ortho(m_Left, m_Right, m_Bottom, m_Top, m_Near, m_Far);
}

CameraOrtho &CameraOrtho::BuildFrustrum(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result) {
	Result[0] = LWVector4f(Fwrd, m_Near);
	Result[1] = LWVector4f(-Fwrd, m_Far);
	Result[2] = LWVector4f(Right, -m_Left);
	Result[3] = LWVector4f(-Right, m_Right);
	Result[4] = LWVector4f(-Up, m_Top);
	Result[5] = LWVector4f(Up, -m_Bottom);
	return *this;
}

CameraOrtho &CameraOrtho::BuildFrustrumPoints(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result) {
	LWVector3f NC = Fwrd * m_Near;
	LWVector3f FC = Fwrd * m_Far;
	Result[0] = LWVector4f(NC + Right * m_Left + Up * m_Bottom, m_Near);
	Result[1] = LWVector4f(NC + Right * m_Right + Up * m_Bottom, m_Far);
	Result[2] = LWVector4f(NC + Right * m_Right + Up * m_Top, 0.0f);

	Result[3] = LWVector4f(FC + Right * m_Left + Up * m_Bottom, 0.0f);
	Result[4] = LWVector4f(FC + Right * m_Right + Up * m_Bottom, 0.0f);
	Result[5] = LWVector4f(FC + Right * m_Right + Up * m_Top, 0.0f);
	return *this;
}

CameraOrtho::CameraOrtho(float Left, float Right, float Near, float Far, float Top, float Bottom) : m_Left(Left), m_Right(Right), m_Near(Near), m_Far(Far), m_Top(Top), m_Bottom(Bottom) {}


uint32_t Camera::GetListForSphereInCameras(Camera **Cameras, uint32_t CameraCnt, const LWVector3f &Position, float Radius) {
	uint32_t ListBits = 0;
	for (uint32_t i = 0; i < CameraCnt; i++) {
		if (Cameras[i]->SphereInFrustrum(Position, Radius)) ListBits |= lFrame::GetIDBit(Cameras[i]->GetListID());
	}
	return ListBits;
}

uint32_t Camera::GetListForSphereInCamerasf(uint32_t CameraCnt, const LWVector3f &Position, float Radius, ...) {
	Camera *CamList[64];
	va_list lst;
	va_start(lst, Radius);
	for (uint32_t i = 0; i < CameraCnt; i++) CamList[i] = va_arg(lst, Camera*);
	va_end(lst);
	return GetListForSphereInCameras(CamList, CameraCnt, Position, Radius);
}

uint32_t Camera::GetListForConeInCameras(Camera **Cameras, uint32_t CameraCnt, const LWVector3f &Position, const LWVector3f &Direction, float Length, float Theta) {
	auto ConeInPlane = [](const LWVector4f &Plane, const LWVector3f &Pos, const LWVector3f &Dir, float Len, float Radius) {
		LWVector3f M = LWVector3f(Plane.x, Plane.y, Plane.z).Cross(Dir).Cross(Dir).Normalize();
		LWVector3f Q = Pos + Dir * Len - M * Radius;
		float md = LWVector4f(Pos, 1.0f).Dot(Plane);
		float mq = LWVector4f(Q, 1.0f).Dot(Plane);
		return mq >= 0.0f || md >= 0.0f;
	};
	uint32_t ListBits = 0;
	float Radi = tanf(Theta)*Length;
	for (uint32_t i = 0; i < CameraCnt; i++) {
		LWVector3f P = Position - Cameras[i]->GetPosition();
		const LWVector4f *CF = Cameras[i]->GetViewFrustrum();
		bool Inside = true;
		for (uint32_t n = 0; n < 6 && Inside; n++) Inside = ConeInPlane(CF[n], P, Direction, Length, Radi);
		if(!Inside) continue;
		ListBits |= lFrame::GetIDBit(Cameras[i]->GetListID());
	}
	return ListBits;
}

uint32_t Camera::GetListForConeInCamerasf(uint32_t CameraCnt, const LWVector3f &Position, const LWVector3f &Direction, float Length, float Theta, ...) {
	Camera *CamList[64];
	va_list lst;
	va_start(lst, Theta);
	for (uint32_t i = 0; i < CameraCnt; i++) CamList[i] = va_arg(lst, Camera*);
	va_end(lst);
	return GetListForConeInCameras(CamList, CameraCnt, Position, Direction, Length, Theta);
}


LWVector2f Camera::MakeSphereDirection(const LWVector3f &Direction) {
	return LWVector2f(atan2f(Direction.z, Direction.x), asinf(Direction.y));
}

LWVector3f Camera::MakeDirection(const LWVector2f &SphereDir) {
	float c = cosf(SphereDir.y);
	return LWVector3f(cosf(SphereDir.x)*c, sinf(SphereDir.y), sinf(SphereDir.x)*c);
}

Camera &Camera::SetCameraControlled(bool Control) {
	m_CameraControlled = Control;
	return *this;
}

Camera &Camera::SetPosition(const LWVector3f &Position) {
	m_Position = Position;
	return *this;
}

Camera &Camera::SetDirection(const LWVector3f &Direction) {
	m_Direction = Direction;
	return *this;
}

Camera &Camera::SetUp(const LWVector3f &Up) {
	m_Up = Up;
	return *this;
}

Camera &Camera::SetAspect(float Aspect) {
	if (IsPointCamera()) return *this;
	if (IsOrthoCamera()) return *this;
	m_Perspective.m_Aspect = Aspect;
	return *this;
}

Camera &Camera::BuildCascadeCameraViews(const LWVector3f &LightDir, Camera *CamBuffer, uint32_t CascadeCnt, const LWVector3f &SceneAABBMin, const LWVector3f &SceneAABBMax, uint32_t ListOffset) {
	LWVector3f U = m_Up;
	if (fabs(U.Dot(LightDir)) >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWVector3f(1.0f, 0.0f, 0.0f);
	LWVector3f R = LightDir.Cross(U).Normalize();
	U = R.Cross(LightDir);
	LWVector4f F[6];
	BuildFrustrumPoints(F);
	LWVector3f NTL = LWVector3f(F[0].x, F[0].y, F[0].z);
	LWVector3f NTR = LWVector3f(F[1].x, F[1].y, F[1].z);
	LWVector3f NBL = LWVector3f(F[2].x, F[2].y, F[2].z);

	LWVector3f FTL = LWVector3f(F[3].x, F[3].y, F[3].z);
	LWVector3f FTR = LWVector3f(F[4].x, F[4].y, F[4].z);
	LWVector3f FBL = LWVector3f(F[5].x, F[5].y, F[5].z);

	LWVector3f NX = NTR - NTL;
	LWVector3f NY = NBL - NTL;

	LWVector3f FX = FTR - FTL;
	LWVector3f FY = FBL - FTL;

	LWVector3f NBR = NTL + NX + NY;
	LWVector3f FBR = FTL + FX + FY;

	LWVector3f TL = FTL - NTL;
	LWVector3f TR = FTR - NTR;
	LWVector3f BL = FBL - NBL;
	LWVector3f BR = FBR - NBR;

	//Max of 4 cascades.
	CascadeCnt = std::min<uint32_t>(CascadeCnt, 4);
	float Far = m_Perspective.m_Far;
	float MinDistance = 200.0f;
	//Manually adjusted cascaded distances, depending on CasecadeCnt, and minimum distance
	float SDistances[5] = { 0.0f, MinDistance/Far, (MinDistance*3.0f)/Far, 0.5f, 1.0f };
	SDistances[CascadeCnt] = 1.0f;

	LWVector3f AABBSize = SceneAABBMax - SceneAABBMin;
	LWVector3f AABBPnts[8] = { LWVector3f(SceneAABBMin), SceneAABBMin + LWVector3f(AABBSize.x, 0.0f, 0.0f), SceneAABBMin + LWVector3f(0.0f, 0.0f, AABBSize.z), SceneAABBMin + LWVector3f(AABBSize.x, 0.0f, AABBSize.z), SceneAABBMin + LWVector3f(0.0f, AABBSize.y, 0.0f), SceneAABBMin + LWVector3f(AABBSize.x, AABBSize.y, 0.0f), SceneAABBMin + LWVector3f(0.0f, AABBSize.y, AABBSize.z), SceneAABBMax };
	LWMatrix4f ProjViewMatrix = GetProjViewMatrix();
	for (uint32_t i = 0; i < 8; i++) AABBPnts[i] = AABBPnts[i] * ProjViewMatrix;

	for (uint32_t i = 0; i < CascadeCnt; i++) {
		float iL = SDistances[i];
		float nL = SDistances[i+1];		
		LWVector3f P[8];
		P[0] = NTL + iL * TL;
		P[1] = NTR + iL * TR;
		P[2] = NBL + iL * BL;
		P[3] = NBR + iL * BR;

		P[4] = NTL + nL * TL;
		P[5] = NTR + nL * TR;
		P[6] = NBL + nL * BL;
		P[7] = NBR + nL * BR;

		LWVector3f Min = LWVector3f();
		LWVector3f Max = LWVector3f();
		for (uint32_t n = 1; n < 8; n++) {
			//std::cout << n << ": " << P[n] << std::endl;
			LWVector3f C = P[n]-P[0];
			LWVector3f Pnt = LWVector3f(R.Dot(C), U.Dot(C), LightDir.Dot(C));
			Min = Min.Min(Pnt);
			Max = Max.Max(Pnt);
		}
		
		for (uint32_t i = 0; i < 8; i++) {
			float z = LightDir.Dot(AABBPnts[i]);
			Min.z = std::min<float>(z, Min.z);
		}
		//zMin = (zMin-2000.0f);// *= 20.0f; //Move the near back a bit to prevent near objects being inadvertently clipped.
		P[0] += m_Position + LightDir * Min.z;
		//std::cout << P[0] << " | " << xMin << " " << xMax << " | " << yMin << " " << yMax << " | " << zMin << " " << zMax << " | " << R << " | " << U << " | " << LightDir << std::endl;
		CamBuffer[i] = Camera(P[0], LightDir, m_Up, ListOffset+i, Min.x, Max.x, Min.y, Max.y, 0.0f, Max.z - Min.z, true);
		//std::cout << "CamPnt: " << CamBuffer[i].IsPointCamera() << std::endl;
		//CamBuffer[i] = Camera(P[0], LightDir, m_Up, 1.0f, LW_PI_4, 1.0f, zMax-zMin);
		CamBuffer[i].BuildFrustrum();
	}
	return *this;
}

Camera &Camera::ProcessDirectionInputThird(const LWVector3f &Center, float Radius, const LWVector2f &MouseDis, float HorizontalSens, float VerticalSens, const LWVector4f &MinMaxXY, bool Controlling) {
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

Camera &Camera::ProcessDirectionInputFirst(const LWVector2f &MouseDis, float HorizontalSens, float VerticalSens, const LWVector4f &MinMaxXY, bool Controlling) {
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

Camera &Camera::SetSphericalDirection(const LWVector2f &SphereCoordinates) {
	m_Direction = MakeDirection(SphereCoordinates);
	return *this;
}

Camera &Camera::ToggleCameraControl(void) {
	m_CameraControlled = !m_CameraControlled;
	return *this;
}

LWVector3f Camera::UnProject(const LWVector2f &ScreenPnt, float Depth, const LWVector2f &WndSize) const {
	LWVector4f Pnt = LWVector4f(ScreenPnt / WndSize * 2.0f - 1.0f, Depth*2.0f - 1.0f, 1.0f);
	Pnt = Pnt * (GetViewMatrix()*GetProjMatrix()).Inverse();
	if (fabs(Pnt.w) <= std::numeric_limits<float>::epsilon()) return m_Position;
	Pnt.w = 1.0f / Pnt.w;
	return LWVector3f(Pnt.x, Pnt.y, Pnt.z) * Pnt.w;
}

LWVector3f Camera::Project(const LWVector3f &Pnt, const LWVector2f &WndSize) const {
	LWVector4f P = LWVector4f(Pnt, 1.0f);
	P = P * GetViewMatrix()*GetProjMatrix();
	if (fabs(P.w) <= std::numeric_limits<float>::epsilon()) return LWVector3f(-1.0f);
	P.w = 1.0f / P.w;
	P *= P.w;
	return LWVector3f((P.x*0.5f + 0.5f)*WndSize.x, (P.y*0.5f + 0.5f)*WndSize.y, (1.0f+P.z)*0.5f);
}

Camera &Camera::SetOrtho(bool isOrtho) {
	m_Flag = (m_Flag&~OrthoSource) | (isOrtho ? OrthoSource : 0);
	return *this;
}

Camera &Camera::SetPointSource(bool isPointSource) {
	m_Flag = (m_Flag&~PointSource) | (isPointSource ? PointSource : 0);
	return *this;
}

Camera &Camera::SetShadowCaster(bool isShadowCaster) {
	m_Flag = (m_Flag&~ShadowCaster) | (isShadowCaster ? ShadowCaster : 0);
	return *this;
}

Camera &Camera::SetListID(uint32_t ListID) {
	m_ListID = ListID;
	return *this;
}

Camera &Camera::BuildFrustrum(void) {
	LWVector3f U = m_Up;
	if (fabs(m_Direction.Dot(U)) >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWVector3f(1.0f, 0.0f, 0.0f);
	LWVector3f R = m_Direction.Cross(U).Normalize();
	U = R.Cross(m_Direction);

	if (IsPointCamera()) m_Point.BuildFrustrum(m_Direction, U, R, m_ViewFrustrum);
	else if (IsOrthoCamera()) m_Ortho.BuildFrustrum(m_Direction, U, R, m_ViewFrustrum);
	else m_Perspective.BuildFrustrum(m_Direction, U, R, m_ViewFrustrum);
	return *this;
}

Camera &Camera::BuildFrustrumPoints(LWVector4f *Result) {

	LWVector3f U = m_Up;
	if (fabs(m_Direction.Dot(U)) >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWVector3f(1.0f, 0.0f, 0.0f);
	LWVector3f R = m_Direction.Cross(U).Normalize();
	U = R.Cross(m_Direction);

	if (IsPointCamera()) m_Point.BuildFrustrumPoints(m_Direction, U, R, Result);
	else if (IsOrthoCamera()) m_Ortho.BuildFrustrumPoints(m_Direction, U, R, Result);
	else m_Perspective.BuildFrustrumPoints(m_Direction, U, R, Result);
	return *this;
}

LWMatrix4f Camera::GetViewMatrix(void) const {
	LWVector3f U = m_Up;
	float D = fabs(U.Dot(m_Direction));
	if (D >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWVector3f(1.0f, 0.0f, 0.0f);

	return LWMatrix4f::LookAt(m_Position, m_Position + m_Direction, U).Inverse();
}

LWMatrix4f Camera::GetDirectionMatrix(void) const {
	LWVector3f U = m_Up;
	float D = fabs(U.Dot(m_Direction));
	if (D >= 1.0f - std::numeric_limits<float>::epsilon()) U = LWVector3f(1.0f, 0.0f, 0.0f);

	return LWMatrix4f::LookAt(m_Position, m_Position + m_Direction, U);
}

LWMatrix4f Camera::GetProjMatrix(void) const {
	if (IsPointCamera()) return m_Point.MakeMatrix();
	if (IsOrthoCamera()) return m_Ortho.MakeMatrix();
	return m_Perspective.MakeMatrix();
}

LWMatrix4f Camera::GetProjViewMatrix(void) const {
	return GetViewMatrix()*GetProjMatrix();
}

LWVector3f Camera::GetPosition(void) const {
	return m_Position;
}

LWVector3f Camera::GetDirection(void) const {
	return m_Direction;
}

LWVector3f Camera::GetFlatDirection(void) const {
	LWVector3f Dir = m_Direction;
	Dir.y = 0.0f;
	return Dir.Normalize();
}

LWVector3f Camera::GetUp(void) const {
	return m_Up;
}

CameraOrtho &Camera::GetOrthoPropertys(void) {
	return m_Ortho;
}

CameraPerspective &Camera::GetPerspectivePropertys(void) {
	return m_Perspective;
}

CameraPoint &Camera::GetPointPropertys(void) {
	return m_Point;
}

LWVector2f Camera::GetSphericalDirection(void) const {
	return MakeSphereDirection(m_Direction);
}

const LWVector4f *Camera::GetViewFrustrum(void) const {
	return m_ViewFrustrum;
}

bool Camera::SphereInFrustrum(const LWVector3f &Position, float Radius) {
	LWVector4f P = LWVector4f(Position - m_Position, 1.0f);
	float d0 = m_ViewFrustrum[0].Dot(P);
	float d1 = m_ViewFrustrum[1].Dot(P);
	float d2 = m_ViewFrustrum[2].Dot(P);
	float d3 = m_ViewFrustrum[3].Dot(P);
	float d4 = m_ViewFrustrum[4].Dot(P);
	float d5 = m_ViewFrustrum[5].Dot(P);
	float m = std::min<float>(std::min<float>(std::min<float>(d0, d1), std::min<float>(d2, d3)), std::min<float>(d4, d5));
	return m >= -Radius;
}

bool Camera::ConeInFrustrum(const LWVector3f &Position, const LWVector3f &Direction, float Length, float Theta) {
	auto ConeInPlane = [](const LWVector4f &Plane, const LWVector3f &Pos, const LWVector3f &Dir, float Len, float Radius) {
		LWVector3f M = LWVector3f(Plane.x, Plane.y, Plane.z).Cross(Dir).Cross(Dir).Normalize();
		LWVector3f Q = Pos + Dir * Len - M * Radius;
		float md = LWVector4f(Pos, 1.0f).Dot(Plane);
		float mq = LWVector4f(Q, 1.0f).Dot(Plane);
		return mq >= 0.0f || md >= 0.0f;
	};
	LWVector3f P = Position - m_Position;
	float Radi = tanf(Theta)*Length;
	for (uint32_t i = 0; i < 6; i++) {
		if (!ConeInPlane(m_ViewFrustrum[i], P, Direction, Length, Radi)) {
			return false;
		}
	}
	return true;
}

bool Camera::GetCameraControlled(void) const {
	return m_CameraControlled;
}

bool Camera::IsOrthoCamera(void) const {
	return (m_Flag&OrthoSource)!=0;
}

bool Camera::IsPointCamera(void) const {
	return (m_Flag&PointSource)!=0;
}

bool Camera::IsShadowCaster(void) const {
	return (m_Flag&ShadowCaster) != 0;
}

uint32_t Camera::GetListID(void) const {
	return m_ListID;
}

Camera::Camera(const LWVector3f &Position, const LWVector3f &ViewDirection, const LWVector3f &Up, uint32_t ListID, float Aspect, float Fov, float Near, float Far, bool ShadowCast) : m_Position(Position), m_Direction(ViewDirection), m_Up(Up),  m_CameraControlled(false), m_PrevCameraControlled(false), m_ListID(ListID), m_Flag(ShadowCast?ShadowCaster:0) {
	m_Perspective = CameraPerspective(Fov, Aspect, Near, Far);
	BuildFrustrum();
}

Camera::Camera(const LWVector3f &Position, const LWVector3f &ViewDirection, const LWVector3f &Up, uint32_t ListID, float Left, float Right, float Bottom, float Top, float Near, float Far, bool ShadowCast) : m_Position(Position), m_Direction(ViewDirection), m_Up(Up), m_CameraControlled(false), m_PrevCameraControlled(false), m_ListID(ListID), m_Flag((ShadowCast?ShadowCaster:0) | OrthoSource) {
	m_Ortho = CameraOrtho(Left, Right, Near, Far, Top, Bottom);
	BuildFrustrum();
}

Camera::Camera(const LWVector3f &Position, float Radius, uint32_t ListID, bool ShadowCast) : m_Position(Position), m_Direction(LWVector3f(0.0f, 0.0f, 1.0f)), m_Up(LWVector3f(0.0f, 1.0f, 0.0f)), m_CameraControlled(false), m_PrevCameraControlled(false), m_ListID(ListID), m_Flag((ShadowCast?ShadowCaster:0)|PointSource) {
	m_Point = CameraPoint(Radius);
	BuildFrustrum();
}

Camera::Camera(uint32_t ListID) : m_Position(LWVector3f()), m_Direction(LWVector3f(1.0f, 0.0f, 0.0f)), m_Up(LWVector3f(0.0f, 1.0f, 0.0f)), m_CameraControlled(false), m_PrevCameraControlled(false), m_ListID(ListID), m_Flag(0) {
	m_Perspective = CameraPerspective(LW_PI_4, 1.0f, 0.1f, 10000.0f);	
}

Camera::Camera() : m_Position(LWVector3f(0.0f)), m_Direction(LWVector3f(1.0f, 0.0f, 0.0f)), m_Up(LWVector3f(0.0f, 1.0f, 0.0f)), m_CameraControlled(false), m_PrevCameraControlled(false), m_ListID(0), m_Flag(0) {
	m_Perspective = CameraPerspective(LW_PI_4, 1.0f, 0.1f, 10000.0f);
}
