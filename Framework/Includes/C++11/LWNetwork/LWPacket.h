#ifndef LWPACKET_H
#define LWPACKET_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWByteBuffer.h"
#include "LWNetwork/LWTypes.h"

/*!< \brief the packet object is a simple object that is the root of all packets that can be sent and received for certain protocols, it provides a simple overloadable interface for serializing and deserializing data, which can be easily registed to a packet manager. if a chain of packets are placed together, only the lead packets client data is used. */
class LWPacket{
public:

	enum{
		PacketAck = 0x0, /*!< \brief packet type is an Ack packet response. */

		MajorBits = 0xFFFF0000, /*!< \brief bits for the major component of the type flag. */
		MajorOffset = 0x10, /*!< \brief offset of bits to get the major component of the type flag. */

		LWPacketID = 0x10000,

		Ack = 0x80000000, /*!< \brief flag to indicate the packet expects an Ack response. */
	};

	/*!< \brief deserializes a basic packet object, is also useful for chaining deserialization. 
		 \param Buffer the byte buffer object to read data from to fill out the packet.
		 \param DeselrializeType the type of object being deserialized, this allows the same deserialization function to be used with multiple types. 
		 \param Packet the packet which may or may not have been already allocated, since serialization and deserialization are meant to be used in a inheritance pattern, the current packet to fill out may have already been allocated.
		 \param Allocator the allocator used to create the packet.
		 \param Manager the packet manager that is deallocating the packet.
		 \return the allocated packet object, null upon failure.
	*/

	static LWPacket *Deserialize(LWByteBuffer *Buffer, uint32_t DeserializeType, LWPacket *Packet, LWAllocator &Allocator, LWPacketManager *Manager);

	/*!< \brief method for serializing the packet for a network stream 
		 \return the number of bytes to be written to the buffer.
	*/
	virtual uint32_t Serialize(LWByteBuffer *Buffer, LWPacketManager *Manager);

	/*!< \brief sets the client data the associated protocol can use to identify the client with the packet. */
	LWPacket &SetClient(void *Client);

	/*!< \brief sets the source data the associated protocol can use to identify where the client sent his data to. */
	LWPacket &SetSource(void *Source);

	/*!< \brief sets the packet id. */
	LWPacket &SetPacketID(uint32_t PacketID);

	/*!< \brief sets the packet flag. */
	LWPacket &SetFlag(uint32_t PacketFlag);

	/*!< \brief sets the next packet in the packet chain. */
	LWPacket &SetNext(LWPacket *Pack);

	/*!< \brief sets the packet ack id. */
	LWPacket &SetPacketAckID(uint32_t AckID);

	/*!< \brief sets the time to be sent at. */
	LWPacket &SetSendTime(uint64_t SendTime);

	/*!< \brief returns the client data the associated protocol will use to identify the client. */
	void *GetClient(void) const;

	/*!< \brief returns the source data the associated protocol can use to determine where the client sent data to(i.e: the receiving socket). */
	void *GetSource(void) const;

	/*!< \brief returns the packets sub type, used for identifying and routing the packet. */
	uint32_t GetType(void) const;

	/*!< \bried returns the packets major type, used for deserializing the packet. */
	uint32_t GetMajorType(void) const;

	/*!< \brief returns the raw packet type which has both the sub type and major type. */
	uint32_t GetRawType(void) const;

	/*!< \brief returns the packets id. */
	uint32_t GetPacketID(void) const;

	/*!< \brief returns the packets Ack id, when supporting reliable ack. */   
	uint32_t GetPacketAckID(void) const;

	/*!< \brief returns the minimum time for when to send the packet. */
	uint64_t GetSendTime(void) const;

	/*!< \brief returns the flag associated with the packet. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns the next packet in the packet chain. */
	LWPacket *GetNext(void) const;

	/*!< \brief constructs a simple packet object. */
	LWPacket(uint32_t PacketID, void *Client, uint32_t Type, uint32_t Flag);
	
	LWPacket() = default;

	/*!< \brief destructs a packet object. */
	virtual ~LWPacket();
protected:
	uint64_t m_SendTime = 0;
	void *m_Client = nullptr;
	void *m_Source = nullptr;
	LWPacket *m_Next = nullptr;
	uint32_t m_Type = 0;
	uint32_t m_PacketID = 0;
	uint32_t m_PacketAckID = 0;
	uint32_t m_Flag = 0;

};

