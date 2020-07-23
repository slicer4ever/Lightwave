#include "LWVideo/LWFrameBuffer.h"
#include <cstdarg>

//LWFrameBuffer
LWFrameBuffer &LWFrameBuffer::SetAttachment(uint32_t AttachmentID, LWTexture *Texture, uint32_t Layer, uint32_t Mipmap){
	m_Attachments[AttachmentID] = { Texture, Layer, Mipmap, 0 };
	m_Flag |= LWFrameBuffer::Dirty;
	return *this;
}

LWFrameBuffer &LWFrameBuffer::SetCubeAttachment(uint32_t AttachmentID, LWTexture *Texture, uint32_t Face, uint32_t Layer, uint32_t Mipmap) {
	m_Attachments[AttachmentID] = { Texture, Layer, Mipmap, Face };
	m_Flag |= LWFrameBuffer::Dirty;
	return *this;
}

LWFrameBuffer &LWFrameBuffer::ClearAttachments(void) {
	for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) m_Attachments[i].m_Source = nullptr;
	m_Flag |= LWFrameBuffer::Dirty;
	return *this;
}

bool LWFrameBuffer::isDirty(void) const{
	return (m_Flag&LWFrameBuffer::Dirty);
}

LWFrameBuffer &LWFrameBuffer::ClearDirty(void) {
	m_Flag &= ~LWFrameBuffer::Dirty;
	return *this;
}

LWVector2i LWFrameBuffer::GetSize(void) const{
	return m_Size;
}

LWVector2f LWFrameBuffer::GetSizef(void) const {
	return m_Size.CastTo<float>();
}

LWFrameBufferAttachment &LWFrameBuffer::GetAttachment(uint32_t AttachmentID){
	return m_Attachments[AttachmentID];
}

uint32_t LWFrameBuffer::GetFlag(void) const{
	return m_Flag;
}

LWFrameBuffer::LWFrameBuffer(const LWVector2i &Size) : m_Flag(Dirty), m_Size(Size) {}