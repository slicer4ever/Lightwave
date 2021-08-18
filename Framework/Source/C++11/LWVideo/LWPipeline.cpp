#include "LWVideo/LWPipeline.h"
#include "LWCore/LWUnicode.h"
#include "LWCore/LWCrypto.h"
#include "LWVideo/LWVideoDriver.h"
#include <cstdarg>


LWPipeline &LWPipeline::SetUniformBlock(uint32_t i, LWVideoBuffer *Buffer, uint32_t Offset) {
	LWShaderResource &B = m_BlockList[m_BlockMap[i]];
	if (B.m_Resource == Buffer && B.m_Offset == Offset) return *this;
	B.m_Resource = Buffer;
	B.m_Offset = Offset;
	m_Flag |= Dirty;
	return *this;
}

LWPipeline &LWPipeline::SetUniformBlock(const LWUTF8Iterator &Name, LWVideoBuffer *Buffer, uint32_t Offset) {
	uint32_t i = FindBlock(Name);
	if (i == -1) return *this;
	return SetUniformBlock(i, Buffer, Offset);
}

LWPipeline &LWPipeline::SetResource(uint32_t i, LWVideoBuffer *Buffer, uint32_t Offset) {
	LWShaderResource &R = m_ResourceList[m_ResourceMap[i]];
	if (R.m_Resource == Buffer && R.m_Offset == Offset) return *this;
	R.m_Resource = Buffer;
	R.m_Offset = Offset;
	m_Flag |= Dirty;
	return *this;
}

LWPipeline &LWPipeline::SetResource(uint32_t i, LWTexture *Texture) {
	LWShaderResource &R = m_ResourceList[m_ResourceMap[i]];
	if (R.m_Resource == Texture) return *this;
	R.m_Resource = Texture;
	m_Flag |= Dirty;
	return *this;
}

LWPipeline &LWPipeline::SetResource(const LWUTF8Iterator &Name, LWVideoBuffer *Buffer, uint32_t Offset) {
	uint32_t i = FindResource(Name);
	if (i == -1) return *this;
	return SetResource(i, Buffer, Offset);
}

LWPipeline &LWPipeline::SetResource(const LWUTF8Iterator &Name, LWTexture *Texture) {
	uint32_t i = FindResource(Name);
	if (i == -1) return *this;
	return SetResource(i, Texture);
}

