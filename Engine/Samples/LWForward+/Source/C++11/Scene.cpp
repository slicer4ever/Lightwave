#include "Scene.h"
#include <LWPlatform/LWFileStream.h>
#include <LWVideo/LWImage.h>
#include <LWEJson.h>
#include <LWCore/LWByteBuffer.h>
#include "Camera.h"
#include <LWEGLTFParser.h>
#include <vector>
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWWindow.h>

Scene *Scene::LoadGLTF(const LWUTF8Iterator &Path, LWVideoDriver *Driver, LWAllocator &Allocator) {
	LWEGLTFParser P;
	fmt::print("Loading scene: '{}'\n", Path);
	if (!LWEGLTFParser::LoadFile(P, Path, Allocator)) {
		return nullptr;
	}
	std::vector<uint32_t> NodeList;
	std::vector<uint32_t> MeshList;
	std::vector<uint32_t> SkinList;
	std::vector<uint32_t> LightList;
	std::vector<uint32_t> MaterialList;
	std::vector<uint32_t> TextureList;
	std::vector<uint32_t> ImageList;

	auto MapIDToListIndex = [](std::vector<uint32_t> &List, uint32_t ID)->uint32_t {
		for (uint32_t i = 0; i < List.size(); i++) {
			if (ID == List[i]) return i;
		}
		return -1;
	};

	std::function<void(uint32_t, Scene *)> ParseNode = [&P, &ParseNode, &MapIDToListIndex, &NodeList, &MeshList, &SkinList, &LightList](uint32_t NodeID, Scene *S) {
		LWEGLTFNode *Node = P.GetNode(NodeID);
		uint32_t LightID = MapIDToListIndex(LightList, Node->m_LightID);
		SceneNode SN;
		SN.m_SkeletonID = MapIDToListIndex(SkinList, Node->m_SkinID);
		SN.m_ModelID = MapIDToListIndex(MeshList, Node->m_MeshID);
		SN.m_ParentID = MapIDToListIndex(NodeList, Node->m_ParentID);
		P.BuildNodeAnimation(SN.m_Animation, NodeID);


		if (LightID != -1) { //Animation is not implemented for lights.
			LWMatrix4f Transform = P.GetNodeWorldTransform(NodeID)*Node->m_TransformMatrix.Inverse();
			LWVector3f pPos;
			LWVector3f Scale;
			LWQuaternionf Rot;
			Transform.Decompose(Scale, Rot, pPos, true);
			LWEGLTFLight *GL = P.GetLight(Node->m_LightID);
			LWVector4f Pos = Transform.m_Rows[3];
			LWVector3f Dir = (LWVector4f(0.0f, -1.0f, 0.0f, 0.0f)*Transform).xyz();
			Light L;
			//This is not an accurate implementation to gltf, the Spotlight and point lights use modified formula's from another project, so this attempts to match them as best as i can.
			if (GL->m_Type == LWEGLTFLight::POINT) {
				float InteriorRange = GL->m_Range*0.2f;
				float FalloffRange = GL->m_Range*0.8f;
				L = Light(LWVector3f(Pos.x, Pos.y, Pos.z), InteriorRange, FalloffRange, LWVector4f(GL->m_Color, 1.0f), GL->m_Intensity, LWVector4i(-1));
			} else if (GL->m_Type == LWEGLTFLight::DIRECTIONAL) {
				L = Light(Dir, LWVector4f(GL->m_Color, 1.0f), GL->m_Intensity, LWVector4i(-1));
			} else if (GL->m_Type == LWEGLTFLight::SPOT) {
				L = Light(LWVector3f(Pos.x, Pos.y, Pos.z), Dir, GL->m_Range, GL->m_OuterConeTheta, LWVector4f(GL->m_Color, 1.0f), GL->m_Intensity, LWVector4i(-1));
			} else {
				std::cout << "Error: encountered unsupported light type." << std::endl;
			}
			S->PushLight(L);
		}
		for (auto &&Iter : Node->m_Children) SN.m_Children.push_back(MapIDToListIndex(NodeList, Iter));
		S->PushNode(SN);
		for (auto &&Iter : Node->m_Children) ParseNode(Iter, S);
		return;
	};

	std::function<void(Skeleton &, LWEGLTFSkin*, uint32_t, LWMatrix4f *)> ParseSkinNode = [&P, &ParseSkinNode, &MapIDToListIndex](Skeleton &Skel, LWEGLTFSkin *Skin, uint32_t JntNodeID, LWMatrix4f *InvBindMatrices) {
		LWEGLTFNode *JNode = P.GetNode(JntNodeID);
		LWEGLTFNode *PNode = P.GetNode(JNode->m_ParentID);
		uint32_t JntIdx = MapIDToListIndex(Skin->m_JointList, JntNodeID);
		uint32_t ChildIdx = -1;
		uint32_t NextIdx = -1;
		if (JNode->m_Children.size()) ChildIdx = MapIDToListIndex(Skin->m_JointList, JNode->m_Children[0]);
		if (PNode) {
			uint32_t i = 0;
			for (; i < PNode->m_Children.size() && PNode->m_Children[i] != JntNodeID; i++) {}
			if ((i + 1) < PNode->m_Children.size()) NextIdx = MapIDToListIndex(Skin->m_JointList, PNode->m_Children[i + 1]);
		}

		Joint J = Joint(InvBindMatrices[JntIdx], JNode->m_TransformMatrix, ChildIdx, NextIdx);
		P.BuildNodeAnimation(J.m_Animation, JntNodeID);

		Skel.PushJoint(J);
		return;
	};

	LWEGLTFScene *GScene = P.BuildSceneOnlyList(P.GetDefaultSceneID(), NodeList, MeshList, SkinList, LightList, MaterialList, TextureList, ImageList);
	if (!GScene) {
		fmt::print("Error no default scene specified.\n");
		return nullptr;
	}
	fmt::print("Scene: {} Materials: {} Meshes: {} Textures: {} Images: {} Skins: {}\n", P.GetDefaultSceneID(), MaterialList.size(), MeshList.size(), TextureList.size(), ImageList.size(), SkinList.size());

	Scene *S = Allocator.Create<Scene>(Driver);
	for(uint32_t i=0;i<ImageList.size();i++){
		LWImage Image;
		LWTexture *Tex = nullptr;
		if (!P.LoadImage(Image, ImageList[i], Allocator)) {
			std::cout << "Error could not load image." << std::endl;
		} else {
			Tex = Driver->CreateTexture(0, Image, Allocator);
		}
		S->PushImage(Tex);
	}
	for (uint32_t i = 0; i < TextureList.size(); i++) {
		LWEGLTFTexture Tex = *P.GetTexture(TextureList[i]);
		Tex.m_ImageID = MapIDToListIndex(ImageList, Tex.m_ImageID);
		S->PushTexture(Tex);
	}
	for (uint32_t i = 0; i < MaterialList.size(); i++) {
		LWEGLTFMaterial *Mat = P.GetMaterial(MaterialList[i]);
		LWEGLTFMatMetallicRoughness &MatRough = Mat->m_MetallicRoughness;
		LWEGLTFMaterial M = *Mat;
		M.m_MetallicRoughness.m_BaseColorTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_MetallicRoughness.m_BaseColorTexture.m_TextureIndex);
		M.m_MetallicRoughness.m_MetallicRoughnessTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_MetallicRoughness.m_MetallicRoughnessTexture.m_TextureIndex);
		M.m_SpecularGlossy.m_DiffuseTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_SpecularGlossy.m_DiffuseTexture.m_TextureIndex);
		M.m_SpecularGlossy.m_SpecularGlossyTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_SpecularGlossy.m_SpecularGlossyTexture.m_TextureIndex);

		M.m_NormalMapTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_NormalMapTexture.m_TextureIndex);
		M.m_OcclussionTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_OcclussionTexture.m_TextureIndex);
		M.m_EmissiveTexture.m_TextureIndex = MapIDToListIndex(TextureList, M.m_EmissiveTexture.m_TextureIndex);
		S->PushMaterial(M);
	}

	for (uint32_t i = 0; i < SkinList.size(); i++) {
		LWEGLTFSkin *Skin = P.GetSkin(i);
		LWEGLTFNode *SkelNode = P.GetNode(Skin->m_SkeletonNode);
		uint32_t JointCnt = (uint32_t)Skin->m_JointList.size();
		if (JointCnt > ModelData::MaxBones) {
			std::cout << "Error skin joints exceed ModelData::MaxBones(" << ModelData::MaxBones << "): " << JointCnt << std::endl;
			continue;
		}		
		LWMatrix4f RootMatrix;
		if (SkelNode) RootMatrix = P.GetNodeWorldTransform(Skin->m_SkeletonNode);
		LWMatrix4f InvMatrices[ModelData::MaxBones];
		LWEGLTFAccessorView InvView;
		if (P.CreateAccessorView(InvView, Skin->m_InverseBindMatrices)) {
			InvView.ReadValues<float>(&InvMatrices[0].m_Rows[0].x, sizeof(LWMatrix4f), JointCnt);
		}
		Skeleton Skel((uint32_t)Skin->m_JointList.size(), RootMatrix);
		for (uint32_t i = 0; i < Skin->m_JointList.size(); i++) {
			ParseSkinNode(Skel, Skin, Skin->m_JointList[i], InvMatrices);
		}

		S->PushSkeleton(Skel);
	}

	for (uint32_t i = 0; i < MeshList.size(); i++) {
		LWEGLTFMesh *Mesh = P.GetMesh(MeshList[i]);
		uint32_t PrimCnt = std::min<uint32_t>(Model::MaxPrimitives, (uint32_t)Mesh->m_Primitives.size());
		Model M;
		for (uint32_t n = 0; n < PrimCnt; n++) {
			LWEGLTFPrimitive &GPrim = Mesh->m_Primitives[n];
			LWEGLTFAccessorView PosView;
			LWEGLTFAccessorView TangentView;
			LWEGLTFAccessorView TexView;
			LWEGLTFAccessorView NrmView;
			LWEGLTFAccessorView WeightsView;
			LWEGLTFAccessorView BIndicesView;
			LWEGLTFAccessorView IdxView;
			if (!P.CreateAccessorView(PosView, GPrim.FindAttributeAccessor(LWEGLTFAttribute::POSITION))) continue;;
			uint32_t VertCount = PosView.m_Count;
			uint32_t IndexCount = 0;
			LWVideoBuffer *VBuffer = nullptr;
			LWVideoBuffer *IBuffer = nullptr;
			Vertice *V = Allocator.AllocateA<Vertice>(VertCount);
			PosView.ReadValues<float>(&V[0].m_Position.x, sizeof(Vertice), VertCount);
			if (P.CreateAccessorView(TangentView, GPrim.FindAttributeAccessor(LWEGLTFAttribute::TANGENT))) {
				TangentView.ReadValues<float>(&V[0].m_Tangent.x, sizeof(Vertice), VertCount);
			}
			if (P.CreateAccessorView(TexView, GPrim.FindAttributeAccessor(LWEGLTFAttribute::TEXCOORD_0))) {
				TexView.ReadValues<float>(&V[0].m_TexCoord.x, sizeof(Vertice), VertCount);
			}
			if (P.CreateAccessorView(NrmView, GPrim.FindAttributeAccessor(LWEGLTFAttribute::NORMAL))) {
				NrmView.ReadValues<float>(&V[0].m_Normal.x, sizeof(Vertice), VertCount);
			}
			if (P.CreateAccessorView(WeightsView, GPrim.FindAttributeAccessor(LWEGLTFAttribute::WEIGHTS_0))) {
				WeightsView.ReadValues<float>(&V[0].m_BoneWeights.x, sizeof(Vertice), VertCount);
			}
			if (P.CreateAccessorView(BIndicesView, GPrim.FindAttributeAccessor(LWEGLTFAttribute::JOINTS_0))) {
				BIndicesView.ReadValues<int32_t>(&V[0].m_BoneIndices.x, sizeof(Vertice), VertCount);
			}
			if (P.CreateAccessorView(IdxView, GPrim.m_IndiceID)) {
				IndexCount = IdxView.m_Count;
				uint32_t *I = Allocator.AllocateA<uint32_t>(IndexCount);
				IdxView.ReadValues<uint32_t>(I, sizeof(uint32_t), IndexCount);
				IBuffer = Driver->CreateVideoBuffer<uint32_t>(LWVideoBuffer::Index32, LWVideoBuffer::Static, IndexCount, Allocator, I);
				LWAllocator::Destroy(I);
			}
			VBuffer = Driver->CreateVideoBuffer<Vertice>(LWVideoBuffer::Vertex, LWVideoBuffer::Static, VertCount, Allocator, V);
			LWVector3f AAMin;
			LWVector3f AAMax;
			if (VertCount) {
				LWVector3f P = LWVector3f(V[0].m_Position.x, V[0].m_Position.y, V[0].m_Position.z);
				AAMin = AAMax = P;
				for (uint32_t k = 1; k < VertCount; k++) {
					P = LWVector3f(V[k].m_Position.x, V[k].m_Position.y, V[k].m_Position.z);
					AAMin = AAMin.Min(P);
					AAMax = AAMax.Max(P);
				}
			}
			LWAllocator::Destroy(V);
			Primitive Prim;
			Prim.m_AABBMin = AAMin;
			Prim.m_AABBMax = AAMax;
			Prim.m_Geometry = Allocator.Create<LWMesh<Vertice>>(VBuffer, IBuffer, VertCount, IndexCount);
			Prim.m_MaterialID = MapIDToListIndex(MaterialList, GPrim.m_MaterialID);			
			M.PushPrimitive(Prim);
		}
		S->PushModel(M);
	}


	for (auto &&Iter : GScene->m_NodeList) ParseNode(Iter, S);
	return S;
}

