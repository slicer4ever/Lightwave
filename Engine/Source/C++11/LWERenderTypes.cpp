#include "LWERenderTypes.h"
#include "LWERenderer.h"
#include <LWCore/LWCrypto.h>


//LWERenderVideoBuffer
bool LWERenderVideoBuffer::UploadData(LWERenderer *Renderer, bool DiscardData) {
	m_ID = Renderer->PushPendingResource(LWERenderPendingResource(m_ID, DiscardData ? 0 : LWERenderPendingResource::NoDiscard, LWERenderPendingBuffer(m_Data, m_BufferType, m_BufferUsage, m_TypeSize, m_Count)));
	if (!m_ID) return false;
	if (DiscardData) m_Data = nullptr;
	return true;
}

bool LWERenderVideoBuffer::ReleaseData(LWERenderer *Renderer) {
	bool lSuccess = true;
	if (m_ID && Renderer) {
		lSuccess = Renderer->PushPendingResource(LWERenderPendingResource(m_ID, LWERenderPendingBuffer())) == 0;
	}
	m_Data = LWAllocator::Destroy(m_Data);
	return lSuccess;
}

LWERenderVideoBuffer::LWERenderVideoBuffer(uint32_t BufferType, uint32_t BufferUsage) : m_BufferType(BufferType), m_BufferUsage(BufferUsage) {}

LWERenderVideoBuffer::~LWERenderVideoBuffer() {
	LWAllocator::Destroy(m_Data);
}


//LWERenderMaterialTexture:
LWERenderMaterialTexture::LWERenderMaterialTexture(const LWUTF8I &ResourceName, uint32_t TextureID, uint32_t TextureState) : m_ResourceName(ResourceName.Hash()), m_TextureID(TextureID), m_TextureState(TextureState) {}

LWERenderMaterialTexture::LWERenderMaterialTexture(uint32_t ResourceName, uint32_t TextureID, uint32_t TextureState) : m_ResourceName(ResourceName), m_TextureID(TextureID), m_TextureState(TextureState) {}

//LWERenderMaterial:
uint32_t LWERenderMaterial::Hash(void) const {
	return LWCrypto::HashFNV1A((uint8_t*)this, sizeof(LWERenderMaterial));
}

uint32_t LWERenderMaterial::PushTexture(const LWERenderMaterialTexture &Texture, uint32_t TextureID) {
	if (m_TextureCount >= LWEMaxBindableTextures) return 0;
	m_TextureList[m_TextureCount++] = Texture;
	return (1<<TextureID);
}

void LWERenderMaterial::SetTextureModel(LWEGeometryModelData &ModelData, const LWERenderMaterialTexture &Texture, uint32_t TextureID, const LWVector4f &SubTexture) {
	m_TextureList[TextureID] = Texture;
	ModelData.m_HasTextureFlag |= Texture.m_TextureID ? (1 << TextureID) : 0;
	ModelData.m_SubTextures[TextureID] = SubTexture;
	m_TextureCount = std::max(m_TextureCount, TextureID + 1);
	return;
}

LWERenderMaterial::LWERenderMaterial(const LWUTF8I &PipelineName, LWERenderMaterialTexture *TextureList, uint32_t TextureCount) : LWERenderMaterial(PipelineName.Hash(), TextureList, TextureCount) {}

LWERenderMaterial::LWERenderMaterial(uint32_t PipelineNameHash, LWERenderMaterialTexture *TextureList, uint32_t TextureCount) : m_PipelineName(PipelineNameHash), m_TextureCount(TextureCount) {
	std::copy(TextureList, TextureList + m_TextureCount, m_TextureList);
}

//LWEGeometryModelData:
LWEGeometryModelData::LWEGeometryModelData(const LWSMatrix4f &Transform, uint32_t BoneID) : m_Transform(Transform), m_BoneID(BoneID) {}

//LWEShaderLightData:

LWBitField32Define(LWEShaderLightData::ShadowPassIDBits);
LWBitField32Define(LWEShaderLightData::ShadowSubPassIdxBits);
LWBitField32Define(LWEShaderLightData::ShadowLayerBits);
uint32_t LWEShaderLightData::MakeShadowFlag(uint32_t PassID, uint32_t SubPassIdx, uint32_t LayerID) {
	return (PassID << ShadowPassIDBitsOffset) | (SubPassIdx << ShadowSubPassIdxBitsOffset) | (LayerID << ShadowLayerBitsOffset);
}

LWEShaderLightData::LWEShaderLightData(float Intensity, const LWVector4f &Color) : m_Position(LWSVector4f(0.0f, 0.0f, 0.0f, -1.0f-Intensity)), m_Color(Color) {}

LWEShaderLightData::LWEShaderLightData(const LWSVector4f &Direction, const LWVector4f &Color) : m_Direction(Direction), m_Color(Color) {}

LWEShaderLightData::LWEShaderLightData(const LWSVector4f &Position, const LWSVector4f &Direction, float Theta, float Length, const LWVector4f &Color) : m_Position(Position.xyz1() + LWSVector4f(0.0f, 0.0f, 0.0f, Theta)), m_Direction(Direction.xyz0() + LWSVector4f(0.0f, 0.0f, 0.0f, Length)), m_Color(Color) {}

LWEShaderLightData::LWEShaderLightData(const LWSVector4f &Position, float InteriorRadi, float FalloffRadi, const LWVector4f &Color) : m_Position(Position.xyz1()), m_Direction(LWSVector4f(InteriorRadi, FalloffRadi, 0.0f, 0.0f)), m_Color(Color) {}


//LWEGeometryRenderable
const uint32_t LWEGeometryRenderable::BufferVideoBuffer; //Adds bit to DrawCount if BlockBufferNameHash should instead be interpreted as video buffer id's(with the upper 16 bit being vertex buffer id, and lower 16 being index buffer id).

LWEGeometryRenderable::LWEGeometryRenderable(uint64_t BlockBufferNameHash, const LWERenderMaterial &Material, uint32_t DrawCount) : m_BlockBufferNameHash(BlockBufferNameHash), m_Material(Material), m_DrawCount(DrawCount) {}

//LWEShaderGlobalData:

//LWEShaderPassData:
LWEShaderPassData::LWEShaderPassData(uint32_t PassID, uint32_t SubPassID) : m_PassID(PassID), m_SubPassID(SubPassID) {}
