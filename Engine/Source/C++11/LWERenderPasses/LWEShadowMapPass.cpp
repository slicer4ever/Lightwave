#include "LWERenderPasses/LWEShadowMapPass.h"
#include "LWERenderer.h"
#include "LWECamera.h"
#include <LWCore/LWLogger.h>

//LWEShadowMapPass:
LWBitField32Define(LWEShadowMapPass::CubeFaceIndexBits);
const uint32_t LWEShadowMapPass::CubeIndex;

LWEPass *LWEShadowMapPass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	LWEShadowMapPass *SMPass = Pass ? (LWEShadowMapPass*)Pass : Allocator.Create<LWEShadowMapPass>();
	if (!LWEGeometryPass::ParseXML(Node, SMPass, Renderer, AssetManager, Allocator)) {
		if (!Pass) LWAllocator::Destroy(SMPass);
		return nullptr;
	}
	LWEPassPropertys Props = SMPass->GetPropertys();
	Props.m_Flag |= LWEPassPropertys::ClearDepth;
	SMPass->SetPropertys(Props);
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWEXMLAttribute *BucketsAttr = Node->FindAttribute("Buckets");
	LWEXMLAttribute *ArraySizeAttr = Node->FindAttribute("ArraySize");
	LWEXMLAttribute *PassBitIDAttr = Node->FindAttribute("PassBitID");
	uint32_t PassBitID = PassBitIDAttr ? atoi(PassBitIDAttr->GetValue().c_str()) : 0;
	uint32_t BucketCount = BucketsAttr ? (uint32_t)atoi(BucketsAttr->GetValue().c_str()) : 0;
	SMPass->SetPassBitID(PassBitID);
	if (!BucketCount) {
		LWLogCritical("Shadowmap Pass must contain > 0 'Buckets' attributes.");
		if (!Pass) LWAllocator::Destroy(SMPass);
		return nullptr;
	}
	if (!ArraySizeAttr) {
		LWLogCritical("Shadowmap pass must specify a 'ArraySize' attribute.");
		if (!Pass) LWAllocator::Destroy(SMPass);
		return nullptr;
	}
	LWEXMLAttribute *CubeSizeAttr = Node->FindAttribute("CubeSize");
	LWEXMLAttribute *CascadeCountAttr = Node->FindAttribute("CascadeCount");
	LWERenderPendingTexture ArrayTex = LWERenderPendingTexture(LWVector2i(), LWERenderTextureProps(LWTexture::MinLinear | LWTexture::MagLinear | LWTexture::CompareModeRefTexture | LWTexture::CompareLessEqual | LWTexture::RenderTarget, LWImage::DEPTH32, LWTexture::Texture2DArray, BucketCount, 0));
	LWERenderPendingTexture CubeTex = LWERenderPendingTexture(LWVector2i(), LWERenderTextureProps(LWTexture::MinLinear | LWTexture::MagLinear | LWTexture::CompareModeRefTexture | LWTexture::CompareLessEqual | LWTexture::RenderTarget, LWImage::DEPTH32, LWTexture::TextureCubeMapArray, BucketCount, 0));
	LWERenderPendingFrameBuffer ArrayFB;
	LWERenderPendingFrameBuffer CubeFB;
	if(!LWLogCriticalIf<256>(Renderer->ParseXMLSizeAttribute(ArraySizeAttr, ArrayTex.m_StaticSize, ArrayTex.m_DynamicSize, ArrayTex.m_NamedDynamic), "Could not parse ArraySize attribute: '{}'", ArraySizeAttr->GetValue())) {
		if (!Pass) LWAllocator::Destroy(SMPass);
		return nullptr;
	}
	uint32_t ShadowCubeTexID = 0;
	uint32_t ShadowCubeFBID = 0;
	ArrayFB.m_DynamicSize = ArrayTex.m_DynamicSize;
	ArrayFB.m_StaticSize = ArrayTex.m_StaticSize;
	ArrayFB.m_NamedDynamic = ArrayTex.m_NamedDynamic;
	uint32_t ShadowArrayID = Renderer->PushPendingResource(LWERenderPendingResource(0, LWUTF8I::Fmt<128>("{}TexArray", NameAttr->GetValue()), 0, ArrayTex));
	uint32_t ShadowArrayFBID = Renderer->PushPendingResource(LWERenderPendingResource(0, 0, ArrayFB));
	if (CubeSizeAttr) {
		if(LWLogCriticalIf<256>(Renderer->ParseXMLSizeAttribute(CubeSizeAttr, CubeTex.m_StaticSize, CubeTex.m_DynamicSize, CubeTex.m_NamedDynamic), "Could not parse CubeSize attribute: '{}'", CubeSizeAttr->GetValue())) {
			CubeFB.m_DynamicSize = CubeTex.m_DynamicSize;
			CubeFB.m_StaticSize = CubeTex.m_StaticSize;
			CubeFB.m_NamedDynamic = CubeTex.m_NamedDynamic;
			ShadowCubeTexID = Renderer->PushPendingResource(LWERenderPendingResource(0, LWUTF8I::Fmt<128>("{}CubeArray", NameAttr->GetValue()), 0, CubeTex));
			ShadowCubeFBID = Renderer->PushPendingResource(LWERenderPendingResource(0, 0, CubeFB));
		}
	}
	SMPass->SetShadowIDs(ShadowArrayID, ShadowArrayFBID, ShadowCubeTexID, ShadowCubeFBID);
	if (CascadeCountAttr) SMPass->SetCascadeCount((uint32_t)atoi(CascadeCountAttr->GetValue().c_str()));
	SMPass->SetGeometryBucketCount(BucketCount);
	for (uint32_t i = 0; i < BucketCount; i++) SMPass->SetGeometryBucketPropertys(i, LWEBucketPropertys(LWEPassPropertys(), 0, PassBitID));
	Renderer->ProcessPendingResources((uint64_t)-1); //need to make the named resources exposed.
	return SMPass;
}