Scene &Scene::Update(float deltaTime) {
	return *this;
};

uint32_t Scene::DrawScene(uint64_t lCurrentTime, const LWMatrix4f &SceneTransform, Renderer *R, lFrame &F, Camera &C, LWVideoDriver *Driver) {
	if (!m_StartTime) m_StartTime = lCurrentTime;
	float Elapsed = LWTimer::ToMilliSecond(lCurrentTime - m_StartTime) / 1000.0f;
	float T = Elapsed * LW_DEGTORAD*3.0f;
	uint32_t TotalVertices = 0;
	uint32_t MatOffset = F.m_MaterialCount;
	struct ShadowedLights {
		uint32_t m_Index;
		float m_DistanceSq;
	};

	auto SetMaterialTexture = [this](MaterialTextureInfo &Info, uint32_t TextureID, uint32_t Bit)->uint32_t {
		if (TextureID == -1) return 0;
		LWEGLTFTexture &Tex = GetTexture(TextureID);
		Info.m_Texture = GetImage(Tex.m_ImageID);
		Info.m_TextureFlag = Tex.m_SamplerFlag;
		return Info.m_Texture ? Bit : 0;
	};

	auto InsertShadowLightIndex = [](float DistanceSq, uint32_t FrameLightIndex, ShadowedLights *ShadowLightList, uint32_t &ShadowCnt)->void {
		uint32_t i = 0;
		for (; i < ShadowCnt; i++) {
			if (ShadowLightList[i].m_DistanceSq > DistanceSq) break;
		}
		if (i >= ListData::MaxShadows) return;
		if (ShadowCnt < ListData::MaxShadows) {
			std::copy_backward(ShadowLightList + i, ShadowLightList + ShadowCnt, ShadowLightList + (ShadowCnt + 1));
			ShadowCnt++;
		} else {
			std::copy_backward(ShadowLightList + i, ShadowLightList + (ShadowCnt - 1), ShadowLightList + ShadowCnt);
		}
		ShadowLightList[i] = { FrameLightIndex, DistanceSq };
		return;
	};

	auto EvaluateLight = [this, &InsertShadowLightIndex](Light &L, uint32_t FrameLightIndex, Camera &MainView, ShadowedLights *ShadowLightList, uint32_t &ShadowCnt)->bool {
		LWVector3f Pos = LWVector3f(L.m_Position.x, L.m_Position.y, L.m_Position.z);
		if (L.m_Position.w == 0.0f) {
			InsertShadowLightIndex(0.0f, FrameLightIndex, ShadowLightList, ShadowCnt);
		} else if (L.m_Position.w == 1.0f) {
			float Radi = (L.m_Direction.x + L.m_Direction.y);
			if (!MainView.SphereInFrustrum(Pos, Radi)) return false;
			InsertShadowLightIndex(Pos.DistanceSquared(MainView.GetPosition()), FrameLightIndex, ShadowLightList, ShadowCnt);
		} else if (L.m_Position.w > 1.0f) {
			float Theta = L.m_Position.w - 1.0f;
			LWVector3f Dir = LWVector3f(L.m_Direction.x, L.m_Direction.y, L.m_Direction.z);
			if (!MainView.ConeInFrustrum(Pos, Dir, L.m_Direction.w, Theta)) return false;
			InsertShadowLightIndex(Pos.DistanceSquared(MainView.GetPosition()), FrameLightIndex, ShadowLightList, ShadowCnt);
		}
		return true;
	};

	for (uint32_t i = 0; i < m_MaterialCount; i++) {
		LWEGLTFMaterial &Mat = m_MaterialList[i];
		LWEGLTFMatMetallicRoughness &MatMetRough = Mat.m_MetallicRoughness;
		LWEGLTFMatSpecularGlossyness &MatSpecularGloss = Mat.m_SpecularGlossy;
		uint32_t MatType = Mat.GetType();
		MaterialData MatData;
		MaterialInfo MatInfo;
		MatInfo.m_Type = MatType;
		MatData.m_HasTextureFlag = 0;
		if (MatType == LWEGLTFMaterial::MetallicRoughness) {
			MatData.m_MaterialColorA = MatMetRough.m_BaseColorFactor;
			MatData.m_MaterialColorB = LWVector4f(MatMetRough.m_MetallicFactor, MatMetRough.m_RoughnessFactor, 0.0f, 0.0f);
			MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_MaterialTextureA, MatMetRough.m_BaseColorTexture.m_TextureIndex, MaterialInfo::MaterialTextureABit);
			MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_MaterialTextureB, MatMetRough.m_MetallicRoughnessTexture.m_TextureIndex, MaterialInfo::MaterialTextureBBit);
		} else if (MatType == LWEGLTFMaterial::SpecularGlossyness) {
			MatData.m_MaterialColorA = MatSpecularGloss.m_DiffuseFactor;
			MatData.m_MaterialColorB = LWVector4f(MatSpecularGloss.m_SpecularFactor, MatSpecularGloss.m_Glossiness);
			MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_MaterialTextureA, MatSpecularGloss.m_DiffuseTexture.m_TextureIndex, MaterialInfo::MaterialTextureABit);
			MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_MaterialTextureB, MatSpecularGloss.m_SpecularGlossyTexture.m_TextureIndex, MaterialInfo::MaterialTextureBBit);
		} else if (MatType == LWEGLTFMaterial::Unlit) {
			MatData.m_MaterialColorA = MatMetRough.m_BaseColorFactor;
			MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_MaterialTextureA, MatMetRough.m_BaseColorTexture.m_TextureIndex, MaterialInfo::MaterialTextureABit);
		}
		MatData.m_EmissiveFactor = Mat.m_EmissiveFactor;
		MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_NormalTexture, Mat.m_NormalMapTexture.m_TextureIndex, MaterialInfo::NormalTextureBit);
		MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_OcclussionTexture, Mat.m_OcclussionTexture.m_TextureIndex, MaterialInfo::OcclussionTextureBit);
		MatData.m_HasTextureFlag |= SetMaterialTexture(MatInfo.m_EmissiveTexture, Mat.m_EmissiveTexture.m_TextureIndex, MaterialInfo::EmissiveTextureBit);
		
		MatInfo.m_Opaque = (Mat.m_Flag&LWEGLTFMaterial::AlphaBits)!=LWEGLTFMaterial::AlphaBlend;
		F.WriteMaterial(MatData, MatInfo, Driver);
	}
	uint32_t ListCnt = 0;
	uint32_t ShadowCnt = 0;
	uint32_t ShadowViewCnt = 0;
	ShadowedLights ShadowList[ListData::MaxShadows];
	Camera ShadowViews[ListData::MaxShadows];
	Camera *CameraList[lFrame::MaxLists];
	CameraList[ListCnt++] = &C;
	F.MakeList(*CameraList[0]);
	for (uint32_t i = 0; i < m_LightCount; i++) {
		Light &L = m_LightList[i];
		if (!EvaluateLight(L, F.m_LightCount, C, ShadowList, ShadowCnt)) continue;
		F.WriteLight(L, Driver);
		LWVector3f Dir = LWVector3f(L.m_Direction.x, L.m_Direction.y, L.m_Direction.z);
		LWVector3f Pos = LWVector3f(L.m_Position.x, L.m_Position.y, L.m_Position.z);
		LWVector4f Clr = LWVector4f(L.m_Color.x, L.m_Color.y, L.m_Color.z, 1.0f);
		if (L.m_Position.w == 0.0f) {
			R->WriteDebugPoint(F, -Dir * 1000.0f, 30.0f, Clr);
		} else if (L.m_Position.w == 1.0f) {
			R->WriteDebugPoint(F, Pos, 1.0f, Clr);
		} else if (L.m_Position.w > 1.0f) {
			R->WriteDebugCone(F, Pos, Dir, L.m_Position.w - 1.0f, 3.0f, Clr);
		}
	}
	//Setup shadow casters.
	for (uint32_t i = 0; i < ShadowCnt && ShadowViewCnt<ListData::MaxShadows; i++) {
		Light *L = (Light*)F.m_LightBuffer+ ShadowList[i].m_Index;
		if (L->m_Position.w == 0.0f) {
			if(ShadowViewCnt+3>ListData::MaxShadows) continue;
			L->m_ShadowIdx = LWVector4i(ShadowViewCnt, ShadowViewCnt + 1, ShadowViewCnt + 2, -1);
			LWVector3f Dir = L->m_Direction.xyz();
			C.BuildCascadeCameraViews(Dir, ShadowViews + ShadowViewCnt, 3, m_AABBMin, m_AABBMax, 0);
			CameraList[ListCnt++] = &ShadowViews[ShadowViewCnt];
			CameraList[ListCnt++] = &ShadowViews[ShadowViewCnt + 1];
			CameraList[ListCnt++] = &ShadowViews[ShadowViewCnt + 2];
			ShadowViewCnt += 3;
		} else {
			L->m_ShadowIdx = LWVector4i(ShadowViewCnt, -1, -1, -1);
			L->MakeCamera(ShadowViews[ShadowViewCnt]);
			CameraList[ListCnt++] = &ShadowViews[ShadowViewCnt];
			ShadowViewCnt++;
		}
	}

	for (uint32_t i = 1; i < ListCnt; i++) F.MakeList(*CameraList[i]);

	bool First = true;
	std::function<uint32_t(uint32_t, const LWMatrix4f &)> DrawNode = [&First, &Elapsed, &CameraList, &ListCnt, &MatOffset, &F, &Driver, this, &DrawNode](uint32_t NodeID, const LWMatrix4f &ParentMatrix)->uint32_t {
		SceneNode &Node = m_NodeList[NodeID];
		LWMatrix4f Transform = Node.m_Animation.GetFrame(Elapsed, true) * ParentMatrix;
		uint32_t Vertices = 0;
		if (Node.m_ModelID != -1) {
			Model &M = m_ModelList[Node.m_ModelID];
			LWVector3f AAMin = M.GetAABBMin();
			LWVector3f AAMax = M.GetAABBMax();
			TransformAABB(AAMin, AAMax, Transform, First);
			First = false;
			LWVector3f hSize = (AAMax - AAMin)*0.5f;
			float Radi = std::max<float>(std::max<float>(hSize.x, hSize.y), hSize.z)*1.5f;
			LWVector3f Ctr = AAMin + hSize;
			uint32_t ListBits = Camera::GetListForSphereInCameras(CameraList, ListCnt, Ctr, Radi);
			if (ListBits) {
				ModelData MdlData;
				Instance Inst;
				MdlData.m_TransformMatrix = Transform;
				Inst.m_HasSkin = Node.m_SkeletonID != -1;
				if (Inst.m_HasSkin) {
					Skeleton &Skel = m_SkeletonList[Node.m_SkeletonID];
					Skel.BuildFrame(Elapsed, true, MdlData);
				}
				Inst.m_PrimitiveCount = M.GetPrimitiveCount();
				for (uint32_t n = 0; n < Inst.m_PrimitiveCount; n++) {
					Primitive &Prim = M.GetPrimitive(n);
					InstancePrimitive &IPrim = Inst.m_PrimitiveList[n];
					IPrim.m_MaterialID = Prim.m_MaterialID + MatOffset;
					IPrim.m_Mesh = Prim.m_Geometry;
					Vertices += Prim.m_Geometry->GetRenderCount();
				}
				F.WriteInstance(Inst, MdlData, ListBits, Driver);
			}
		}
		for (auto &&Iter : Node.m_Children) Vertices += DrawNode(Iter, Transform);
		return Vertices;
	};
	for (auto &&Iter : m_RootNodes) TotalVertices += DrawNode(Iter, SceneTransform);
	return TotalVertices;
}

