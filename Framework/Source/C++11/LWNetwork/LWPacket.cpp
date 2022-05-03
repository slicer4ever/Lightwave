#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWAllocator.h"
#include "LWNetwork/LWPacket.h"

LWPacket *LWPacket::Deserialize(LWByteBuffer *Buffer, uint32_t DeserializeType, LWPacket *Packet, LWAllocator &Allocator, LWPacketManager *){
	LWPacket *Pack = Packet ? Packet : Allocator.Create<LWPacket>(0, nullptr, DeserializeType|LWPacketID, 0);
	Pack->SetPacketID(Buffer->Read<uint32_t>());
	Pack->SetFlag(Buffer->Read<uint32_t>());
	return Pack;
}

uint32_t LWPacket::Serialize(LWByteBuffer *Buffer, LWPacketManager *){
	uint32_t o = 0;
	o += Buffer->Write(m_PacketID);
	o += Buffer->Write(m_Flag);
	return o;
}

LWPacket &LWPacket::SetClient(void *Client){
	m_Client = Client;
	return *this;
}

LWPacket &LWPacket::SetSource(void *Source) {
	m_Source = Source;
	return *this;
}

LWPacket &LWPacket::SetPacketID(uint32_t PacketID){
	m_PacketID = PacketID;
	return *this;
}

LWPacket &LWPacket::SetFlag(uint32_t PacketFlag){
	m_Flag = PacketFlag;
	return *this;
}

LWPacket &LWPacket::SetNext(LWPacket *Pack){
	m_Next = Pack;
	return *this;
}

LWPacket &LWPacket::SetPacketAckID(uint32_t AckID){
	m_PacketAckID = AckID;
	return *this;
}

LWPacket &LWPacket::SetSendTime(uint64_t SendTime){
	m_SendTime = SendTime;
	return *this;
}

void *LWPacket::GetClient(void) const{
	return m_Client;
}

void *LWPacket::GetSource(void) const {
	return m_Source;
}

uint32_t LWPacket::GetType(void) const{
	return m_Type&~LWPacket::MajorBits;
}

uint32_t LWPacket::GetMajorType(void) const {
	return m_Type >> LWPacket::MajorOffset;
}

uint32_t LWPacket::GetRawType(void) const {
	return m_Type;
}

uint32_t LWPacket::GetPacketID(void) const{
	return m_PacketID;
}

uint32_t LWPacket::GetPacketAckID(void) const{
	return m_PacketAckID;
}

uint64_t LWPacket::GetSendTime(void) const{
	return m_SendTime;
}

uint32_t LWPacket::GetFlag(void) const{
	return m_Flag;
}

LWPacket *LWPacket::GetNext(void) const{
	return m_Next;
}

LWPacket::LWPacket(uint32_t PacketID, void *Client, uint32_t Type, uint32_t Flag) : m_Client(Client), m_Type(Type), m_PacketID(PacketID), m_Flag(Flag){}

LWPacket::~LWPacket(){}