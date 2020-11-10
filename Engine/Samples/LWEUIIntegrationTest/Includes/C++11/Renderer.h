#ifndef RENDERER_H
#define RENDERER_H
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWETypes.h>
#include <LWEUIManager.h>

struct GFrame {
	static const uint32_t MaxUIVertices = 1024 * 6;
	LWEUIFrame m_UIFrame;
	
	void Initalize();

	void Release(LWVideoDriver *Driver);

	GFrame(LWVideoDriver *Driver, LWAllocator &Allocator);

	GFrame() = default;
};

class Renderer {
public:
	static const uint32_t MaxFrames = 3;

	GFrame *BeginFrame(void);

	void EndFrame(void);

	void SizeUpdate(LWWindow *Window);

	void ApplyFrame(GFrame &F);

	void RenderUIFrame(LWEUIFrame &F);

	void RenderFrame(GFrame &F);

	void Render(LWWindow *Window);

	void LoadAssets(LWEAssetManager *AM, LWAllocator &Allocator);

	Renderer(LWVideoDriver *Driver, LWAllocator &Allocator);

	~Renderer();
private:
	GFrame m_Frames[MaxFrames];
	LWVideoDriver *m_Driver = nullptr;
	LWVideoBuffer *m_UIUniform = nullptr;
	LWShader *m_UIColorShader = nullptr;
	LWShader *m_UITextureShader = nullptr;
	LWShader *m_UIFontShader = nullptr;
	LWPipeline *m_UIPipeline = nullptr;
	uint32_t m_ReadFrame = 0;
	uint32_t m_WriteFrame = 0;
	bool m_SizeChanged = true;
};

#endif