uint32_t Scene::DrawDebugSkeleton(Renderer *R, lFrame &F, Skeleton &Skel, const ModelData &SourceMdl, LWVideoDriver *Driver) {
	std::function<void(uint32_t, uint32_t)> DrawJoint = [&R, &F, &Skel, &SourceMdl, &Driver, &DrawJoint](uint32_t JointID, uint32_t ParentID) {
		Joint &J = Skel.GetJoint(JointID);
		LWVector3f Pos = Skel.TransformPoint(LWVector3f(), SourceMdl, JointID);//SourceMdl.m_TransformMatrix*(LWVector3f()*BindMat);// *SourceMdl.m_BoneMatrixs[JointID];
		if (ParentID != -1) {
			R->WriteDebugLine(F, Pos, Skel.TransformPoint(LWVector3f(), SourceMdl, ParentID), 0.1f, LWVector4f(1.0f, 0.0f, 0.0f, 1.0f));
		}
		if (J.m_ChildIdx != -1) DrawJoint(J.m_ChildIdx, JointID);
		if (J.m_NextIdx != -1) DrawJoint(J.m_NextIdx, ParentID);
		R->WriteDebugPoint(F, Pos, 0.3f, LWVector4f(0.0f, 1.0f, 0.0f, 1.0f));
	};
	DrawJoint(0, -1);
	return 0;
}
	
