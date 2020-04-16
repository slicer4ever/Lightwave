#ifndef SKELETON_H
#define SKELETON_H
#include <LWEGLTFParser.h>

struct ModelData;

struct Joint {

	Joint() = default;

	Joint(const LWMatrix4f &InvBindMatrix, const LWMatrix4f &TransformMatrix, uint32_t ChildIdx, uint32_t NextIdx);

	LWMatrix4f m_InvBindMatrix;
	LWMatrix4f m_BindMatrix;
	LWMatrix4f m_TransformMatrix;
	LWEGLTFAnimTween m_Animation;
	uint32_t m_ChildIdx = -1;
	uint32_t m_NextIdx = -1;
};

class Skeleton {
public:
	Skeleton &BuildFrame(float AnimTime, bool Loop, ModelData &Mdl);

	Skeleton &BuildBindFrame(ModelData &Mdl);

	LWVector3f TransformPoint(const LWVector3f &Pnt, const ModelData &Frame, int32_t JointID);

	LWVector3f TransformPoint(const LWVector3f &Pnt, const ModelData &Frame, const LWVector4f &JointWeights, const LWVector4i &JointIndices);

	Joint &GetJoint(uint32_t i);

	bool PushJoint(Joint &J);

	uint32_t GetJointCount(void) const;

	float GetTotalTime(void) const;

	LWMatrix4f GetRootMatrix(void) const;

	Skeleton(uint32_t JointCnt, const LWMatrix4f &RootMatrix);

	Skeleton() = default;
private:
	LWMatrix4f m_RootMatrix;
	//LWMatrix4f m_InvRootScaleMatrix;
	std::vector<Joint> m_JointList;
	float m_TotalTIme = 0.0f;
};

#endif