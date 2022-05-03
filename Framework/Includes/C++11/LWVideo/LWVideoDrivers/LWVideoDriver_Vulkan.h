#ifndef LWVIDEODRIVER_VULKAN_H
#define LWVIDEODRIVER_VULKAN_H
#include "LWVideo/LWVideoDriver.h"
#include "LWPlatform/LWPlatform.h"

#ifdef LWVIDEO_IMPLEMENTED_VULKAN

class LWVideoDriver_Vulkan : public LWVideoDriver {
public:

	static bool DebugResult(const char *ErrorMessage, VkResult Result, bool Verbose);

	static LWVideoDriver_Vulkan *MakeVideoDriver(LWWindow *Window, uint32_t Type);

	static bool DestroyVideoContext(LWVideoDriver_Vulkan *Driver);

	virtual bool Update(void);

	virtual LWVideoDriver &ClearColor(uint32_t Color);

	virtual LWVideoDriver &ClearColor(const LWVector4f &Color);

	virtual LWVideoDriver &ClearDepth(float Depth);

	virtual LWVideoDriver &ClearStencil(uint8_t Stencil);

	virtual LWVideoDriver &ViewPort(const LWVector4i &Viewport);

	virtual bool SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias);

	virtual bool SetFrameBuffer(LWFrameBuffer *Buffer, bool ChangeViewport = false);

	virtual bool SetPipeline(LWPipeline *Pipeline, LWPipelineInputStream *InputStream, LWVideoBuffer *IndiceBuffer, LWVideoBuffer *IndirectBuffer);

	virtual LWVideoDriver &Present(uint32_t SwapInterval);

	virtual LWShader *CreateShader(uint32_t ShaderType, const LWUTF8Iterator &Source, LWAllocator &Allocator, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen);

	virtual LWShader *CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledLen, LWAllocator &Allocator, char8_t *ErrorBuffer, uint32_t ErroBufferLen);

	virtual LWTexture *CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture2DMS(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator);

	virtual LWTexture *CreateTexture2DMSArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, uint32_t Layers, LWAllocator &Allocator);

	virtual LWVideoBuffer *CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer);

	virtual LWPipeline *CreatePipeline(LWShader **Stages, uint64_t Flag, LWAllocator &Allocator);

	virtual LWFrameBuffer *CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator);

	virtual bool UpdateTexture(LWTexture *Texture);

	virtual bool UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size);

	virtual bool UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length, uint32_t Offset=0);

	virtual void *MapVideoBuffer(LWVideoBuffer *VideoBuffer, uint32_t Length = 0, uint32_t Offset = 0);

	virtual bool UnmapVideoBuffer(LWVideoBuffer *VideoBuffer);

	virtual bool DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, uint8_t *Buffer);

	virtual bool DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	virtual bool DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	virtual bool DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer);

	virtual bool DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length);

	virtual LWVideoDriver &DestroyPipeline(LWPipeline *Pipeline);

	virtual LWVideoDriver &DestroyVideoBuffer(LWVideoBuffer *Buffer);

	virtual LWVideoDriver &DestroyShader(LWShader *Shader);

	virtual LWVideoDriver &DestroyTexture(LWTexture *Texture);

	virtual LWVideoDriver &DestroyFrameBuffer(LWFrameBuffer *FrameBuffer);

	virtual LWVideoDriver &DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWPipelineInputStream *InputStreams, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t Offset = 0);

	virtual LWVideoDriver &DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWPipelineInputStream *InputStreams, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t InstanceCount = 0, uint32_t Offset = 0);

	virtual LWVideoDriver &DrawIndirectBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWPipelineInputStream *InputStreams, LWVideoBuffer *IndexBuffer, LWVideoBuffer *IndirectBuffer, uint32_t IndirectCount, uint32_t IndirectOffset = 0);

	virtual LWVideoDriver &Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension);

	/*!< \brief returns driver specific context information, which should not be needed by most common applications. */
	LWVulkan_Context &GetContext(void);

	LWVideoDriver_Vulkan(LWWindow *Window, LWVulkan_Context &Context, uint32_t UniformBlockSize);
private:
	virtual LWPipeline *CreatePipeline(LWPipeline *Source, LWAllocator &Allocator);

	virtual LWVideoDriver &ClonePipeline(LWPipeline *Target, LWPipeline *Source);

	LWVulkan_Context m_Context;
};


#endif
#endif
