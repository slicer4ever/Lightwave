#include "LWERenderPass.h"
#include <LWPlatform/LWWindow.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWLogger.h>
#include <LWEAsset.h>
#include "LWERenderFrame.h"
#include "LWERenderer.h"
#include "LWECamera.h"

//LWEPassResource:
const uint32_t LWEPassResource::VideoBufferBit;
const uint32_t LWEPassResource::FrameBufferBit;

LWEPassResource::LWEPassResource(const LWUTF8Iterator &BindName, LWVideoBuffer *Buffer, uint32_t Offset) : m_Resource(Buffer), m_BindNameHash(BindName.Hash()), m_Offset(Offset) {}

LWEPassResource::LWEPassResource(const LWUTF8Iterator &BindName, LWTexture *Texture) : m_Resource(Texture), m_BindNameHash(BindName.Hash()) {}

LWEPassResource::LWEPassResource(const LWUTF8Iterator &BindName, uint32_t ResourceID, uint32_t Offset) : m_ResourceID(ResourceID), m_BindNameHash(BindName.Hash()), m_Offset(Offset) {}

LWEPassResource::LWEPassResource(uint32_t BindNameHash, LWVideoBuffer *Buffer, uint32_t Offset) : m_Resource(Buffer), m_BindNameHash(BindNameHash), m_Offset(Offset) {}

LWEPassResource::LWEPassResource(uint32_t BindNameHash, LWTexture *Texture) : m_Resource(Texture), m_BindNameHash(BindNameHash) {}

LWEPassResource::LWEPassResource(uint32_t BindNameHash, uint32_t ResourceID, uint32_t Offset) : m_ResourceID(ResourceID), m_BindNameHash(BindNameHash), m_Offset(Offset) {}

//LWEPassPipelineDescriptor:
bool LWEPassPipelinePropertys::PushBlock(const LWEPassResource &Block) {
	if (m_BlockCount >= LWShader::MaxBlocks) return false;
	m_BlockList[m_BlockCount++] = Block;
	return true;
}

bool LWEPassPipelinePropertys::PushResource(const LWEPassResource &Resource) {
	if (m_ResourceCount >= LWShader::MaxResources) return false;
	m_ResourceList[m_ResourceCount++] = Resource;
	return true;
}

LWEPassPipelinePropertys::LWEPassPipelinePropertys(LWPipeline *Pipeline) : m_DefaultPipeline(Pipeline) {}

//LWEPassPropertys:
const uint32_t LWEPassPropertys::ClearColor;
const uint32_t LWEPassPropertys::ClearDepth;
const uint32_t LWEPassPropertys::ClearStencil;
const uint32_t LWEPassPropertys::CustomViewport;

bool LWEPassPropertys::isClearColor(void) const {
	return (m_Flag & ClearColor) != 0;
}

bool LWEPassPropertys::isClearDepth(void) const {
	return (m_Flag & ClearDepth) != 0;
}

bool LWEPassPropertys::isClearStencil(void) const {
	return (m_Flag & ClearStencil) != 0;
}

bool LWEPassPropertys::isCustomViewport(void) const {
	return (m_Flag & CustomViewport) != 0;
}

//LWEPass:
const uint32_t LWEPass::Disaabled; //Disabled pass's partake in all settings except RenderPass.

