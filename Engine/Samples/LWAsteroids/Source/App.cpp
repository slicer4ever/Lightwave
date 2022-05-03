#include "App.h"
#include <LWPlatform/LWWindow.h>
#include <LWPlatform/LWPlatform.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWPlatform/LWVideoMode.h>
#include <LWCore/LWTimer.h>
#include <LWEUI/LWEUILabel.h>
#include <LWEGLTFParser.h>
#include <LWERenderPass.h>
#include <LWERenderPasses/LWEShadowMapPass.h>
#include <LWERenderPasses/LWEUIPass.h>
#include <LWCore/LWLogger.h>

//PrimitiveMaterial:
PrimitiveMaterial::PrimitiveMaterial(LWEGLTFParser &P, LWEGLTFMaterial *Mat, const LWSMatrix4f &Transform, LWERenderer *R, LWAllocator &Allocator) {
	m_Data.m_Transform = Transform;
	uint32_t MatType = Mat->GetType();
	m_Data = LWEGeometryModelData(Transform);
	m_Data.m_MaterialData[LWEPBREmissiveFactorID] = Mat->m_EmissiveFactor;
	
	auto LoadTexture = [this, &R, &P, &Allocator](LWEGLTFTextureInfo &TexInfo, uint32_t ResourceName, uint32_t TextureID, bool bIsSRGB = false) {
		LWEGLTFTexture *Tex = P.GetTexture(TexInfo.m_TextureIndex);
		if (!Tex) return;
		LWImage *Img = Allocator.Create<LWImage>();
		if (!P.LoadImage(*Img, Tex->m_ImageID, Allocator)) return;
		Img->SetSRGBA(bIsSRGB);
		uint32_t ID = R->PushPendingResource(LWERenderPendingResource(0, 0, LWERenderPendingTexture(Img, Tex->m_SamplerFlag)));
		if (ID == 0) {
			LWAllocator::Destroy(Img);
			return;
		}
		m_Data.m_SubTextures[TextureID] = LWVector4f(0.0f, 0.0f, 1.0f, 1.0f);
		m_Data.m_HasTextureFlag |= m_Material.PushTexture(LWERenderMaterialTexture(ResourceName, ID, Tex->m_SamplerFlag), TextureID);
	};

	if (MatType == LWEGLTFMaterial::MetallicRoughness) {
		m_Material = LWERenderMaterial("PBRMR", nullptr, 0);
		m_Data.m_MaterialData[LWEPBRMRBaseColorID] = Mat->m_MetallicRoughness.m_BaseColorFactor;
		m_Data.m_MaterialData[LWEPBRMRFactorID] = LWVector4f(Mat->m_MetallicRoughness.m_MetallicFactor, Mat->m_MetallicRoughness.m_RoughnessFactor, 0.0f, 0.0f);
		LoadTexture(Mat->m_MetallicRoughness.m_BaseColorTexture, LWEPBRMRAlbedoTexNameHash, LWEPBRMRAlbedoTexID, true);
		LoadTexture(Mat->m_MetallicRoughness.m_MetallicRoughnessTexture, LWEPBRMRTexNameHash, LWEPBRMRFactorTexID);
	}else if (MatType == LWEGLTFMaterial::Unlit) {
		m_Material = LWERenderMaterial("PBRUnlit", nullptr, 0);
		m_Data.m_MaterialData[LWEPBRUnlitColorFactorID] = Mat->m_MetallicRoughness.m_BaseColorFactor;
		LoadTexture(Mat->m_MetallicRoughness.m_BaseColorTexture, LWEPBRUnlitColorTexNameHash, LWEPBRUnlitColorTexID, true);
	} else if (MatType == LWEGLTFMaterial::SpecularGlossyness) {
		m_Material = LWERenderMaterial("PBRSG", nullptr, 0);
		m_Data.m_MaterialData[LWEPBRSGDiffuseID] = Mat->m_SpecularGlossy.m_DiffuseFactor;
		m_Data.m_MaterialData[LWEPBRSGSpecularID] = LWVector4f(Mat->m_SpecularGlossy.m_SpecularFactor, Mat->m_SpecularGlossy.m_Glossiness);
		LoadTexture(Mat->m_SpecularGlossy.m_DiffuseTexture, LWEPBRSGDiffuseTexNameHash, LWEPBRSGDiffuseTexID, true);
		LoadTexture(Mat->m_SpecularGlossy.m_SpecularGlossyTexture, LWEPBRSGSpecularTexNameHash, LWEPBRSGSpecularTexID, true);
	}
	LoadTexture(Mat->m_NormalMapTexture, LWEPBRNormalTexNameHash, LWEPBRNormalTexID);
	LoadTexture(Mat->m_OcclusionTexture, LWEPBROcclusionTexNameHash, LWEPBROcclusionTexID);
	LoadTexture(Mat->m_EmissiveTexture, LWEPBREmissiveTexNameHash, LWEPBREmissiveTexID);

}

