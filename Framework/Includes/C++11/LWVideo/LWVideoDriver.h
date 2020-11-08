#ifndef LWVIDEODRIVER_H
#define LWVIDEODRIVER_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWAllocator.h"
#include "LWPlatform/LWTypes.h"
#include "LWPlatform/LWPlatform.h"
#include "LWVideo/LWTypes.h"
#include "LWVideo/LWShader.h"
#include "LWVideo/LWTexture.h"
#include "LWVideo/LWVideoBuffer.h"
#include <cstdarg>
#include <unordered_map>

/*! \addtogroup LWVideo
	@{
*/
/*! \brief Video driver class that provides a set of universal api's for accessing the graphics device and graphical operations, regardless of OpenGL or DirectX underlying context. the lower api's such as OpenGL2.1/OpenGL3.2/OpenGLES2.0 do not support every feature that this api supports, calling such unsupported functions will result in nothing happening, if you require a feature not supported by those api's then you must be sure to remove those api's as options when requesting a video driver in MakeVideoDriver.
*/

class LWVideoDriver{
public:
	static const uint32_t Unspecefied = 0xFFF; /*!< \brief automatically selects the best and most suitable driver when calling CreateVideoContext. */
	static const uint32_t OpenGL3_3   = 0x1; /*!< \brief Driver type for passing to CreateVideoContext which creates an openGL 3.2 context. */
	static const uint32_t OpenGL2_1   = 0x2; /*!< \brief Driver type for passing to CreateVideoContext which creates an openGL 2.1 context. */
	static const uint32_t DirectX11_1 = 0x4; /*!< \brief Driver type for passing to CreateVideoContext which creates an DirectX11 context. */
	static const uint32_t OpenGLES2   = 0x8; /*!< \brief Driver type for passing to CreateVideoContext which creates an OpenGL ES 2.0 context. */
	static const uint32_t OpenGL4_5 = 0x10;  /*!< \brief Driver type for passing to CreateVideoContext which creates an OpenGL4.5 context. */
	static const uint32_t DirectX12 = 0x20;  /*!< \brief Driver type for passing to CreateVideoContext which creates an DirectX12 context. */
	static const uint32_t DirectX9C = 0x40;  /*!< \brief Driver type for passing to CreateVideoContext which creates an DirectX9C context. */
	static const uint32_t OpenGLES3 = 0x80;  /*!< \brief Driver type for passing to CreateVideoContext which creates an OpenGL ES 3.0 context. */
	static const uint32_t Metal = 0x100; /*!< \brief Driver type for passing to CreateVideoContext which creates an iOS Metal context. */
	static const uint32_t Vulkan = 0x200; /*!< \brief Driver type for passing to CreateVideoContext which creates an OpenGL Vulkan context. */
	static const uint32_t DebugLayer = 0x80000000; /*!< \brief flag to add to MakeVideoDriver Type paramater which will enable a debug layer output if the driver api supports it(such as directX). */
	enum{
		Points = 0, /*!< \brief drawing expects a series of points per-vertex. */
		LineStrip, /*!< \brief drawing expected input is a line strip. */
		Lines, /*!< \brief drawing expected input is a series of lines. */
		TriangleStrip, /*!< \brief drawing expected input is a triangle strip. */
		Triangle /*!< \brief drawing expected input is a triangle. */
	};

	/*! \brief allocates a video driver for the specified type, and attaches it to the specified window.
		\note attaching more than 1 video driver at a time to the window is likely to cause a crash!
		\return null if unsuccessfully, otherwise an video driver object is returned, which is allocated with the same allocator used to make the window. */
	static LWVideoDriver *MakeVideoDriver(LWWindow *Window, uint32_t Type = Unspecefied);

	/*!< \brief constructs a single has for the inputted rasterflags, allows for caching raster states. */
	static uint32_t MakeRasterStateHash(uint64_t RasterFlags, float Bias, float ScaleBias);

	/*! \brief deallocates and destroys the specified video context, and detaches it from the associated window. */
	static bool DestroyVideoDriver(LWVideoDriver *Driver);