uint32_t LWEShadowMapPass::RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex) {
	LWPipelineInputStream InputStreams[LWShader::MaxInputs];
	uint32_t p = 0;
	LWFrameBuffer *ArrayFB = Renderer->GetFrameBuffer(m_ShadowArrayFBID);
	LWFrameBuffer *CubeFB = m_ShadowCubeFBID ? Renderer->GetFrameBuffer(m_ShadowCubeFBID) : nullptr;
	LWTexture *ArrayTex = Renderer->GetTexture(m_ShadowTexArrayID);
	LWTexture *CubeTex = m_ShadowCubeArrayID ? Renderer->GetTexture(m_ShadowCubeArrayID) : nullptr;
	for (uint32_t i = m_GeometryPassIdx; i < m_GeometryPassIdx + m_GeometryBucketCount; ++i, ++p) {
		if (!Frame.GetGeometryBucket(i).isInitialized()) continue;
		if ((m_FBTargetIndex[p] & CubeIndex) != 0) {
			uint32_t CubeLayer = m_FBTargetIndex[p] & (~(CubeIndex | CubeFaceIndexBits));
			uint32_t CubeFace = LWBitFieldGet(CubeFaceIndexBits, m_FBTargetIndex[p]);
			CubeFB->SetCubeAttachment(LWFrameBuffer::Depth, CubeTex, CubeFace, CubeLayer, 0, (void*)(uintptr_t)LWERenderFramebufferTexture::FrameBufferName);
			m_Propertys.m_TargetFB = m_ShadowCubeFBID;
		} else {
			ArrayFB->SetAttachment(LWFrameBuffer::Depth, ArrayTex, m_FBTargetIndex[p], 0, (void*)(uintptr_t)LWERenderFramebufferTexture::FrameBufferName);
			m_Propertys.m_TargetFB = m_ShadowArrayFBID;
		}
		PreparePass(Driver, Renderer, m_Propertys);
		uint32_t RenderableOffset = 0;
		//Draw all opaque objects:
		for (uint32_t o = 0; o < RenderableCount[i].first; o++) {
			LWEGeometryRenderable &Renderable = RenderableList[i][o];
			LWVideoBuffer *IndiceBuffer = nullptr;
			uint32_t Count = 0;
			LWPipeline *Pipeline = PrepareRendablePipeline(Renderable, LWERenderMaterial(), IndiceBuffer, Count, InputStreams, Driver, Renderer, p, SubPassIndex);
			if (!Count) continue;
			Driver->DrawIndirectBuffer(Pipeline, LWVideoDriver::Triangle, InputStreams, IndiceBuffer, m_IndirectBuffer[p], Count, RenderableOffset);
			RenderableOffset += Count;
		}
	}
	return m_GeometryBucketCount;
}

