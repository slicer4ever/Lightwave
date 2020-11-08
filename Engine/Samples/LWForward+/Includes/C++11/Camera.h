#ifndef CAMERA_H
#define CAMERA_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWMatrix.h>

class Model;

//For point shadow's.
struct CameraPoint {
	float m_Radius;
	float m_Padding[5];

	LWMatrix4f MakeMatrix(void) const;

	CameraPoint &BuildFrustrum(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result);

	CameraPoint &BuildFrustrumPoints(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result);

	CameraPoint(float Radius);

	CameraPoint() = default;
};

struct CameraPerspective {
	float m_FOV;
	float m_Aspect;
	float m_Near;
	float m_Far;
	float m_Padding[2];

	LWMatrix4f MakeMatrix(void) const;

	CameraPerspective &BuildFrustrum(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result);

	CameraPerspective &BuildFrustrumPoints(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result);

	CameraPerspective(float FOV, float Aspect, float Near, float Far);

	CameraPerspective() = default;
};

struct CameraOrtho {
	float m_Left;
	float m_Right;
	float m_Near;
	float m_Far;
	float m_Top;
	float m_Bottom;

	LWMatrix4f MakeMatrix(void) const;

	CameraOrtho &BuildFrustrum(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result);

	CameraOrtho &BuildFrustrumPoints(const LWVector3f &Fwrd, const LWVector3f &Up, const LWVector3f &Right, LWVector4f *Result);

	CameraOrtho(float Left, float Right, float Near, float Far, float Top, float Bottom);

	CameraOrtho() = default;
};

class Camera {
public:
	enum {
		PointSource = 0x1,
		OrthoSource = 0x2,
		ShadowCaster=0x4
	};

	static LWVector2f MakeSphereDirection(const LWVector3f &Direction);

	static LWVector3f MakeDirection(const LWVector2f &SphereDir);

	static uint32_t GetListForSphereInCameras(Camera **Cameras, uint32_t CameraCnt, const LWVector3f &Position, float Radius);

	static uint32_t GetListForSphereInCamerasf(uint32_t CameraCnt, const LWVector3f &Position, float Radius, ...);

	static uint32_t GetListForConeInCameras(Camera **Cameras, uint32_t CameraCnt, const LWVector3f &Position, const LWVector3f &Direction, float Length, float Theta);

	static uint32_t GetListForConeInCamerasf(uint32_t CameraCnt, const LWVector3f &Position, const LWVector3f &Direction, float Length, float Theta, ...);

	Camera &SetPosition(const LWVector3f &Position);

	Camera &SetDirection(const LWVector3f &Direction);

	//Min/Max = -180-180, x/y = Horizontal Min and Max.  z/w = Vertical Min and Max.
	Camera &ProcessDirectionInputFirst(const LWVector2f &MouseDis, float HorizontalSens, float VerticalSens, const LWVector4f &MinMaxXY, bool Controlling);

	//Min/Max = -180-180, x/y = Horizontal Min and Max. z/w = Vertical Min and Max.
	Camera &ProcessDirectionInputThird(const LWVector3f &Center, float Radius, const LWVector2f &MouseDis, float HorizontalSens, float VericalSens, const LWVector4f &MinMaxXY, bool Controlling);

	Camera &BuildCascadeCameraViews(const LWVector3f &LightDir, Camera *CamBuffer, uint32_t CascadeCnt, const LWVector3f &SceneAABBMin, const LWVector3f &SceneAABBMax, uint32_t ListOffset);

	Camera &SetUp(const LWVector3f &Up);

	Camera &SetAspect(float Aspect);

	Camera &BuildFrustrum(void);

	Camera &BuildFrustrumPoints(LWVector4f *Result); //Builds 6 frustrum points, where 0 = Near top left, 1 = top right, 2 = bottom left.  3 = Far top left, 4 = top right, 5 = bottom left.  used in light culling system to build sub frustrums.

	Camera &SetCameraControlled(bool Control);

	Camera &SetSphericalDirection(const LWVector2f &SphereCoordinates);

	Camera &ToggleCameraControl(void);

	LWVector3f UnProject(const LWVector2f &ScreenPnt, float Depth, const LWVector2f &WndSize) const;

	LWVector3f Project(const LWVector3f &Pnt, const LWVector2f &WndSize) const;

	Camera &SetOrtho(bool isOrtho);

	Camera &SetPointSource(bool isPointLightSource);

	Camera &SetShadowCaster(bool isShadowCaster);

	Camera &SetListID(uint32_t ListID);

	CameraOrtho &GetOrthoPropertys(void);

	CameraPerspective &GetPerspectivePropertys(void);

	CameraPoint &GetPointPropertys(void);

	//Use this matrix for transforming objects around the camera(this inverses the GetDirectionMatrix()).
	LWMatrix4f GetViewMatrix() const;

	//Use this matrix if extracting the camera's fwrd/up/right for particles.
	LWMatrix4f GetDirectionMatrix() const;

	LWMatrix4f GetProjMatrix() const;

	LWMatrix4f GetProjViewMatrix() const;

	LWVector3f GetPosition(void) const;

	LWVector3f GetDirection(void) const;

	LWVector3f GetFlatDirection(void) const;

	LWVector3f GetUp(void) const;

	LWVector2f GetSphericalDirection(void) const;

	const LWVector4f *GetViewFrustrum(void) const;

	bool GetCameraControlled(void) const;

	bool SphereInFrustrum(const LWVector3f &Position, float Radius);

	bool ConeInFrustrum(const LWVector3f &Position, const LWVector3f &Direction, float Length, float Theta);

	bool IsOrthoCamera(void) const;

	bool IsPointCamera(void) const;

	bool IsShadowCaster(void) const;

	uint32_t GetListID(void) const;

	Camera(const LWVector3f &Position, const LWVector3f &ViewDirection, const LWVector3f &Up, uint32_t ListID, float Aspect, float Fov, float Near, float Far, bool ShadowCast);

	Camera(const LWVector3f &Position, const LWVector3f &ViewDirection, const LWVector3f &Up, uint32_t ListID, float Left, float Right, float Bottom, float Top, float Near, float Far, bool ShadowCast);

	Camera(const LWVector3f &Position, float Radius, uint32_t ListID, bool ShadowCast);

	Camera(uint32_t ListID);

	Camera();
private:
	union {
		CameraPerspective m_Perspective;
		CameraOrtho m_Ortho;
		CameraPoint m_Point;
	};
	LWVector4f m_ViewFrustrum[6];
	LWVector3f m_Position;
	LWVector3f m_Direction;
	LWVector3f m_Up;
	bool m_CameraControlled;
	bool m_PrevCameraControlled;
	uint32_t m_ListID;
	uint32_t m_Flag;

};

#endif