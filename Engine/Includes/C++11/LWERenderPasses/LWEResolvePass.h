#ifndef LWERESOLVEPASS_H
#define LWERESOLVEPASS_H
#include "LWERenderPass.h"

//LWEPostProcessPass:
struct LWEResolvable {
	uint32_t m_SourceID = 0;
	uint32_t m_SourceAttachment = 0;
	uint32_t m_TargetID = 0;
	uint32_t m_TargetAttachment = 0;
	uint32_t m_MipLevel = 0;
};

class LWEResolvePass : public LWEPass {
public:

	/*!< \brief ResolvePass, no additional parameters beyond the default are required.
		 \optional children: (Resolve, Target(Required, Texture target to resolve into), Source(Required, MSAA texture to resolve into Target)).
	*/
	static LWEPass *ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	virtual uint32_t RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex);

	virtual LWEPass &WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator);

	virtual uint32_t InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator);

	virtual void DestroyPass(LWVideoDriver *Driver, bool DestroySelf = true);

	virtual LWEPass &InitializeFrame(LWERenderFrame &Frame);

	virtual LWEPass &PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver);

	virtual LWEPass &PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver);

	virtual LWEPass &CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	virtual LWEPass &ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver);

	bool PushResolve(const LWEResolvable &Resolve);

protected:
	std::vector<LWEResolvable> m_ResolveList;
};


#endif