bool LWEPass::ParseXMLPassPropertys(LWEXMLNode *Node, LWEPassPropertys &Propertys, LWERenderer *Renderer) {
	LWEXMLAttribute *ClearColorAttr = Node->FindAttribute("ClearColor");
	LWEXMLAttribute *ClearDepthAttr = Node->FindAttribute("ClearDepth");
	LWEXMLAttribute *ClearStencilAttr = Node->FindAttribute("ClearStencil");
	LWEXMLAttribute *ViewportAttr = Node->FindAttribute("Viewport");
	LWEXMLAttribute *TargetAttr = Node->FindAttribute("Target");
	if (ClearColorAttr) {
		if (ClearColorAttr->GetValue().AtEnd()) Propertys.m_Flag &= ~LWEPassPropertys::ClearColor;
		else {
			sscanf(ClearColorAttr->GetValue().c_str(), "#%x", &Propertys.m_ClearColor);
			Propertys.m_Flag |= LWEPassPropertys::ClearColor;
		}
	}
	if (ClearDepthAttr) {
		if (ClearDepthAttr->GetValue().AtEnd()) Propertys.m_Flag &= ~LWEPassPropertys::ClearDepth;
		else {
			Propertys.m_ClearDepth = (float)atof(ClearDepthAttr->GetValue().c_str());
			Propertys.m_Flag |= LWEPassPropertys::ClearDepth;
		}
	}
	if (ClearStencilAttr) {
		if (ClearStencilAttr->GetValue().AtEnd()) Propertys.m_Flag &= ~LWEPassPropertys::ClearStencil;
		else {
			Propertys.m_ClearStencil = (uint8_t)atoi(ClearStencilAttr->GetValue().c_str());
			Propertys.m_Flag |= LWEPassPropertys::ClearStencil;
		}
	}
	if (ViewportAttr) {
		sscanf(ViewportAttr->GetValue().c_str(), "%d | %d | %d | %d", &Propertys.m_Viewport.x, &Propertys.m_Viewport.y, &Propertys.m_Viewport.z, &Propertys.m_Viewport.w);
		Propertys.m_Flag |= LWEPassPropertys::CustomViewport;
	}
	if (TargetAttr) {
		Propertys.m_TargetFB = Renderer->FindNamedFrameBuffer(TargetAttr->GetValue());
	}
	return true;
}

bool LWEPass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	if (!Pass) return false;
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWEXMLAttribute *DisabledAttr = Node->FindAttribute("Disabled");
	LWEXMLAttribute *DefaultAttr = Node->FindAttribute("Default");
	if (!NameAttr) {
		LWLogCritical<128>("'{}' has no Name.", Node->GetName());
		return false;
	}
	LWEPassPropertys Props;
	if (!ParseXMLPassPropertys(Node, Props, Renderer)) return false;
	Pass->SetName(NameAttr->GetValue()).SetPropertys(Props).SetDisabled(DisabledAttr!=nullptr);
	if (DefaultAttr) {
		uint32_t PipelineNameHash = LWUTF8I::EmptyHash;
		LWEPassPipelinePropertys PipelineDesc;
		if(ParseXMLPipelinePropertys(Node, PipelineNameHash, PipelineDesc, Renderer, AssetManager, Allocator, true)) Pass->AddPipeline(PipelineNameHash, PipelineDesc);
	}
	return true;
}

bool LWEPass::ParseXMLPipelineBlock(LWEXMLAttribute &Attr, LWEPassResource &Block, LWEAssetManager *AssetManager, LWERenderer *Renderer) {
	LWUTF8Iterator SplitList[2];
	uint32_t SplitCnt = Attr.GetValue().SplitToken(SplitList, 2, ':');
	uint32_t Offset = SplitCnt == 2 ? atoi(SplitList[1].c_str()) : 0;
	uint32_t RenderID = Renderer->FindNamedVideoBuffer(SplitList[0], false);
	if (RenderID) Block = LWEPassResource(Attr.GetName(), RenderID, Offset);
	else {
		LWVideoBuffer *Asset = AssetManager->GetAsset<LWVideoBuffer>(SplitList[0]);
		if (!Asset) return false;
		Block = LWEPassResource(Attr.GetName(), Asset, Offset);
	}
	return true;
}

bool LWEPass::ParseXMLPipelineResource(LWEXMLAttribute &Attr, LWEPassResource &Resource, LWEAssetManager *AssetManager, LWERenderer *Renderer) {
	LWUTF8Iterator SplitList[2];
	uint32_t SplitCnt = Attr.GetValue().SplitToken(SplitList, 2, ':');
	uint32_t RenderID = 0; //Search renderer first, then assetmanager:
	RenderID = Renderer->FindNamedTexture(SplitList[0], false);
	if (RenderID) Resource = LWEPassResource(Attr.GetName(), RenderID);
	else {
		RenderID = Renderer->FindNamedFrameBuffer(SplitList[0], false);
		if (RenderID) {
			uint32_t Offset = SplitCnt == 2 ? SplitList[1].CompareList("Color", "Color1", "Color2", "Color3", "Color4", "Color5", "Depth") : 0;
			if(!LWLogCriticalIf<256>(Offset!=-1, "Framebuffer attachment specifier has unknown name: {}", SplitList[1])) return false;
			Resource = LWEPassResource(Attr.GetName(), RenderID | LWEPassResource::FrameBufferBit, Offset);
		} else {
			RenderID = Renderer->FindNamedVideoBuffer(SplitList[0], false);
			if (RenderID) {
				uint32_t Offset = SplitCnt == 2 ? (uint32_t)atoi(SplitList[1].c_str()) : 0;
				Resource = LWEPassResource(Attr.GetName(), RenderID | LWEPassResource::VideoBufferBit, Offset);
			} else {
				uint32_t Offset = SplitCnt == 2 ? (uint32_t)atoi(SplitList[1].c_str()) : 0;
				LWEAsset *Asset = AssetManager->GetAsset(SplitList[0]);
				if (!Asset) return false;
				Resource = LWEPassResource(Attr.GetName(), (LWVideoBuffer*)Asset->GetAsset(), Offset); //Asset will be correctly interpreted by the pipeline.
			}
		}
	}
	return true;
}

