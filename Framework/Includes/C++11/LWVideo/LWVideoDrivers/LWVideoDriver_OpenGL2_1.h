#ifndef LWVIDEODRIVER_OPENGL2_1_H
#define LWVIDEODRIVER_OPENGL2_1_H
#include "LWVideo/LWVideoDriver.h"
#include "LWPlatform/LWPlatform.h"

#ifdef LWVIDEO_IMPLEMENTED_OPENGL2_1
#include "LWVideo/LWFrameBuffer.h"

/*! \cond */
class LWVideoDriver_OpenGL2_1 : public LWVideoDriver {
public:

	static LWVideoDriver_OpenGL2_1 *MakeVideoDriver(LWWindow *Window, uint32_t Type);

	static bool DestroyVideoContext(LWVideoDriver_OpenGL2_1 *Driver);

	virtual bool Update(void);
	
	virtual LWVideoDriver &ClearColor(uint32_t Color);

	virtual LWVideoDriver &ClearColor(const LWVector4f &Color);

	virtual LWVideoDriver &ClearDepth(float Depth);

	virtual LWVideoDriver &ClearStencil(uint8_t Stencil);

	virtual LWVideoDriver &ViewPort(const LWVector4i &Viewport);

	virtual bool SetFrameBuffer(LWFrameBuffer *Buffer, bool ChangeViewport = false);

	virtual bool SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VerticeStride, uint32_t Offset);

	virtual bool SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias);

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

	virtual LWPipeline *CreatePipeline(LWShader **Stages, uint64_t Flags, LWAllocator &Allocator);

	virtual LWFrameBuffer *CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator);

	virtual bool UpdateTexture(LWTexture *Texture);

	virtual bool UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size);

	virtual bool UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size);

	virtual bool UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	virtual bool UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

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

	virtual LWVideoDriver &DestroyPipeline(LWPipeline *Pipeline);

	virtual LWVideoDriver &DestroyTexture(LWTexture *Texture);

	virtual LWVideoDriver &DestroyFrameBuffer(LWFrameBuffer *FrameBuffer);

	virtual LWVideoDriver &DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset = 0);

	virtual LWVideoDriver &DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStide, uint32_t InstanceCount = 0, uint32_t Offset = 0);

	virtual LWVideoDriver &Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension);

	/*!< \brief returns driver specific context information, which should not be needed by most common applications. */
	LWOpenGL2_1Context &GetContext(void);

	LWVideoDriver_OpenGL2_1(LWWindow *Window, LWOpenGL2_1Context &Context, uint32_t UniformBlockSize);
protected:
	virtual LWPipeline *CreatePipeline(LWPipeline *Source, LWAllocator &Allocator);

	virtual LWVideoDriver &ClonePipeline(LWPipeline *Target, LWPipeline *Source);

	LWOpenGL2_1Context m_Context;
};


struct LWOpenGL2_1BlockUniforms {
	uint32_t m_Index; /*!< \brief the video index for the underlying uniform. */
	uint32_t m_Type; /*!< \brief the typing of the input. */
	int32_t m_Size; /*!< \brief the size length of the input(1 for non-array types). */
	int32_t m_Offset; /*!< \brief the offset into the underlying buffer for the type. */
};

struct LWOpenGL2_1Block {
	static const uint32_t MaxUniforms = 64; /*!< \brief total maximum number of uniforms per block. */
	LWOpenGL2_1BlockUniforms m_Uniforms[MaxUniforms]; /*!< \brief the underlying uniform and data types. */
	uint32_t m_UniformCount; /*!< \brief the total number of uniforms. */
	uint32_t m_Size; /*!< \brief the size of the block. */
};

/*!< \brief This context is the underlying pipeline context used in the opengl 3.2 pipeline. the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the openGL api. */
struct LWOpenGL2_1PipelineContext {
	static const uint32_t MaxBlocks = 32; /*!< \brief total maximum blocks any pipeline is expected to ever have. */
	LWOpenGL2_1Block m_Blocks[MaxBlocks]; /*<! \brief array of the underlying blocks for the uniforms. */
	int32_t m_ProgramID = 0; /*!< \brief the program id for use in opengl api calls. */
};

/*!< \brief This context is the underlying context used in the opengl 2.1 video buffer.  the application should never require accessing it directly, but it is provided here incase the application specifically targeting the openGL api. */
struct LWOpenGL2_1VideoBufferContext {
	uint8_t *m_Buffer = nullptr;
	uint32_t m_VideoID = 0;
};

/*!< \brief this context is the underlying framebuffer context used in the opengl 2.1 pipeline.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the openGL api. */
struct LWOpenGL2_1FrameBufferContext {
	uint32_t m_FBOID = 0;
	int32_t m_Attached[LWFrameBuffer::Count] = {};
};

typedef LWPipelineCon<LWOpenGL2_1PipelineContext> LWOpenGL2_1Pipeline;
typedef LWTextureCon<uint32_t> LWOpenGL2_1Texture;
typedef LWVideoBufferCon<LWOpenGL2_1VideoBufferContext> LWOpenGL2_1Buffer;
typedef LWShaderCon<uint32_t> LWOpenGL2_1Shader;
typedef LWFrameBufferCon<LWOpenGL2_1FrameBufferContext> LWOpenGL2_1FrameBuffer;

/*! \endcond */

#endif
#endif