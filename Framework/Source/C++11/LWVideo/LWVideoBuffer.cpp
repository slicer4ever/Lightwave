#include "LWVideo/LWVideoBuffer.h"
#include "LWCore/LWAllocator.h"
#include <cstring>

LWVideoBuffer &LWVideoBuffer::SetEditLength(uint32_t Length, bool DontUpdate){
	m_EditLength = Length;
	if (!DontUpdate) m_Flag |= LWVideoBuffer::Dirty;
	return *this;
}

bool LWVideoBuffer::isDirty(void) const{
	return (m_Flag&Dirty);
}

LWVideoBuffer &LWVideoBuffer::ClearDirty(void) {
	m_Flag &= ~Dirty;
	return *this;
}

uint8_t *LWVideoBuffer::MakeLocalBuffer(void){
	if (m_LocalBuffer || !m_Allocator) return m_LocalBuffer;
	m_LocalBuffer = m_Allocator->AllocateA<uint8_t>(m_TypeSize*m_Length);
	m_Flag |= (m_LocalBuffer ? LWVideoBuffer::LocalCopy : 0);
	return m_LocalBuffer;
}

LWVideoBuffer &LWVideoBuffer::DestroyLocalBuffer(void){
	if(m_LocalBuffer){
		LWAllocator::Destroy(m_LocalBuffer);
		m_Flag ^= LWVideoBuffer::LocalCopy;
	}
	return *this;
}

LWVideoBuffer &LWVideoBuffer::operator = (LWVideoBuffer &&Buffer){
	m_Allocator = Buffer.m_Allocator;
	m_LocalBuffer = Buffer.m_LocalBuffer;
	m_EditLength = Buffer.m_EditLength;
	m_TypeSize = Buffer.m_TypeSize;
	m_Flag = Buffer.m_Flag;

	Buffer.m_LocalBuffer = nullptr;
	return *this;
}

LWAllocator *LWVideoBuffer::GetAllocator(void) const{
	return m_Allocator;
}

uint8_t *LWVideoBuffer::GetLocalBuffer(void) const{
	return m_LocalBuffer;
}

uint32_t LWVideoBuffer::GetFlag(void) const{
	return m_Flag;
}

uint32_t LWVideoBuffer::GetLength(void) const{
	return m_Length;
}

uint32_t LWVideoBuffer::GetTypeSize(void) const {
	return m_TypeSize;
}

uint32_t LWVideoBuffer::GetRawLength(void) const {
	return m_Length * m_TypeSize;
}

uint32_t LWVideoBuffer::GetEditLength(void) const{
	return m_EditLength;
}

uint32_t LWVideoBuffer::GetType(void) const {
	return m_Flag&TypeFlag;
}

LWVideoBuffer::LWVideoBuffer(LWVideoBuffer &&Buffer) : m_Allocator(Buffer.m_Allocator), m_LocalBuffer(Buffer.m_LocalBuffer), m_Flag(Buffer.m_Flag), m_Length(Buffer.m_Length), m_TypeSize(Buffer.m_TypeSize), m_EditLength(Buffer.m_EditLength) {
	Buffer.m_LocalBuffer = nullptr;
}

LWVideoBuffer::LWVideoBuffer(){}

LWVideoBuffer::LWVideoBuffer(const uint8_t *Buffer, LWAllocator *Allocator, uint32_t TypeSize, uint32_t Length, uint32_t Flag) : m_Allocator(Allocator), m_TypeSize(TypeSize), m_Length(Length), m_Flag(Flag){
	if(m_Flag&LocalCopy){
		if(m_Allocator){
			m_LocalBuffer = m_Allocator->AllocateA<uint8_t>(m_TypeSize*m_Length);
			if (Buffer) std::copy(Buffer, Buffer + (m_TypeSize*Length), m_LocalBuffer);
		} else m_Flag ^= LocalCopy;
	}
}

LWVideoBuffer::~LWVideoBuffer(){
	LWAllocator::Destroy(m_LocalBuffer);
}