#ifndef LWERENDERPASS_H
#define LWERENDERPASS_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWUnicodeIterator.h>
#include <LWVideo/LWPipeline.h>
#include <LWVideo/LWFrameBuffer.h>
#include <LWEUI/LWEUI.h>
#include <LWEXML.h>
#include "LWERenderTypes.h"

struct LWEPassResource {
	static const uint32_t VideoBufferBit = 0x40000000;
	static const uint32_t FrameBufferBit = 0x80000000;

	void *m_Resource = nullptr;
	uint32_t m_ResourceID = 0; //Non zero resource id mean's looking up id in Renderer.
	uint32_t m_BindNameHash = LWUTF8I::EmptyHash; //Name of the resource to bind to.
	uint32_t m_Offset = 0;

	LWEPassResource(const LWUTF8Iterator &BindName, LWVideoBuffer *Buffer, uint32_t Offset = 0);

	LWEPassResource(const LWUTF8Iterator &BindName, LWTexture *Texture);

	LWEPassResource(const LWUTF8Iterator &BindName, uint32_t ResourceID, uint32_t Offset = 0);

	LWEPassResource(uint32_t BindNameHash, LWVideoBuffer *Buffer, uint32_t Offset = 0);

	LWEPassResource(uint32_t BindNameHash, LWTexture *Texture);

	LWEPassResource(uint32_t BindNameHash, uint32_t ResourceID, uint32_t Offset = 0);

	LWEPassResource() = default;
};

struct LWEPassPipelinePropertys {

	bool PushBlock(const LWEPassResource &Block);

	bool PushResource(const LWEPassResource &Resource);

	LWEPassPipelinePropertys(LWPipeline *DefaultPipeline);

	LWEPassPipelinePropertys() = default;

	LWEPassResource m_BlockList[LWShader::MaxBlocks];
	LWEPassResource m_ResourceList[LWShader::MaxResources];
	std::unordered_map<uint32_t, LWPipeline*> m_GeometryPipelineMap;
	LWPipeline *m_DefaultPipeline = nullptr;
	uint32_t m_BlockCount = 0;
	uint32_t m_ResourceCount = 0;
};

struct LWEPassPropertys {
	static const uint32_t ClearColor = 0x1;
	static const uint32_t ClearDepth = 0x2;
	static const uint32_t ClearStencil = 0x4;
	static const uint32_t CustomViewport = 0x8;

	bool isClearColor(void) const;

	bool isClearDepth(void) const;

	bool isClearStencil(void) const;

	bool isCustomViewport(void) const;

	LWVector4i m_Viewport = LWVector4i();
	uint32_t m_TargetFB = 0;
	uint32_t m_ClearColor = 0x0;
	float m_ClearDepth = 1.0f;
	uint32_t m_Flag = 0;
	uint8_t m_ClearStencil = 0;
};

class LWEPass {
public:
	static const uint32_t Disaabled = 0x1; //Disabled pass's partake in all settings except RenderPass.

	/*!< \brief parse's pass attributes, geometry bucket's also have pass property(they inherit the overall pass's propertys by default). 
		 ClearColor - color to clear to, excluding will not clear the color of the framebuffer/swapchain, if inheriting from a parent's propertys, leaving this value-less will remove the clearcolor flag.
		 ClearDepth - depth to clear to, excluding will not clear the depth of the framebuffer/swapchain, if inheriting from a parent's propertys, leaving this value-less will remove the cleardepth flag.
		 ClearStencil - stencil to clear to, excluding will not clear the stencil of the framebuffer/swapchain, if inheriting from a parent's propertys, leaving this value-less will remove the clear stencil flag.
		 Viewport - x|y|width|height, values for custom viewport to set to, otherwise the viewport of the framebuffer/window will be used.
		 Target - Named framebuffer target to use for the pass.

	*/
	static bool ParseXMLPassPropertys(LWEXMLNode *Node, LWEPassPropertys &Propertys, LWERenderer *Renderer);

	/*!< \brief search's for video buffer resource's only(VideoBufferName:Offset(in bytes)). first in Renderer, then in AssetManager. */
	static bool ParseXMLPipelineBlock(LWEXMLAttribute &Attr, LWEPassResource &Block, LWEAssetManager *AssetManager, LWERenderer *Renderer);

	/*!< \brief search's for video buffer(VideoBufferName:Offset(in bytes)), or texture resource's(TextureName), or framebuffer textures(FrameBufferName:AttachmentPnt). first is Renderer then in assetManager*/
	static bool ParseXMLPipelineResource(LWEXMLAttribute &Attr, LWEPassResource &Resource, LWEAssetManager *AssetManager, LWERenderer *Renderer);