//App:
void App::UpdateJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime) {
	LWERenderFrame *F = m_Renderer->BeginFrame();
	if (!F) return;
	float dTime = m_FrameMetric.MakeDeltaTime(LWTimer::GetCurrent());
	if (m_DebugLabel->isVisible()) m_DebugLabel->SetText(LWUTF8I::Fmt<128>("Frame: {}\nInput: {}\nRender: {} - {}fps\n{}", m_FrameMetric, m_InputMetric, m_RenderMetric, m_RenderMetric.MakeFPS(), m_SwapInterval?"vsync on":""));

	m_Camera.SetAspect(m_Window->GetAspect()).BuildFrustrum();
	F->InitializeBucket(m_Camera, 0);
	//F->PushLight(LWEShaderLightData(m_LightPos, m_LightDir, LW_PI_8, 80.0f), true);
	F->PushLight(LWEShaderLightData(0.05f, LWVector4f(1.0f)));
	F->PushLight(LWEShaderLightData(m_LightDir), true);
	m_Renderer->FindNamedPass<LWEShadowMapPass>("ShadowMapPass")->InitializeShadowCastorBuckets(*F);

	F->WriteDebugLine(0x1, LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f), LWSVector4f(2.0f, 0.0f, 0.0f, 1.0f), 0.2f, LWVector4f(1.0f, 0.0f, 0.0f, 0.5f));
	F->WriteDebugPoint(0x1, LWSVector4f(1.0f, 1.0f, 1.0f, 1.0f), 0.25f, LWVector4f(1.0f, 1.0f, 0.0f, 0.5f));
	F->WriteDebugLine(0x1, LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f), LWSVector4f(0.0f, 2.0f, 0.0f, 1.0f), 0.2f, LWVector4f(0.0f, 1.0f, 0.0f, 0.5f));

	F->WriteDebugPoint(0x1, LWSVector4f(-1.0f, 1.0f, 1.0f, 1.0f), 0.25f, LWVector4f(1.0f, 1.0f, 1.0f, 1.0f));
	F->WriteDebugLine(0x1, LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f), LWSVector4f(0.0f, 0.0f, 2.0f, 1.0f), 0.2f, LWVector4f(0.0f, 0.0f, 1.0f, 1.0f));
	
	F->WriteDebugPoint(0x1, LWSVector4f(1.0f, 1.0f, -1.0f, 1.0f), 0.25f, LWVector4f(1.0f, 0.0f, 1.0f, 0.5f));
	
	for(auto &&Mdl : m_ModelList) {
		LWSMatrix4f BoneMats[LWEMaxBones];
		uint32_t BoneCnt = Mdl->m_Mesh.BuildBindTransforms(BoneMats);
		uint32_t BoneID = F->NextBoneID(BoneCnt);
		if (BoneCnt) Mdl->m_Mesh.BuildRenderMatrixs(BoneMats, F->GetBoneDataAt(BoneID));
		uint32_t PrimCnt = Mdl->m_Mesh.GetPrimitiveCount();
		for (uint32_t p = 0; p < PrimCnt; p++) {
			auto &Prim = Mdl->m_Mesh.GetPrimitive(p);
			auto &PrimMat = Mdl->m_MaterialList[p];
			LWSVector4f MinBounds, MaxBounds;
			Mdl->m_Mesh.TransformBounds(PrimMat.m_Data.m_Transform, MinBounds, MaxBounds);

			PrimMat.m_Data.m_BoneID = BoneID;
			F->PushModel(LWEGeometryModel(LWEGeometryModelBlock(Mdl->m_Mesh.GetBlockGeometry(), Mdl->m_Mesh.GetID(), Prim.m_Offset, Prim.m_Count), PrimMat.m_Material), PrimMat.m_Data, 0x3, MinBounds, MaxBounds);
		}
	}

	LWEUIFrame &UIFrame = m_Renderer->FindNamedPass<LWEUIPass>("UIPass")->GetUIFrame(*F);
	m_UIManager->Draw(UIFrame, lCurrentTime);
	m_Renderer->EndFrame();
	m_FrameMetric.RecordTime();
	return;
}

