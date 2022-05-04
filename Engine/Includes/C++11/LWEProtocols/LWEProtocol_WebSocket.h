#ifndef PROTOCOL_WEBSOCKET_H
#define PROTOCOL_WEBSOCKET_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWConcurrent/LWFIFO.h>
#include <LWCore/LWUnicode.h>
#include <functional>

#define LWEWEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define LWEWEBSOCKET_SUPPVER 13

struct LWEWebPacket;

struct LWEWebSocket;

class LWEProtocol_WebSocket;

typedef std::function<void(LWRef<LWEWebSocket>&, LWEProtocol_WebSocket &)> LWEWebSocketClosedCallback;
typedef std::function<void(LWRef<LWEWebSocket>&, LWEProtocol_WebSocket &)> LWEWebSocketConnectedCallback;
typedef std::function<void(LWRef<LWEWebSocket>&, const LWEWebPacket&, LWEProtocol_WebSocket&)> LWEWebSocketReceivedCallback;

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
	LWRef<LWEWebSocket> m_WebSocket;
	uint32_t m_DataLen = 0;
	uint32_t m_DataPos = 0;
	uint32_t m_FramePos = 0;
	uint32_t m_ControlFlag = 0;
	uint32_t m_Mask = 0;

	uint32_t Deserialize(const void *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	uint32_t Serialize(void *Buffer, uint32_t BufferLen, bool isClient);

	uint32_t GetOp(void) const;

	LWUTF8Iterator AsText(void) const; //Returns iterator to data as if this packet is text(op == CONTROL_TEXT).

	void WorkFinished(void);

	bool isBinaryPacket(void) const;

	bool isConnectingPacket(void) const;

	bool isFinished(void) const;

	LWEWebPacket &operator = (LWEWebPacket &&Other);

	LWEWebPacket &operator = (const LWEWebPacket &Other);
	
	LWEWebPacket() = default;

	LWEWebPacket(LWEWebPacket &&Other);

	LWEWebPacket(const void *Data, uint32_t DataLen, LWAllocator &Allocator, uint32_t ControlFlag, LWRef<LWEWebSocket> &WebSocket);

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
	char8_t m_Host[HeaderMaxLength]={};
	char8_t m_Path[HeaderMaxLength]={};
	char8_t m_Origin[HeaderMaxLength]={};
	char8_t m_SecKey[HeaderMaxLength]={};
	char8_t m_SecProtocols[HeaderMaxLength]={};
	LWEWebSocketClosedCallback m_OnClosedCallback = nullptr;
	LWEWebSocketConnectedCallback m_OnConnectedCallback = nullptr;
	LWEWebSocketReceivedCallback m_OnReceivedCallback = nullptr;
	LWEWebPacket m_ActivePacket;
	LWRef<LWSocket> m_Socket;
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

	template<class Method, class Obj>
	LWEWebSocket &SetOnClosed(Method M, Obj *O) {
		m_OnClosedCallback = std::bind(M, O, std::placeholders::_1, std::placeholders::_2);
		return *this;
	}

	template<class Method, class Obj>
	LWEWebSocket &SetOnConnected(Method M, Obj *O) {
		m_OnConnectedCallback = std::bind(M, O, std::placeholders::_1, std::placeholders::_2);
		return *this;
	}

	template<class Method, class Obj>
	LWEWebSocket &SetOnReceived(Method M, Obj *O) {
		m_OnReceivedCallback = std::bind(M, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		return *this;
	}

	LWEWebSocket(const LWUTF8Iterator &URI, const LWUTF8Iterator &Origin = LWUTF8Iterator(), LWEWebSocketReceivedCallback RecvCallback = nullptr, LWEWebSocketConnectedCallback ConnectedCallback = nullptr, LWEWebSocketClosedCallback ClosedCallback = nullptr);
};

class LWEProtocol_WebSocket : virtual public LWProtocol {
public:
	static const uint32_t WebSocketVersion = 13;
	enum {
		PacketBufferSize = 1024
	};
	virtual LWProtocol &Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual LWProtocol &SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual uint32_t Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len);

	bool ProcessWSReadPacket(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t BufferLen);

	//Processes outbound packets, if lTimeout is not 0 then well return after amount of time has elapsed, otherwise returns only once the queue has been emptied.
	LWEProtocol_WebSocket &ProcessOutboundPackets(uint64_t lTimeout = 0);

	LWRef<LWEWebSocket> OpenSocket(const LWUTF8Iterator &URI, LWProtocolManager &ProtocolManager, const LWUTF8Iterator &Origin = LWUTF8Iterator(), LWEWebSocketReceivedCallback RecvCallback = nullptr, LWEWebSocketConnectedCallback ConnectedCallback = nullptr, LWEWebSocketClosedCallback ClosedCallback = nullptr);

	template<class RecvMethod, class ConnMethod, class ClosedMethod, class Obj>
	LWRef<LWEWebSocket> OpenSocket(const LWUTF8Iterator &URI, LWProtocolManager &ProtocolManager, const LWUTF8Iterator &Origin, RecvMethod RecvCallback, ConnMethod ConnCallback, ClosedMethod ClosedCallback, Obj *O) {
		return OpenSocket(URI, ProtocolManager, Origin, std::bind(RecvCallback, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), std::bind(ConnCallback, O, std::placeholders::_1, std::placeholders::_2), std::bind(ClosedCallback, O, std::placeholders::_1, std::placeholders::_2));
	}

	bool PushOutPacket(const void *Buffer, uint32_t BufferLen, LWRef<LWEWebSocket> &Socket, uint32_t ControlFlag = LWEWebPacket::CONTROL_BINARY);

	//Push's a text packet to socket.
	bool PushOutPacket(const LWUTF8Iterator &Text, LWRef<LWEWebSocket> &Socket);

	LWEProtocol_WebSocket &SetServer(const LWUTF8Iterator &Server);

	LWEProtocol_WebSocket &SetUserAgent(const LWUTF8Iterator &Agent);

	LWEProtocol_WebSocket &SetSubProtocol(const LWUTF8Iterator &SubProtocol);

	LWEProtocol_WebSocket(uint32_t ProtocolID, LWAllocator &Allocator);

protected:
	char m_Server[LWEWebSocket::HeaderMaxLength]={};
	char m_UserAgent[LWEWebSocket::HeaderMaxLength]={};
	char m_SubProtocol[LWEWebSocket::HeaderMaxLength]={};
	LWConcurrentFIFO<LWRef<LWEWebPacket>, PacketBufferSize> m_OutPackets;

	std::unordered_map<uint32_t, LWRef<LWEWebSocket>> m_WebSocketMap;
	std::shared_mutex m_WebSocketMapMutex;
	LWAllocator &m_Allocator;
	uint32_t m_KeySeed = 0;
};

#endif