bool LWEPass::ParseXMLPipelinePropertys(LWEXMLNode *Node, uint32_t &PipelineNameHash, LWEPassPipelinePropertys &Desc, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator, bool InPassLine) {

	auto ParsePipelineBlocksNode = [&AssetManager, &Renderer](LWEXMLNode *Node, LWEPassPipelinePropertys &Desc) {
		for (uint32_t i = 0; i < Node->m_AttributeCount; i++) {
			LWEXMLAttribute &Attr = Node->m_Attributes[i];
			LWEPassResource Blk;
			if(!ParseXMLPipelineBlock(Attr, Blk, AssetManager, Renderer)) continue;
			Desc.PushBlock(Blk);
		}
	};

	auto ParsePipelineResourceNode = [&AssetManager, &Renderer](LWEXMLNode *Node, LWEPassPipelinePropertys &Desc) {
		for (uint32_t i = 0; i < Node->m_AttributeCount; i++) {
			LWEXMLAttribute &Attr = Node->m_Attributes[i];
			LWEPassResource Rsc;
			if(!ParseXMLPipelineResource(Attr, Rsc, AssetManager, Renderer)) continue;
			Desc.PushResource(Rsc);
		}
	};
	
	LWEXMLAttribute *NameAttr = InPassLine ? nullptr : Node->FindAttribute("Name");
	LWEXMLAttribute *DefaultAttr = Node->FindAttribute("Default");
	Desc = LWEPassPipelinePropertys();
	if (DefaultAttr) {
		Desc.m_DefaultPipeline = AssetManager->GetAsset<LWPipeline>(DefaultAttr->GetValue());
		if(!LWLogCriticalIf<256>(Desc.m_DefaultPipeline, "{} can not find default Pipeline named: '{}'", NameAttr ? NameAttr->GetValue() : "", DefaultAttr->GetValue())) return false;
	}
	for (uint32_t i = 0; i < Node->m_AttributeCount; i++) {
		LWEXMLAttribute &Attr = Node->m_Attributes[i];
		if (&Attr == NameAttr || &Attr == DefaultAttr) continue;
		if (!Renderer->FindNamedBlockGeometryMap(Attr.m_NameHash, !InPassLine)) continue;
		Desc.m_GeometryPipelineMap.insert({ Attr.m_NameHash, AssetManager->GetAsset<LWPipeline>(Attr.GetValue()) });
	}

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t cID = C->GetName().CompareList("Blocks", "Resources");
		if (cID == 0) ParsePipelineBlocksNode(C, Desc);
		else if (cID == 1) ParsePipelineResourceNode(C, Desc);
		else if(!InPassLine) LWLogCritical<256>("Unknown pipeline child '{}'", C->GetName());
	}
	PipelineNameHash = NameAttr ? NameAttr->GetValue().Hash() : LWUTF8I::EmptyHash;
	return true;
}

bool LWEPass::ParseXMLChild(LWEXMLNode *cNode, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	uint32_t NodeType = cNode->GetName().CompareList("Pipeline");

	auto ParsePipelineNode = [&Pass, &AssetManager, &Renderer, &Allocator](LWEXMLNode *Node)->bool {
		uint32_t PipelineName = LWUTF8I::EmptyHash;
		LWEPassPipelinePropertys PipelineDesc;
		if (!ParseXMLPipelinePropertys(Node, PipelineName, PipelineDesc, Renderer, AssetManager, Allocator)) return false;
		return Pass->AddPipeline(PipelineName, PipelineDesc);
	};

	if (NodeType == 0) return ParsePipelineNode(cNode);
	LWLogCritical<256>("unknown pass child node encountered: '{}'", cNode->GetName());
	return false;
}

