#ifndef LWPIPELINE_H
#define LWPIPELINE_H
#include <LWCore/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWVideo/LWShader.h>



class LWPipeline {
public:
	enum : uint64_t {

		Unknown = 0, /*!< \brief Resource is not a known type. */
		Texture, /*!< \brief Resource type is an Texture resource(Texture/Sampler object). */
		TextureBuffer, /*!< \brief Resource type is a texture buffer resource(StructureBuffer object). */
		Image, /*!< \brief Resource type is an image.(RWTexture/Image object) */
		ImageBuffer, /*!< \brief Resource is an unordered buffer.(RWStructureBuffer, shader storage buffer object). */
		UniformBlock, /*!< \brief Resource is a uniform block. */

		Compute = 0, /*!< \brief compute stage of shader(this should be the only stage for a pipeline, this overrides the vertex shader, and will only configure the shader for compute). */
		Vertex = 0, /*!< \brief Vertex stage shader. */
		Geometry, /*!< \brief geometry stage shader. */
		Pixel, /*!< \brief pixel stage shader. */
		StageCount, /*!< \brief total supported stages. */

		DEPTH_TEST = 0x1, /*!< \brief depth testing is enabled. */
		BLENDING = 0x2, /*!< \brief blending is enabled, and uses the specified blending modes. */
		No_Depth = 0x4, /*!< \brief flag to specify that writing to the depth buffer should be disabled. */
		No_ColorR = 0x8, /*!< \brief flag to specify that writing to the red component of the color texture should be disabled. */
		No_ColorG = 0x10, /*!< \brief flag to specify that writing to the green component of the color texture should be disabled. */
		No_ColorB = 0x20, /*<! \brief flag to specify that writing to the blue component of the color texture should be disabled. */
		No_ColorA = 0x40, /*<! \brief flag to specify that writing to the alpha component of the color texture should be disabled. */
		No_Color = 0x78, /*<! \brief combined flag of all no write color flags. */
		CLIPPLANE0 = 0x80, /*!< \brief flag to specify that clip plane 0 is enabled/disabled(this is necessary for opengl instances that use gl_clipPlanes). */
		CLIPPLANE1 = 0x100, /*!< \brief flag to specify that clip plane 0 is enabled/disabled(this is necessary for opengl instances that use gl_clipPlanes). */
		CLIPPLANE2 = 0x200, /*!< \brief flag to specify that clip plane 0 is enabled/disabled(this is necessary for opengl instances that use gl_clipPlanes). */
		STENCIL_TEST = 0x400, /*!< \brief flag to specify that stencil testing is enabled/disabled. */
		DEPTH_BIAS = 0x2000000000000, /*!< \brief flag to specify that depth biased/scale bias is to be enabled when writing to the depth buffer(for opengl this flag enable's GL_POLYGON_OFFSET_FILL). */

		SOLID = 0x0, /*!< \brief fill mode is set to solid. */
		WIREFRAME = 0x1, /*!< \brief fill mode is set to wireframe. */

		FILL_MODE_BITS = 0x800, /*!< \brief fill mode bits to get from flag. */
		FILL_MODE_BITOFFSET = 0xB, /*!< \brief fill mode bit offset to get just the fill mode. */

		CULL_NONE = 0x0, /*!< \brief no culling is enabled. */
		CULL_CCW = 0x1, /*!< \brief culling mode is counter clockwise. */
		CULL_CW = 0x2, /*!< \brief culling mode is clockwise. */

		CULL_BITS = 0x3000, /*!< \brieff culling mode bits to get from flag. */
		CULL_BITOFFSET = 0xC, /*!< \brief bit offset to get just the cull mode. */

		BLEND_SRC_BITS = 0x3C000, /*!< \brief the source blending bits in the activeState flag. */
		BLEND_SRC_BITOFFSET = 0xE, /*!< \brief the offset of bits to get the possible source blending flag. */
		BLEND_DST_BITS = 0x3C0000, /*!< \brief the destination blending bits in the activeState flag. */
		BLEND_DST_BITOFFSET = 0x12, /*!< \brief the offset of bits to get the possible destination blending flag. */

		DEPTH_COMPARE_BITS = 0x1C00000, /*!< \brief the depth comparison bits to get the comparison mode used in depth testing. */
		DEPTH_COMPARE_BITOFFSET = 0x16, /*!< \brief the bit offset to get the depth comparison bits. */

