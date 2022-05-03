#include "LWERenderPasses/LWEGausianBlurPass.h"
#include "LWERenderer.h"
#include <LWCore/LWLogger.h>

//LWEGausianPassPropertys
LWEGausianPassPropertys::LWEGausianPassPropertys(LWEPassResource &Source, bool bHorizontal, float Intensity, float Radius) : m_Source(Source), m_Radius(Radius), m_Intensity(Intensity), m_Horizontal(bHorizontal) {}

//LWEPPGausianBlur:
const LWVector4f LWEGausianBlurPass::Kernel = LWVector4f(0.06136f, 0.24477f, 0.38774f, 0.0f);


LWEPass *LWEGausianBlurPass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	LWEGausianBlurPass *GBPass = Pass ? (LWEGausianBlurPass*)Pass : Allocator.Create<LWEGausianBlurPass>();
	if(!LWLogCriticalIf<256>(LWEPass::ParseXML(Node, GBPass, Renderer, AssetManager, Allocator), "Could not create pass '{}'", Node->GetName())) {
		if (!Pass) LWAllocator::Destroy(GBPass);
		return nullptr;
	}

	LWEXMLAttribute *RadiusAttr = Node->FindAttribute("Radius");
	LWEXMLAttribute *IntensityAttr = Node->FindAttribute("Intensity");
	float DefaultRadius = RadiusAttr ? (float)atof(RadiusAttr->GetValue().c_str()) : 1.0f;
	float DefaultIntensity = IntensityAttr ? (float)atof(IntensityAttr->GetValue().c_str()) : 1.0f;

	auto ParseSubPass = [&GBPass, &AssetManager, &Renderer, &DefaultRadius, &DefaultIntensity](LWEXMLNode *N)->bool {
		LWEXMLAttribute *SourceAttr = N->FindAttribute("Source");
		if (!SourceAttr) {
			LWLogCritical("Gaussian blur subpass is missing required 'Source' Attribute.");
			return false;
		}
		LWEXMLAttribute *VerticalAttr = N->FindAttribute("Vertical");
		LWEXMLAttribute *IntensityAttr = N->FindAttribute("Intensity");
		LWEXMLAttribute *RadiusAttr = N->FindAttribute("Radius");

		float Radius = RadiusAttr ? (float)atof(RadiusAttr->GetValue().c_str()) : DefaultRadius;
		float Intensity = IntensityAttr ? (float)atof(IntensityAttr->GetValue().c_str()) : DefaultIntensity;

		LWEPassPropertys Props;
		LWEPassResource SourceResource;
		if (!LWEPass::ParseXMLPassPropertys(N, Props, Renderer)) return false;
		if(!LWLogCriticalIf<256>(LWEPass::ParseXMLPipelineResource(*SourceAttr, SourceResource, AssetManager, Renderer), "Source '{}' could not be found.", SourceAttr->GetValue())) return false;

		SourceResource.m_BindNameHash = LWUTF8Iterator("BlurTex").Hash();
		GBPass->PushSubPass(Props, LWEGausianPassPropertys(SourceResource, VerticalAttr==nullptr, Intensity, Radius));
		return true;
	};

	uint32_t SubPassCount = 0;
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t NodeID = C->GetName().CompareList("SubPass");
		if (NodeID == 0) ParseSubPass(C);
		else ParseXMLChild(C, GBPass, Renderer, AssetManager, Allocator);
	}
	return GBPass;
}


uint32_t LWEGausianBlurPass::RenderPass(LWERenderFrame & Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t PassIndex) {
	for (uint32_t i = 0; i < m_SubPassCount; i++) {
		PreparePass(Driver, Renderer, m_SubPassList[i]);
		LWPipeline *Pipeline = PreparePipeline(LWERenderMaterial(m_SubPassPipeline[i], nullptr, 0), LWUTF8I::EmptyHash, Driver, Renderer, PassIndex + i);
		LWEPass::PipelineBindResource(Pipeline, Renderer, m_SubPassGausPropertys[i].m_Source);
		Driver->DrawBuffer(Pipeline, LWVideoDriver::Triangle, m_ScreenPPVertices, nullptr, 6, sizeof(LWVertexTexture));
	}
	return m_SubPassCount;
}

LWEPass &LWEGausianBlurPass::PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver) {
	LWEPPPass::PreFinalizeFrame(Frame, IndirectBufferList, IDBufferList, Renderer, Driver);
	LWEShaderGlobalData &GlobalData = Frame.GetGlobalData();
	LWEShaderGlobalPassData &PassData = GlobalData.m_PassData[m_PassID];
	for (uint32_t i = 0; i < m_SubPassCount; i++) {
		LWEGausianPassPropertys &GProps = m_SubPassGausPropertys[i];
		PassData.m_SubPassData[i] = Kernel * GProps.m_Intensity;
		if (GProps.m_Horizontal) PassData.m_FrameSize[i].z = GProps.m_Radius;
		else PassData.m_FrameSize[i].w = GProps.m_Radius;
	}
	return *this;
}

LWEGausianPassPropertys &LWEGausianBlurPass::GetSubPassGausPropertys(uint32_t SubPassID) {
	return m_SubPassGausPropertys[SubPassID];
}

const LWEGausianPassPropertys &LWEGausianBlurPass::GetSubPassGausPropertys(uint32_t SubPassID) const {
	return m_SubPassGausPropertys[SubPassID];
}


bool LWEGausianBlurPass::PushSubPass(const LWEPassPropertys &SubPass, const LWEGausianPassPropertys &GausProps) {
	if(!LWLogCriticalIf(m_SubPassCount<LWEMaxPasses, "GaussianBlur has exceeded max subpasses.")) return false;
	m_SubPassPipeline[m_SubPassCount] = LWUTF8I::EmptyHash;
	m_SubPassList[m_SubPassCount] = SubPass;
	m_SubPassGausPropertys[m_SubPassCount] = GausProps;
	m_SubPassCount++;
	return true;
}