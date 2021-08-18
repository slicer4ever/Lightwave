#include "LWVideo/LWShader.h"
#include "LWCore/LWUnicode.h"
#include <cstdarg>

//LWShaderInput:
LWShaderInput &LWShaderInput::SetType(uint32_t lType) {
	m_Flag = LWBitFieldSet(Type, m_Flag, lType);
	return *this;
}

LWShaderInput &LWShaderInput::SetBindIndex(uint32_t lBindIndex) {
	m_Flag = LWBitFieldSet(BindIndex, m_Flag, lBindIndex);
	return *this;
}

LWShaderInput &LWShaderInput::SetOffset(uint32_t lOffset) {
	assert((lOffset % 4) == 0);
	lOffset /= 4;
	m_Flag = LWBitFieldSet(Offset, m_Flag, lOffset);
	return *this;
}

LWShaderInput &LWShaderInput::SetLength(uint32_t lLength) {
	m_Flag = LWBitFieldSet(Length, m_Flag, lLength);
	return *this;
}

LWShaderInput &LWShaderInput::SetInstanceFrequency(uint32_t lFrequency) {
	m_Flag = LWBitFieldSet(InstanceFrequency, m_Flag, lFrequency);
	return *this;
}

uint32_t LWShaderInput::GetType(void) const {
	return LWBitFieldGet(Type, m_Flag);
}

uint32_t LWShaderInput::GetBindIndex(void) const {
	return LWBitFieldGet(BindIndex, m_Flag);
}

uint32_t LWShaderInput::GetOffset(void) const {
	return (uint32_t)LWBitFieldGet(Offset, m_Flag)*4;
}

uint32_t LWShaderInput::GetLength(void) const {
	return LWBitFieldGet(Length, m_Flag);
}

uint32_t LWShaderInput::GetInstanceFrequency(void) const {
	return LWBitFieldGet(InstanceFrequency, m_Flag);
}


LWShaderInput::LWShaderInput(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length, uint32_t InstanceFreq) : m_NameHash(Name.Hash()), m_Flag((Type<<TypeBitsOffset) | (Length<<LengthBitsOffset) | (InstanceFreq << InstanceFrequencyBitsOffset)) {}

LWShaderInput::LWShaderInput(uint32_t NameHash, uint32_t Type, uint32_t Length, uint32_t InstanceFreq) : m_NameHash(NameHash), m_Flag((Type<<TypeBitsOffset) | (Length<<LengthBitsOffset) | (InstanceFreq << InstanceFrequencyBitsOffset)) {}

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
		uint32_t Size = TypeSizes[InputMap[i].GetType()] * InputMap[i].GetLength();
		uint32_t Remain = (Offset % 16 == 0 ? 0 : 16 - (Offset % 16));
		if (Size > Remain) Offset += Remain;
		InputMap[i].SetOffset(Offset);
		Offset += Size;
	}
	return Offset;
}

LWShader &LWShader::SetInputMap(uint32_t Count, const LWShaderInput *InputMap) {
	std::copy(InputMap, InputMap + Count, m_InputMap);
	GenerateInputOffsets(Count, m_InputMap);
	m_InputCount = Count;
	return *this;
}

LWShader &LWShader::SetResourceMap(uint32_t Count, const uint32_t *NameHashs) {
	for (uint32_t i = 0; i < Count; i++) {
		m_ResourceMap[i].m_NameHash = NameHashs[i];
	}
	m_ResourceCount = Count;
	return *this;
}

LWShader &LWShader::SetBlockMap(uint32_t Count, const uint32_t *NameHashs) {
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

const LWShaderInput *LWShader::FindInputMap(const LWUTF8Iterator &Name) const {
	return FindInputMap(Name.Hash());
}

const LWShaderInput *LWShader::FindInputMap(uint32_t NameHash) const {
	for (uint32_t i = 0; i < m_InputCount; i++) {
		if (m_InputMap[i].m_NameHash == NameHash) return &m_InputMap[i];
	}
	return nullptr;
}

const LWShaderResource *LWShader::FindResourceMap(const LWUTF8Iterator &Name) const {
	return FindResourceMap(Name.Hash());
}

const LWShaderResource *LWShader::FindResourceMap(uint32_t NameHash) const {
	for (uint32_t i = 0; i < m_ResourceCount; i++) {
		if (m_ResourceMap[i].m_NameHash == NameHash) return &m_ResourceMap[i];
	}
	return nullptr;
}

const LWShaderResource *LWShader::FindBlockMap(const LWUTF8Iterator &Name) const {
	return FindBlockMap(Name.Hash());
}

const LWShaderResource *LWShader::FindBlockMap(uint32_t NameHash) const {
	for (uint32_t i = 0; i < m_BlockCount; i++) {
		if (m_BlockMap[i].m_NameHash == NameHash) return &m_BlockMap[i];
	}
	return nullptr;
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