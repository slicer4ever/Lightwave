#include "LWERenderPasses/LWEResolvePass.h"
#include "LWERenderer.h"
#include "LWELogger.h"

LWEPass *LWEResolvePass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	LWEResolvePass *RPass = Pass ? (LWEResolvePass*)Pass : Allocator.Create<LWEResolvePass>();
	if (!LWEPass::ParseXML(Node, RPass, Renderer, AssetManager, Allocator)) {
		LWELogCritical<256>("could not create pass '{}'", Node->GetName());
		if (!Pass) LWAllocator::Destroy(RPass);
		return nullptr;
	}
	auto FindRenderID = [&Renderer](const LWUTF8Iterator &Name, uint32_t &Attachment) -> uint32_t {
		LWUTF8Iterator SplitList[2];
		uint32_t SplitCnt = Name.SplitToken(SplitList, 2, ':');
		uint32_t RenderID = 0; //Search renderer:
		RenderID = Renderer->FindNamedTexture(SplitList[0], false);
		if (RenderID) return RenderID;
		else {
			RenderID = Renderer->FindNamedFrameBuffer(SplitList[0], false);
			if (RenderID) {
				Attachment = SplitCnt == 2 ? SplitList[1].CompareList("Color", "Color1", "Color2", "Color3", "Color4", "Color5", "Depth") : 0;
				if (Attachment == -1) {
					LWELogCritical<256>("Framebuffer attachment specifier is incorrectly named: {}", SplitList[1]);
					return 0;
				}
				return RenderID | LWEPassResource::FrameBufferBit;
			}
		}
		return 0;
	};

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t NodeID = C->GetName().CompareList("Resolve");
		if (NodeID == 0) {
			LWEXMLAttribute *SourceAttr = C->FindAttribute("Source");
			LWEXMLAttribute *TargetAttr = C->FindAttribute("Target");
			LWEXMLAttribute *MipmapLevelAttr = C->FindAttribute("MipmapLevel");
			if (!SourceAttr) {
				LWELogCritical<256>("Could not find attribute 'Source' for ResolvePass.");
				continue;
			}
			if (!TargetAttr) {
				LWELogCritical<256>("Could not find attribute 'Target' for ResolvePass.");
				continue;
			}
			LWEResolvable Res;
			if (!(Res.m_SourceID = FindRenderID(SourceAttr->GetValue(), Res.m_SourceAttachment))) {
				LWELogCritical<256>("Could not find Source '{}'.", SourceAttr->GetValue());
				continue;
			}
			if (!(Res.m_TargetID = FindRenderID(TargetAttr->GetValue(), Res.m_TargetAttachment))) {
				LWELogCritical<256>("Could not find Target '{}'.", TargetAttr->GetValue());
				continue;
			}
			if (MipmapLevelAttr) Res.m_MipLevel = (uint32_t)atoi(MipmapLevelAttr->GetValue().c_str());
			RPass->PushResolve(Res);
		}else ParseXMLChild(C, RPass, Renderer, AssetManager, Allocator);
	}
	return RPass;
}

uint32_t LWEResolvePass::RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex) {
	for (auto &&R : m_ResolveList) {
		LWTexture *Source = nullptr;
		LWTexture *Dest = nullptr;
		if ((R.m_SourceID & LWEPassResource::FrameBufferBit) != 0) {
			LWFrameBuffer *SrcFB = Renderer->GetFrameBuffer(R.m_SourceID & ~LWEPassResource::FrameBufferBit);
			assert(SrcFB != nullptr);
			Source = SrcFB->GetAttachment(R.m_SourceAttachment).m_Source;
		} else Source = Renderer->GetTexture(R.m_SourceID);
		assert(Source != nullptr);
		if ((R.m_TargetID & LWEPassResource::FrameBufferBit) != 0) {
			LWFrameBuffer *TarFB = Renderer->GetFrameBuffer(R.m_TargetID & ~LWEPassResource::FrameBufferBit);
			assert(TarFB != nullptr);
			Dest = TarFB->GetAttachment(R.m_TargetAttachment).m_Source;
		} else Dest = Renderer->GetTexture(R.m_TargetID);
		assert(Dest != nullptr);
		Driver->ResolveMSAA(Source, Dest, R.m_MipLevel);
	}
	return 0;
}

LWEPass &LWEResolvePass::WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator) {
	return *this;
}

uint32_t LWEResolvePass::InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator) {
	return 0;
}

void LWEResolvePass::DestroyPass(LWVideoDriver *Driver, bool DestroySelf) {
	if (DestroySelf) LWAllocator::Destroy(this);
}


LWEPass &LWEResolvePass::InitializeFrame(LWERenderFrame &Frame) {
	return *this;
}

LWEPass &LWEResolvePass::PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver) {
	return *this;
}

LWEPass &LWEResolvePass::PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver) {
	return *this;
}

LWEPass &LWEResolvePass::CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	return *this;
}

LWEPass &LWEResolvePass::ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver) {
	return *this;
}

bool LWEResolvePass::PushResolve(const LWEResolvable &Resolve) {
	m_ResolveList.push_back(Resolve);
	return true;
}
