#include <LWPlatform/LWWindow.h>
#include <LWCore/LWMatrix.h>
#include <LWVideo/LWVideoBuffer.h>
#include <LWVideo/LWFrameBuffer.h>
#include <LWVideo/LWImage.h>
#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

const LWVector2i Renderer::TileSize = LWVector2i(32, 32);
const LWVector2i Renderer::LocalThreads = LWVector2i(32, 32);


bool Light::MakeCamera(Camera &C) {
	if (m_Position.w == 1.0f) C = Camera(m_Position.xyz(), m_Direction.x + m_Direction.y, 0, true);
	else if(m_Position.w>1.0f) C = Camera(m_Position.xyz(), m_Direction.xyz(), LWVector3f(0.0f, 1.0f, 0.0f), 0, 1.0f, (m_Position.w - 1.0f)*2.0f, 1.0f, m_Direction.w, true);
	return true;
}

Light::Light(const LWVector4f &Color, float Intensity) : m_Position(LWVector4f(0.0f, 0.0f, 0.0f, -1.0f-Intensity)), m_Color(Color), m_ShadowIdx(LWVector4i(-1)) {}

Light::Light(const LWVector3f &Direction, const LWVector4f &Color, float Intensity, const LWVector4i &ShadowIndexs) : m_Direction(Direction, 0.0f), m_Color(LWVector4f(Color.xyz(), Intensity)), m_ShadowIdx(ShadowIndexs) {}

Light::Light(const LWVector3f &Position, float InteriorRadius, float FalloffRadius, const LWVector4f &Color, float Intensity, const LWVector4i &ShadowIndexs) : m_Position(Position, 1.0f), m_Direction(LWVector4f(FalloffRadius, InteriorRadius, 0.0f, 0.0f)), m_Color(LWVector4f(Color.xyz(), Intensity)), m_ShadowIdx(ShadowIndexs) {}

Light::Light(const LWVector3f &Position, const LWVector3f &Dir, float Length, float Theta, const LWVector4f &Color, float Intensity, const LWVector4i &ShadowIndexs) : m_Position(LWVector4f(Position, 1.0f + Theta)), m_Direction(LWVector4f(Dir, Length)), m_Color(LWVector4f(Color.xyz(), Intensity)), m_ShadowIdx(ShadowIndexs) {}


uint32_t lFrameList::WriteInstance(uint32_t InstanceID, bool Opaque) {
	if (m_OpaqueCnt + m_TransparentCnt >= MaxListInstances) return -1;
	//Keeps transparent objects in order of appearance, but opaque objects will be shuffled since their sorting is handled by the depth buffer.
	if (!Opaque) {
		m_InstanceIDs[m_TransparentCnt + m_OpaqueCnt] = m_InstanceIDs[m_TransparentCnt];
		m_InstanceIDs[m_TransparentCnt] = InstanceID;
		m_TransparentCnt++;
		return m_TransparentCnt - 1;
	} 
	m_InstanceIDs[m_TransparentCnt + m_OpaqueCnt] = InstanceID;
	m_OpaqueCnt++;
	return m_OpaqueCnt + m_TransparentCnt - 1;
}

uint32_t lFrame::GetIDBit(uint32_t ID) {
	return 1 << ID;
}

uint32_t lFrame::MakeList(Camera &C) {
	if (m_ListCount >= MaxLists) return -1;
	C.BuildFrustrumPoints(m_Lists[m_ListCount].m_FrustumPoints);
	lFrameList &L = m_Lists[m_ListCount];
	L.m_ViewPosition = LWVector4f(C.GetPosition(), 1.0f);
	L.m_ProjViewMatrix = C.GetProjViewMatrix();
	L.m_Flag = (C.IsPointCamera() ? lFrameList::PointView : 0) | (C.IsShadowCaster() ? lFrameList::ShadowView : 0);
	L.m_OpaqueCnt = L.m_TransparentCnt = 0;
	C.SetListID(m_ListCount);
	m_ListCount++;
	return m_ListCount - 1;
}

