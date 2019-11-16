#ifndef LWPACKETMANAGER_H
#define LWPACKETMANAGER_H
#include "LWCore/LWTypes.h"
#include "LWNetwork/LWTypes.h"
#include <functional>
#include <map>

/*!< \brief this is the stored header for a packet, it contains the necessary information for buffering the packets content.*/
struct LWPacketHeader{
	uint32_t m_RecvSize;
	uint32_t m_TotalSize;
	void *m_Client;
	void *m_Source;
};

/*!< \brief this is the raw packet header, the layout of all packets is preceded with this information, and it is imperative that at minimum this amount of data is sent. */
struct LWPacketRawHeader{
	uint32_t m_Header;
	uint32_t m_TotalSize;
};

/*!< \brief packet manager, which manages in and out packets and manages split packets as well, as well as providing selective resending of packets marked for requiring an Ack, this provides a reliable udp implementation if needed. */
class LWPacketManager{
public:
	enum{
		MaxOutPackets = 1024, /*!< \brief the maximum number of packets which can be queied at any time. */
		Header = 0x4C57504B, /*!< \brief special header to indicate start of a packet: LWPK */

		ResendFrequency = 3 /*!< \brief the resend frequency for sending packets from a client. */
	};
	
	/*!< \brief Serializes a single packet object into buffer. */
	uint32_t SerializePacket(LWPacket *Packet, char *Buffer, uint32_t BufferLen);

	/*!< \brief attempts to deserialize a buffer into a series of packets if possible.
		 \param Buffer the buffer containing the raw packet data.
		 \param BUfferLen the length of the buffer, this method will deserialize as many packets as it can find, or stop if it reaches something it cannot deserialize.
		 \param Client the client associated with the packet.
		 \param Source the source of the received data.
		 \param PacketAllocator the allocator used to create the packets.
		 \return pointer to an allocated packet that the caller now owns, or null if failure.
	*/
	LWPacket *DeserializePacket(const char *Buffer, uint32_t BufferLen, void *Client, void *Source);

	/*!< \brief processes raw packet data and either forms it into a packet, or stores it if the entire packet has not been received yet. 
		 \
	*/
	bool ProcessRawData(void *Client, void *Source, const char *Buffer, uint32_t BufferLen);

	/*!< \brief sets the packets user data. */
	LWPacketManager &SetUserData(void *UserData);

	/*!< \brief processes outgoing packets, and potential ack responses. 
		 \param lCurrentTime the current time, most commonly used by LWTimer::GetCurrent
		 \param ClockResolutuion the resolution the clock uses for 1 second, most commonly captured via LWTimer::GetResolution.
	*/
	LWPacketManager &Update(uint64_t lCurrentTime, uint64_t ClockResolution);

	/*!< \brief purges all out going and temporary packets marked by client most commonly due to client disconnect. */
	LWPacketManager &PurgeClient(void *Client);

	/*!< \brief registers a deserialization function with type identifier. */
	LWPacketManager &RegisterDeserialization(uint32_t TypeID, std::function<LWPacket*(LWByteBuffer*, uint32_t, LWPacket*, LWAllocator &, LWPacketManager*)> Func);

	/*!< \brief adds an outgoing packet to the packet list, and takes ownership of that packet. 
		 \return rather packet was succesfully added to the queue.
		 \note this function is not multi-threaded friendly, do not push packets and call update from seperate threads!
	*/
	bool PushOutPacket(LWPacket *Packet);

	/*!< \brief returns the packets user data. */
	void *GetUserData(void) const;

	/*!< \brief returns the packet allocator used for allocating packets. */
	LWAllocator &GetPacketAllocator(void) const;

	/*!< \brief constructs a packet manager object.
		 \param PacketBufferSize the size in bytes of the packet buffer which stores packets which have not been fully transmitted.
		 \param BufferAllocator the allocator used to allocate the packet buffer.
		 \param PacketAllocator the allocator used when deserializing packets.
		 \param ReceivePacketFunc the function callback used when a packet has been deserialized, must return true if the packet is being owned by the applicaiton, or false if the packet is to be destroyed by the packet manager.
		 \param SendPacketFunc the function callback used when a packet is ready to be sent(only called inside the update method). must return true is the packet was sent, and can be safely destroyed(or stored until an ack is received), or false if the packet could not be sent.
		 \note LWPacket::PacketAck type is automatically registered as a deserialization type.
	*/
	LWPacketManager(uint32_t PacketBufferSize, LWAllocator &BufferAllocator, LWAllocator &PacketAllocator, std::function<bool(LWPacket *Pack, LWPacketManager *Man)> ReceivePacketFunc, std::function<bool(LWPacket *Pack, LWPacketManager *Man)> SendPacketFunc);

	/*!< \brief destructs a packet manager object. */
	~LWPacketManager();
private:
	LWPacket *m_OutPackets[MaxOutPackets];
	char *m_PacketBuffer;
	void *m_UserData = nullptr;
	std::function<bool(LWPacket *Pack, LWPacketManager *Man)> m_ReceivePacketFunction;
	std::function<bool(LWPacket *Pack, LWPacketManager *Man)> m_SendPacketFunction;
	std::map<uint32_t, std::function<LWPacket*(LWByteBuffer*, uint32_t, LWPacket*, LWAllocator&, LWPacketManager*)>> m_DeserializeFunctions;
	LWAllocator *m_PacketAllocator;
	uint32_t m_NextAckID = 0;
	uint32_t m_OutPacketCount = 0;
	uint32_t m_PacketBufferLength;
	uint32_t m_PacketBufferPosition = 0;
};

#endif