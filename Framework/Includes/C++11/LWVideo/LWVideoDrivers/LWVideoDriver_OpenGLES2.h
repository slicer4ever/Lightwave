#ifndef LWVIDEODRIVER_OPENGLES2_H
#define LWVIDEODRIVER_OPENGLES2_H
#include "LWVideo/LWVideoDriver.h"
#include "LWPlatform/LWPlatform.h"

#ifdef LWVIDEO_IMPLEMENTED_OPENGLES2
/*! \cond */

class LWVideoDriver_OpenGLES2 : public LWVideoDriver {
public:

	static LWVideoDriver_OpenGLES2 *MakeVideoDriver(LWWindow *Window, uint32_t Type);

	static bool DestroyVideoContext(LWVideoDriver_OpenGLES2 *Driver);

	virtual bool Update(void);

	virtual LWVideoDriver &Present(uint32_t SwapInterval);

	virtual LWVideoDriver &ClearColor(uint32_t Color);

	virtual LWVideoDriver &ClearDepth(float Depth);

	virtual LWVideoDriver &ClearStencil(uint8_t Stencil);

	virtual LWVideoDriver &ViewPort(const LWVector4i &Viewport);

	virtual bool SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias);

	virtual bool SetFrameBuffer(LWFrameBuffer *Buffer);

	virtual bool SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VerticeStride, uint32_t Offset);

	virtual LWShader *CreateShader(uint32_t ShaderType, const char *Source, LWAllocator &Allocator, char *CompiledBuffer, char *ErrorBuffer, uint32_t CompiledBufferLen, uint32_t ErrorBufferLen) = 0;

	virtual LWShader *CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErroBufferLen);

	virtual LWTexture *CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTextureCubeMapArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator);

	virtual LWVideoBuffer *CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer);

	virtual LWPipeline *CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint32_t Flags, uint64_t StencilFlags, LWAllocator &Allocator);

	virtual LWPipeline *CreatePipeline(LWShader *ComputeShader, LWAllocator &Allocator);

	virtual LWFrameBuffer *CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator);

	virtual bool UpdateTexture(LWTexture *Texture);

	virtual bool UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size);

	virtual bool UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTextureCubeMapArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length);

	virtual bool DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, uint8_t *Buffer);

	virtual bool DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	virtual bool DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	virtual bool DownloadTextureCubeMapArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer);

	virtual bool DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length);

	virtual LWVideoDriver &DestroyPipeline(LWPipeline *Pipeline);

	virtual LWVideoDriver &DestroyVideoBuffer(LWVideoBuffer *Buffer);

	virtual LWVideoDriver &DestroyShader(LWShader *Shader);

	virtual LWVideoDriver &DestroyTexture(LWTexture *Texture);

	virtual LWVideoDriver &DestroyFrameBuffer(LWFrameBuffer *FrameBuffer);

	virtual LWVideoDriver &DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset = 0);

	virtual LWVideoDriver &DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStide, uint32_t InstanceCount = 0, uint32_t Offset = 0);

	virtual LWVideoDriver &Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension);

	LWOpenGLES2Context &GetContext(void);

	LWVideoDriver_OpenGLES2(LWWindow *Window, LWOpenGLES2Context &Context, int32_t DefaultFrameBuffer, uint32_t UniformBlockSize);

private:
	LWOpenGLES2Context m_Context;
	int32_t m_DefaultFrameBuffer;
};
/*! \endcond */
#endif
#endif