uint32_t lFrame::WriteInstance(const Instance &Inst, const ModelData &InstanceData, uint32_t ListBits, LWVideoDriver *Driver) {
	if (m_InstanceCount >= MaxInstances) return -1;
	ModelData *MD = Driver->GetUniformPaddedAt<ModelData>(m_InstanceCount, m_InstanceBuffer);
	*MD = InstanceData;
	m_Instances[m_InstanceCount] = Inst;
	bool isOpaque = true;
	for (uint32_t i = 0; i < Inst.m_PrimitiveCount && isOpaque; i++) {
		isOpaque = m_Materials[Inst.m_PrimitiveList[i].m_MaterialID].m_Opaque;
	}
	for (uint32_t i = 0; i < MaxLists; i++) {
		if((ListBits&(1<<i))==0) continue;
		m_Lists[i].WriteInstance(m_InstanceCount, isOpaque);
	}
	m_InstanceCount++;
	return m_InstanceCount - 1;
}

uint32_t lFrame::WriteMaterial(const MaterialData &Mat, const MaterialInfo &MatInfo, LWVideoDriver *Driver) {
	if (m_MaterialCount >= MaxMaterials) return -1;
	MaterialData *MD = Driver->GetUniformPaddedAt<MaterialData>(m_MaterialCount, m_MaterialBuffer);
	*MD = Mat;
	m_Materials[m_MaterialCount] = MatInfo;
	m_MaterialCount++;
	return m_MaterialCount - 1;
}

uint32_t lFrame::WriteLight(Light &L, LWVideoDriver *Driver) {
	if (m_LightCount >= MaxLights) return -1;
	Light *Lb = ((Light*)m_LightBuffer) + m_LightCount;
	*Lb = L;
	m_LightCount++;
	return m_LightCount - 1;
}

bool Renderer::WriteDebugLine(lFrame &F, const LWVector3f &APnt, const LWVector3f &BPnt, float Thickness, const LWVector4f &Color) {
	LWVector3f Dir = (BPnt - APnt);
	LWVector3f nDir = Dir.Normalize();
	LWMatrix4f Rot;
	LWVector3f Up;
	LWVector3f Right;
	nDir.Othogonal(Right, Up);
	Rot = LWMatrix4f::Rotation(nDir, Up);

	MaterialInfo MatInfo;
	MatInfo.m_Type = LWEGLTFMaterial::Unlit;
	MatInfo.m_Opaque = Color.w >= 1.0f;
	uint32_t MatID = F.WriteMaterial({ Color, LWVector4f(0.0f), LWVector4f(0.0f), 0 }, MatInfo, m_Driver);

	Instance Instance;
	ModelData InstanceData;
	InstanceData.m_TransformMatrix = LWMatrix4f(Thickness, Thickness, Dir.Length()*0.5f, 1.0f)*Rot*LWMatrix4f::Translation(APnt + nDir * Dir.Length()*0.5f);
	Instance.m_PrimitiveList[0] = { m_DebugCube, MatID };
	Instance.m_HasSkin = false;
	Instance.m_PrimitiveCount = 1;
	return F.WriteInstance(Instance, InstanceData, (1<<lFrame::MainView), m_Driver)!=-1;
}

bool Renderer::WriteDebugPoint(lFrame &F, const LWVector3f &Pnt, float Radius, const LWVector4f &Color) {

	MaterialInfo MatInfo;
	MatInfo.m_Type = LWEGLTFMaterial::Unlit;
	MatInfo.m_Opaque = Color.w >= 1.0f;
	uint32_t MatID = F.WriteMaterial({ Color, LWVector4f(0.0f), LWVector4f(0.0f), 0 }, MatInfo, m_Driver);
	Instance Instance;
	ModelData InstanceData;
	InstanceData.m_TransformMatrix = LWMatrix4f(Radius, Radius, Radius, 1.0f)*LWMatrix4f::Translation(Pnt);
	Instance.m_PrimitiveList[0] = { m_DebugPoint, MatID };
	Instance.m_HasSkin = false;
	Instance.m_PrimitiveCount = 1;
	return F.WriteInstance(Instance, InstanceData, (1 << lFrame::MainView), m_Driver) != -1;
}