void App::InputJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime) {
	float dTime = m_InputMetric.MakeDeltaTime(lCurrentTime);
	LWKeyboard *KB = m_Window->GetKeyboardDevice();
	LWMouse *Ms = m_Window->GetMouseDevice();
	LWVector2f WndCenter = m_Window->GetSizef() * 0.5f;
	m_Window->Update(lCurrentTime);
	m_JobQueue.SetFinished(KB->ButtonPressed(LWKey::Esc) || m_Window->isFinished());
	m_UIManager->Update(lCurrentTime);
	LWVector2f MoveDir = Ms->GetPositionf() - WndCenter;
	bool bMovable = MoveDir.LengthSquared() >= 1.0f;
	if (m_Camera.isControlToggled()) {
		if(!bMovable) m_Camera.SetCameraControlled(true);
	}
	if (m_Camera.isCameraControlled()) {
		float Speed = 45.0f * dTime;
		LWSVector4f Dir = LWSVector4f();
		LWSVector4f Fwrd = m_Camera.GetFlatDirection();
		LWSVector4f Up = m_Camera.GetUp();
		LWSVector4f Rght = Fwrd.Cross3(Up).Normalize3();
		if (bMovable) m_Camera.ProcessDirectionInputFirst(MoveDir, dTime, dTime, LWVector4f(-LW_PI, LW_PI, -LW_PI_2 * 0.9f, LW_PI_2 * 0.9f), true);
		if (KB->ButtonDown(LWKey::W)) Dir += Fwrd;
		else if (KB->ButtonDown(LWKey::S)) Dir -= Fwrd;
		if (KB->ButtonDown(LWKey::A)) Dir -= Rght;
		else if (KB->ButtonDown(LWKey::D)) Dir += Rght;
		if (KB->ButtonDown(LWKey::Space)) Dir += Up;
		else if (KB->ButtonDown(LWKey::LShift)) Dir -= Up;
		m_Camera.SetPosition(m_Camera.GetPosition() + Dir.Normalize3() * Speed);
	}
	if (KB->ButtonDown(LWKey::Z)) {
		m_LightPos = m_Camera.GetPosition();
		m_LightDir = m_Camera.GetDirection();
	}
	if (KB->ButtonPressed(LWKey::F3)) m_DebugLabel->SetVisible(!m_DebugLabel->isVisible());
	if (KB->ButtonPressed(LWKey::F2)) m_SwapInterval = !m_SwapInterval;
	if (m_Camera.isControlToggled() || m_Camera.isCameraControlled()) {
		if(bMovable) m_Window->SetMousePosition(WndCenter.CastTo<int32_t>());
	}
	m_Window->SetMouseVisible(!m_Camera.isCameraControlled());
	if (KB->ButtonPressed(LWKey::LAlt)) m_Camera.ToggleCameraControl();
	m_InputMetric.RecordTime();

	return;
}