		STENCIL_COMPARE_BITS = 0xE000000, /*!< \brief the stencil comparison bits to get the comparison mode used in stencil testing. */
		STENCIL_COMPARE_BITOFFSET = 0x19, /*!< \brief the bit offset to get the stencil comparison bits. */

		STENCIL_OP_KEEP = 0x0, /*!< \brief stencil operation to keep the current stencil value. */
		STENCIL_OP_ZERO = 0x1, /*!< \brief stencil operation to zero out the current stencil value. */
		STENCIL_OP_REPLACE = 0x2, /*!< \brief stencil operation to replace the current stencil value with the compared value. */
		STENCIL_OP_INCR = 0x3, /*!< \brief stencil operation to increase by 1 the current stencil value(clamps at max value(255)). */
		STENCIL_OP_DECR = 0x4, /*!< \brief stencil operation to decrease by 1 the current stencil value(clamps at min value(0)). */
		STENCIL_OP_INCR_WRAP = 0x5, /*!< \brief stencil operation to increase by 1 and wrap back to 0 when reaching max value(255). */
		STENCIL_OP_DECR_WRAP = 0x6, /*!< \brief stencil operation to decrease by 1 and wrap to 255 when reaching min value(0). */
		STENCIL_OP_INVERT = 0x7, /*!< \brief stencil operation to invert the bit value of the current stencil value. */

		STENCIL_REF_VALUE_BITS = 0xFF0000000, /*!< \brief the stencil reference value bits. */
		STENCIL_REF_VALUE_BITOFFSET = 0x1C, /*!< \brief the bit offset to get just the stencil reference value. */
		STENCIL_READMASK_BITS = 0xFF000000000, /*!< \brief the stencil read mask bits for when doing comparison does compare((value&mask), ref value). */
		STENCIL_READMASK_BITOFFSET = 0x24, /*!< \brief the bit offset to get just the read mask bits. */
		STENCIL_WRITEMASK_BITS = 0xFF00000000000, /*!< \brief the stencil write mask bits for when writing the value to the stencil buffer. */
		STENCIL_WRITEMASK_BITOFFSET = 0x2C, /*!< \brief the bit offset to get just the write mask bits. */

		STENCIL_OP_SFAIL_BITS = 0x70000000000, /*!< \brief the bit offset to get the stencil fail operation for when stencil testing. */
		STENCIL_OP_SFAIL_BITOFFSET = 0x28, /*!< \brief the bit offset to get the stencil fail operation bits. */
		STENCIL_OP_DFAIL_BITS = 0x380000000000, /*!< \brief the bit offset to get the stencil depth fail operation for when stencil testing. */
		STENCIL_OP_DFAIL_BITOFFSET = 0x2B, /*!< \brief the bit offset to get the stencil depth fail operation bits. */
		STENCIL_OP_PASS_BITS = 0x1C00000000000, /*!< \brief the bit offset to get the stencil pass operation for when stencil testing. */
		STENCIL_OP_PASS_BITOFFSET = 0x2E, /*!< \brief the bit offset to get the stencil pass operation bits. */

		BLEND_ZERO = 0x0, /*!< \brief blend mode multiplies all components to 0. */
		BLEND_ONE = 0x1, /*!< \brief blend mode multiplies all components to 1. */
		BLEND_SRC_COLOR = 0x2, /*!< \brief blend mode multiples all components to the source color component. */
		BLEND_DST_COLOR = 0x3, /*!< \brief blend mode multiplies all components to the destination color. */
		BLEND_SRC_ALPHA = 0x4, /*!< \brief blend mode multiplies by source alpha. */
		BLEND_DST_ALPHA = 0x5, /*!< \brief blend mode multiplies by destination alpha. */
		BLEND_ONE_MINUS_SRC_COLOR = 0x6, /*!< \brief blend mode multiplies by the inverse source colors. */
		BLEND_ONE_MINUS_DST_COLOR = 0x7, /*!< \brief blend mode multiplies by the inverse dest colors. */
		BLEND_ONE_MINUS_SRC_ALPHA = 0x8, /*!< \brief blend mode multiplies by the inverse source alpha. */
		BLEND_ONE_MINUS_DST_ALPHA = 0x9, /*!< \brief blend mode multiplies by the inverse dest alpha. */

