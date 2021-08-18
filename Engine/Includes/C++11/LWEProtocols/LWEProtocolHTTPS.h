#ifndef LWEPROTOCOLHTTPS_H
#define LWEPROTOCOLHTTPS_H
#include "LWEProtocols/LWEProtocolTLS.h"
#include "LWEProtocols/LWEProtocolHTTP.h"

class LWEProtocolHttps : public LWEProtocolTLS, public LWEProtocolHttp {
public:

	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen);

	virtual uint32_t Send(LWSocket &Socket, const char *Buffer, uint32_t Len);

	LWEProtocolHttps(uint32_t HttpsProtocolID, uint32_t TLSProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile = LWUTF8Iterator(), const LWUTF8Iterator &KeyFile = LWUTF8Iterator());
protected:


};

#endif