LWEShadowMapPass &LWEShadowMapPass::SetShadowIDs(uint32_t ShadowTexArrayID, uint32_t ShadowArrayFBID, uint32_t ShadowCubeArrayID, uint32_t ShadowCubeFBID) {
	m_ShadowTexArrayID = ShadowTexArrayID;
	m_ShadowArrayFBID = ShadowArrayFBID;
	m_ShadowCubeArrayID = ShadowCubeArrayID;
	m_ShadowCubeFBID = ShadowCubeFBID;
	return *this;
}

LWEShadowMapPass &LWEShadowMapPass::SetCascadeCount(uint32_t CascadeCount) {
	m_CascadeCount = std::min<uint32_t>(CascadeCount, 4);
	return *this;
}

LWEShadowMapPass &LWEShadowMapPass::SetPassBitID(uint32_t PassID) {
	m_PassBitID = PassID;
	return *this;
}

LWEShadowMapPass &LWEShadowMapPass::InitializeShadowCastorBuckets(LWERenderFrame &Frame) {
	LWECamera CameraBuffer[6];
	uint32_t PrimaryBucket = Frame.GetPrimaryBucketID();
	LWEGeometryBucket &PBucket = Frame.GetPrimaryBucket();
	if (!PBucket.isInitialized()) return *this;
	LWEShaderGlobalData &GlobalData = Frame.GetGlobalData();
	LWEShaderGeometryBucketData &PBucketData = GlobalData.m_BucketList[PrimaryBucket];

	float BucketFar = PBucket.m_Frustum[1].w;
	LWSVector4f AAMin = PBucket.m_ViewTransform[3] - LWSVector4f(BucketFar, BucketFar, BucketFar, 0.0f);
	LWSVector4f AAMax = PBucket.m_ViewTransform[3] + LWSVector4f(BucketFar, BucketFar, BucketFar, 0.0f);

	uint32_t ShadowCount = std::min<uint32_t>(Frame.GetShadowCount(), LWEMaxLights);
	LWEShadowBucketItem *ShadowList = Frame.GetShadowList();
	std::sort(ShadowList, ShadowList + ShadowCount, LWEShadowBucketItem::SortFrontToBack);
	uint32_t NextBucket = 0;
	uint32_t NextTexture = 0;
	uint32_t NextCube = 0;
	for (uint32_t i = 0; i < ShadowCount; i++) {
		LWEShaderLightData &Light = Frame.GetLight(ShadowList[i].m_LightIndex);
		if (Light.m_Position.w == 0.0f) { //Directional light:
			if (NextBucket + m_CascadeCount > m_GeometryBucketCount) break;
			LWECamera::MakeCascadeCameraViews(Light.m_Direction, PBucket.m_ViewTransform[3], PBucket.m_FrustumPoints, PBucketData.m_ProjViewTransform, CameraBuffer, m_CascadeCount, AAMin, AAMax);
			for (uint32_t i = 0; i < m_CascadeCount; i++) {
				Frame.InitializeBucket(CameraBuffer[i], m_PassBitID, NextBucket);
				Light.m_ShadowIndexs[i] = LWEShaderLightData::MakeShadowFlag(m_PassID, NextBucket, NextTexture);
				m_FBTargetIndex[NextBucket] = NextTexture++;
				NextBucket++;
			}
		} else if (Light.m_Position.w == 1.0f) {//Point light:
			if (m_ShadowCubeArrayID == 0) continue; //cube shadow's are not configured.
			if (NextBucket + 6 > m_GeometryBucketCount) break;

		} else if (Light.m_Position.w > 1.0f) {//Spot light:
			if (NextBucket + 1 > m_GeometryBucketCount) break;
			LWECamera Spot = LWECamera(Light.m_Position.xyz1(), Light.m_Direction.xyz0(), LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f), 1.0f, (Light.m_Position.w - 1.0f) * 2.0f, 0.1f, Light.m_Direction.w, 0);
			Frame.InitializeBucket(Spot, m_PassBitID, NextBucket);
			Light.m_ShadowIndexs.x = LWEShaderLightData::MakeShadowFlag(m_PassID, NextBucket, NextTexture);
			m_FBTargetIndex[NextBucket] = NextTexture++;
			NextBucket++;
		}
	}
	GlobalData.m_ShadowPassID = m_PassID;
	GlobalData.m_ShadowCount = NextBucket;

	return *this;
}

uint32_t LWEShadowMapPass::GetPassBitID(void) const {
	return m_PassBitID;
}

LWEShadowMapPass::LWEShadowMapPass(uint32_t ShadowMapCount) : LWEGeometryPass(ShadowMapCount) {}

