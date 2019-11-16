#ifndef PROTOCOL_WEBSOCKET_H
#define PROTOCOL_WEBSOCKET_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWConcurrent/LWFIFO.h>
#include <functional>

#define LWEWEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define LWEWEBSOCKET_SUPPVER 13

struct LWEWebSocket;

struct LWEWebPacket {
	enum {
		CONTROL_TEXT=1,
		CONTROL_BINARY,
		CONTROL_CLOSED,
		CONTROL_PING,
		CONTROL_PONG,
		CONTROL_CONNECT=0x4000,
		CONTROL_FINISHED=0x8000,
	};
	char *m_Data;
	uint32_t m_DataLen;
	uint32_t m_DataPos;
	uint32_t m_FramePos;
	uint32_t m_ControlFlag;
	uint32_t m_Mask;
	LWEWebSocket *m_WebSocket;

	uint32_t Deserialize(const char *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	uint32_t Serialize(char *Buffer, uint32_t BufferLen, bool isClient);

	uint32_t GetOp(void);

	void WorkFinished(void);

	bool Finished(void);

	LWEWebPacket &operator = (LWEWebPacket &&Other);

	LWEWebPacket &operator = (const LWEWebPacket &Other);
	
	LWEWebPacket();

	LWEWebPacket(LWEWebPacket &&Other);

	LWEWebPacket(const char *Data, uint32_t DataLen, LWAllocator &Allocator, uint32_t ControlFlag, LWEWebSocket *WebSocket);

	~LWEWebPacket();
};

struct LWEWebSocket {
	enum {
		CONNECTED_CLIENT = 0,
		CONNECTED_SERVER,
		CONNECTING_CLIENT,
		CONNECTING_SERVER,
	};
	char m_Host[128];
	char m_Path[128];
	char m_Origin[128];
	char m_SecKey[128];
	char m_SecProtocols[128];
	LWEWebPacket m_ActivePacket;
	LWSocket *m_Socket;
	uint32_t m_Flag;
	uint16_t m_Port;

	LWEWebSocket &SetURI(const char *URI);

	LWEWebSocket &SetURIf(const char *Fmt, ...);

	LWEWebSocket &SetHost(const char *Host);

	LWEWebSocket &SetHostf(const char *Fmt, ...);

	LWEWebSocket &SetPath(const char *Path);

	LWEWebSocket &SetPathf(const char *Fmt, ...);

	LWEWebSocket &SetOrigin(const char *Origin);

	LWEWebSocket &SetOriginf(const char *Fmt, ...);

	LWEWebSocket &SetSecKey(const char *Key);

	LWEWebSocket &SetSecKeyf(const char *Fmt, ...);

	LWEWebSocket &SetSecProtocols(const char *Protocols);

	LWEWebSocket &SetSecProtocolsf(const char *Fmt, ...);

	LWEWebSocket &GenerateKey(uint32_t seed);

	uint32_t GetConnectStatus(void);

	bool IsConnected(void);

	LWEWebSocket(const char *URI, const char *Origin = nullptr);

	~LWEWebSocket();
};

class LWEProtocolWebSocket : public LWProtocol {
public:
	enum {
		PacketBufferSize = 64
	};
	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	bool Send(LWSocket &Socket, const char *Buffer, uint32_t Len);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen);

	LWEProtocolWebSocket &ProcessOutPackets(void);

	LWEWebSocket *OpenSocket(const char *URI, uint32_t ProtocolID, const char *Origin = nullptr);

	bool PushOutPacket(const char *Buffer, uint32_t BufferLen, LWEWebSocket *Socket, uint32_t ControlFlag = LWEWebPacket::CONTROL_BINARY);

	bool GetNextPacket(LWEWebPacket &Packet);

	LWEProtocolWebSocket &SetServer(const char *Server);

	LWEProtocolWebSocket &SetUserAgent(const char *Agent);

	LWEProtocolWebSocket &SetSubProtocol(const char *SubProtocol);

	LWEProtocolWebSocket &SetWebSocketClosedCallback(std::function<bool(LWSocket &, LWEWebSocket*, LWProtocolManager*)> WebSocketClosedCallback);

	LWEProtocolWebSocket &SetWebSocketChangedCallback(std::function<void(LWSocket &, LWSocket&, LWEWebSocket*, LWProtocolManager*)> WebSocketChangedCallback);

	template<class T, class C>
	LWEProtocolWebSocket &SetWebSocketClosedCallbackMethod(C Method, T *Obj) {
		return SetWebSocketClosedCallback(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	template<class T, class C>
	LWEProtocolWebSocket &SetWebSocketChangedCallbackMethod(C Method, T *Obj) {
		return SetWebSocketChangedCallback(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	}

	LWEProtocolWebSocket(uint32_t ProtocolID, LWAllocator &Allocator, LWProtocolManager *Manager);

protected:
	char m_Server[256];
	char m_UserAgent[256];
	char m_SubProtocol[256];
	LWConcurrentFIFO<LWEWebPacket, PacketBufferSize> m_OutPackets;
	LWConcurrentFIFO<LWEWebPacket, PacketBufferSize> m_InPackets;

	std::function<bool(LWSocket &, LWEWebSocket*, LWProtocolManager*)> m_WebSocketClosedCallback;
	std::function<void(LWSocket &, LWSocket &, LWEWebSocket*, LWProtocolManager*)> m_WebSocketChangedCallback;
	LWProtocolManager *m_Manager;
	LWAllocator &m_Allocator;
	uint32_t m_ProtocolID;
	uint32_t m_KeySeed;
};

#endif