#include "LWCore/LWAllocator.h"
#include "LWCore/LWByteBuffer.h"
#include "LWNetwork/LWPacketManager.h"
#include "LWNetwork/LWPacket.h"
#include <cstring>
#include <iostream>

uint32_t LWPacketManager::SerializePacket(LWPacket *Packet, char *Buffer, uint32_t BufferLen){
	LWByteBuffer ByteBuf((int8_t*)Buffer, BufferLen, LWByteBuffer::Network);
	uint32_t o = sizeof(LWPacketRawHeader);
	ByteBuf.Seek(sizeof(LWPacketRawHeader));
	for (LWPacket *C = Packet, *N = C ? C->GetNext() : C; C; C = N, N = N ? N->GetNext() : N){
		o += ByteBuf.Write(C->GetRawType());
		o += C->Serialize(&ByteBuf, this);
	}
	ByteBuf.SetPosition(0);
	ByteBuf.Write<uint32_t>(LWPacketManager::Header);
	ByteBuf.Write(o);
	return o;
}

LWPacket *LWPacketManager::DeserializePacket(const char *Buffer, uint32_t BufferLen, void *Client, void *Source){
	LWByteBuffer ByteBuf((const int8_t*)Buffer, BufferLen, LWByteBuffer::Network );
	ByteBuf.Seek(sizeof(LWPacketRawHeader)); //offset past the header.
	LWPacket *F = nullptr;
	LWPacket *C = nullptr;
	while(!ByteBuf.IsEndOfReadData()){
		uint32_t TypeID = ByteBuf.Read<uint32_t>();
		uint32_t MajorID = TypeID&LWPacket::MajorBits;
		TypeID &= ~LWPacket::MajorBits;
		auto Iter = m_DeserializeFunctions.find(MajorID);
		if (Iter == m_DeserializeFunctions.end()) return F;
		LWPacket *R = Iter->second(&ByteBuf, TypeID, nullptr, *m_PacketAllocator, this);
		if (!R) return F;
		R->SetClient(Client);
		R->SetSource(Source);
		if (!C) F = C = R;
		else{
			C->SetNext(R);
			C = R;
		}
	}
	return F;
}

bool LWPacketManager::ProcessRawData(void *Client, void *Source, const char *Buffer, uint32_t BufferLen){	
	LWByteBuffer ByteBuf((const int8_t*)Buffer, BufferLen, LWByteBuffer::Network);
	LWPacketRawHeader Header;
	LWPacketHeader *BufferHeader = (LWPacketHeader *)m_PacketBuffer;
	Header.m_Header = ByteBuf.Read<uint32_t>();
	Header.m_TotalSize = ByteBuf.Read<uint32_t>();
	uint32_t ReadData = 0;
	LWPacket *Pack = nullptr;
	if (Header.m_Header == LWPacketManager::Header){
		if (Header.m_TotalSize <= BufferLen) Pack = DeserializePacket(Buffer, BufferLen, Client, Source);
		else if (m_PacketBufferPosition + Header.m_TotalSize < m_PacketBufferLength) {
			BufferHeader = (LWPacketHeader *)(m_PacketBuffer + m_PacketBufferPosition);
			BufferHeader->m_Client = Client;
			BufferHeader->m_Source = Source;
			BufferHeader->m_RecvSize = BufferLen;
			BufferHeader->m_TotalSize = Header.m_TotalSize;
			memcpy(m_PacketBuffer + m_PacketBufferPosition + sizeof(LWPacketHeader), Buffer, sizeof(char)*BufferLen);
			m_PacketBufferPosition += Header.m_TotalSize + sizeof(LWPacketHeader);
		} else return false;
		ReadData = std::min<uint32_t>(BufferLen, Header.m_TotalSize);
	}else{
		while ((char*)BufferHeader < (m_PacketBuffer + m_PacketBufferPosition)){
			LWPacketHeader *NextHeader = (LWPacketHeader*)(((char*)BufferHeader) + sizeof(LWPacketHeader) + BufferHeader->m_TotalSize);
			if(BufferHeader->m_Client==Client && BufferHeader->m_Source==Source){ //we assume this is the same packet!
				ReadData = std::min<uint32_t>(BufferLen, BufferHeader->m_TotalSize - BufferHeader->m_RecvSize);
				memcpy(((char*)BufferHeader) + sizeof(LWPacketHeader) + BufferHeader->m_RecvSize, Buffer, sizeof(char)*ReadData);
				BufferHeader->m_RecvSize += ReadData;
				if(BufferHeader->m_RecvSize==BufferHeader->m_TotalSize){
					Pack = DeserializePacket(((char*)BufferHeader) + sizeof(LWPacketHeader), BufferHeader->m_TotalSize, Client, Source);
					uintptr_t Len = (uintptr_t)m_PacketBuffer + m_PacketBufferPosition - (uintptr_t)NextHeader;
					m_PacketBufferPosition -= BufferHeader->m_TotalSize + sizeof(LWPacketHeader);
					memcpy(BufferHeader, NextHeader, sizeof(char)*Len);
					break;
				}
				return ReadData==BufferLen?true:ProcessRawData(Client, Source, Buffer+ReadData, BufferLen-ReadData);
			}
			BufferHeader = NextHeader;
		}
		if (!Pack) return false;
	}
	if (!Pack) return true;
	if (Pack->GetType() == LWPacket::PacketAck){
		for (uint32_t i = 0; i < m_OutPacketCount;i++){
			if(m_OutPackets[i]->GetPacketAckID()==Pack->GetPacketID()){
				for (LWPacket *C = m_OutPackets[i], *N = C->GetNext(); C; C = N, N = N ? N->GetNext() : N) LWAllocator::Destroy(C);
				m_OutPackets[i] = m_OutPackets[m_OutPacketCount - 1];
				m_OutPacketCount--;
				break;
			}
		}
	}else{
		bool Result = false;
		if (m_ReceivePacketFunction) Result = m_ReceivePacketFunction(Pack, this);
		if (Pack->GetFlag()&LWPacket::Ack){
			LWPacket *AckPack = m_PacketAllocator->Create<LWPacket>(Pack->GetPacketAckID(), Client, LWPacket::PacketAck, 0);
			if(!PushOutPacket(AckPack))	LWAllocator::Destroy(AckPack); //if we failed to add our ack pack, then we'll have to try again later when the client resends the packet.
		}
		if (!Result){
			for (LWPacket *C = Pack, *N = C->GetNext(); C; C = N, N = N ? N->GetNext() : N) LWAllocator::Destroy(C);
		}
	}
	return (ReadData==BufferLen)?true:ProcessRawData(Client, Source, Buffer+ReadData, BufferLen-ReadData);
}

