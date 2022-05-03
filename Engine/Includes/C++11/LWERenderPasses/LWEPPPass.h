#ifndef LWEPPPASS_H
#define LWEPPPASS_H
#include "LWERenderPass.h"

//LWEPostProcessPass:
class LWEPPPass : public LWEPass {
public:
	static const uint32_t MaxSubPasses = 16;

	/*!< \brief PostProcessPass, no additional parameters beyond the default are required.
		 \optional children: (SubPass, Pipeline(Name of bound pipelines to pass), binds Node's pass propertys as defaults, then take's pass Property overrides, and does another PP pass with the default bound pipeline, if a subpass is included, then the default pass is not used.)).
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

	bool PushSubPass(uint32_t PipelineName, const LWEPassPropertys &SubPass);

protected:
	uint32_t m_SubPassPipeline[MaxSubPasses];
	LWEPassPropertys m_SubPassList[MaxSubPasses];
	LWVideoBuffer *m_ScreenPPVertices;
	uint32_t m_SubPassCount = 0;
};

#endif