		ALWAYS = 0x0, /*!< \brief comparison func is set to always occur. */
		NEVER = 0x1, /*!< \brief comparison func is set to never occur. */
		LESS = 0x2, /*!< \brief comparison func is set to occur on less than. */
		GREATER = 0x3, /*!< \brief comparison func is set to occur on greater than. */
		LESS_EQL = 0x4, /*!< \brief comparison func is set to occur on less than or equal to. */
		GREATER_EQL = 0x5, /*!< \brief comparison func is set to occur on greater than or equal to. */

		InternalPipeline = 0x1000000000000000, /*<! \brief flag indicating the pipeline is an internal cached version. */
		ComputePipeline = 0x2000000000000000, /*!< \brief flag indicating the pipeline is a compute shader. */
		DirtyStages = 0x4000000000000000, /*!< \brief flag indicating the pipeline stages themselves are dirty. */
		Dirty = 0x8000000000000000, /*!< \brief flag indicating the resources have changed. */
		RasterFlags = ~(InternalPipeline | ComputePipeline | DirtyStages | Dirty) /*!< \brief bitwise flags to get just the raster flags for the pipeline. */
	};
	/*!< \brief Set's the index of the uniform block to the specified video buffer. */
	LWPipeline &SetUniformBlock(uint32_t i, LWVideoBuffer *Buffer, uint32_t Offset = 0);

	/*!< \brief searches for the specified Named uniform and set's the uniform block buffer if the named block is found. */
	LWPipeline &SetUniformBlock(const LWText &Name, LWVideoBuffer *Buffer, uint32_t Offset = 0);

	/*!< \brief set's the index of the uniform block to the specified video buffer, and calculates the padded offset for the specified type. */
	template<class Type>
	LWPipeline &SetPaddedUniformBlock(uint32_t i, LWVideoBuffer *Buffer, uint32_t Offset, LWVideoDriver *Driver) {
		return SetUniformBlock(i, Buffer, Driver->GetUniformBlockOffset<Type>(Offset));
	}

	/*!< \brief set's the named block of the uniform block to the specified video buffer, and calculates the padded offset for the specified type. */
	template<class Type>
	LWPipeline &SetPaddedUniformBlock(const LWText &Name, LWVideoBuffer *Buffer, uint32_t Offset, LWVideoDriver *Driver) {
		return SetUniformBlock(Name, Buffer, Driver->GetUniformBlockOffset<Type>(Offset));
	}

	/*!< \brief Set's the mapped index of the resource to the specified video buffer(uniform buffers/texturebuffers/std430 buffers). */
	LWPipeline &SetResource(uint32_t i, LWVideoBuffer *Buffer, uint32_t Offset = 0);

	/*!< \brief changes the pipeline's vertex shader stage to the specified shader. (note: resource maps are not created until Driver->SetPipeline is called either directly, or indirectly). */
	LWPipeline &SetVertexShader(LWShader *Shader);

	/*!< \brief changes the pipeline's geometry shader stage to the specified shader. */
	LWPipeline &SetGeometryShader(LWShader *Shader);

	/*!< \brief changes the pipeline's pixel shader stage to the specified shader. */
	LWPipeline &SetPixelShader(LWShader *Shader);

	/*!< \brief changes the pipeline's compute shader stage to the specified shader. */
	LWPipeline &SetComputeShader(LWShader *Shader);

	/*!< \brief set's the index of the resource to the specified textured. */
	LWPipeline &SetResource(uint32_t i, LWTexture *Texture);
	
	/*!< \brief searches for the specified Named resource and set's the resource to the texture if the named resource is found. */
	LWPipeline &SetResource(const LWText &Name, LWTexture *Texture);

	/*!< \brief searches for the specified Named resource and set's the resource to the video buffer(uniform buffers/texturebuffer/shader storage object buffers) if the named resource is found. */
	LWPipeline &SetResource(const LWText &Name, LWVideoBuffer *Buffer, uint32_t Offset = 0);

	/*!< \brief change's raster mode to enable/disable depth testing, and what comparison to use. */
	LWPipeline &SetDepthMode(bool doDepthTest, uint64_t CompareMode);

