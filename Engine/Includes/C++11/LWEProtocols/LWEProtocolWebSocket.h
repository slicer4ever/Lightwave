#ifndef PROTOCOL_WEBSOCKET_H
#define PROTOCOL_WEBSOCKET_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWConcurrent/LWFIFO.h>
#include <LWCore/LWUnicode.h>
#include <functional>

#define LWEWEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define LWEWEBSOCKET_SUPPVER 13

struct LWEWebSocket;

typedef std::function<bool(LWSocket &, LWEWebSocket*, LWProtocolManager*)> LWEWebSocketClosedCallback;
typedef std::function<void(LWSocket &, LWSocket &, LWEWebSocket*, LWProtocolManager*)> LWEWebSocketChangedCallback;


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
	char *m_Data = nullptr;
	uint32_t m_DataLen = 0;
	uint32_t m_DataPos = 0;
	uint32_t m_FramePos = 0;
	uint32_t m_ControlFlag = 0;
	uint32_t m_Mask = 0;
	LWEWebSocket *m_WebSocket = nullptr;

	uint32_t Deserialize(const char *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	uint32_t Serialize(char *Buffer, uint32_t BufferLen, bool isClient);

	uint32_t GetOp(void);

	void WorkFinished(void);

	bool Finished(void);

	LWEWebPacket &operator = (LWEWebPacket &&Other);

	LWEWebPacket &operator = (const LWEWebPacket &Other);
	
	LWEWebPacket() = default;

	LWEWebPacket(LWEWebPacket &&Other);

	LWEWebPacket(const char *Data, uint32_t DataLen, LWAllocator &Allocator, uint32_t ControlFlag, LWEWebSocket *WebSocket);

	~LWEWebPacket();
};

struct LWEWebSocket {
	static const uint32_t HeaderMaxLength = 128;
	enum {
		CONNECTED_CLIENT = 0,
		CONNECTED_SERVER,
		CONNECTING_CLIENT,
		CONNECTING_SERVER,
	};
	char8_t m_Host[HeaderMaxLength]="";
	char8_t m_Path[HeaderMaxLength]="";
	char8_t m_Origin[HeaderMaxLength]="";
	char8_t m_SecKey[HeaderMaxLength]="";
	char8_t m_SecProtocols[HeaderMaxLength]="";
	LWEWebPacket m_ActivePacket;
	LWSocket *m_Socket = nullptr;
	uint32_t m_Flag = 0;
	uint16_t m_Port = 80;

	LWEWebSocket &SetURI(const LWUTF8Iterator &URI);

	LWEWebSocket &SetHost(const LWUTF8Iterator &Host);

	LWEWebSocket &SetPath(const LWUTF8Iterator &Path);

	LWEWebSocket &SetOrigin(const LWUTF8Iterator &Origin);

	LWEWebSocket &SetSecKey(const LWUTF8Iterator &Key);

	LWEWebSocket &SetSecProtocols(const LWUTF8Iterator &Protocols);

	LWEWebSocket &GenerateKey(uint32_t seed);

	LWUTF8Iterator GetHost(void) const;

	LWUTF8Iterator GetPath(void) const;

	LWUTF8Iterator GetOrigin(void) const;

	LWUTF8Iterator GetSecKey(void) const;

	LWUTF8Iterator GetSecProtocols(void) const;

	uint32_t GetConnectStatus(void);

	bool IsConnected(void);

	LWEWebSocket(const LWUTF8Iterator &URI, const LWUTF8Iterator &Origin = LWUTF8Iterator());
};

class LWEProtocolWebSocket : virtual public LWProtocol {
public:
	enum {
		PacketBufferSize = 64
	};
	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual uint32_t Send(LWSocket &Socket, const char *Buffer, uint32_t Len);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen);

	LWEProtocolWebSocket &ProcessOutPackets(void);

	LWEWebSocket *OpenSocket(const LWUTF8Iterator &URI, uint32_t ProtocolID, const LWUTF8Iterator &Origin = LWUTF8Iterator());

	bool PushOutPacket(const char *Buffer, uint32_t BufferLen, LWEWebSocket *Socket, uint32_t ControlFlag = LWEWebPacket::CONTROL_BINARY);

	bool GetNextPacket(LWEWebPacket &Packet);

	LWEProtocolWebSocket &SetServer(const LWUTF8Iterator &Server);

	LWEProtocolWebSocket &SetUserAgent(const LWUTF8Iterator &Agent);

	LWEProtocolWebSocket &SetSubProtocol(const LWUTF8Iterator &SubProtocol);

	LWEProtocolWebSocket &SetWebSocketClosedCallback(LWEWebSocketClosedCallback WebSocketClosedCallback);

	LWEProtocolWebSocket &SetWebSocketChangedCallback(LWEWebSocketChangedCallback WebSocketChangedCallback);

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
	char m_Server[LWEWebSocket::HeaderMaxLength]="";
	char m_UserAgent[LWEWebSocket::HeaderMaxLength]="";
	char m_SubProtocol[LWEWebSocket::HeaderMaxLength]="";
	LWConcurrentFIFO<LWEWebPacket, PacketBufferSize> m_OutPackets;
	LWConcurrentFIFO<LWEWebPacket, PacketBufferSize> m_InPackets;

	LWEWebSocketClosedCallback m_WebSocketClosedCallback = nullptr;
	LWEWebSocketChangedCallback m_WebSocketChangedCallback = nullptr;
	LWProtocolManager *m_Manager = nullptr;
	LWAllocator &m_Allocator;
	uint32_t m_ProtocolID = 0;
	uint32_t m_KeySeed = 0;
};

#endif