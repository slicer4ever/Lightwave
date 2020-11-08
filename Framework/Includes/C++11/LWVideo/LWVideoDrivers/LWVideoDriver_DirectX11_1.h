#ifndef LWVIDEODRIVER_DIRECTX11_H
#define LWVIDEODRIVER_DIRECTX11_H
#include "LWVideo/LWVideoDriver.h"
#include "LWPlatform/LWPlatform.h"
#ifdef LWVIDEO_IMPLEMENTED_DIRECTX11
#include <unordered_map>
/*! \cond */

class LWVideoDriver_DirectX11_1 : public LWVideoDriver{
public:

	static LWVideoDriver_DirectX11_1 *MakeVideoDriver(LWWindow *Window, uint32_t Type);

	static bool DestroyVideoContext(LWVideoDriver_DirectX11_1 *Driver);

	virtual bool Update(void);

	virtual LWVideoDriver &Present(uint32_t SwapInterval);

	virtual LWVideoDriver &ClearColor(uint32_t Color);

	virtual LWVideoDriver &ClearColor(const LWVector4f &Color);

	virtual LWVideoDriver &ClearDepth(float Depth);

	virtual LWVideoDriver &ClearStencil(uint8_t Stencil);

	virtual LWVideoDriver &ViewPort(const LWVector4i &Viewport);

	virtual bool SetFrameBuffer(LWFrameBuffer *FrameBuffer, bool ChangeViewport = false);

	virtual bool SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VerticeStride, uint32_t Offset);

	virtual bool SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias);

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

	virtual LWPipeline *CreatePipeline(LWShader **ShaderStages, uint64_t Flags, LWAllocator &Allocator);

	virtual LWVideoBuffer *CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer);

	virtual LWFrameBuffer *CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator);

	virtual bool UpdateTexture(LWTexture *Texture);

	virtual bool UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size);

	virtual bool UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length);

	virtual bool DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	virtual bool DownloadTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, uint8_t *Buffer);

	virtual bool DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	virtual bool DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	virtual bool DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer);

	virtual bool DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length);

	virtual LWVideoDriver &DestroyVideoBuffer(LWVideoBuffer *Buffer);

	virtual LWVideoDriver &DestroyShader(LWShader *Shader);

	virtual LWVideoDriver &DestroyTexture(LWTexture *Texture);

	virtual LWVideoDriver &DestroyPipeline(LWPipeline *Pipeline);

	virtual LWVideoDriver &DestroyFrameBuffer(LWFrameBuffer *FrameBuffer);

	virtual LWVideoDriver &DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset = 0);

	virtual LWVideoDriver &DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount = 0, uint32_t Offset = 0);

	virtual LWVideoDriver &Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension);

	LWDirectX11_1Context &GetContext(void);

	LWVideoDriver_DirectX11_1(LWWindow *Window, LWDirectX11_1Context &Context, uint32_t UniformBlockSize);

private:
	virtual LWPipeline *CreatePipeline(LWPipeline *Source, LWAllocator &Allocator);

	virtual LWVideoDriver &ClonePipeline(LWPipeline *Target, LWPipeline *Source);

	LWDirectX11_1Context m_Context;
};

/*!< \brief this context is the underlying video context used for a shader object.  the application should never require accessing it directly. */
struct LWDirectX11_1ShaderResource {
	uint32_t m_Name = 0;
	uint32_t m_Type = 0;
	uint32_t m_Length = 0;
};

/*!< \brief this context is the underlying video context used for a shader object. the application should never require accessing it directly.  */
struct LWDirectX11_1ShaderContext {
	static const uint32_t MaxResources = 32;
	static const uint32_t MaxBlocks = 32;
	static const uint32_t MaxInputs = 32;
	union {
		ID3D11VertexShader *m_VertexShader;
		ID3D11PixelShader *m_PixelShader;
		ID3D11GeometryShader *m_GeometryShader;
		ID3D11ComputeShader *m_ComputeShader;
	};
	ID3D11InputLayout *m_InputLayout = nullptr;
	LWDirectX11_1ShaderResource m_ResourceList[MaxResources];
	LWDirectX11_1ShaderResource m_BlockList[MaxBlocks];
	LWDirectX11_1ShaderResource m_InputList[MaxInputs];
	uint32_t m_ResourceCount = 0;
	uint32_t m_BlockCount = 0;
	uint32_t m_InputCount = 0;
};

/*!< \brief this context is the underlying video context used for a video buffer object.  The application should never require accessing it directly, but it is provided here incase the application is specifically targeting the directX api. */
struct LWDirectX11_1BufferContext {
	ID3D11Buffer *m_Buffer;
	ID3D11UnorderedAccessView *m_UView;
	ID3D11ShaderResourceView *m_SView;
};

/*!< \brief this context is the underlying video context used for a texture2d object.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the directX api. */
struct LWDirectX11_1TextureContext {
	ID3D11Resource *m_Texture = nullptr;
	ID3D11ShaderResourceView *m_View = nullptr;
	ID3D11SamplerState *m_Sampler = nullptr;
	std::unordered_map<uint32_t, ID3D11RenderTargetView*> m_RenderTargetViewList;
	std::unordered_map<uint32_t, ID3D11DepthStencilView*> m_DepthStencilViewList;
	std::unordered_map<uint32_t, ID3D11UnorderedAccessView*> m_UnorderedViewList;

	static uint32_t MakeHash(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel);

	ID3D11RenderTargetView *GetRenderTargetView(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel, LWTexture *Tex, LWDirectX11_1Context &DriverContext);

	ID3D11DepthStencilView *GetDepthStencilView(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel, LWTexture *Tex, LWDirectX11_1Context &DriverContext);

	ID3D11UnorderedAccessView *GetUnorderedAccessView(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel, LWTexture *Tex, LWDirectX11_1Context &DriverContext);

};

/*!< \brief this context is the underlying video context used for a pipeline object. the application should never require accessing it directly, but it is provided here incase the application is targeting the directX api. */
struct LWDirectX11_1PipelineContext {
	ID3D11VertexShader *m_VertexShader = nullptr;
	ID3D11GeometryShader *m_GeometryShader = nullptr;
	ID3D11PixelShader *m_PixelShader = nullptr;
	ID3D11ComputeShader *m_ComputeShader = nullptr;
	ID3D11InputLayout *m_InputLayout = nullptr;
};

typedef LWPipelineCon<LWDirectX11_1PipelineContext> LWDirectX11_1Pipeline;
typedef LWTextureCon<LWDirectX11_1TextureContext> LWDirectX11_1Texture;
typedef LWVideoBufferCon<LWDirectX11_1BufferContext> LWDirectX11_1Buffer;
typedef LWShaderCon<LWDirectX11_1ShaderContext> LWDirectX11_1Shader;
typedef LWFrameBuffer LWDirectX11_1FrameBuffer;

/*! \endcond */
#endif
#endif