	/*!< \brief change's raster mode to enable/disable blending, and what src/dst blending modes to use. */
	LWPipeline &SetBlendMode(bool doBlending, uint64_t SrcBlendMode, uint64_t DstBlendMode);

	/*!< \brief changes raster mode to enable/disable stenciling and what comparison to use. 
		 \param StencilFailOp Operation to do when the stencil test fails.
		 \param DepthFailOp Operation to do when the stencil test passes, but the depth test fails.
		 \param PassOp Operation to do when the stencil and depth test passes. 
		 \param ReadMask the read mask to be applied when comparing stencil values(0-0xff).
		 \param WriteMask the write mask to be applied when writing the stencil value(0-0xff)
	*/
	LWPipeline &SetStencilMode(bool doStencilTest, uint64_t CompareMode, uint64_t StencilFailOp, uint64_t DepthFailOp, uint64_t PassOp);

	/*!< \brief set's the stencil's read mask when doing compaison. */
	LWPipeline &SetStencilReadMask(uint64_t ReadMask);

	/*!< \brief set's the stencil's write mask when writing a new value. */
	LWPipeline &SetStencilWriteMask(uint64_t WriteMask);

	/*!< \brief set's the reference stencil value to be compared against. (0-0xFF) */
	LWPipeline &SetStencilValue(uint64_t StencilValue);

	/*!< \brief change's culling mode to the specefied cull. */
	LWPipeline &SetCullMode(uint64_t CullMode);

	/*!< \brief enable's/disable's writing to the depth buffer. */
	LWPipeline &SetDepthOutput(bool depthOut);

	/*!< \brief enable's/disable's writing to all color channels. */
	LWPipeline &SetColorOutput(bool colorOut);

	/*!< \brief enable's/disable's writing to individual color channels. */
	LWPipeline &SetColorChannelOutput(bool rOut, bool gOut, bool bOut, bool aOut);

	/*!< \brief change's raster mode for filling triangles. */
	LWPipeline &SetFillMode(uint64_t FillMode);

	/*!< \brief change's raster mode for enabling/disabling different clipping planes. */
	LWPipeline &SetClipping(bool clipPlane0, bool clipPlane1 = false, bool clipPlane2 = false);

	/*!< \brief changes the overall flag for raster operations. */
	LWPipeline &SetFlag(uint64_t Flag);

	/*!< \brief changes the depth bias and sloped scaled bias. */
	LWPipeline &SetDepthBias(bool Enabled, float Bias = 0.0f, float SlopedScaleBias = 0.0f);

	/*!< \brief returns true if the dirty flag is set. */
	bool isDirty(void) const;

	/*!< \brief clears the dirty flag. */
	LWPipeline &ClearDirty(void);

	/*!< \brief looks up all resources to find resource with specified name, returns -1 if not found. */
	uint32_t FindResource(const LWText &Name);

	/*!< \brief looks up all blocks to find a block with the specified name, returns -1 if not found. */
	uint32_t FindBlock(const LWText &Name);

	/*!< \brief looks up all inputs to find a input with the specified name, returns -1 if not found. */
	uint32_t FindInput(const LWText &Name);

	/*!< \brief returns the pipeline resource object at the specified index(non mapped resource position). */
	LWShaderResource &GetResource(uint32_t i);

	/*!< \brief returns the pipeline block object at the specified index(non mapped block position). */
	LWShaderResource &GetBlock(uint32_t i);

	/*!< \brief returns the pipeline's internal resource list. */
	LWShaderResource *GetResourceList(void);

	/*!< \brief returns the pipeline's internal block list. */
	LWShaderResource *GetBlockList(void);

	/*!< \brief returns the pipeline's internal resource mapping. */
	uint32_t *GetResourceMap(void);

	/*!< \brief returns the pipeline's internal block mapping. */
	uint32_t *GetBlockMap(void);

	/*!< \brief returns the pipeline's internal input list. */
	LWShaderInput *GetInputList(void);

	/*!< \brief returns the input object at the specified index. */
	LWShaderInput &GetInput(uint32_t i);

	/*!< \brief rebuilds the internal mapping's based on the bound shader maps. */
	LWPipeline &BuildMappings(void);

	/*!< \brief clones a pipeline to this pipeline while preserving the underlying resource bindings/mappings(this is called internally by LWVideoDriver and should never need to be called by an application.) */
	LWPipeline &ClonePipeline(LWPipeline *Pipe);