bool Scene::PushModel(Model &M) {
	if (m_ModelCount >= MaxModels) return false;
	m_ModelList[m_ModelCount] = M;
	m_ModelCount++;
	return true;
}

bool Scene::PushSkeleton(Skeleton &Skel) {
	if (m_SkeletonCount >= MaxSkeletons) return false;
	m_SkeletonList[m_SkeletonCount] = std::move(Skel);
	m_SkeletonCount++;
	return true;
}

bool Scene::PushLight(const Light &L) {
	if (m_LightCount >= MaxLights) return false;
	m_LightList[m_LightCount] = L;
	m_LightCount++;
	return true;
}

bool Scene::PushMaterial(const LWEGLTFMaterial &Mat) {
	if (m_MaterialCount >= MaxMaterials) return false;
	m_MaterialList[m_MaterialCount] = Mat;
	m_MaterialCount++;
	return true;
}

bool Scene::PushNode(SceneNode &N) {
	if (m_NodeCount >= MaxNodes) return false;
	if (N.m_ParentID == -1) m_RootNodes.push_back(m_NodeCount);
	m_NodeList[m_NodeCount] = std::move(N);
	m_NodeCount++;
	return true;
}

void Scene::TransformAABB(LWVector3f &AAMin, LWVector3f &AAMax, const LWMatrix4f &Transform, bool RebuildSceneAABB) {
	LWVector4f Min = LWVector4f(AAMin, 1.0f);
	LWVector4f Max = LWVector4f(AAMax, 1.0f);
	LWVector4f hSize = (Max - Min)*0.5f;
	LWVector4f Ctr = Transform * (Min + hSize);
	LWVector4f xDir = Transform * LWVector4f(hSize.x, 0.0f, 0.0f, 0.0f);
	LWVector4f yDir = Transform * LWVector4f(0.0f, hSize.y, 0.0f, 0.0f);
	LWVector4f zDir = Transform * LWVector4f(0.0f, 0.0f, hSize.z, 0.0f);
	LWVector4f PntList[8] = { Ctr + xDir + yDir + zDir,
							 Ctr + xDir + yDir - zDir,
							 Ctr + xDir - yDir + zDir,
							 Ctr + xDir - yDir - zDir,
							 Ctr - xDir + yDir + zDir,
							 Ctr - xDir + yDir - zDir,
							 Ctr - xDir - yDir + zDir,
							 Ctr - xDir - yDir - zDir };

	Min = Max = PntList[0];
	for (uint32_t i = 1; i < 8; i++) {
		Min = Min.Min(PntList[i]);
		Max = Max.Max(PntList[i]);
	}
	AAMin = LWVector3f(Min.x, Min.y, Min.z);
	AAMax = LWVector3f(Max.x, Max.y, Max.z);
	if (RebuildSceneAABB) {
		m_AABBMin = AAMin;
		m_AABBMax = AAMax;
	} else {
		m_AABBMin = m_AABBMin.Min(AAMin);
		m_AABBMax = m_AABBMax.Max(AAMax);
	}
	return;
}