template<uint32_t MaxBufferSize>
class LWPacketGeneric : public LWPacket {
public:
	static LWPacketGeneric<MaxBufferSize> *Deserialize(LWByteBuffer *Buffer, uint32_t DeserializeType, LWPacket *Packet, LWAllocator &Allocator, LWPacketManager *Manager) {
		LWPacketGeneric<MaxBufferSize> *Pack = Packet ? (LWPacketGeneric<MaxBufferSize>*)Packet : Allocator.Create<LWPacketGeneric<MaxBufferSize>>(0, nullptr, DeserializeType, 0);
		LWPacket::Deserialize(Buffer, DeserializeType, Pack, Allocator, Manager);
		uint32_t BufferSize = Buffer->Read<uint32_t>();
		if (BufferSize > MaxBufferSize) return Pack;
		
		Buffer->Read<char>(Pack->GetBuffer(), BufferSize);
		Pack->SetBufferSize(BufferSize);
		return Pack;
	};

	
	virtual uint32_t Serialize(LWByteBuffer *Buffer, LWPacketManager *Manager) {
		uint32_t o = 0;
		o += LWPacket::Serialize(Buffer, Manager);
		o += Buffer->Write<uint32_t>(m_BufferSize);
		uint32_t pPos = Buffer->GetPosition();
		o += Buffer->Write<char>(m_BufferSize, m_Buffer);
		return o;
	};

	LWPacketGeneric &SetBufferPosition(uint32_t Position) {
		m_BufferPosition = Position;
		return *this;
	}

	LWPacketGeneric &SetBufferSize(uint32_t Size) {
		m_BufferSize = std::min<uint32_t>(MaxBufferSize, Size);
		return *this;
	}

	template<class Type>
	bool Write(Type In) {
		if (m_BufferPosition + sizeof(Type) > MaxBufferSize) return false;
		LWByteBuffer B((int8_t*)m_Buffer, MaxBufferSize, LWByteBuffer::Network);
		B.SetPosition(m_BufferPosition);
		m_BufferPosition+=B.Write(In);
		m_BufferSize = std::max<uint32_t>(m_BufferSize, m_BufferPosition);
		return true;
	};

	template<class Type>
	bool Write(Type *In, uint32_t Len) {
		if (m_BufferPosition + sizeof(Type)*Len > MaxBufferSize) return false;
		LWByteBuffer B((int8_t*)m_Buffer, MaxBufferSize, LWByteBuffer::Network );
		B.SetPosition(m_BufferPosition);
		m_BufferPosition += B.Write(Len, In);
		m_BufferSize = std::max<uint32_t>(m_BufferSize, m_BufferPosition);
		return true;
	}

	template<class Type>
	Type Read() {
		LWByteBuffer B((int8_t*)m_Buffer, MaxBufferSize, LWByteBuffer::Network);
		Type O = B.Read<Type>(m_BufferPosition);
		m_BufferPosition += sizeof(Type);
		return O;
	};

	template<class Type>
	bool Read(Type *Out, uint32_t Len) {
		if (m_BufferPosition + sizeof(Type)*Len >= MaxBufferSize) return false;
		LWByteBuffer B((int8_t*)m_Buffer, MaxBufferSize, LWByteBuffer::Network);
		B.Read(Out, Len, m_BufferPosition);
		m_BufferPosition += sizeof(Type)*Len;
		return true;
	}

	char *GetBuffer(void) {
		return m_Buffer;
	}

	const char *GetBuffer(void) const {
		return m_Buffer;
	}

	uint32_t GetBufferPosition(void) const {
		return m_BufferPosition;
	}

	uint32_t GetBufferSize(void) const {
		return m_BufferSize;
	}

	LWPacketGeneric(uint32_t PacketID, void *Client, uint32_t Type, uint32_t Flag) : LWPacket(PacketID, Client, Type, Flag), m_BufferSize(0), m_BufferPosition(0) {}

	LWPacketGeneric() = default;

protected:
	char m_Buffer[MaxBufferSize];
	uint32_t m_BufferPosition = 0;
	uint32_t m_BufferSize = 0;
};
#endif