	/*!< \brief parser function to strip down a shared shader and find the specific module requested for a particular environment.
		 \param ShaderCode the shader code to parse
		 \param Environment the environment that we are looking for each module for(i.e: DirectX11, OpenGL3_2, etc).
		 \param ModuleName the targeted module to find.
		 \param DefinedCount the define list for the module to do either string replacement or #ifdef checks with.
		 \param DefinedList the list of defined variables, List is expected to be a pair of Name, then followed by Value iterator(this can be a blank iterator if no value exists), count should be the total combined Name+Value pairs.
		 \param ModuleBuffer the buffer to store resulting module.
		 \param ModuleBufferLen the length of the buffer where the module will be stored.
		 \note This function performs basic preparsing logic with #ifdef, #ifndef, and string replacement with #Name where name is a defined name, #module is used to find a target environment/module, the parameters for #module [Module] [Environments, ...]: (Module can be any of: Vertex, Pixel, Geometry, Compute, you can also combine them by adding a | between the two, such as Vertex|Pixel, note that their must be no spaces between the |).  (Environment, the supported environments, OpenGL4_5, DirectX11, OpenGL3_3, OpenGL2_1, etc.  multiple environments can be specified in order of appearance(i.e: OpenGL3_3 OpenGL4_5).
		 \return the number number of bytes to store the module.
	*/
	static uint32_t FindModule(const LWUTF8Iterator &ShaderCode, const LWUTF8Iterator &Environment, const LWUTF8Iterator &ModuleName, uint32_t DefinedCount, const LWUTF8Iterator *DefinedList, char8_t *ModuleBuffer, uint32_t ModuleBufferLen);

	/*!< \brief performs analysis on Dx/openGL error's and inserts the offending line into the error buffer for outputting, since Lightwave supports files with multiple shader's this can make debugging error's in shader's problematic, as such this function helps pinpoint the error's more quickly. */
	static uint32_t PerformErrorAnalysis(const LWUTF8Iterator &Source, char8_t *ErrorBuffer, uint32_t ErrorBufferLength);

	/*! \brief process any window events the driver needs to know about. 
		\return rather or not it is safe to continue rendering, if it is false, then the application should absolutely not issue any rendering commands.
	*/
	virtual bool Update(void);

	/*!< \brief clear's all of the currently bound framebuffer's color components or backbuffer to the 8-bit RGBA value. */
	virtual LWVideoDriver &ClearColor(uint32_t Color);

	/*!< \brief clears a color channel to floating point value. */
	virtual LWVideoDriver &ClearColor(const LWVector4f &Value);

	/*!< \brief clear's the currently bound framebuffer depth componet or backbuffer depth. */
	virtual LWVideoDriver &ClearDepth(float Depth);

	/*!< \bried clear's the currently bound framebuffer stencil componet or backbuffer stencil. */
	virtual LWVideoDriver &ClearStencil(uint8_t Stencil);

	/*! \brief set's the current viewport to the size of the attached window.*/
	virtual LWVideoDriver &ViewPort(void);

	/*!< \brief set's the current viewport to the size of the target framebuffer. */
	virtual LWVideoDriver &ViewPort(const LWFrameBuffer *FrameBuffer);

	/*!< \brief create's a texture from an LWImage. */
	virtual LWTexture *CreateTexture(uint32_t TextureState, LWImage &Image, LWAllocator &Allocator);

	/*! \brief set's the viewport the x, y, width, and height.
		\param Viewport a vector4 where xyzw is x, y, width, and height in that order.
	*/
	virtual LWVideoDriver &ViewPort(const LWVector4i &Viewport);

	/*! \brief swaps the video buffer. 
		\param SwapInterval the interval of v-syncs to wait.
	*/
	virtual LWVideoDriver &Present(uint32_t SwapInterval = 0);