bool Renderer::WriteDebugRect(lFrame &F, const LWVector3f &Pnt, const LWVector3f &Size, const LWQuaternionf &Rot, const LWVector4f &Color) {
	LWVector3f hSize = Size * 0.5f;
	MaterialInfo MatInfo;
	MatInfo.m_Type = LWEGLTFMaterial::Unlit;
	MatInfo.m_Opaque = Color.w >= 1.0f;
	uint32_t MatID = F.WriteMaterial({ Color, LWVector4f(0.0f), LWVector4f(0.0f), 0 }, MatInfo, m_Driver);
	Instance Instance;
	ModelData InstanceData;
	InstanceData.m_TransformMatrix = LWMatrix4f(hSize.x, hSize.y, hSize.z, 1.0f)*Rot*LWMatrix4f::Translation(Pnt);
	Instance.m_PrimitiveList[0] = { m_DebugCube, MatID };
	Instance.m_HasSkin = false;
	Instance.m_PrimitiveCount = 1;
	return F.WriteInstance(Instance, InstanceData, (1 << lFrame::MainView), m_Driver) != -1;
}

bool Renderer::WriteDebugCone(lFrame &F, const LWVector3f &Pnt, const LWVector3f &Dir, float Theta, float Length, const LWVector4f &Color) {
	const LWVector3f ConeForward = LWVector3f(1.0f, 0.0f, 0.0f);

	MaterialInfo MatInfo;
	MatInfo.m_Type = LWEGLTFMaterial::Unlit;
	MatInfo.m_Opaque = Color.w >= 1.0f;
	uint32_t MatID = F.WriteMaterial({ Color, LWVector4f(0.0f), LWVector4f(0.0f), 0 }, MatInfo, m_Driver);

	float T = tanf(Theta)*Length;

	LWMatrix4f Rotation;
	float d = ConeForward.Dot(Dir);
	if (d < -1.0f + std::numeric_limits<float>::epsilon()) Rotation = LWMatrix4f(-1.0f, 1.0f, 1.0f, 1.0f); //Flip 180Degrees.
	else if (d < 1.0f - std::numeric_limits<float>::epsilon()) {
		LWVector3f Up = ConeForward.Cross(Dir).Normalize();
		LWVector3f Rt = Dir.Cross(Up).Normalize();
		LWVector3f Fwrd = Up.Cross(Rt);
		Rotation = LWMatrix4f(LWVector4f(Fwrd, 0.0f), LWVector4f(Up, 0.0f), LWVector4f(Rt, 0.0f), LWVector4f(0.0f, 0.0f, 0.0f, 1.0f));
	}
	Instance Instance;
	ModelData InstanceData;
	InstanceData.m_TransformMatrix = LWMatrix4f(Length, T, T, 1.0f)*Rotation * LWMatrix4f::Translation(Pnt);
	Instance.m_PrimitiveList[0] = { m_DebugCone, MatID };
	Instance.m_HasSkin = false;
	Instance.m_PrimitiveCount = 1;
	return F.WriteInstance(Instance, InstanceData, (1 << lFrame::MainView), m_Driver)!=-1;
}


lFrame *Renderer::BeginFrame(void) {
	if (m_WriteFrame - m_ReadFrame >= FrameCount - 1) return nullptr;
	lFrame &F = m_Frames[m_WriteFrame%FrameCount];
	F.m_FontWriter.m_TextureCount = 0;
	F.m_InstanceCount = 0;
	F.m_MaterialCount = 0;
	F.m_ListCount = 0;
	F.m_LightCount = 0;
	return &F;
}

Renderer &Renderer::EndFrame(void) {
	m_WriteFrame++;
	return *this;
}

