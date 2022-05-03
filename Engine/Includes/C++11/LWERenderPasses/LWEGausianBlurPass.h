#ifndef LWEGAUSIANBLURPASS_H
#define LWEGAUSIANBLURPASS_H
#include "LWERenderPasses/LWEPPPass.h"

//LWEPPGausianBlur
struct LWEGausianPassPropertys {
	LWEPassResource m_Source;
	float m_Radius = 1.0f;
	float m_Intensity = 1.0f;
	bool m_Horizontal = true;

	LWEGausianPassPropertys(LWEPassResource &Source, bool bHorizontal = true, float Intensity = 1.0f, float Radius = 1.0f);

	LWEGausianPassPropertys() = default;
};

//simple 5 tap gausian blur kernel.  shader source is LWEGausianBlur(Vertex/Pixel);
class LWEGausianBlurPass : public LWEPPPass {
public:
	static const LWVector4f Kernel;
	static const float KernelSize;
	//Parse's normal pass xml, with child nodes of each subpass is like the PostProcess Pass subpass, with addtional parameters: Source(Renderer FrameBuffer:Attachment, or Texture to use(set's the resource name 'BlurTex' in the default pipeline).  Non-valued either Horizontal or Vertical attribute to indicate which direction to blur in.
	//Each sub pass can have the following Attributes: Intensity(Default: 1.0, Scalar factor to bluring).  Radius(Default: 1.0, Radius multiplier for directional steps when sampling the source texture).  As well the GaussianPass node itself can define these propertys as defaults to use for all subpasses.
	static LWEPass *ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	virtual uint32_t RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex);

	virtual LWEPass &PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver);

	LWEGausianPassPropertys &GetSubPassGausPropertys(uint32_t SubPassID);

	const LWEGausianPassPropertys &GetSubPassGausPropertys(uint32_t SubPassID) const;

	bool PushSubPass(const LWEPassPropertys &SubPass, const LWEGausianPassPropertys &GausProps);
protected:
	LWEGausianPassPropertys m_SubPassGausPropertys[MaxSubPasses];
};

#endif