	/*!< \brief draws a mesh buffer with the associated inputs.
		 \param Pipeline updates and set's the active pipeline to be used for drawing.
		 \param DrawMode the rasterization mode for drawing.
		 \param InputBlock the vertex input data.
		 \param IndexBuffer the Index buffer used for the input data(pass null if no index buffer is to be used).
		 \param Count the number of vertices(not polygons) to draw.
		 \param VertexStride the stride between each vertex in the input block.
		 \param Offset the vertices to offset from when drawing.
	*/
	virtual LWVideoDriver &DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset = 0);

	/*!< \brief draws an instanced mesh buffer with the associated inputs.
		 \param Pipeline the pipeline to be used for drawing.
		 \param DrawMode the rasterization mode for drawing.
		 \param InputBlock the vertex input data.
		 \param IndexBuffer the Index buffer used for the input data(pass null if no index buffer is to be used).
		 \param Count the number of vertices(not polygons) to draw.
		 \param VertexStride the stride between each vertex in the input block.
		 \param Offset the vertices to offset from when drawing.
		 \param InstanceCount the total number of instances to be used for drawing.
	*/
	virtual LWVideoDriver &DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount = 0, uint32_t Offset = 0);

	/*!< \brief dispatchs a compute shader.
		 \param Pipeline the pipeline with the compute shader to dispatch for.
		 \param GroupDimension compute shader group sizes.
	*/
	virtual LWVideoDriver &Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension);

	/*!< \brief draws a mesh with the associated mesh. */
	LWVideoDriver &DrawMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh);

	/*!< \brief draws an instanced mesh with the associated mesh. */
	LWVideoDriver &DrawInstancedMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh, uint32_t InstanceCount);

	/*!< \brief draws a partial mesh with the associated mesh. */
	LWVideoDriver &DrawMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh, uint32_t Count, uint32_t Offset = 0);

	/*!< \brief draws an instanced mesh with the associated mesh. */
	LWVideoDriver &DrawInstancedMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh, uint32_t Count, uint32_t InstanceCount, uint32_t Offset = 0);

	/*!< \brief updates the mesh, without having to render it. */
	LWVideoDriver &UpdateMesh(LWBaseMesh *Mesh);

	/*!< \brief changes the active framebuffer for rendering. pass null to render to the backbuffer. returns true if the framebuffer was changed. 
		 \brief set's the viewport dimensions to the framebuffer's dimensions of ChangeViewport is set to true.
	*/
	virtual bool SetFrameBuffer(LWFrameBuffer *Buffer, bool ChangeViewport = false);

	/*!< \brief changes the internal pipeline stages and resources if the pipeline stages have been changed. */
	bool UpdatePipelineStages(LWPipeline *Pipeline);

	/*!< \brief setss and updates the active pipeline for rendering, this function is more for internal conveniences and shouldn't ever have to be called by the application as the Draw commands all require a bound pipeline as well.  returns true when the pipeline settings are changed. 
		 \param Pipeline the pipeline object to make active.
		 \param Vertices the bound vertice input for the pipeline.
		 \param Indices the bound indices for the pipeline.
		 \param stride for the vertices, Lightwave expects data to be interleaved in a single struct.
		 \param Offset if indices is not null then this is the offset into the indice buffer, otherwise it is the offset into the vertices buffer.
	*/
	virtual bool SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *Vertices, LWVideoBuffer *Indices, uint32_t VerticeStride, uint32_t Offset);

	/*!< \brief set's and updates the active raster state for rendering, this function is more for internal conveniences and shouldn't ever have to be called by the application as the Draw commands all require the bounded pipeline and will override the raster settings with the pipeline settings. */
	virtual bool SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias);

	/*!< \brief loads a file from the specified path and passes the resulting buffer to ParseShader, allows for quick shader loading.  Internally uses a stack buffer of 32KB, if the file is larger then this, then the application must load it.  */
	virtual LWShader *LoadShader(uint32_t ShaderType, const LWUTF8Iterator &Path, LWAllocator &Allocator, uint32_t DefinedCount, const LWUTF8Iterator *DefinedList, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen, LWFileStream *ExistingStream = nullptr);

	/*!< \brief compiles a shader from a shared shader source buffer(using FindModule for the relevant driver).
		 \param Source the source buffer to be parsed.
		 \param Allocator the allocator used for creating the resulting shader object.
		 \param DefinedCount the number of defines that are declared.
		 \param DefineList the define list for the the shader source #defines, does string replacement with #Name in shader with the define(if declared)(declared as "Name:Value")
		 \param CompiledBuffer the compiled result, pass null if un-needed.
		 \note if the resulting shader is greater than 32KB then the result will be undefined, as such the application will have to implement it's own loading scheme.
		 \note the modules come as either Vertex, Geometry, Pixel, or Compute.  for compiled variants CVertex, CGeometry, CPixel, or CCompute.  pre-compiled variants are searched for first, followed by the non-compiled variant.
	*/
	virtual LWShader *ParseShader(uint32_t ShaderType, const LWUTF8Iterator &Source, LWAllocator &Allocator, uint32_t DefinedCount, const LWUTF8Iterator *DefinedList, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen);

	/*!< \brief compiles shader code and create's a shader for a pipeline object. if CompiledBuffer is not null the compiled byte code is written out for storing to speed up future shader creations.  
		 \param ShaderType the type of shader(vertex, pixel, geometry, compute) compute shaders are only supported on the latest api's
		 \param Source the uncompiled source to while will be compiled as passed to createShaderCompiled.  if you want to store this compiled state.  only certain api's support outputting the the compiled code for storage, so if CompiledBufferLen is 0 even when you pass in compiled buffer, this means the api in use doesn't support this feature.
		 \param Allocator the allocator used for creating the shader object.
		 \param CompiledBuffer an optional pointer to a buffer to hold a copy of the compiled shader code if the platform supports such things(D3D11+, OpenGL4.4+ support).
		 \param ErrorBuffer an optional pointer to a buffer to receive outputted error from compilation.
		 \param CompiledBufferLen a pointer to a variable which holds the length of CompiledBuffer, and when the function complete is set to the resultant data written to the buffer, 0 will be written if the platform doesn't support compiled binarys.
		 \param ErrorBufferLen the length of the errorbuffer to be written to.
		 \note OpenGL2.1 + ES2.0 do not support true uniform buffers, as such their data layout for different Blocks should be placed into a struct, then the uniform declared as the struct type.  this allows LightWave to detect simulated structs by examing the naming scheme of each uniform, see sample shaders or LWFont embedded shader on how to arrange uniform buffers.  Lightwave does not support single uniform types but only buffers of data or textures/samplers.
	*/
	virtual LWShader *CreateShader(uint32_t ShaderType, const LWUTF8Iterator &Source, LWAllocator &Allocator, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen);

	/*!< \brief creates a shader from the pre-compiled shader code.  */
	virtual LWShader *CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledLen, LWAllocator &Allocator, char8_t *ErrorBuffer, uint32_t ErrorBufferLen);


	/*!< \brief constructs a pipeline object for use in drawing from the supplied shader stages. */
	virtual LWPipeline *CreatePipeline(LWShader **ShaderStages, uint64_t Flags, LWAllocator &Allocator);

	/*!< \brief constructs a pipeline object for use in drawing from the Vertex/Geom/PixelShaders.
		 \param VertexShader vertex shader which used by the pipeline. 
		 \param GeomShader optional geometry shader to be used by the pipeline.
		 \param PixelShader optional pixel shader which is used by the pipeline, not including the pixelshader will still have the depth buffer be written to for shadowmapping.
		 \param Flags the raster flags for the pipeline.
	*/
	LWPipeline *CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, LWAllocator &Allocator);


	/*!< \brief constructs a pipeline object for use in dispatching compute calls.
		 \param ComputeShader Compute shader which used by the pipeline for.
		 \param GeomShader optional geometry shader to be used by the pipeline.
		 \param PixelShader optional pixel shader which is used by the pipeline, not including the pixelshader will still have the depth buffer be written to for shadowmapping.
		 \param Flags the raster flags for the pipeline.
	*/
	LWPipeline *CreatePipeline(LWShader *ComputeShader, LWAllocator &Allocator);

	/*!< \brief constructs a pipeleine object with the specified raster state options, this function is for convenience of the most common specified flags and will do the appropriate bitshifting for each flag. */
	LWPipeline *CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, LWAllocator &Allocator);

	/*!< \brief constructs a pipeleine object with the specified raster state options, this function is for convenience of the most common specified flags and will do the appropriate bitshifting for each flag. */
	LWPipeline *CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, uint64_t SrcBlendMode, uint64_t DstBlendMode, LWAllocator &Allocator);

	/*!< \brief constructs a pipeleine object with the specified raster state options, this function is for convenience of the most common specified flags and will do the appropriate bitshifting for each flag. */
	LWPipeline *CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, uint64_t StencilCompareFunc, uint64_t StencilFailOp, uint64_t StencilDepthFailOp, uint64_t StencilPassOp, LWAllocator &Allocator);

	/*!< \brief constructs a pipeleine object with the specified raster state options, this function is for convenience of the most common specified flags and will do the appropriate bitshifting for each flag. */
	LWPipeline *CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, uint64_t SrcBlendMode, uint64_t DstBlendMode, uint64_t StencilCompareFunc, uint64_t StencilFailOp, uint64_t StencilDepthFailOp, uint64_t StencilPassOp, LWAllocator &Allocator);

	/*!< \brief constructs a 1D texture, pass null to Texels to specify no data to be passed to the texture.
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture(s), the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Texels the source texels and array of mipmaps total texels expected is MipmapCnt+1(or null for no data).
		 \param MipmapCnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Mipmaps the mipmap data to be used, each mipmap is expected to be in correspondence to the number of required mipmaps to reach a size of 1. i.e a length of 128, expects 7 additional mipmaps: 64, 32, 16, 8, 4, 2, 1, which can be calculated via: floor(log2(size)) or use LWImage::MipmapCount
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	/*!< \brief constructs a 2D texture, pass null to Texels to specify no data to be passed to the texture. 
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture, the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Texels the source texels and array of mipmaps  total texels expected is MipmapCnt+1. pass null if no default data is to be presented.
		 \param Mipmapcnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	/*!< \brief constructs a 3D texture, pass null to Texels to specify no data to be passed to the texture.
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture, the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Texels the texels to intially create the texture with. pass null if no default data is to be presented.
		 \param Mipmapcnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	/*!< \brief constructs a cube map texture, Texels expects all data for all 6 faces.
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture, the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Texels the texels to intially create the cubemap texture the order texels is each face+each mipmap level, then next face.  face order expected is: POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z. pass null if no default data is to be presented.
		 \param Mipmapcnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	/*! \brief constructs a 2D multisampled texture.
		\param TextureState is a series of Or'd texture flags that represent the current state of texture for filtering.
		\param PackType represents the packing type of the texture, the supported types are found in LWImage
		\param Size the width and height of the texture.
		\param Samples the number of samples at each texel, this value should be a 2n number, and not recommended to exceed 8. 
		\param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture2DMS(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, LWAllocator &Allocator);

	/*!< \brief constructs a 1D texture, pass null to Texels to specify no data to be passed to the texture.
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture(s), the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Layers the number of layers for the array.
		 \param Texels the source texels and array of mipmaps total texels expected is MipmapCnt+1(or null for no data).
		 \param MipmapCnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Mipmaps the mipmap data to be used, each mipmap is expected to be in correspondence to the number of required mipmaps to reach a size of 1. i.e a length of 128, expects 7 additional mipmaps: 64, 32, 16, 8, 4, 2, 1, which can be calculated via: floor(log2(size)) or use LWImage::MipmapCount
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	/*!< \brief constructs a 2D array of textures, pass null to Texels to specify no data to be passed to the texture.
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture, the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Layers the number of layers for the array.
		 \param Texels the source texels and array of mipmaps  total texels expected is Layers*MipmapCnt+1. pass null if no default data is to be presented.
		 \param Mipmapcnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator);

	/*!< \brief constructs a cube map array of textures, Texels expects all data for all 6 faces and for all indices.
		 \param TextureState is a series of Or'd texture flags that represent the current state of the texture for filtering.
		 \param PackType represents the packing type of the texture, the supported types are found in LWImage
		 \param Size the number of texels the texture is to contain.
		 \param Layers the number of layers for the array.
		 \param Texels the texels to intially create the cubemap texture the order texels is each face+each mipmap level, then next face.  face order expected is: POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z. pass null if no default data is to be presented then the next array.
		 \param Mipmapcnt the number of mipmaps with the texture, expected size of each mipmap is based on the level of the mipmap.
		 \param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator);

	/*! \brief constructs a 2D multisampled array texture.
		\param TextureState is a series of Or'd texture flags that represent the current state of texture for filtering.
		\param PackType represents the packing type of the texture, the supported types are found in LWImage
		\param Size the width and height of the texture.
		\param Samples the number of samples at each texel, this value should be a 2n number, and not recommended to exceed 8. .
		\param Allocator the allocator used to create the texture object.
	*/
	virtual LWTexture *CreateTexture2DMSArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, uint32_t Layers, LWAllocator &Allocator);


	/*!< \brief constructs a video buffer for use. 
		 \param Type the type of the video buffer to create.
		 \param TypeSize the size of the underlying type.
		 \param Length the total number of typed objects the buffer is to have (final length is (TypeSize*Length).
		 \param Allocator the allocator used to allocate the video buffer object.
		 \param UsageFlag a OR'd flag to specify the intended usage of the buffer(such as if it is a gpu read only buffer, then that implies data will be uploaded once, if it is marked as cpu write, and write discard, this implies that on each update the cpu is writing new data into the buffer, and the old data is discarable.
		 \param Buffer data to be uploaded to the video buffer.
	*/
	virtual LWVideoBuffer *CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer);


	/*!< \brief constructs a video buffer of type for use.
		 \param Type the type of the video buffer to create.
		 \param Length the total number of typed objects the buffer is to have (final length is (TypeSize*Length)).
		 \param Allocator the allocator used to allocate the video buffer object.
		 \param UsageFlag a OR'd flag to specify the intended usage of the buffer(such as if it is a gpu read only buffer, then that implies data will be uploaded once, if it is marked as cpu write, and write discard, this implies that on each update the cpu is writing new data into the buffer, and the old data is discarable.
		 \param Buffer data to be uploaded to the video buffer, pass null if no initial data is to be sent, this buffer should be the same length as the sizeof(Type)*Length parameter.
	*/
	template<class ObjType>
	LWVideoBuffer *CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t Length, LWAllocator &Allocator, const ObjType *Buffer) {
		return CreateVideoBuffer(Type, UsageFlag, sizeof(ObjType), Length, Allocator, (const uint8_t*)Buffer);
	}


	/*!< \brief constructs a video buffer of UniformBlockPaddedSize(ObjType) for use.
		 \param Type the type of the video buffer to create.
		 \param Length the total number of typed objects the buffer is to have (final length is (UniformBlockPaddedSize(TypeSize)*Length)).
		 \param Allocator the allocator used to allocate the video buffer object.
		 \param UsageFlag a OR'd flag to specify the intended usage of the buffer(such as if it is a gpu read only buffer, then that implies data will be uploaded once, if it is marked as cpu write, and write discard, this implies that on each update the cpu is writing new data into the buffer, and the old data is discarable.
		 \param Buffer padded data to be uploaded to the video buffer, pass null if no initial data is to be sent, this buffer should be the same length as the UniformBlockPaddedSize(sizeof(Type))*Length parameter.  remember that the typeSize is the padded size, and not just the sizeof(ObjType)
		 \note Because the buffer is padded, if you provide a Buffer for inital data it must take into account the padded size of the underlying type, otherwise this will cause problems. 
	*/
	template<class ObjType>
	LWVideoBuffer *CreatePaddedVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t Length, LWAllocator &Allocator, void *Buffer) {
		return CreateVideoBuffer(Type, UsageFlag, GetUniformBlockPaddedSize(sizeof(ObjType)), Length, Allocator, (const uint8_t*)Buffer);
	};

	/*!< \brief constructs a frame buffer for rendering.
		 \param Size the width and height of the framebuffer.
	*/
	virtual LWFrameBuffer *CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator);

	/*!< \brief updates a texture's flags. 
		 \return boolean result if successfully able to update.
	*/
	virtual bool UpdateTexture(LWTexture *Texture);

	/*!< \brief updates a 1D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Texels the texels to be updated.
		 \param Position the offset of texel to be updated.
		 \param Size the number of texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	virtual bool UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size);

	/*!< \brief updates the entire 1D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropiate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = bast Texture, 1 = first mipmap, etc).
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels);

	/*!< \brief updates a 2D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		\param Texture the texture to be updated.
		\param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		\param Texels the texels to be updated.
		\param Position the position in the texture to be updated.
		\param Size the rectangle size of the texels to be updated.
		\return boolean result if successfully able to update.
	*/
	virtual bool UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	/*!< \brief updates the entire 2D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropiate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = bast Texture, 1 = first mipmap, etc).
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels);

	/*!< \brief updates a 3D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		\param Texture the texture to be updated.
		\param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		\param Texels the texels to be updated.
		\param Position the position in the texture to be updated.
		\param Size the rectangle size of the texels to be updated.
		\return boolean result if successfully able to update.
	*/
	virtual bool UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size);

	/*!< \brief updates the entire 3D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = bast Texture, 1 = first mipmap, etc).
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels);

	/*!< \brief updates a cubemap texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Texels the texels to be updated.
		 \param Face the face to be updated.
		 \param Position the position in the texture to be updated.
		 \param Size the rectangle size of the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	virtual bool UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);
	
	/*!< \brief updates the entire face 2D texture, apply's any texture flags that need to be applied, and if texels is not null, will update the appropiate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = bast Texture, 1 = first mipmap, etc).
		 \param Face the cube map face to be updated.
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels);

	/*!< \brief partial updates a 1D texture array, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Layer the texture layer to be updated.
		 \param Texels the texels to be updated.
		 \param Position the offset of texel to be updated.
		 \param Size the number of texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	virtual bool UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size);

	/*!< \brief updates an 1D texture array layer, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Layer the texture layer to be updated.
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels);

	/*!< \brief partial updates a 2D texture array, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Layer the texture layer to be updated.
		 \param Texels the texels to be updated.
		 \param Position the offset of texel to be updated.
		 \param Size the number of texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	virtual bool UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	/*!< \brief updates an 2D texture array layer, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Layer the texture layer to be updated.
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels);

	/*!< \brief partial updates a cubemap texture array, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Layer the texture layer to be updated.
		 \param Face the face of the cube map to be updated.
		 \param Texels the texels to be updated.
		 \param Position the offset of texel to be updated.
		 \param Size the number of texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	virtual bool UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size);

	/*!< \brief updates an cubemap texture array layer and face, apply's any texture flags that need to be applied, and if texels is not null, will update the appropriate mipmap level being specified.
		 \param Texture the texture to be updated.
		 \param MipmapLevel the mipmaps level to be updated(0 = base Texture, 1 = first mipmap, etc).
		 \param Layer the texture layer to be updated.
		 \param Texels the texels to be updated.
		 \return boolean result if successfully able to update.
	*/
	bool UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels);

	/*!< \brief updates the video buffer and uses the internal buffer for updating the data. this function will only update the underlying video buffer if the updated flag is set, and will subsequently clear the flag once the buffer has been updated. */
	virtual bool UpdateVideoBuffer(LWVideoBuffer *Buffer);

	/*!< \brief updates the video buffer with the supplied buffer. independent of the internal local buffer.
		 \param VideoBuffer the video buffer to be updated.
		 \param Buffer the buffer to use to update the video buffer.
		 \param Length the number of bytes to be updated.
		 \note because lightwave prefers a deferred context for rendering, partial buffer updates are not possible with the renderer's available.
	*/
	virtual bool UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length);

	/*!< \brief Downloads the entire 1D texture into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	/*!< \brief Downloads the entire 2D texture into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	/*!< \brief Downloads the entire 3D texture into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer);

	/*!< \brief Downloads the entire 2D face texture into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, uint8_t *Buffer);

	/*!< \brief Downloads the entire 1D array layered texture into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	/*!< \brief Downloads the entire 2D array layered texture into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer);

	/*!< \brief Downloads the entire 2D face texture of a cubemap layer into client side memory.
		 \param Texture the texture object to download.
		 \param MipmapLevel the mipmap level to download from(0 is the base image).
		 \param Buffer the buffer to receive the downloaded texture.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer);

	/*!< \brief Downloads a portion(or the entire buffer) of a video buffer into client side memory.
		 \param VBuffer the video buffer to download.
		 \param Buffer the buffer to receive the data.
		 \param Offset the offset in bytes into the video buffer to read from.
		 \param Length the length of the buffer to download in bytes.
		 \return rather the data was successfully downloaded or not.
	*/
	virtual bool DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length);

	/*!< \brief destroys a video buffer that was created by the "CreateVideoBuffer" method. */
	virtual LWVideoDriver &DestroyVideoBuffer(LWVideoBuffer *Buffer);

	/*!< \brief destroys a frame buffer object that was created by the "CreateFrameBuffer" method. */
	virtual LWVideoDriver &DestroyFrameBuffer(LWFrameBuffer *FrameBuffer);

	/*!< \brief destroys a video driver that was created by the "CreateShader" method. */
	virtual LWVideoDriver &DestroyShader(LWShader *Shader);

	/*!< \brief destroys a pipeline that was created by the "CreatePipeline" method. */
	virtual LWVideoDriver &DestroyPipeline(LWPipeline *Pipeline);

	/*!< \brief destroys a texture that was created with any of the "CreateTextureX" methods. */
	virtual LWVideoDriver &DestroyTexture(LWTexture *Texture);

	/*!< \brief calculates the padded size needed for the specified uniform block's raw size. */
	uint32_t GetUniformBlockPaddedSize(uint32_t RawSize) const;

	/*!< \brief returns the indexed padded position in buffer for the requested type. */
	template<class Type>
	Type *GetUniformPaddedAt(uint32_t Index, void *Buffer) const {
		return (Type*)((uint8_t*)Buffer + GetUniformBlockPaddedSize(sizeof(Type))*Index);
	}

	/*!< \brief returns the padded size of the requested type of length. */
	template<class Type>
	uint32_t GetUniformPaddedLength(uint32_t Length) const {
		return GetUniformBlockPaddedSize(sizeof(Type))*Length;
	}

	/*!< \brief allocates an array of bytes for the padded type of length to occupy. */
	template<class Type>
	char *AllocatePaddedArray(uint32_t Length, LWAllocator &Allocator) {
		return Allocator.AllocateA<char>(GetUniformBlockPaddedSize(sizeof(Type))*Length);
	}

	/*!< \brief calculates the uniform block's offset from the rawsize(taking into account how many uniform blocks are taken up by the struct.) */
	uint32_t GetUniformBlockOffset(uint32_t RawSize, uint32_t Offset) const;

	/*!< \brief calculates the uniform block's offset from the supplied type. */
	template<class Type>
	uint32_t GetUniformBlockOffset(uint32_t Offset) const {
		return GetUniformBlockOffset(sizeof(Type), Offset);
	}

	/*! \brief returns the parent window object.*/
	LWWindow *GetWindow(void) const;

	/*! \brief returns the current viewport of the driver. */
	LWVector4i GetViewPort(void) const;

	/*!< \brief returns the current raster flags enabled. */
	uint64_t GetRasterFlags(void) const;

	/*!< \brief returns the current bias to be applied to the depth buffer. */
	float GetBias(void) const;

	/*!< \brief returns the current bias scalar to be applied to the depth buffer when depth bias flag is set. */
	float GetSlopedBias(void) const;

	/*! \brief returns the underlying driver type that this object is. */
	uint32_t GetDriverType(void) const;

	/*!< \brief returns the driver type as a 0 indexed id. */
	uint32_t GetDriverID(void) const;

	/*!< \brief returns the uniforms minimum block size used when setting the shader's offset and calculating the amount of padding when batching multiple uniforms into one buffer. */
	uint32_t GetUniformBlockSize(void) const;

	/*! \brief creates an video object with the parent window object. */
	LWVideoDriver(LWWindow *Window, uint32_t DriverType, uint32_t UniformBlockSize);

