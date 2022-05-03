#ifndef LWEPROTOCOLWEBSOCKETSECURE_H
#define LWEPROTOCOLWEBSOCKETSECURE_H
#include "LWEProtocols/LWEProtocol_TLS.h"
#include "LWEProtocols/LWEProtocol_WebSocket.h"
#include <LWCore/LWTypes.h>

class LWEProtocol_WebSocketSecure : public LWEProtocol_TLS, public LWEProtocol_WebSocket {
public:
	virtual LWProtocol &Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual LWProtocol &SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual LWProtocol &ProcessTLSData(LWRef<LWSocket> &Socket, const void *Data, uint32_t DataLen);

	virtual uint32_t Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len);

	LWEProtocol_WebSocketSecure(uint32_t ProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile = LWUTF8Iterator(), const LWUTF8Iterator &KeyFile = LWUTF8Iterator());

protected:
};

#endif