LWPacketManager &LWPacketManager::SetUserData(void *UserData){
	m_UserData = UserData;
	return *this;
}

LWPacketManager &LWPacketManager::Update(uint64_t lCurrentTime, uint64_t ClockResolution){
	for (uint32_t i = 0; i < m_OutPacketCount;i++){
		if(lCurrentTime >=m_OutPackets[i]->GetSendTime()){
			bool Remove = (m_OutPackets[i]->GetFlag()&LWPacket::Ack) == 0;
			m_OutPackets[i]->SetSendTime(lCurrentTime + ClockResolution*LWPacketManager::ResendFrequency);
			if (m_SendPacketFunction){
				bool Result = m_SendPacketFunction(m_OutPackets[i], this);
				Remove = Remove ? Result : Remove;
			}
			if(Remove){
				for (LWPacket *C = m_OutPackets[i], *N = C->GetNext(); C; C = N, N = N ? N->GetNext() : N) LWAllocator::Destroy(C);
				m_OutPackets[i] = m_OutPackets[m_OutPacketCount - 1];
				m_OutPacketCount--;
				i--;
			}
		}
	}
	return *this;
}

LWPacketManager &LWPacketManager::PurgeClient(void *Client){
	for (uint32_t i = 0; i < m_OutPacketCount;i++){
		if(m_OutPackets[i]->GetClient()==Client){
			for (LWPacket *C = m_OutPackets[i], *N = C->GetNext(); C; C = N, N = N ? N->GetNext() : N) LWAllocator::Destroy(C);
			m_OutPackets[i] = m_OutPackets[m_OutPacketCount - 1];
			m_OutPacketCount--;
			i--;
		}
	}
	LWPacketHeader *BufferHeader = (LWPacketHeader*)m_PacketBuffer;
	while((char*)BufferHeader!=m_PacketBuffer+m_PacketBufferPosition){
		LWPacketHeader *NextHeader = (LWPacketHeader *)(((char*)BufferHeader) + sizeof(LWPacketHeader) + BufferHeader->m_TotalSize);
		if (BufferHeader->m_Client == Client){
			uintptr_t Len = (uintptr_t)m_PacketBuffer + m_PacketBufferPosition - (uintptr_t)NextHeader;
			m_PacketBufferPosition -= BufferHeader->m_TotalSize + sizeof(LWPacketHeader);
			memcpy(BufferHeader, NextHeader, sizeof(char)*Len);
		} else BufferHeader = NextHeader;
	}
	return *this;
}

LWPacketManager &LWPacketManager::RegisterDeserialization(uint32_t TypeID, std::function<LWPacket*(LWByteBuffer*, uint32_t, LWPacket*, LWAllocator &, LWPacketManager*)> Func){
	m_DeserializeFunctions[TypeID] = Func;
	return *this;
}

bool LWPacketManager::PushOutPacket(LWPacket *Packet){
	if (m_OutPacketCount >= LWPacketManager::MaxOutPackets) return false;
	m_OutPackets[m_OutPacketCount++] = Packet;
	if (Packet->GetFlag()&LWPacket::Ack) Packet->SetPacketAckID(m_NextAckID++);
	Packet->SetSendTime(0);
	return true;
}

void *LWPacketManager::GetUserData(void) const{
	return m_UserData;
}

LWAllocator &LWPacketManager::GetPacketAllocator(void) const{
	return *m_PacketAllocator;
}

LWPacketManager::LWPacketManager(uint32_t PacketBufferSize, LWAllocator &BufferAllocator, LWAllocator &PacketAllocator, std::function<bool(LWPacket *Pack, LWPacketManager *Man)> ReceivePacketFunc, std::function<bool(LWPacket *Pack, LWPacketManager *Man)> SendPacketFunc) : m_PacketBuffer(BufferAllocator.Allocate<char>(PacketBufferSize)), m_ReceivePacketFunction(ReceivePacketFunc), m_SendPacketFunction(SendPacketFunc), m_PacketBufferLength(PacketBufferSize), m_PacketAllocator(&PacketAllocator), m_OutPacketCount(0){
	RegisterDeserialization(LWPacket::PacketAck, LWPacket::Deserialize);
}

LWPacketManager::~LWPacketManager(){
	LWAllocator::Destroy(m_PacketBuffer);
	for (uint32_t i = 0;i< m_OutPacketCount;i++){
		for (LWPacket *C = m_OutPackets[i], *N = C->GetNext(); C; C=N, N=N?N->GetNext():N) LWAllocator::Destroy(N);
	}
}