Renderer &Renderer::SizeChanged(LWWindow *Window) {
	if (!m_SizeChanged) return *this;
	LWVector2f WndSize = Window->GetSizef();
	if (WndSize.x < 1.0f || WndSize.y < 1.0f) return *this;
	LWVector2i ThreadGroups = GetThreadGroups(Window->GetSize());
	LWMatrix4f Ortho = LWMatrix4f::Ortho(0.0f, WndSize.x, 0.0f, WndSize.y, 0.0f, 1.0f);
	std::copy(&Ortho, &Ortho + 1, (LWMatrix4f*)m_TextUniformBuffer->GetLocalBuffer());
	m_TextUniformBuffer->SetEditLength(sizeof(LWMatrix4f));

	LWVector2i TotalThreads = ThreadGroups * LocalThreads;
	if (m_LightIndexBuffer) m_Driver->DestroyVideoBuffer(m_LightIndexBuffer);
	m_LightIndexBuffer = m_Driver->CreateVideoBuffer<uint32_t>(LWVideoBuffer::ImageBuffer, LWVideoBuffer::GPUResource, MaxLightsPerTile*TotalThreads.x*TotalThreads.y, m_Allocator, nullptr);
	m_LightCullPipeline->SetResource(1, m_LightIndexBuffer);
	m_MetallicRoughnessPipeline->SetResource(1, m_LightIndexBuffer);
	m_SpecularGlossinessPipeline->SetResource(1, m_LightIndexBuffer);
	m_UnlitPipeline->SetResource(1, m_LightIndexBuffer);

	m_SizeChanged = false;
	return *this;
}

Renderer &Renderer::ApplyFrame(lFrame &F, LWWindow *Wnd) {
	F.m_FontWriter.m_Mesh->Finished();
	uint8_t *LData = m_ListUniform->GetLocalBuffer();
	LWVector2i ThreadDimension = GetThreadGroups(Wnd->GetSize());

	ListData *LM = m_Driver->GetUniformPaddedAt<ListData>(0, LData);
	for (uint32_t i = 0; i < F.m_ListCount; i++) {
		lFrameList &L = F.m_Lists[i];
		ListData *LU = m_Driver->GetUniformPaddedAt<ListData>(i, LData);
		std::copy(L.m_FrustumPoints, L.m_FrustumPoints + 6, LU->m_FrustumPoints);
		LU->m_ProjViewMatrix = L.m_ProjViewMatrix;
		LU->m_ViewPosition = L.m_ViewPosition;
		LU->m_ThreadDimensions = ThreadDimension;
		LU->m_TileSize = TileSize;
		LU->m_ScreenSize = Wnd->GetSizef();
		LU->m_LightCount = F.m_LightCount;
		if (i) {
			LM->m_ShadowProjViewMatrix[i - 1] = L.m_ProjViewMatrix;
		}
	}

	m_ListUniform->SetEditLength(m_Driver->GetUniformPaddedLength<ListData>(F.m_ListCount));
	m_Driver->UpdateVideoBuffer(m_LightArrayBuffer, F.m_LightBuffer, sizeof(Light)*F.m_LightCount);
	m_Driver->UpdateVideoBuffer(m_ModelUniform, F.m_InstanceBuffer, m_Driver->GetUniformPaddedLength<ModelData>(F.m_InstanceCount));
	m_Driver->UpdateVideoBuffer(m_MaterialUniform, F.m_MaterialBuffer, m_Driver->GetUniformPaddedLength<MaterialData>(F.m_MaterialCount));
	return *this;
}

Renderer &Renderer::RenderText(LWFontSimpleWriter &FontWriter) {
	uint32_t o = 0;
	for (uint32_t i = 0; i < FontWriter.m_TextureCount; i++) {
		m_FontPipeline->SetResource(0, FontWriter.m_Textures[i]);
		m_Driver->DrawMesh(m_FontPipeline, LWVideoDriver::Triangle, FontWriter.m_Mesh, FontWriter.m_TextureVertices[i], o);
		o += FontWriter.m_TextureVertices[i];
	}
	return *this;
}