protected:
	virtual LWPipeline *CreatePipeline(LWPipeline *Source, LWAllocator &Allocator);

	virtual LWVideoDriver &ClonePipeline(LWPipeline *Target, LWPipeline *Source);

	LWVideoDriver &ClearPipelines(void);

	LWVector4i m_Viewport = LWVector4i(); /*!< \brief the current viewport, defaulted to 0. */
	LWWindow *m_Window; /*!< \brief the parent window owner of the driver. */
	uint32_t m_DriverType; /*!< \brief the actual type of the driver, the list of options can be found in the LWWindow class. */
	std::unordered_map<uint32_t, LWPipeline*> m_PipelineMap; /*!< \brief internal map of pipelines, a cache of different shader's compiled together. */
	LWFrameBuffer *m_ActiveFrameBuffer = nullptr; /*!< \brief the current active framebuffer. */ 
	LWPipeline *m_ActivePipeline = nullptr; /*!< \brief the current active pipeline. */
	uint64_t m_ActiveRasterFlags = -1; /*!< \brief the current active raster flags. */
	float m_ActiveBias = 0.0f; /*!< \brief the current active bias. */
	float m_ActiveSlopedBias = 0.0f; /*!< \brief the current active sloped bias. */
	uint32_t m_UniformBlockSize; /*!< \brief size in bytes that each uniform block must be from each other(in otherwords this is the amount of padding necessary to use the offset parameter when setting the shader block.) */
};
/* @} */