void App::RenderJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime) {
	float dTime = m_RenderMetric.MakeDeltaTime(lCurrentTime);
	m_Renderer->Render(dTime, m_SwapInterval, lCurrentTime, LWTimer::ToHighResolution(4ull));
	m_RenderMetric.RecordTime();
	return;
}

void App::Run(void) {
	m_JobQueue.Start();
	LWEJobQueue::RunThread(&m_JobQueue.GetMainThread(), &m_JobQueue);
	m_JobQueue.WaitForAllJoined();
	m_JobQueue.OutputThreadTimings();
	m_JobQueue.OutputJobTimings();
	return;
}

bool App::LoadAssets(const LWUTF8I &Path) {
	LWEXML X;
	if(!LWLogCriticalIf<256>(LWEXML::LoadFile(X, m_Allocator, Path, true), "Error: Could not open assets file: '{}'", Path)) return false;

	std::unordered_map<uint32_t, LWEPassXMLCreateFunc> PassCreateMap;
	LWERenderer::GenerateDefaultXMLPasses(PassCreateMap);

	m_AssetManager = m_Allocator.Create<LWEAssetManager>(m_VDriver, nullptr, m_Allocator);
	m_UIManager = m_Allocator.Create<LWEUIManager>(m_Window, LWVideoMode::GetActiveMode().GetDPI().x, m_Allocator, nullptr, m_AssetManager);
	m_Renderer = m_Allocator.Create<LWERenderer>(m_VDriver, m_AssetManager, m_Allocator);
	X.PushParser("AssetManager", &LWEAssetManager::XMLParser, m_AssetManager);
	X.PushParser("UIManager", &LWEUIManager::XMLParser, m_UIManager);
	X.PushMethodParser("Renderer", &LWERenderer::ParseXML, m_Renderer, &PassCreateMap);
	X.Process();
	m_DebugLabel = m_UIManager->GetNamedUI<LWEUILabel>("DebugLbl");
	return m_Renderer->FinalizePasses();
}

bool App::LoadGLTF(const LWUTF8I &Path) {
	LWEGLTFParser P;
	if(!LWLogCriticalIf<256>(LWEGLTFParser::LoadFile(P, Path, m_Allocator), "Error: Could not load gltf file '{}'", Path)) return false;

	std::function<void(LWEGLTFParser &, LWEGLTFNode*)> ParseNode = [this, &ParseNode](LWEGLTFParser &P, LWEGLTFNode *N) {
		LWEGLTFMesh *GMesh = P.GetMesh(N->m_MeshID);
		LWEGLTFSkin *GSkin = P.GetSkin(N->m_SkinID);
		//LWSMatrix4f Scalar = LWSMatrix4f(50.0f, 50.0f, 50.0f, 1.0f);
		LWSMatrix4f Scalar = LWSMatrix4f();
		if (GMesh) {
			Model *Mdl = m_Allocator.Create<Model>();
			LWEMesh &Msh = Mdl->m_Mesh;
			Msh.MakeGLTFMesh(P, N, GMesh, m_Allocator);
			if (GSkin) Msh.MakeGLTFSkin(P, N, GSkin, m_Allocator);
			Msh.BuildAABB(LWSMatrix4f(), nullptr);
			uint32_t GeometryBuffer = (Msh.GetVerticePositionTypeSize() == 16 ? LWUTF8I("StaticGeom") : LWUTF8I("SkeletonGeom")).Hash();
			Msh.UploadData(m_Renderer, GeometryBuffer, m_Allocator);
			for (auto &&Prim : GMesh->m_Primitives) {
				LWEGLTFMaterial *GMaterial = P.GetMaterial(Prim.m_MaterialID);
				Mdl->m_MaterialList.push_back(PrimitiveMaterial(P, GMaterial, Scalar*P.GetNodeWorldTransform(N->m_NodeID), m_Renderer, m_Allocator));
			}
			m_ModelList.push_back(Mdl);
		}
		for (auto &&Iter : N->m_Children) ParseNode(P, P.GetNode(Iter));
	};


	LWEGLTFScene *Scene = P.GetScene(P.GetDefaultSceneID());
	if(!LWLogCriticalIf(Scene, "Error: GLTF contains no valid scene.")) return false;
	for (auto &&Iter : Scene->m_NodeList) ParseNode(P, P.GetNode(Iter));

	return true;
}