LWPipeline *Renderer::PreparePipeline(lFrame &F, bool isPointView, bool isShadowCaster, bool isSkinned, bool isOpaque, uint32_t ListIdx, uint32_t InstanceID, uint32_t MaterialID) {

	auto BindTexture = [](LWPipeline *P, uint32_t Index, MaterialTextureInfo &Tex) {
		if (Tex.m_Texture) Tex.m_Texture->SetTextureState(Tex.m_TextureFlag);
		P->SetResource(Index, Tex.m_Texture);
	};

	MaterialInfo &MatInfo = F.m_Materials[MaterialID];
	LWPipeline *Pipe = m_MetallicRoughnessPipeline;
	if (isShadowCaster) Pipe = m_ShadowPipeline;
	else if (MatInfo.m_Type == LWEGLTFMaterial::SpecularGlossyness) Pipe = m_SpecularGlossinessPipeline;
	else if (MatInfo.m_Type == LWEGLTFMaterial::Unlit) Pipe = m_UnlitPipeline;
	//Pipe->SetBlendMode(!isOpaque, LWPipeline::BLEND_SRC_ALPHA, LWPipeline::BLEND_ONE_MINUS_SRC_ALPHA);
	if (isPointView) {
		Pipe->SetCullMode(LWPipeline::CULL_NONE).SetClipping(true);
		Pipe->SetVertexShader(isSkinned ? m_SkeletonPointShader : m_StaticPointShader);
	} else {
		Pipe->SetCullMode(LWPipeline::CULL_CW).SetClipping(false);
		Pipe->SetVertexShader(isSkinned ? m_SkeletonShader : m_StaticShader);
	}
	Pipe->SetUniformBlock(0, m_ListUniform, m_Driver->GetUniformBlockOffset(sizeof(ListData), ListIdx));
	Pipe->SetUniformBlock(1, m_ModelUniform, m_Driver->GetUniformBlockOffset(sizeof(ModelData), InstanceID));
	if (!isShadowCaster) {
		Pipe->SetUniformBlock(2, m_MaterialUniform, m_Driver->GetUniformBlockOffset(sizeof(MaterialData), MaterialID));
		BindTexture(Pipe, 3, MatInfo.m_NormalTexture);
		BindTexture(Pipe, 4, MatInfo.m_OcclussionTexture);
		BindTexture(Pipe, 5, MatInfo.m_EmissiveTexture);
		BindTexture(Pipe, 6, MatInfo.m_MaterialTextureA);
		BindTexture(Pipe, 7, MatInfo.m_MaterialTextureB);
	}

	return Pipe;
}

Renderer &Renderer::RenderList(lFrame &F, uint32_t ListIdx, LWWindow *Window) {
	lFrameList &List = F.m_Lists[ListIdx];
	bool isPointView = (List.m_Flag&lFrameList::PointView) != 0;
	bool isShadowView = (List.m_Flag&lFrameList::ShadowView) != 0;
	for (uint32_t i = 0; i < List.m_TransparentCnt + List.m_OpaqueCnt; i++) {
		uint32_t InstanceID = List.m_InstanceIDs[i];
		Instance &Inst = F.m_Instances[InstanceID];
		for (uint32_t n = 0; n < Inst.m_PrimitiveCount; n++) {
			InstancePrimitive &Prim = Inst.m_PrimitiveList[n];
			LWPipeline *Pipeline = PreparePipeline(F, isPointView, isShadowView, Inst.m_HasSkin, i>=List.m_TransparentCnt, ListIdx, InstanceID, Prim.m_MaterialID);
			if (isPointView) m_Driver->DrawInstancedMesh(Pipeline, LWVideoDriver::Triangle, Prim.m_Mesh, 6);
			else m_Driver->DrawMesh(Pipeline, LWVideoDriver::Triangle, Prim.m_Mesh);
		}
	}
	return *this;
}

Renderer &Renderer::RenderFrame(lFrame &F, LWWindow *Window) {
	LWVector2i ThreadGroups = GetThreadGroups(Window->GetSize());
	m_Driver->Dispatch(m_LightCullPipeline, LWVector3i(ThreadGroups, 1));
	m_Driver->SetFrameBuffer(m_ShadowFB);
	uint32_t ShadowCnt = 0;
	for (uint32_t i = lFrame::MainView + 1; i < F.m_ListCount; i++) {
		lFrameList &L = F.m_Lists[i];
		if((L.m_Flag&lFrameList::ShadowView)==0) continue;
		m_ShadowFB->SetAttachment(LWFrameBuffer::Depth, m_ShadowDepthBuffer, ShadowCnt);
		m_Driver->ClearDepth(1.0f).ViewPort(m_ShadowFB);
		RenderList(F, i, Window);
		ShadowCnt++;
	}
	m_Driver->SetFrameBuffer(nullptr);
	m_Driver->ClearColor(0xFFFF).ClearDepth(1.0f);
	m_Driver->ViewPort();
	
	RenderList(F, lFrame::MainView, Window);
	RenderText(F.m_FontWriter);
	return *this;
}