/* Create dummy implementations for platforms which have no implementations.*/
/*! \cond */

#define LWVIDEO_STUBBED_DRIVER(DriverName) \
class DriverName : public LWVideoDriver { \
public: \
	static LWVideoDriver *MakeVideoDriver(LWWindow *, uint32_t){ return nullptr; } \
	static bool DestroyVideoContext(DriverName *){ return false; } \
};

#ifndef LWVIDEO_IMPLEMENTED_DIRECTX11
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_DirectX11_1)
#endif

#ifndef LWVIDEO_IMPLEMENTED_OPENGL2_1
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_OpenGL2_1)
#endif

#ifndef LWVIDEO_IMPLEMENTED_OPENGL3_3
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_OpenGL3_3)
#endif

#ifndef LWVIDEO_IMPLEMENTED_OPENGL4_5
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_OpenGL4_5)
#endif

#ifndef LWVIDEO_IMPLEMENTED_OPENGLES2
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_OpenGLES2)
#endif

#ifndef LWVIDEO_IMPLEMENTED_DIRECTX12
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_DirectX12)
#endif

#ifndef LWVIDEO_IMPLEMENTED_DIRECTX9C
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_DirectX9C)
#endif

#ifndef LWVIDEO_IMPLEMENTED_OPENGLES3
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_OpenGLES3)
#endif

#ifndef LWVIDEO_IMPLEMENTED_METAL
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_Metal)
#endif

#ifndef LWVIDEO_IMPLEMENTED_VULKAN
LWVIDEO_STUBBED_DRIVER(LWVideoDriver_Vulkan)
#endif

/*! \endcond */
#endif