bool LWEPass::AddPipeline(const LWUTF8Iterator &PipelineName, const LWEPassPipelinePropertys &PipelinePropertys) {
	auto Res = m_PipelineMap.insert({ PipelineName.Hash(), PipelinePropertys });
	return LWLogCriticalIf<256>(Res.second, "Adding pipeline '{}' failed.", PipelineName);
}

bool LWEPass::AddPipeline(uint32_t PipelineNameHash, const LWEPassPipelinePropertys &PipelineProps) {
	auto Res = m_PipelineMap.insert({ PipelineNameHash, PipelineProps });
	return LWLogCriticalIf<256>(Res.second, "Adding pipeline {:#x} failed.", PipelineNameHash);
}

LWEPass &LWEPass::SetPropertys(const LWEPassPropertys &Propertys) {
	m_Propertys = Propertys;
	return *this;
}

LWEPass &LWEPass::SetName(const LWUTF8Iterator &Name) {
	m_NameHash = Name.Hash();
	return *this;
}

LWEPass &LWEPass::SetFrameID(uint32_t ID) {
	m_FrameID = ID;
	return *this;
}

LWEPass &LWEPass::SetDisabled(bool bDisabled) {
	m_Flag = (m_Flag & ~Disaabled) | (bDisabled ? Disaabled : 0);
	return *this;
}

LWEPass &LWEPass::FinalizePassGlobalData(LWEShaderGlobalData &GlobalData, const LWEPassPropertys &PassPropertys, LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t PassID, uint32_t SubPassID) {
	LWFrameBuffer *TargetFB = PassPropertys.m_TargetFB ? Renderer->GetFrameBuffer(PassPropertys.m_TargetFB) : nullptr;
	GlobalData.m_PassData[PassID].m_FrameSize[SubPassID] = LWVector4f(TargetFB ? TargetFB->GetSizef() : Driver->GetWindow()->GetSizef(), 0.0f, 0.0f);
	return *this;
}

LWEPass &LWEPass::PreparePass(LWVideoDriver *Driver, LWERenderer *Renderer, const LWEPassPropertys &Propertys) {
	bool bCustomViewport = Propertys.isCustomViewport();
	LWFrameBuffer *TargetFB = Propertys.m_TargetFB ? Renderer->GetFrameBuffer(Propertys.m_TargetFB) : nullptr;
	Driver->SetFrameBuffer(TargetFB, !bCustomViewport);
	if (bCustomViewport) Driver->ViewPort(Propertys.m_Viewport);
	if (Propertys.isClearColor()) Driver->ClearColor(Propertys.m_ClearColor);
	if (Propertys.isClearDepth()) Driver->ClearDepth(Propertys.m_ClearDepth);
	if (Propertys.isClearStencil()) Driver->ClearStencil(Propertys.m_ClearStencil);
	return *this;
}

bool LWEPass::PipelineBindBlock(LWPipeline *Pipeline, LWERenderer *Renderer, LWEPassResource &Block) {
	uint32_t BlockIdx = Pipeline->FindBlock(Block.m_BindNameHash);
	if (BlockIdx == -1) return false;
	if (Block.m_ResourceID) Pipeline->SetUniformBlock(BlockIdx, Renderer->GetVideoBuffer(Block.m_ResourceID), Block.m_Offset);
	else Pipeline->SetUniformBlock(BlockIdx, (LWVideoBuffer*)Block.m_Resource, Block.m_Offset);
	return true;
}

bool LWEPass::PipelineBindResource(LWPipeline *Pipeline, LWERenderer *Renderer, LWEPassResource &Resource) {
	uint32_t RscrIdx = Pipeline->FindResource(Resource.m_BindNameHash);
	if (RscrIdx == -1) return false;
	if (Resource.m_ResourceID) {
		if ((Resource.m_ResourceID & LWEPassResource::VideoBufferBit) != 0) {
			Pipeline->SetResource(RscrIdx, Renderer->GetVideoBuffer(Resource.m_ResourceID & ~LWEPassResource::VideoBufferBit), Resource.m_Offset);
		} else if ((Resource.m_ResourceID & LWEPassResource::FrameBufferBit) != 0) {
			LWFrameBuffer *FB = Renderer->GetFrameBuffer(Resource.m_ResourceID & ~LWEPassResource::FrameBufferBit);
			if (!FB) return false;
			Pipeline->SetResource(RscrIdx, FB->GetAttachment(Resource.m_Offset).m_Source);
		} else Pipeline->SetResource(RscrIdx, Renderer->GetTexture(Resource.m_ResourceID));
	} else Pipeline->SetResource(RscrIdx, (LWVideoBuffer*)Resource.m_Resource, Resource.m_Offset); //Note if the resource is a texture, the internal resource binds will correctly change this to a texture.
	return true;
}

