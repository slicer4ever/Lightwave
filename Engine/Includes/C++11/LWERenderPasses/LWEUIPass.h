#ifndef LWEUIPASS_H
#define LWEUIPASS_H
#include "LWERenderPass.h"

//LWEUIPass:
struct LWEUIPassFrameData : public LWEPassFrameData {
	LWEUIFrame m_UIFrame;
};

class LWEUIPass : public LWEPass {
public:
	static const uint32_t MaxUIItems = 4096;

	/*!< \brief UIPass has the following additional children nodes:
	*	       -UIShaders: Color=UIColorShaderName, Texture=UITextureShaderName, Font=UIFontShaderName.
	*/
	static LWEPass *ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	virtual uint32_t RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t PassIndex);

	virtual LWEPass &WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator);

	virtual uint32_t InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator);

	virtual void DestroyPass(LWVideoDriver *Driver, bool DestroySelf = true);

	virtual LWEPass &InitializeFrame(LWERenderFrame &Frame);

	virtual LWEPass &PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver);

	virtual LWEPass &PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver);

	virtual LWEPass &CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	virtual LWEPass &ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver);

	LWEUIPass &SetColorShader(LWShader *Shader);

	LWEUIPass &SetTextureShader(LWShader *Shader);

	LWEUIPass &SetFontShader(LWShader *Shader);

	LWEUIFrame &GetUIFrame(LWERenderFrame &Frame);

	const LWShader *GetColorShader(void) const;

	const LWShader *GetTextureShader(void) const;

	const LWShader *GetFontShader(void) const;

	LWEUIPass() = default;
protected:
	LWVideoBuffer *m_UIBuffer = nullptr;
	LWShader *m_ColorShader = nullptr;
	LWShader *m_TextureShader = nullptr;
	LWShader *m_FontShader = nullptr;
};



#endif
