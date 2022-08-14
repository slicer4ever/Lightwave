#ifndef LWEGEOMETRYPASS_H
#define LWEGEOMETRYPASS_H
#include "LWERenderPass.h"


struct LWEBucketPropertys {
	LWBitField32(PassBitIDBits, 8, 0);
	static const uint32_t PrimarySource = 0x1000000;

	LWEPassPropertys m_PassPropertys;
	uint32_t m_BucketFlags = 0;
	uint32_t m_Flags = 0;

	bool isPrimarySource(void) const;

	uint32_t GetPassBitID(void) const;

	LWEBucketPropertys(const LWEPassPropertys &PassProps, uint32_t BucketFlags, uint32_t PassBitID);

	LWEBucketPropertys() = default;
};

//LWEGeometryPass:
class LWEGeometryPass : public LWEPass {
public:
	/*!< \brief parse's a bucket and add's it to the list, bucket pass propertys default to the overall pass propertys, who's propertys can be overridden here.
	*	 OpaqueSort (Optional, ByStates, FrontToBack, BackToFront, None)
	*	 TransparentSort (Optional, ByStates, FrontToBack, BackToFront, None)
	*	 Primary (Non-value attribute that indicates the bucket is considered the primary bucket used when considering which shadow castors to draw.
	*	 PassBitID (id of the geometry pass this bucket is associated with, if not defined uses default pass id set by parent xml node).
	*/
	static bool ParseXMLBucketPropertys(LWEXMLNode *Node, LWEBucketPropertys &BucketProps, LWERenderer *Renderer);

	//Unique Attributes: PassBitID(default: 0, the default geometry pass id for this pass(id bit = 1<<id))
	static LWEPass *ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	LWPipeline *PrepareRendablePipeline(const LWEGeometryRenderable &Rendable, const LWERenderMaterial &Material, LWVideoBuffer *&IndiceBuffer, uint32_t &RenderCount, LWPipelineInputStream InputStream[LWShader::MaxInputs], LWVideoDriver *Driver, LWERenderer *Renderer, uint32_t SubPassIdx, uint32_t SubPassOffset);

	virtual uint32_t RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex);

	virtual uint32_t InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator);

	virtual LWEPass &WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator);

	virtual void DestroyPass(LWVideoDriver *Driver, bool DestroySelf = true);

	virtual LWEPass &InitializeFrame(LWERenderFrame &Frame);

	virtual LWEPass &PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver);

	virtual LWEPass &PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver);

	virtual LWEPass &CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	virtual LWEPass &ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver);

	LWEGeometryPass &SetGeometryBucketCount(uint32_t BucketCount);

	LWEGeometryPass &SetGeometryBucketPropertys(uint32_t BucketIdx, const LWEBucketPropertys &Props);

	uint32_t GetGeometryBucketCount(void) const;

	LWEGeometryPass(uint32_t BucketCount);

	LWEGeometryPass() = default;
protected:
	LWEBucketPropertys m_GeometryPropertys[LWEMaxGeometryBuckets];
	LWVideoBuffer *m_IndirectBuffer[LWEMaxGeometryBuckets] = {};
	LWVideoBuffer *m_IDBuffer[LWEMaxGeometryBuckets] = {};
	uint32_t m_GeometryBucketCount = 0;
	uint32_t m_GeometryPassIdx = 0;
};

#endif