bool Scene::PushTexture(LWEGLTFTexture &Tex) {
	if (m_TextureCount >= MaxTextures) return false;
	m_TextureList[m_TextureCount] = Tex;
	m_TextureCount++;
	return true;
}

bool Scene::PushImage(LWTexture *Image) {
	if (m_ImageCount >= MaxImages) return false;
	m_ImageList[m_ImageCount] = Image;
	m_ImageCount++;
	return true;
}

Model &Scene::GetModel(uint32_t i) {
	return m_ModelList[i];
}

LWEGLTFMaterial &Scene::GetMaterial(uint32_t i) {
	return m_MaterialList[i];
}

SceneNode &Scene::GetNode(uint32_t i) {
	return m_NodeList[i];
}

Light &Scene::GetLight(uint32_t i) {
	return m_LightList[i];
}

Skeleton &Scene::GetSkeleton(uint32_t i) {
	return m_SkeletonList[i];
}

LWEGLTFTexture &Scene::GetTexture(uint32_t i) {
	return m_TextureList[i];
}

LWTexture *Scene::GetImage(uint32_t i) {
	if (i >= m_ImageCount) return nullptr;
	return m_ImageList[i];
}

uint32_t Scene::GetModelCount(void) const {
	return m_ModelCount;
}

uint32_t Scene::GetSkeletonCount(void) const {
	return m_SkeletonCount;
}

uint32_t Scene::GetMaterialCount(void) const {
	return m_MaterialCount;
}

uint32_t Scene::GetNodeCount(void) const {
	return m_NodeCount;
}

uint32_t Scene::GetTextureCount(void) const {
	return m_TextureCount;
}

uint32_t Scene::GetLightCount(void) const {
	return m_LightCount;
}

uint32_t Scene::GetImageCount(void) const {
	return m_ImageCount;
}


LWVector3f Scene::GetAAMin(void) const {
	return m_AABBMin;
}

LWVector3f Scene::GetAAMax(void) const {
	return m_AABBMax;
}


Scene::Scene(LWVideoDriver *Driver) : m_Driver(Driver) {}

Scene::~Scene() {
	for (uint32_t i = 0; i < m_ModelCount; i++) {
		Model &M = m_ModelList[i];
		for (uint32_t n = 0; n < M.GetPrimitiveCount(); n++) {
			Primitive &P = M.GetPrimitive(n);
			P.m_Geometry->Destroy(m_Driver);
		}
	}
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		if(m_ImageList[i]) m_Driver->DestroyTexture(m_ImageList[i]);
	}
}