LWPipeline *LWEPass::PreparePipeline(const LWERenderMaterial &Material, uint32_t GeometryNameBlock, LWVideoDriver *Driver, LWERenderer *Renderer, uint32_t PassIndex) {
	auto Iter = m_PipelineMap.find(Material.m_PipelineName);
	if(!LWLogCriticalIf<256>(Iter!=m_PipelineMap.end(), "Could not find pipeline with name hash: {:#x}", Material.m_PipelineName)) return nullptr;

	LWEPassPipelinePropertys &PipeProps = (*Iter).second;
	LWPipeline *Pipeline = PipeProps.m_DefaultPipeline;
	auto GeomIter = PipeProps.m_GeometryPipelineMap.find(GeometryNameBlock);
	if (GeomIter != PipeProps.m_GeometryPipelineMap.end()) {
		if (GeomIter->second) Pipeline = GeomIter->second;
	}
	for (uint32_t i = 0; i < PipeProps.m_BlockCount; i++) PipelineBindBlock(Pipeline, Renderer, PipeProps.m_BlockList[i]);
	for (uint32_t i = 0; i < PipeProps.m_ResourceCount; i++) PipelineBindResource(Pipeline, Renderer, PipeProps.m_ResourceList[i]);
	for (uint32_t i = 0; i < Material.m_TextureCount; i++) {
		const LWERenderMaterialTexture &MatTex = Material.m_TextureList[i];
		LWTexture *Tex = Renderer->GetTexture(MatTex.m_TextureID);
		uint32_t ResourceID = Pipeline->FindResource(MatTex.m_ResourceName);
		if(ResourceID==-1) continue;
		if (Tex) Tex->SetTextureState(MatTex.m_TextureState);
		Pipeline->SetResource(ResourceID, Tex);
	}
	Pipeline->SetUniformBlock("LWEGlobal", Renderer->GetFrameGlobalDataBuffer());
	Pipeline->SetPaddedUniformBlock<LWEShaderPassData>("LWEPass", Renderer->GetFramePassDataBuffer(), PassIndex, Driver);
	Pipeline->SetResource("LWEModel", Renderer->GetFrameModelDataBuffer());
	Pipeline->SetResource("LWEBone", Renderer->GetFrameBoneDataBuffer());
	Pipeline->SetResource("LWELights", Renderer->GetFrameLightDataBuffer());
	return Pipeline;
}

LWEPass &LWEPass::SetPassID(uint32_t PassID) {
	m_PassID = PassID;
	return *this;
}

const LWEPassPropertys &LWEPass::GetPropertys(void) const {
	return m_Propertys;
}

LWEPassPipelinePropertys *LWEPass::FindPipeline(const LWUTF8I &Name, bool Verbose) {
	auto Iter = m_PipelineMap.find(Name.Hash());
	if (Iter == m_PipelineMap.end()) {
		if (Verbose) LWLogWarn<256>("could not find pipeline with name {}", Name);
		return nullptr;
	}
	return &(*Iter).second;
}

LWEPassPipelinePropertys *LWEPass::FindPipeline(uint32_t NameHash, bool Verbose) {
	auto Iter = m_PipelineMap.find(NameHash);
	if (Iter == m_PipelineMap.end()) {
		if (Verbose) LWLogWarn<256>("could not find pipeline with hash {:#x}", NameHash);
		return nullptr;
	}
	return &(*Iter).second;
}

uint32_t LWEPass::GetNameHash(void) const {
	return m_NameHash;
}

uint32_t LWEPass::GetFrameID(void) const {
	return m_FrameID;
}

uint32_t LWEPass::GetPassID(void) const {
	return m_PassID;
}

bool LWEPass::isDisabled(void) const {
	return (m_Flag & Disaabled) != 0;
}