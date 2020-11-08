#include "LWVideo/LWShader.h"
#include "LWCore/LWUnicode.h"
#include <cstdarg>


LWShaderInput::LWShaderInput(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length) : m_NameHash(Name.Hash()), m_Type(Type), m_Length(Length) {}

LWShaderInput::LWShaderInput(uint32_t NameHash, uint32_t Type, uint32_t Length) : m_NameHash(NameHash), m_Type(Type), m_Length(Length) {}

uint32_t LWShaderResource::GetTypeID(void) {
	return (m_Flag&TypeBits) >> TypeBitOffset;
}

uint32_t LWShaderResource::GetLength(void) {
	return (m_Flag&LengthBits) >> LengthBitOffset;
}

LWShaderResource::LWShaderResource(const LWUTF8Iterator &Name, uint32_t Flag, uint32_t Type, uint32_t Length) : m_NameHash(Name.Hash()) {
	m_Flag = Flag | (Type << TypeBitOffset) | (Length << LengthBitOffset);
}

LWShaderResource::LWShaderResource(uint32_t NameHash, uint32_t Flag, uint32_t Type, uint32_t Length) : m_NameHash(NameHash) {
	m_Flag = Flag | (Type << TypeBitOffset) | (Length << LengthBitOffset);
}

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