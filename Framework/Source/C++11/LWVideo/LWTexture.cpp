#include "LWVideo/LWTexture.h"

LWTexture &LWTexture::SetTextureState(uint32_t TextureState){
	if (TextureState == m_TextureState) return *this;
	m_TextureState = TextureState | LWTexture::Dirty;
	return *this;
}

LWTexture &LWTexture::GenerateMipmaps(void) {
	m_TextureState = (m_TextureState | LWTexture::MakeMipmaps | LWTexture::Dirty);
	return *this;
}

bool LWTexture::isDirty(void) const {
	return (m_TextureState&Dirty);
}

LWTexture &LWTexture::ClearDirty(void){
	m_TextureState &= ~(Dirty|MakeMipmaps);
	return *this;
}

uint32_t LWTexture::GetTextureState(void) const{
	return m_TextureState;
}

bool LWTexture::isMultiSampled(void) const {
	return m_Type == Texture2DMS || m_Type == Texture2DMSArray;
}

uint32_t LWTexture::GetMipmapCount(void) const {
	if (isMultiSampled()) return 0;
	return m_MipmapCount;
}

uint32_t LWTexture::GetSamples(void) const {
	if (isMultiSampled()) return m_MipmapCount;
	return 0;
}

uint32_t LWTexture::GetType(void) const{
	return m_Type;
}
uint32_t LWTexture::GetPackType(void) const{
	return m_PackType;
}

uint32_t LWTexture::Get1DSize(void) const {
	return m_Size.x;
}

float LWTexture::Get1DSizef(void) const {
	return (float)m_Size.x;
}

LWVector2i LWTexture::Get2DSize(void) const {
	return m_Size.xy();
}

LWVector2f LWTexture::Get2DSizef(void) const {
	return m_Size.xy().CastTo<float>();
}

LWVector3i LWTexture::Get3DSize(void) const {
	return m_Size;
}

LWVector3f LWTexture::Get3DSizef(void) const {
	return m_Size.CastTo<float>();
}

uint32_t LWTexture::Get1DLayers(void) const {
	return m_Size.y;
}

uint32_t LWTexture::Get2DLayers(void) const {
	return m_Size.z;
}

LWTexture::LWTexture(uint32_t TextureState, uint32_t PackType, uint32_t MipmapCount, const LWVector3i &Size, uint32_t TexType) : m_TextureState(TextureState | Dirty), m_PackType(PackType), m_MipmapCount(MipmapCount), m_Type(TexType), m_Size(Size) {}
