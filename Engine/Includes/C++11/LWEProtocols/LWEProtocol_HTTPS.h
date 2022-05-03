#ifndef LWEPROTOCOLHTTPS_H
#define LWEPROTOCOLHTTPS_H
#include "LWEProtocols/LWEProtocol_TLS.h"
#include "LWEProtocols/LWEProtocol_HTTP.h"

class LWEProtocol_HTTPS : public LWEProtocol_TLS, public LWEProtocol_HTTP {
public:

	virtual LWProtocol &Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual LWProtocol &SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual LWProtocol &ProcessTLSData(LWRef<LWSocket> &Socket, const void *Data, uint32_t DataLen);

	virtual uint32_t Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len);

	LWEProtocol_HTTPS(uint32_t ProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile = LWUTF8Iterator(), const LWUTF8Iterator &KeyFile = LWUTF8Iterator());

protected:


};

#endif