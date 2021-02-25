#include "LWVideo/LWShader.h"
#include "LWCore/LWUnicode.h"
#include <cstdarg>

//LWShaderInput:
LWShaderInput::LWShaderInput(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length) : m_NameHash(Name.Hash()), m_Type(Type), m_Length(Length) {}

LWShaderInput::LWShaderInput(uint32_t NameHash, uint32_t Type, uint32_t Length) : m_NameHash(NameHash), m_Type(Type), m_Length(Length) {}

//LWShaderResource:
LWShaderResource &LWShaderResource::SetStageBinding(uint32_t StageID, uint32_t Idx) {
	uint32_t StageBits = VertexBindingBits << (StageID * BindingBitCount);
	m_StageBindings = (m_StageBindings & ~StageBits) | (Idx << (StageID * BindingBitCount));
	m_Flag |= (VertexStage << StageID);
	return *this;
}

LWShaderResource &LWShaderResource::SetVertexStageBinding(uint32_t Idx) {
	m_StageBindings = LWBitFieldSet(VertexBinding, m_StageBindings, Idx);
	m_Flag |= VertexStage;
	return *this;
}

LWShaderResource &LWShaderResource::SetComputeStageBinding(uint32_t Idx) {
	m_StageBindings = LWBitFieldSet(ComputeBinding, m_StageBindings, Idx);
	m_Flag |= ComputeStage;
	return *this;
}

LWShaderResource &LWShaderResource::SetPixelStageBinding(uint32_t Idx) {
	m_StageBindings = LWBitFieldSet(PixelBinding, m_StageBindings, Idx);
	m_Flag |= PixelStage;
	return *this;
}

LWShaderResource &LWShaderResource::SetGeometryStageBinding(uint32_t Idx) {
	m_StageBindings = LWBitFieldSet(GeometryBinding, m_StageBindings, Idx);
	m_Flag |= GeometryStage;
	return *this;
}

uint32_t LWShaderResource::GetTypeID(void) const {
	return LWBitFieldGet(Type, m_Flag);
}

uint32_t LWShaderResource::GetLength(void) const {
	return LWBitFieldGet(Length, m_Flag);
}

uint32_t LWShaderResource::GetStageBinding(uint32_t StageID) const {
	uint32_t StageBits = VertexBindingBits << (StageID * BindingBitCount);
	return (m_StageBindings & StageBits) >> (StageID * BindingBitCount);
}

uint32_t LWShaderResource::GetVertexStageBinding(void) const {
	return LWBitFieldGet(VertexBinding, m_StageBindings);
}

uint32_t LWShaderResource::GetComputeStageBinding(void) const {
	return LWBitFieldGet(ComputeBinding, m_StageBindings);
}

uint32_t LWShaderResource::GetPixelStageBinding(void) const {
	return LWBitFieldGet(PixelBinding, m_StageBindings);
}

uint32_t LWShaderResource::GetGeometryStageBinding(void) const {
	return LWBitFieldGet(GeometryBinding, m_StageBindings);
}

bool LWShaderResource::HasStage(uint32_t StageID) const {
	return (m_Flag & (VertexStage << StageID)) != 0;
}

bool LWShaderResource::hasVertexStage(void) const {
	return (m_Flag & VertexStage) != 0;
}

bool LWShaderResource::hasComputeStage(void) const {
	return (m_Flag & ComputeStage) != 0;
}

bool LWShaderResource::hasPixelStage(void) const {
	return (m_Flag & PixelStage) != 0;
}

bool LWShaderResource::hasGeometryStage(void) const {
	return (m_Flag & GeometryStage) != 0;
}

LWShaderResource::LWShaderResource(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length) : m_NameHash(Name.Hash()) {
	m_Flag = (Type << TypeBitsOffset) | (Length << LengthBitsOffset);
}

LWShaderResource::LWShaderResource(uint32_t NameHash, uint32_t Type, uint32_t Length) : m_NameHash(NameHash) {
	m_Flag = (Type << TypeBitsOffset) | (Length << LengthBitsOffset);
}

LWShaderResource::LWShaderResource(uint32_t NameHash, uint32_t Type, uint32_t Length, uint32_t StageID, uint32_t StageBindIdx) : m_NameHash(NameHash) {
	m_Flag = (Type << TypeBitsOffset) | (Length << LengthBitsOffset);
	SetStageBinding(StageID, StageBindIdx);
}

LWShaderResource::LWShaderResource(uint32_t NameHash, uint32_t Type, uint32_t Length, uint32_t StageBindIdx) : m_NameHash(NameHash), m_StageBindings(StageBindIdx) {
	m_Flag = (Type << TypeBitsOffset) | (Length << LengthBitsOffset);
}

//LWShader:
uint32_t LWShader::GenerateInputOffsets(uint32_t Count, LWShaderInput *InputMap) {
	//	                           Float, UInt, Int, Double, Vec2, Vec3, Vec4, uvec2, uvec3, uvec4, iVec2, iVec3, iVec4, dVec2, dVec3, dVec4
	const uint32_t TypeSizes[] = { 4,     4,    4,   8,      8,    12,   16,   8,     12,    16,    8,     12,    16,    16,    24,    32 };
	uint32_t Offset = 0;
	for (uint32_t i = 0; i < Count; i++) {
		uint32_t Size = TypeSizes[InputMap[i].m_Type] * InputMap[i].m_Length;
		uint32_t Remain = (Offset % 16 == 0 ? 0 : 16 - (Offset % 16));
		if (Size > Remain) Offset += Remain;
		InputMap[i].m_Offset = Offset;
		Offset += Size;
	}
	return Offset;
}

LWShader &LWShader::SetInputMap(uint32_t Count, LWShaderInput *InputMap) {
	GenerateInputOffsets(Count, InputMap);
	std::copy(InputMap, InputMap + Count, m_InputMap);
	m_InputCount = Count;
	return *this;
}

LWShader &LWShader::SetResourceMap(uint32_t Count, uint32_t *NameHashs) {
	for (uint32_t i = 0; i < Count; i++) {
		m_ResourceMap[i].m_NameHash = NameHashs[i];
	}
	m_ResourceCount = Count;
	return *this;
}

LWShader &LWShader::SetBlockMap(uint32_t Count, uint32_t *NameHashs) {
	for (uint32_t i = 0; i < Count; i++) {
		m_BlockMap[i].m_NameHash = NameHashs[i];
	}
	m_BlockCount = Count;
	return *this;
}

uint32_t LWShader::GetShaderType(void) const {
	return m_Type;
}

uint32_t LWShader::GetShaderHash(void) const {
	return m_ShaderHash;
}

const LWShaderInput *LWShader::GetInputMap(void) const {
	return m_InputMap;
}

const LWShaderResource *LWShader::GetBlockMap(void) const {
	return m_BlockMap;
}

const LWShaderResource *LWShader::GetResourceMap(void) const {
	return m_ResourceMap;
}

uint32_t LWShader::GetInputCount(void) const{
	return m_InputCount;
}

uint32_t LWShader::GetBlockMapCount(void) const {
	return m_BlockCount;
}

uint32_t LWShader::GetResourceMapCount(void) const {
	return m_ResourceCount;
}

LWShader::LWShader(uint32_t ShaderHash, uint32_t ShaderType) : m_ShaderHash(ShaderHash), m_Type(ShaderType) {}