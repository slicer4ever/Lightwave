#ifndef CAMERA_H
#define CAMERA_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWSMatrix.h>

struct LWECameraPoint {
	float m_Radius;
	float m_Padding[5] = {};

	LWSMatrix4f MakeMatrix(void) const;

	const LWECameraPoint &BuildFrustrum(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const;

	const LWECameraPoint &BuildFrustrumPoints(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const;

	LWECameraPoint(float Radius);

	LWECameraPoint() = default;
};

struct LWECameraPerspective {
	float m_FOV;
	float m_Aspect;
	float m_Near;
	float m_Far;
	float m_Padding[2] = {};

	LWSMatrix4f MakeMatrix(void) const;

	const LWECameraPerspective &BuildFrustrum(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const;

	const LWECameraPerspective &BuildFrustrumPoints(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const;

	LWECameraPerspective(float FOV, float Aspect, float Near, float Far);

	LWECameraPerspective() = default;
};

struct LWECameraOrtho {
	float m_Left;
	float m_Right;
	float m_Near;
	float m_Far;
	float m_Top;
	float m_Bottom;

	LWSMatrix4f MakeMatrix(void) const;

	const LWECameraOrtho &BuildFrustrum(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const;

	const LWECameraOrtho &BuildFrustrumPoints(const LWSVector4f &Fwrd, const LWSVector4f &Up, const LWSVector4f &Right, LWSVector4f *Result) const;

	LWECameraOrtho(float Left, float Right, float Near, float Far, float Top, float Bottom);

	LWECameraOrtho() = default;
};

class LWECamera {
public:
	enum {
		PointSource = 0x1,
		OrthoSource = 0x2,
		ShadowCaster=0x4,
		Reflection=0x8
	};

	static LWVector2f MakeSphereDirection(const LWSVector4f &Direction);

	static LWSVector4f MakeDirection(const LWVector2f &SphereDir);
	
	static uint32_t MakeCascadeCameraViews(const LWSVector4f &LightDir, const LWSVector4f &ViewPosition, const LWSVector4f *ViewFrustumPoints, const LWSMatrix4f &ProjViewMatrix, LWECamera *CamBuffer, uint32_t CascadeCnt, const LWSVector4f &SceneAABBMin, const LWSVector4f &SceneAABBMax);

	LWECamera &SetPosition(const LWSVector4f &Position);

	LWECamera &SetDirection(const LWSVector4f &Direction);

	//Min/Max = -180-180, x/y = Horizontal Min and Max.  z/w = Vertical Min and Max.
	LWECamera &ProcessDirectionInputFirst(const LWVector2f &MouseDis, float HorizontalSens, float VerticalSens, const LWVector4f &MinMaxXY, bool Controlling);

	//Min/Max = -180-180, x/y = Horizontal Min and Max. z/w = Vertical Min and Max.
	LWECamera &ProcessDirectionInputThird(const LWSVector4f &Center, float Radius, const LWVector2f &MouseDis, float HorizontalSens, float VericalSens, const LWVector4f &MinMaxXY, bool Controlling);

	LWECamera &SetUp(const LWSVector4f &Up);

	LWECamera &SetAspect(float Aspect);

	LWECamera &SetOrthoPropertys(float Left, float Right, float Bottom, float Top, float Near, float Far);

	LWECamera &BuildFrustrum(void);

	const LWECamera &BuildFrustrumPoints(LWSVector4f *Result) const; //Builds 6 frustrum points, where 0 = Near top left, 1 = top right, 2 = bottom left.  3 = Far top left, 4 = top right, 5 = bottom left.  used in light culling system to build sub frustrums.

	LWECamera &SetCameraControlled(bool Control);

	LWECamera &SetSphericalDirection(const LWVector2f &SphereCoordinates);

	LWECamera &ToggleCameraControl(void);

	LWSVector4f UnProject(const LWVector2f &ScreenPnt, float Depth, const LWVector2f &WndSize) const;

	LWSVector4f Project(const LWSVector4f &Pnt, const LWVector2f &WndSize) const;

	//Returns true if the screenPnt is within WndSize bounds, otherwise returns false.
	bool Project(const LWSVector4f &Pnt, const LWVector2f &WndSize, LWSVector4f &ScreenPnt) const;

	bool UnProjectAgainstPlane(const LWVector2f &ScreenPnt, const LWVector2f &WndSize, const LWSVector4f &Plane, LWSVector4f &Pnt) const;

	LWECamera &SetOrtho(bool isOrtho);

	LWECamera &SetPointSource(bool isPointLightSource);

	LWECamera &SetShadowCaster(bool isShadowCaster);

	LWECameraOrtho &GetOrthoPropertys(void);

	LWECameraPerspective &GetPerspectivePropertys(void);

	LWECameraPoint &GetPointPropertys(void);

	const LWECamera &MakeViewDirections(LWSVector4f &Forward, LWSVector4f &Right, LWSVector4f &Up) const;

	//Use this matrix for transforming objects around the camera(this inverses the GetDirectionMatrix()).
	LWSMatrix4f GetViewMatrix() const;

	//Use this matrix if extracting the camera's fwrd/up/right for particles.
	LWSMatrix4f GetDirectionMatrix() const;

	LWSMatrix4f GetProjMatrix(void) const;

	LWSMatrix4f GetProjViewMatrix(void) const;

	LWSVector4f GetPosition(void) const;

	LWSVector4f GetDirection(void) const;

	LWSVector4f GetFlatDirection(void) const;

	LWSVector4f GetUp(void) const;

	LWVector2f GetSphericalDirection(void) const;

	const LWSVector4f *GetViewFrustrum(void) const;

	bool isCameraControlled(void) const;

	bool isControlToggled(void) const;

	bool SphereInFrustrum(const LWSVector4f &Position, float Radius);

	bool ConeInFrustrum(const LWSVector4f &Position, const LWSVector4f &Direction, float Length, float Theta);

	bool AABBInFrustrum(const LWSVector4f &AAMin, const LWSVector4f &AAMax);

	bool IsOrthoCamera(void) const;

	bool IsPointCamera(void) const;

	bool IsShadowCaster(void) const;

	bool IsReflection(void) const;

	LWECamera(const LWSVector4f &Position, const LWSVector4f &ViewDirection, const LWSVector4f &Up, float Aspect, float Fov, float Near, float Far, uint32_t Flag);

	LWECamera(const LWSVector4f &Position, const LWSVector4f &ViewDirection, const LWSVector4f &Up, float Left, float Right, float Bottom, float Top, float Near, float Far, uint32_t Flag);

	LWECamera(const LWSVector4f &Position, float Radius, uint32_t Flag);

	LWECamera();
private:
	union {
		LWECameraPerspective m_Perspective;
		LWECameraOrtho m_Ortho;
		LWECameraPoint m_Point;
	};
	LWSVector4f m_ViewFrustrum[6];
	LWSVector4f m_Position;
	LWSVector4f m_Direction = LWSVector4f(0.0f, 0.0f, -1.0f, 0.0f);
	LWSVector4f m_Up = LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f);
	bool m_CameraControlled = false;
	bool m_PrevCameraControlled = false;
	bool m_ControlToggled = false;
	uint32_t m_Flag = 0;
};

#endif