Renderer &Renderer::Render(LWWindow *Window) {
	m_SizeChanged = Window->SizeUpdated() || m_SizeChanged;
	if (!m_Driver->Update()) return *this;
	SizeChanged(Window);
	if (!m_WriteFrame) return *this;
	if (m_ReadFrame != m_WriteFrame) {
		ApplyFrame(m_Frames[m_ReadFrame%FrameCount], Window);
		m_ReadFrame++;
	}
	m_Driver->ViewPort();
	m_Driver->ClearColor(0xFF);
	RenderFrame(m_Frames[(m_ReadFrame - 1) % FrameCount], Window);
	m_Driver->Present(1);
	return *this;
}

LWVector2i Renderer::GetThreadGroups(const LWVector2i WndSize) {
	LWVector2i TotalThreads = (WndSize + (LocalThreads - 1)) / LocalThreads;
	return (TotalThreads + (LocalThreads - 1)) / LocalThreads;
}

Renderer::Renderer(LWVideoDriver *Driver, App *A, LWEAssetManager *AssetManager, LWAllocator &Allocator) : m_Allocator(Allocator), m_Driver(Driver) {
	
	m_FontPipeline = AssetManager->GetAsset<LWPipeline>("FontPipeline");
	m_TextUniformBuffer = Driver->CreateVideoBuffer(LWVideoBuffer::Uniform, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, sizeof(LWMatrix4f), 1, Allocator, nullptr);

	m_StaticShader = AssetManager->GetAsset<LWShader>("StaticShader");
	m_StaticPointShader = AssetManager->GetAsset<LWShader>("StaticPointShader");
	m_SkeletonShader = AssetManager->GetAsset<LWShader>("SkeletonShader");
	m_SkeletonPointShader = AssetManager->GetAsset<LWShader>("SkeletonPointShader");

	m_MetallicRoughnessPipeline = AssetManager->GetAsset<LWPipeline>("MetallicRoughnessPipeline");
	m_SpecularGlossinessPipeline = AssetManager->GetAsset<LWPipeline>("SpecularGlossinessPipeline");
	m_UnlitPipeline = AssetManager->GetAsset<LWPipeline>("UnlitPipeline");
	m_ShadowPipeline = AssetManager->GetAsset<LWPipeline>("ShadowPipeline");

	m_LightCullPipeline = AssetManager->GetAsset<LWPipeline>("LightCullPipeline");

	m_ListUniform = AssetManager->GetAsset<LWVideoBuffer>("ListBuffer");
	m_ModelUniform = AssetManager->GetAsset<LWVideoBuffer>("ModelsBuffer");
	m_MaterialUniform = AssetManager->GetAsset<LWVideoBuffer>("MaterialsBuffer");
	m_LightArrayBuffer = AssetManager->GetAsset<LWVideoBuffer>("LightsBuffer");

	m_ShadowPipeline->SetUniformBlock(0, m_ListUniform);

	m_ShadowFB = Driver->CreateFrameBuffer(LWVector2i(4096), Allocator);
	m_ShadowDepthBuffer = Driver->CreateTexture2DArray(LWTexture::RenderTarget | LWTexture::CompareModeRefTexture | LWTexture::CompareLessEqual, LWImage::DEPTH24STENCIL8, m_ShadowFB->GetSize(), ListData::MaxShadows, nullptr, 0, Allocator);
	
	m_MetallicRoughnessPipeline->SetResource(2, m_ShadowDepthBuffer);
	m_SpecularGlossinessPipeline->SetResource(2, m_ShadowDepthBuffer);
	m_UnlitPipeline->SetResource(2, m_ShadowDepthBuffer);

	m_FontPipeline->SetUniformBlock(0, m_TextUniformBuffer);
	
	for (uint32_t i = 0; i < FrameCount; i++) {
		lFrame &F = m_Frames[i];
		LWVideoBuffer *Buf = Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, sizeof(LWVertexUI), MaxCharacters * 6, Allocator, nullptr);
		F.m_FontWriter.m_Mesh = LWVertexUI::MakeMesh(Allocator, Buf, 0);
		F.m_LightBuffer = Allocator.AllocateArray<uint8_t>(m_LightArrayBuffer->GetRawLength());
		F.m_InstanceBuffer = Allocator.AllocateArray<uint8_t>(m_ModelUniform->GetRawLength());
		F.m_MaterialBuffer = Allocator.AllocateArray<uint8_t>(m_MaterialUniform->GetRawLength());
	}


	//Build Debug meshes.
	const uint32_t ConeRadiCnt = 30;
	Vertice ConeVerts[ConeRadiCnt + 2];
	uint16_t ConeIdxs[ConeRadiCnt * 6];
	*(ConeVerts + 0) = Vertice(LWVector4f(0.0f, 0.0f, 0.0f, 1.0f));
	*(ConeVerts + 1) = Vertice(LWVector4f(1.0f, 0.0f, 0.0f, 1.0f));
	for (uint32_t n = 0; n < ConeRadiCnt; n++) {
		float T = LW_2PI / ConeRadiCnt * n;
		uint32_t a = n * 3;
		uint32_t b = (n * 3) + (ConeRadiCnt * 3);
		LWVector2f D = LWVector2f::MakeTheta(T);
		*(ConeVerts + n + 2) = { LWVector4f(1.0f, D.x, D.y, 1.0f) };

		*(ConeIdxs + a + 0) = 0;
		*(ConeIdxs + a + 1) = ((n + 1) % ConeRadiCnt) + 2;
		*(ConeIdxs + a + 2) = (n)+2;

		*(ConeIdxs + b + 0) = 1;
		*(ConeIdxs + b + 1) = (n)+2;
		*(ConeIdxs + b + 2) = ((n + 1) % ConeRadiCnt) + 2;
	}
	LWVideoBuffer *VCone = m_Driver->CreateVideoBuffer<Vertice>(LWVideoBuffer::Vertex, LWVideoBuffer::Static, ConeRadiCnt + 2, m_Allocator, ConeVerts);
	LWVideoBuffer *ICone = m_Driver->CreateVideoBuffer<uint16_t>(LWVideoBuffer::Index16, LWVideoBuffer::Static, ConeRadiCnt * 6, m_Allocator, ConeIdxs);
	m_DebugCone = m_Allocator.Allocate<LWMesh<Vertice>>(VCone, ICone, ConeRadiCnt + 2, ConeRadiCnt * 6);

	LWVertexTexture Verts[4] = { {LWVector4f(-1.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 0.0f, 0.0f)},
								 {LWVector4f(-1.0f,-1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 0.0f)},
								 {LWVector4f(1.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f)},
								 { LWVector4f(1.0f,-1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 1.0f, 0.0f, 0.0f) } };
	uint16_t Indices[6] = { 0,1,2, 1,3, 2 };

	LWVideoBuffer *VBuf = Driver->CreateVideoBuffer<LWVertexTexture>(LWVideoBuffer::Vertex, LWVideoBuffer::Static, 4, Allocator, Verts);
	LWVideoBuffer *IBuf = Driver->CreateVideoBuffer<uint16_t>(LWVideoBuffer::Index16, LWVideoBuffer::Static, 6, Allocator, Indices);
	m_DebugRect = LWVertexTexture::MakeMesh(Allocator, VBuf, IBuf, 4, 6);

	Vertice CubeVerts[8];
	uint16_t CubeIdxs[36] = { 0,5,1,0,4,5,  2,3,7,2,7,6,  2,6,4,2,4,0,  3,5,7,3,1,5,  6,7,5,6,5,4,  2,1,3,2,0,1 };
	*(CubeVerts + 0) = { LWVector4f(-1.0f, 1.0f, -1.0f, 1.0f) }; //0
	*(CubeVerts + 1) = { LWVector4f(1.0f, 1.0f, -1.0f, 1.0f) }; //1
	*(CubeVerts + 2) = { LWVector4f(-1.0f, -1.0f, -1.0f, 1.0f) }; //2
	*(CubeVerts + 3) = { LWVector4f(1.0f, -1.0f, -1.0f, 1.0f) }; //3

	*(CubeVerts + 4) = { LWVector4f(-1.0f, 1.0f, 1.0f, 1.0f) }; //4
	*(CubeVerts + 5) = { LWVector4f(1.0f, 1.0f, 1.0f, 1.0f) }; //5
	*(CubeVerts + 6) = { LWVector4f(-1.0f, -1.0f, 1.0f, 1.0f) }; //6
	*(CubeVerts + 7) = { LWVector4f(1.0f, -1.0f, 1.0f, 1.0f) }; //7
	LWVideoBuffer *VCube = Driver->CreateVideoBuffer<Vertice>(LWVideoBuffer::Vertex, LWVideoBuffer::Static, 8, Allocator, CubeVerts);
	LWVideoBuffer *ICube = Driver->CreateVideoBuffer<uint16_t>(LWVideoBuffer::Index16, LWVideoBuffer::Static, 36, Allocator, CubeIdxs);
	m_DebugCube = m_Allocator.Allocate <LWMesh<Vertice>>(VCube, ICube, 8, 36);

	const uint32_t verticalSteps = 20;
	const uint32_t horizontalSteps = 20;
	const uint32_t TotalVertices = verticalSteps * horizontalSteps * 6;
	Vertice SphereVerts[TotalVertices];
	uint32_t o = 0;

	float v = LW_PI_2;
	for (uint32_t y = 1; y <= verticalSteps; y++) {
		float nv = LW_PI_2 + LW_PI / (float)verticalSteps*(float)y;
		float h = 0.0f;
		for (uint32_t x = 1; x <= horizontalSteps; x++) {
			float nh = LW_2PI / (float)horizontalSteps*(float)x;
			LWVector3f TopLeftPnt = LWVector3f(cosf(h)*cosf(v), sinf(v), sinf(h)*cosf(v));
			LWVector3f BtmLeftPnt = LWVector3f(cosf(h)*cosf(nv), sinf(nv), sinf(h)*cosf(nv));
			LWVector3f TopRightPnt = LWVector3f(cosf(nh)*cosf(v), sinf(v), sinf(nh)*cosf(v));
			LWVector3f BtmRightPnt = LWVector3f(cosf(nh)*cosf(nv), sinf(nv), sinf(nh)*cosf(nv));

			*(SphereVerts + o++) = { LWVector4f(TopLeftPnt, 1.0f) };
			*(SphereVerts + o++) = { LWVector4f(TopRightPnt, 1.0f) };
			*(SphereVerts + o++) = { LWVector4f(BtmRightPnt, 1.0f) };
			*(SphereVerts + o++) = { LWVector4f(TopLeftPnt, 1.0f) };
			*(SphereVerts + o++) = { LWVector4f(BtmRightPnt, 1.0f) };
			*(SphereVerts + o++) = { LWVector4f(BtmLeftPnt, 1.0f) };
			h = nh;
		}
		v = nv;
	}
	LWVideoBuffer *VSphere = Driver->CreateVideoBuffer<Vertice>(LWVideoBuffer::Vertex, LWVideoBuffer::Static, TotalVertices, Allocator, SphereVerts);
	m_DebugPoint = m_Allocator.Allocate<LWMesh<Vertice>>(VSphere, TotalVertices);

}

Renderer::~Renderer() {
	for (uint32_t i = 0; i < FrameCount; i++) {
		lFrame &F = m_Frames[i];
		F.m_FontWriter.m_Mesh->Destroy(m_Driver);
		LWAllocator::Destroy(F.m_InstanceBuffer);
		LWAllocator::Destroy(F.m_MaterialBuffer);
		LWAllocator::Destroy(F.m_LightBuffer);
	}
	m_Driver->DestroyVideoBuffer(m_TextUniformBuffer);

	if(m_LightIndexBuffer) m_Driver->DestroyVideoBuffer(m_LightIndexBuffer);
	if (m_ShadowFB){
		m_Driver->DestroyTexture(m_ShadowDepthBuffer);
		m_Driver->DestroyFrameBuffer(m_ShadowFB);
	}
	m_DebugRect->Destroy(m_Driver);
	m_DebugCube->Destroy(m_Driver);
	m_DebugPoint->Destroy(m_Driver);
	m_DebugCone->Destroy(m_Driver);
}