LWPipeline &LWPipeline::SetDepthMode(bool doDepthTest, uint64_t CompareMode) {
	m_Flag = (m_Flag&~(DEPTH_TEST | DEPTH_COMPARE_BITS)) | (doDepthTest ? DEPTH_TEST : 0) | (CompareMode << DEPTH_COMPARE_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetBlendMode(bool doBlending, uint64_t SrcBlendMode, uint64_t DstBlendMode) {
	m_Flag = (m_Flag&~(BLENDING | BLEND_SRC_BITS | BLEND_DST_BITS)) | (doBlending ? BLENDING : 0) | (SrcBlendMode << BLEND_SRC_BITOFFSET) | (DstBlendMode << BLEND_DST_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetStencilMode(bool doStencilTest, uint64_t CompareMode, uint64_t StencilFailOp, uint64_t DepthFailOp, uint64_t PassOp){
	m_Flag = (m_Flag&~(STENCIL_TEST | STENCIL_COMPARE_BITS | STENCIL_OP_SFAIL_BITS | STENCIL_OP_DFAIL_BITS | STENCIL_OP_PASS_BITS)) | (doStencilTest ? 0 : STENCIL_TEST) | (CompareMode<<STENCIL_COMPARE_BITOFFSET) | (StencilFailOp << STENCIL_OP_SFAIL_BITOFFSET) | (DepthFailOp << STENCIL_OP_DFAIL_BITOFFSET) | (PassOp << STENCIL_OP_PASS_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetStencilReadMask(uint64_t ReadMask) {
	m_Flag = (m_Flag&~(STENCIL_READMASK_BITS)) | (ReadMask << STENCIL_READMASK_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetStencilWriteMask(uint64_t WriteMask) {
	m_Flag = (m_Flag&~(STENCIL_WRITEMASK_BITS)) | (WriteMask << STENCIL_WRITEMASK_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetStencilValue(uint64_t StencilValue){
	m_Flag = (m_Flag&~STENCIL_REF_VALUE_BITS) | (StencilValue << STENCIL_REF_VALUE_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetCullMode(uint64_t CullMode) {
	m_Flag = (m_Flag&~(CULL_BITS)) | (CullMode<<CULL_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetDepthOutput(bool depthOut) {
	m_Flag = (m_Flag&~No_Depth) | (depthOut ? 0 : No_Depth);
	return *this;
}

LWPipeline &LWPipeline::SetColorOutput(bool colorOut) {
	m_Flag = (m_Flag&~No_Color) | (colorOut ? 0 : No_Color);
	return *this;
}

LWPipeline &LWPipeline::SetColorChannelOutput(bool rOut, bool gOut, bool bOut, bool aOut) {
	m_Flag = (m_Flag&~No_Color) | (rOut ? 0 : No_ColorR) | (gOut ? 0 : No_ColorG) | (bOut ? 0 : No_ColorB) | (aOut ? 0 : No_ColorA);
	return *this;
}

LWPipeline &LWPipeline::SetFillMode(uint64_t FillMode) {
	m_Flag = (m_Flag&~FILL_MODE_BITS) | (FillMode<<FILL_MODE_BITOFFSET);
	return *this;
}

LWPipeline &LWPipeline::SetClipping(bool clipPlane0, bool clipPlane1, bool clipPlane2) {
	m_Flag = (m_Flag&~(CLIPPLANE0 | CLIPPLANE1 | CLIPPLANE2)) | (clipPlane0 ? CLIPPLANE0 : 0) | (clipPlane1 ? CLIPPLANE1 : 0) | (clipPlane2 ? CLIPPLANE2 : 0);
	return *this;
}

LWPipeline &LWPipeline::SetDepthBias(bool Enabled, float Bias, float SlopedScaleBias) {
	m_Flag = (m_Flag&~(DEPTH_BIAS)) | (Enabled ? DEPTH_BIAS : 0);
	m_Bias = Bias;
	m_SlopedBias = SlopedScaleBias;
	return *this;
}

uint32_t LWPipeline::MakeInterleavedInputStream(LWPipelineInputStream *StreamBuffer, const LWPipelineInputStream *InstanceStream, LWVideoBuffer *InterleavedBuffer, uint32_t InterleaveStride, uint32_t BufferCount) {
	uint32_t c = std::min<uint32_t>(BufferCount, m_InputCount);
	uint32_t n = 0;
	for (uint32_t i = 0; i < c; i++) {
		LWShaderInput &In = m_InputList[i];
		if (In.GetInstanceFrequency() == 0) StreamBuffer[i] = { InterleavedBuffer, In.GetOffset(), InterleaveStride };
		else StreamBuffer[i] = InstanceStream[n++];
	}
	return c;
}

LWPipeline &LWPipeline::SetVertexShader(LWShader *Shader) {
	if (m_ShaderStages[Vertex] == Shader) return *this;
	m_ShaderStages[Vertex] = Shader;
	m_Flag = (m_Flag&~ComputePipeline) | DirtyStages;
	return *this;
}

LWPipeline &LWPipeline::SetGeometryShader(LWShader *Shader) {
	if (m_ShaderStages[Geometry] == Shader) return *this;
	m_ShaderStages[Geometry] = Shader;
	m_Flag |= DirtyStages;
	return *this;
}

LWPipeline &LWPipeline::SetPixelShader(LWShader *Shader) {
	if (m_ShaderStages[Pixel] == Shader) return *this;
	m_ShaderStages[Pixel] = Shader;
	m_Flag |= DirtyStages;
	return *this;
}

LWPipeline &LWPipeline::SetComputeShader(LWShader *Shader) {
	if (m_ShaderStages[Compute] == Shader) return *this;
	m_ShaderStages[Compute] = Shader;
	m_Flag |= DirtyStages | ComputePipeline;
	return *this;
}


LWPipeline &LWPipeline::SetFlag(uint64_t Flag) {
	m_Flag = Flag | (m_Flag&(ComputePipeline|InternalPipeline));
	return *this;
}

bool LWPipeline::isDirty(void) const {
	return (m_Flag&(Dirty|DirtyStages));
}

LWPipeline &LWPipeline::ClearDirty(void){
	m_Flag &= ~(Dirty|DirtyStages);
	return *this;
}

uint32_t LWPipeline::FindResource(uint32_t NameHash) {
	for (uint32_t i = 0; i < m_ResourceCount; i++) {
		if (NameHash == m_ResourceList[m_ResourceMap[i]].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWPipeline::FindResource(const LWUTF8Iterator &Name) {
	return FindResource(Name.Hash());
}

uint32_t LWPipeline::FindBlock(uint32_t NameHash) {
	for (uint32_t i = 0; i < m_BlockCount; i++) {
		if (NameHash == m_BlockList[m_BlockMap[i]].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWPipeline::FindBlock(const LWUTF8Iterator &Name) {
	return FindBlock(Name.Hash());
}

uint32_t LWPipeline::FindInput(uint32_t NameHash) {
	for (uint32_t i = 0; i < m_InputCount; i++) {
		if (NameHash == m_InputList[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWPipeline::FindInput(const LWUTF8Iterator &Name) {
	return FindInput(Name.Hash());
}

LWPipeline &LWPipeline::BuildMappings(void) {
	uint32_t ResourceNameList[LWShader::MaxResources];
	uint32_t BlockNameList[LWShader::MaxBlocks];
	uint32_t ResourceCount = 0;
	uint32_t BlockCount = 0;

	auto InsertList = [](uint32_t NameHash, uint32_t *List, uint32_t &Cnt) {
		for (uint32_t i = 0; i < Cnt; i++) {
			if (List[i] == NameHash) return;
		}
		List[Cnt++] = NameHash;
		return;
	};

	auto InsertStage = [&InsertList, &ResourceNameList, &BlockNameList, &ResourceCount, &BlockCount](LWShader *Stage) {
		uint32_t RCount = Stage->GetResourceMapCount();
		uint32_t BCount = Stage->GetBlockMapCount();
		const LWShaderResource *RMap = Stage->GetResourceMap();
		const LWShaderResource *BMap = Stage->GetBlockMap();
		for (uint32_t i = 0; i < RCount; i++) InsertList(RMap[i].m_NameHash, ResourceNameList, ResourceCount);
		for (uint32_t i = 0; i < BCount; i++) InsertList(BMap[i].m_NameHash, BlockNameList, BlockCount);
		return;
	};

	if (isComputePipeline()) {
		LWShader *CS = m_ShaderStages[Compute];
		const LWShaderResource *CRM = CS->GetResourceMap();
		const LWShaderResource *CBM = CS->GetBlockMap();
		ResourceCount = CS->GetResourceMapCount();
		BlockCount = CS->GetBlockMapCount();
		for (uint32_t i = 0; i < ResourceCount; i++) ResourceNameList[i] = CRM[i].m_NameHash;
		for (uint32_t i = 0; i < BlockCount; i++) BlockNameList[i] = CBM[i].m_NameHash;
	} else {
		LWShader *VS = m_ShaderStages[Vertex];
		LWShader *GS = m_ShaderStages[Geometry];
		LWShader *PS = m_ShaderStages[Pixel];
		if (VS) {
			uint32_t VICount = VS->GetInputCount();
			const LWShaderInput *VInputs = VS->GetInputMap();
			if (!VICount) LWShader::GenerateInputOffsets(m_InputCount, m_InputList);
			else {
				//re-order input list(and add any missing input's) to match our mapping:
				for (uint32_t n = 0; n < VICount; n++) {
					bool Found = false;
					for (uint32_t i = 0; i < m_InputCount; i++) {
						if (m_InputList[i].m_NameHash != VInputs[n].m_NameHash) continue;
						std::swap(m_InputList[n], m_InputList[i]);
						m_InputList[n].m_Flag = (m_InputList[n].m_Flag & ~(LWShaderInput::OffsetBits | LWShaderInput::InstanceFrequencyBits)) | (VInputs[n].m_Flag & (LWShaderInput::OffsetBits | LWShaderInput::InstanceFrequencyBits));
						Found = true;
						break;
					}
					if (!Found) {
						fmt::print("Input map attribute not found: {:#x}(if an attribute was should not have stripped, check for any misspelling errors in defining input map).\n", VInputs[n].m_NameHash);
						m_InputList[m_InputCount++] = LWShaderInput(VInputs[n].m_NameHash, VInputs[n].GetType(), 0, VInputs[n].GetInstanceFrequency());
						std::swap(m_InputList[n], m_InputList[m_InputCount - 1]);
					}
				}
			}
			InsertStage(VS);
		}
		if (GS) InsertStage(GS);
		if (PS) InsertStage(PS);
	}
	for (uint32_t n = 0; n < ResourceCount; n++) {
		uint32_t i = 0;
		for (; i < m_ResourceCount; i++) {
			if (ResourceNameList[n] != m_ResourceList[i].m_NameHash) continue;
			m_ResourceMap[n] = i;
			break;
		}
		if (i >= m_ResourceCount) m_ResourceMap[n] = m_ResourceCount;
	}
	for (uint32_t n = 0; n < BlockCount; n++) {
		uint32_t i = 0;
		for (; i < m_BlockCount; i++) {
			if (BlockNameList[n] != m_BlockList[i].m_NameHash) continue;
			m_BlockMap[n] = i;
			break;
		}
		if (i >= m_BlockCount) m_BlockMap[n] = m_BlockCount;
	}
	return *this;
}

LWPipeline &LWPipeline::ClonePipeline(LWPipeline *Pipe) {
	LWShaderInput *InputList = Pipe->GetInputList();
	LWShaderResource *BlockList = Pipe->GetBlockList();
	LWShaderResource *ResourceList = Pipe->GetResourceList();
	uint32_t *BlockMap = Pipe->GetBlockMap();
	uint32_t *ResourceMap = Pipe->GetResourceMap();
	m_InputCount = Pipe->GetInputCount();
	m_BlockCount = Pipe->GetBlockCount();
	m_ResourceCount = Pipe->GetResourceCount();
	std::copy(InputList, InputList + m_InputCount, m_InputList);
	for (uint32_t i = 0; i < m_BlockCount; i++) {
		m_BlockList[i].m_NameHash = BlockList[i].m_NameHash;
		m_BlockList[i].m_Flag = BlockList[i].m_Flag;
		m_BlockList[i].m_StageBindings = BlockList[i].m_StageBindings;
	}
	std::copy(ResourceMap, ResourceMap + LWShader::MaxResources, m_ResourceMap);
	for (uint32_t i = 0; i < m_ResourceCount; i++) {
		m_ResourceList[i].m_NameHash = ResourceList[i].m_NameHash;
		m_ResourceList[i].m_Flag = ResourceList[i].m_Flag;
		m_ResourceList[i].m_StageBindings = ResourceList[i].m_StageBindings;
	}
	std::copy(BlockMap, BlockMap + LWShader::MaxBlocks, m_BlockMap);
	return *this;
}

LWShaderResource &LWPipeline::GetResource(uint32_t i) {
	return m_ResourceList[i];
}

LWShaderResource &LWPipeline::GetBlock(uint32_t i) {
	return m_BlockList[i];
}

LWShaderInput &LWPipeline::GetInput(uint32_t i) {
	return m_InputList[i];
}

LWShaderInput *LWPipeline::GetInputList(void) {
	return m_InputList;
}

LWShaderResource *LWPipeline::GetBlockList(void) {
	return m_BlockList;
}

LWShaderResource *LWPipeline::GetResourceList(void) {
	return m_ResourceList;
}

uint32_t *LWPipeline::GetBlockMap(void) {
	return m_BlockMap;
}

uint32_t *LWPipeline::GetResourceMap(void) {
	return m_ResourceMap;
}

uint32_t LWPipeline::GetInputCount(void) const {
	return m_InputCount;
}

uint32_t LWPipeline::GetResourceCount(void) const {
	return m_ResourceCount;
}

uint32_t LWPipeline::GetBlockCount(void) const{
	return m_BlockCount;
}

uint64_t LWPipeline::GetFlag(void) const {
	return m_Flag;
}

uint64_t LWPipeline::GetDepthCompareMode(void) const {
	return (m_Flag&DEPTH_COMPARE_BITS) >> DEPTH_COMPARE_BITOFFSET;
}

uint64_t LWPipeline::GetStencilCompareMode(void) const {
	return (m_Flag&STENCIL_COMPARE_BITS) >> STENCIL_COMPARE_BITOFFSET;
}

uint64_t LWPipeline::GetStencilValue(void) const {
	return (m_Flag&STENCIL_REF_VALUE_BITS) >> STENCIL_REF_VALUE_BITOFFSET;
}

uint64_t LWPipeline::GetStencilSFailOp(void) const {
	return (m_Flag&STENCIL_OP_SFAIL_BITS) >> STENCIL_OP_SFAIL_BITOFFSET;
}

uint64_t LWPipeline::GetStencilDFailOp(void) const {
	return (m_Flag&STENCIL_OP_DFAIL_BITS) >> STENCIL_OP_DFAIL_BITOFFSET;
}

uint64_t LWPipeline::GetStencilPassOp(void) const {
	return (m_Flag&STENCIL_OP_PASS_BITS) >> STENCIL_OP_PASS_BITOFFSET;
}

uint64_t LWPipeline::GetStencilReadMask(void) const {
	return (m_Flag&STENCIL_READMASK_BITS) >> STENCIL_READMASK_BITOFFSET;
}

uint64_t LWPipeline::GetStencilWriteMask(void) const {
	return (m_Flag&STENCIL_WRITEMASK_BITS) >> STENCIL_WRITEMASK_BITOFFSET;
}

uint64_t LWPipeline::GetSrcBlendMode(void) const {
	return (m_Flag&BLEND_SRC_BITS) >> BLEND_SRC_BITOFFSET;
}

uint64_t LWPipeline::GetDstBlendMode(void) const {
	return (m_Flag&BLEND_DST_BITS) >> BLEND_DST_BITOFFSET;
}

uint64_t LWPipeline::GetCullMode(void) const {
	return (m_Flag&CULL_BITS) >> CULL_BITOFFSET;
}

uint64_t LWPipeline::GetFillMode(void) const {
	return (m_Flag&FILL_MODE_BITS) >> FILL_MODE_BITOFFSET;
}

float LWPipeline::GetBias(void) const {
	return m_Bias;
}

float LWPipeline::GetSlopedBias(void) const {
	return m_SlopedBias;
}

uint32_t LWPipeline::GetPipelineHash(void) {
	if ((m_Flag&DirtyStages) == 0) return m_PipelineHash;
	m_PipelineHash = 0;
	uint32_t Buffer[StageCount];
	for (uint32_t i = 0; i < StageCount; i++) {
		if (m_ShaderStages[i]) Buffer[i] = m_ShaderStages[i]->GetShaderHash();
		else Buffer[i] = 0;
	}
	m_PipelineHash = LWCrypto::HashFNV1A((uint8_t*)Buffer, sizeof(Buffer));
	return m_PipelineHash;
}

LWShader *LWPipeline::GetShaderStage(uint32_t Stage) {
	return m_ShaderStages[Stage];
}

bool LWPipeline::isComputePipeline(void) const{
	return (m_Flag&ComputePipeline) != 0;
}

bool LWPipeline::isInternalPipeline(void) const {
	return (m_Flag&InternalPipeline) != 0;
}

LWPipeline::LWPipeline(LWShader **Stages, LWShaderResource *BlockList, LWShaderResource *ResourceList, LWShaderInput *InputList, uint32_t BlockCount, uint32_t ResourceCount, uint32_t InputCount, uint64_t Flag) : m_ResourceCount(ResourceCount), m_BlockCount(BlockCount), m_InputCount(InputCount), m_Flag(Flag | Dirty | DirtyStages) {
	std::copy(Stages, Stages + StageCount, m_ShaderStages);
	std::copy(InputList, InputList + InputCount, m_InputList);
	std::copy(BlockList, BlockList + BlockCount, m_BlockList);
	std::copy(ResourceList, ResourceList + ResourceCount, m_ResourceList);
	for (uint32_t i = 0; i < LWShader::MaxResources; i++) m_ResourceMap[i] = i;
	for (uint32_t i = 0; i < LWShader::MaxBlocks; i++) m_BlockMap[i] = i;
}
