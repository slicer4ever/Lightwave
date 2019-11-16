#include "Skeleton.h"
#include "Renderer.h"
#include <LWCore/LWTypes.h>

Joint::Joint(const LWMatrix4f &InvBindMatrix, const LWMatrix4f &TransformMatrix, uint32_t ChildIdx, uint32_t NextIdx) : m_InvBindMatrix(InvBindMatrix), m_BindMatrix(InvBindMatrix.Inverse()), m_TransformMatrix(TransformMatrix), m_ChildIdx(ChildIdx), m_NextIdx(NextIdx) {}

Skeleton &Skeleton::BuildFrame(uint64_t AnimTime, bool Loop, ModelData &Mdl) {

	std::function<void(uint64_t, ModelData &, uint32_t, const LWMatrix4f &)> BuildJoint = [this, &BuildJoint, &Loop](uint64_t Time, ModelData &Mdl, uint32_t Index, const LWMatrix4f &ParentMatrix) {
		Joint &J = m_JointList[Index];
		LWMatrix4f JMatrix = J.m_Animation.GetFrame(Time, Loop);
		JMatrix *= ParentMatrix;

		Mdl.m_BoneMatrixs[Index] = J.m_InvBindMatrix*JMatrix;
		if (J.m_ChildIdx != -1) BuildJoint(Time, Mdl, J.m_ChildIdx, JMatrix);
		if (J.m_NextIdx != -1) BuildJoint(Time, Mdl, J.m_NextIdx, ParentMatrix);
		return;
	};

	BuildJoint(AnimTime, Mdl, 0, m_RootMatrix);
	return *this;
}

Skeleton &Skeleton::BuildBindFrame(ModelData &Mdl) {

	std::function<void(ModelData &, uint32_t, const LWMatrix4f &)> BuildJoint = [this, &BuildJoint](ModelData &Mdl, uint32_t Index, const LWMatrix4f &ParentMatrix) {
		Joint &J = m_JointList[Index];
		LWMatrix4f JMatrix = J.m_TransformMatrix*ParentMatrix;
		Mdl.m_BoneMatrixs[Index] = J.m_InvBindMatrix * JMatrix;
		if (J.m_ChildIdx != -1) BuildJoint(Mdl, J.m_ChildIdx, JMatrix);
		if (J.m_NextIdx != -1) BuildJoint(Mdl, J.m_NextIdx, ParentMatrix);
		return;
	};

	BuildJoint(Mdl, 0, m_RootMatrix);
	return *this;
}

LWVector3f Skeleton::TransformPoint(const LWVector3f &Pnt, const ModelData &Frame, int32_t JointID) {
	return Frame.m_TransformMatrix * (Pnt * m_JointList[JointID].m_BindMatrix * Frame.m_BoneMatrixs[JointID]);
}

LWVector3f Skeleton::TransformPoint(const LWVector3f &Pnt, const ModelData &Frame, const LWVector4f &JointWeights, const LWVector4i &JointIndices) {
	LWMatrix4f BlendedFinalMat = Frame.m_BoneMatrixs[JointIndices.x] * JointWeights.x + Frame.m_BoneMatrixs[JointIndices.y]*JointWeights.y + Frame.m_BoneMatrixs[JointIndices.z]*JointWeights.z + Frame.m_BoneMatrixs[JointIndices.w]*JointWeights.w;
	LWMatrix4f BlendedBindMatrix = m_JointList[JointIndices.x].m_BindMatrix*JointWeights.x + m_JointList[JointIndices.y].m_BindMatrix*JointWeights.y + m_JointList[JointIndices.z].m_BindMatrix*JointWeights.z + m_JointList[JointIndices.w].m_BindMatrix * JointWeights.w;
	return Frame.m_TransformMatrix * (Pnt * BlendedBindMatrix * BlendedFinalMat);
}


Joint &Skeleton::GetJoint(uint32_t i) {
	return m_JointList[i];
}

bool Skeleton::PushJoint(Joint &J) {
	//std::cout << m_JointList.size() << ": " << J.m_InvBindMatrix << std::endl << J.m_BindMatrix << std::endl;
	m_TotalTIme = std::max<uint64_t>(J.m_Animation.GetTotalTime(), m_TotalTIme);
	m_JointList.push_back(std::move(J));
	return true;
}

LWMatrix4f Skeleton::GetRootMatrix(void) const {
	return m_RootMatrix;
}

uint64_t Skeleton::GetTotalTime(void) const {
	return m_TotalTIme;
}

uint32_t Skeleton::GetJointCount(void) const {
	return (uint32_t)m_JointList.size();
}

Skeleton::Skeleton(uint32_t JointCnt, const LWMatrix4f &RootMatrix) : m_RootMatrix(RootMatrix) {
	LWVector3f iScale = 1.0f / LWVector3f(m_RootMatrix.m_Rows[0].Length(), m_RootMatrix.m_Rows[1].Length(), m_RootMatrix.m_Rows[2].Length());
	m_InvRootScaleMatrix = LWMatrix4f(iScale.x, iScale.y, iScale.z, 1.0f);
	m_JointList.reserve(JointCnt);
}