	/* !< \brief Parses pass attributes: "Name"(Name of pass) + Pass Propertys.
		  Disabled - No value attribute which indicates pass should start in a disabled state.
	*/
	static bool ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	/*!< \brief parse's each pass pipeline(or in-line pipeline(pipeline name is default to UTF8I::EmptyString hash)), Name(Required when InPassLine is false, name of pipeline in the map.), Pipeline(AssetMap named pipeline), PixelShader(Name of pixel shader to use for this pipeline), Extra attributes are considered name of block geometry=Vertex shader to use when that block geometry is in use, if InPassLine is set to true, will not verbose if the named block is not found(as it's likely refering to the pass property itself).
	*		    Children nodes: 
				ResourceMap(Named list of ShaderResourceName=ResourceName(AssetMap texture, AssetMap buffer, Named framebuffer:Attachment Point, Named Texture, Named Buffer:Offset))
				BlockMap(named list of ShaderBlockName=BlockName(AssetMap buffer's(or named buffer), :offset in bytes). 
		 \note if a pipeline descriptor is to change the bound shader of the pipeline, then it will stay changed unless changed by another pipeline binding.
	*/
	static bool ParseXMLPipelinePropertys(LWEXMLNode *Node, uint32_t &PipelineNameHash, LWEPassPipelinePropertys &Desc, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator, bool InPassLine = false);

	/* !< \brief Parses following children for each pass:
		  Pipeline, Attributes: Name(Name of the pipeline to grab).
		  -Children: Blocks, List of named blocks to bind to the pipeline when the pass starts, pointing to the buffer/texture/resource from AssetManager, or Renderer.  adding a :x to an attribute value signifies offset into the buffer.
					 Resources, List of named resources to bind to the pipeline when the pass starts, pointing to the buffer/texture/framebuffer/resource from AssetManager, or Renderer.
	*/
	static bool ParseXMLChild(LWEXMLNode *cNode, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	static bool PipelineBindBlock(LWPipeline *Pipeline, LWERenderer *Renderer, LWEPassResource &Block);

	static bool PipelineBindResource(LWPipeline *Pipeline, LWERenderer *Renderer, LWEPassResource &Resource);

	bool AddPipeline(const LWUTF8Iterator &PipelineName, const LWEPassPipelinePropertys &PipelinePropertys);

	bool AddPipeline(uint32_t PipelineNameHash, const LWEPassPipelinePropertys &PipelinePropertys);

	LWEPass &SetPropertys(const LWEPassPropertys &Propertys);

	LWEPass &SetName(const LWUTF8Iterator &Name);

	LWEPass &SetFrameID(uint32_t ID);

	LWEPass &SetDisabled(bool bDisabled);

	LWEPass &SetPassID(uint32_t PassID);

	LWEPass &PreparePass(LWVideoDriver *Driver, LWERenderer *Renderer, const LWEPassPropertys &Propertys);

	LWEPass &FinalizePassGlobalData(LWEShaderGlobalData &GlobalData, const LWEPassPropertys &PassPropertys, LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t PassID, uint32_t SubPassID = 0);

	LWPipeline *PreparePipeline(const LWERenderMaterial &Material, uint32_t GeometryNameBlock, LWVideoDriver *Driver, LWERenderer *Renderer, uint32_t SubPassIndex);

	virtual uint32_t RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex) = 0;

	//Return the number of geometry bucket's used by the pass(even the pass does not use geometry buckets, then return 1)
	virtual uint32_t InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator) = 0;

	virtual LWEPass &WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator) = 0;

	virtual void DestroyPass(LWVideoDriver *Driver, bool DestroySelf = true) = 0;

	virtual LWEPass &InitializeFrame(LWERenderFrame &Frame) = 0;

	virtual LWEPass &PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver) = 0;
	
	virtual LWEPass &PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver) = 0;

	virtual LWEPass &CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator) = 0;

	virtual LWEPass &ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver) = 0;

	LWEPassPipelinePropertys *FindPipeline(const LWUTF8I &Name, bool Verbose = true);

	LWEPassPipelinePropertys *FindPipeline(uint32_t NameHash, bool Verbose = true);

	const LWEPassPropertys &GetPropertys(void) const;

	uint32_t GetNameHash(void) const;

	uint32_t GetFrameID(void) const;

	uint32_t GetPassID(void) const;

	bool isDisabled(void) const;

	LWEPass() = default;

protected:
	std::unordered_map<uint32_t, LWEPassPipelinePropertys> m_PipelineMap;
	LWEPassPropertys m_Propertys;
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	uint32_t m_Flag = 0;
	uint32_t m_FrameID = -1;
	uint32_t m_PassID = 0;
};

#endif