	/*!< \brief returns the total number of inputs. */
	uint32_t GetInputCount(void) const;

	/*!< \brief returns the total number of resources the pipeline has. */
	uint32_t GetResourceCount(void) const; 

	/*!< \brief returns the total number of blocks the pipeline has. */
	uint32_t GetBlockCount(void) const;

	/*!< \brief returns the flag with the current raster operations. */
	uint64_t GetFlag(void) const;

	/*!< \brief returns the current depth comparison mode. */
	uint64_t GetDepthCompareMode(void) const;

	/*!< \brief returns the current stencil comparison mode. */
	uint64_t GetStencilCompareMode(void) const;

	/*!< \brief returns the current stencil operation when stencil test fails. */
	uint64_t GetStencilSFailOp(void) const;

	/*!< \brief returns the current stencil operation when stencil depth test fails. */
	uint64_t GetStencilDFailOp(void) const;

	/*!< \brief returns the current stencil operation when stencil test passes. */
	uint64_t GetStencilPassOp(void) const;

	/*!< \brief returns the current stencil value being compared against. */
	uint64_t GetStencilValue(void) const;

	/*!< \brief returns the current stencil read mask for comparing values with. */
	uint64_t GetStencilReadMask(void) const;

	/*!< \brief returns the current stencil write mask for writing values ot the stencil buffer with. */
	uint64_t GetStencilWriteMask(void) const;

	/*!< \brief returns the source blending operation. */
	uint64_t GetSrcBlendMode(void) const;

	/*!< \brief returns the destination blending operation. */
	uint64_t GetDstBlendMode(void) const;

	/*!< \brief returns the current fill mode set. */
	uint64_t GetFillMode(void) const;

	/*!< \brief returns the current culling mode set. */
	uint64_t GetCullMode(void) const;

	/*!< \brief returns the bias to be added to the depth buffer if DEPTH_BIAS is flagged. */
	float GetBias(void) const;

	/*!< \brief returns the bias scaling based on slopw to be added to the buffer if DEPTH_BIAS is flagged. */
	float GetSlopedBias(void) const;

	/*!< \brief returns a unique hash for the pipeline based solely on the shader stages attached. */
	uint32_t GetPipelineHash(void);

	/*!< \brief returns the currently bound shader at the specified stage. */
	LWShader *GetShaderStage(uint32_t Stage);

	/*!< \brief returns true if the compute stage has been set for the shader. */
	bool isComputePipeline(void) const;

	/*!< \brief returns true if the pipeline object is an internal mapped object, this function should never need to be called by the application. */
	bool isInternalPipeline(void) const;

	/*!< \brief constructs a pipeline object. */
	LWPipeline(LWShader **Stages, LWShaderResource *BlockList, LWShaderResource *ResourceList, LWShaderInput *InputList, uint32_t BlockCount, uint32_t ResourceCount, uint32_t InputCount, uint64_t Flag);
private:
	LWShader *m_ShaderStages[StageCount];
	LWShaderResource m_BlockList[LWShader::MaxBlocks];
	LWShaderResource m_ResourceList[LWShader::MaxResources];
	LWShaderInput m_InputList[LWShader::MaxInputs];
	uint32_t m_BlockMap[LWShader::MaxBlocks];
	uint32_t m_ResourceMap[LWShader::MaxResources];

	uint64_t m_Flag = 0;
	uint32_t m_ResourceCount = 0;
	uint32_t m_BlockCount = 0;
	uint32_t m_InputCount = 0;
	float m_Bias = 0.0f;
	float m_SlopedBias = 0.0f;
	uint32_t m_PipelineHash = 0;
};

template<class Con>
class LWPipelineCon : public LWPipeline {
public:
	Con &GetContext(void) {
		return m_Context;
	}
	
	LWPipelineCon(Con &Context, LWShader **Stages, LWShaderResource *BlockList, LWShaderResource *ResourceList, LWShaderInput *InputList, uint32_t BlockCount, uint32_t ResourceCount, uint32_t InputCount, uint64_t Flag) : LWPipeline(Stages, BlockList, ResourceList, InputList, BlockCount, ResourceCount, InputCount, Flag), m_Context(Context) {}
private:
	Con m_Context;
};

#endif