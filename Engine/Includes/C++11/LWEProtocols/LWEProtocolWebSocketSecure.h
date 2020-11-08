#ifndef LWEPROTOCOLWEBSOCKETSECURE_H
#define LWEPROTOCOLWEBSOCKETSECURE_H
#include "LWEProtocols/LWEProtocolTLS.h"
#include "LWEProtocols/LWEProtocolWebSocket.h"
#include <LWCore/LWTypes.h>

class LWEProtocolWebSocketSecure : public LWEProtocolTLS, LWEProtocolWebSocket {
public:
	enum {
		PacketBufferSize = 64
	};

	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual LWProtocol &ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen);

	virtual uint32_t Send(LWSocket &Socket, const char *Buffer, uint32_t Len);

	LWEProtocolWebSocketSecure(uint32_t ProtocolID, uint32_t TLSProtocolID, LWAllocator &Allocator, LWProtocolManager *Manager, const LWUTF8Iterator &CertFile = LWUTF8Iterator(), const LWUTF8Iterator &KeyFile = LWUTF8Iterator());

protected:
};

#endif