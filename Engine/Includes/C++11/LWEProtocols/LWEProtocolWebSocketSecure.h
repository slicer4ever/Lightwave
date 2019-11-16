#ifndef LWEPROTOCOLWEBSOCKETSECURE_H
#define LWEPROTOCOLWEBSOCKETSECURE_H
#include "LWEProtocols/LWEProtocolTLS.h"
#include "LWEProtocols/LWEProtocolWebSocket.h"
#include <LWCore/LWTypes.h>

class LWEProtocolWebSocketSecure : public LWEProtocolTLS {
public:
	enum {
		PacketBufferSize = 64
	};

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual LWProtocol &ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen);

	LWEProtocolWebSocketSecure &ProcessOutPackets(void);

	LWEWebSocket *OpenSocket(const char *URI, uint32_t ProtocolID, const char *Origin = nullptr);

	bool PushOutPacket(const char *Buffer, uint32_t BufferLen, LWEWebSocket *Socket, uint32_t ControlFlag = LWEWebPacket::CONTROL_BINARY);

	bool GetNextPacket(LWEWebPacket &Packet);

	LWEProtocolWebSocketSecure &SetServer(const char *Server);

	LWEProtocolWebSocketSecure &SetUserAgent(const char *Agent);

	LWEProtocolWebSocketSecure &SetSubProtocol(const char *SubProtocol);

	LWEProtocolWebSocketSecure &SetWebSocketClosedCallback(std::function<bool(LWSocket &, LWEWebSocket*, LWProtocolManager*)> WebSocketClosedCallback);

	LWEProtocolWebSocketSecure &SetWebSocketChangedCallback(std::function<void(LWSocket &, LWSocket&, LWEWebSocket*, LWProtocolManager*)> WebSocketChangedCallback);

	template<class T, class C>
	LWEProtocolWebSocketSecure &SetWebSocketClosedCallbackMethod(C Method, T *Obj) {
		return SetWebSocketClosedCallback(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	template<class T, class C>
	LWEProtocolWebSocketSecure &SetWebSocketChangedCallbackMethod(C Method, T *Obj) {
		return SetWebSocketChangedCallback(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	}

	LWEProtocolWebSocketSecure(uint32_t ProtocolID, uint32_t TLSProtocolID, LWAllocator &Allocator, LWProtocolManager *Manager, const char *CertFile = nullptr, const char *KeyFile = nullptr);

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
	uint32_t m_wProtocolID;
	uint32_t m_KeySeed;
};

#endif