App::App(LWAllocator &Allocator) : m_Allocator(Allocator) {
	const char8_t AppName[] = "LWAsteroids";
	const char8_t *DriverNames[] = LWVIDEODRIVER_NAMES;
	const char8_t *ArchNames[] = LWARCH_NAMES;
	const char8_t *PlatformNames[] = LWPLATFORM_NAMES;
	LWVideoMode CurrMode = LWVideoMode::GetActiveMode();
	LWVector2i TargetSize = LWVector2i(1280, 720);

	m_Window = m_Allocator.Create<LWWindow>(AppName, AppName, m_Allocator, LWWindow::WindowedMode | LWWindow::KeyboardDevice | LWWindow::MouseDevice, CurrMode.GetSize() / 2 - TargetSize / 2, TargetSize);
	if(!LWLogCriticalIf(!m_Window->DidError(), "Error creating window.")) {
		m_JobQueue.SetFinished(true);
		return;
	}
	uint32_t TargetDriver = LWVideoDriver::DirectX11_1;
	//TargetDriver = LWVideoDriver::OpenGL4_5;
	//TargetDriver |= LWVideoDriver::DebugLayer;
	m_VDriver = LWVideoDriver::MakeVideoDriver(m_Window, TargetDriver);
	if(!LWLogCriticalIf(m_VDriver, "Error creating video driver.")) {
		m_JobQueue.SetFinished(true);
		return;
	}
	m_Window->SetTitle(LWUTF8I::Fmt<128>("{} | {} | {} | {}", AppName, DriverNames[m_VDriver->GetDriverID()], ArchNames[LWARCH_ID], PlatformNames[LWPLATFORM_ID]));

	if (!LoadAssets("App:UI.xml")) {
		m_JobQueue.SetFinished(true);
		return;
	}
	if(!LoadGLTF("C:/Users/Tim/Downloads/glTF-Sample-Models-master/2.0/Buggy/glTF/Buggy.gltf")){
	//if (!LoadGLTF("C:/Users/Tim/Downloads/glTF-Sample-Models-master/2.0/WaterBottle/glTF/WaterBottle.gltf")) {
	//if(!LoadGLTF("C:/Users/Tim/Downloads/glTF-Sample-Models-master/2.0/2CylinderEngine/glTF/2CylinderEngine.gltf")){
	//if(!LoadGLTF("C:/Users/Tim/Downloads/glTF-Sample-Models-master/2.0/WaterBottle/glTF-pbrSpecularGlossiness/WaterBottle.gltf")){
	//if(!LoadGLTF("C:/Users/Tim/Downloads/glTF-Sample-Models-master/2.0/RiggedFigure/glTF/RiggedFigure.gltf")){
		m_JobQueue.SetFinished(true);
		return;
	}
	m_JobQueue.PushJob(LWEJob::MakeMethod(&App::InputJob, this, nullptr, 0, 0, 0, 0, 0, 0, 0x1));
	m_JobQueue.PushJob(LWEJob::MakeMethod(&App::RenderJob, this, nullptr, 0, 0, 0, 0, 0, 0, 0x1));
	m_JobQueue.PushJob(LWEJob::MakeMethod(&App::UpdateJob, this, nullptr, 0, 0, 0, 0, 0, 0, ~0x1));
}

App::~App() {
	for (auto &&Mdl : m_ModelList) LWAllocator::Destroy(Mdl);
	LWAllocator::Destroy(m_UIManager);
	LWAllocator::Destroy(m_AssetManager);
	LWAllocator::Destroy(m_Renderer);
	LWVideoDriver::DestroyVideoDriver(m_VDriver);
	LWAllocator::Destroy(m_Window);
}