#ifndef LWEPROTOCOLHTTPS_H
#define LWEPROTOCOLHTTPS_H
#include "LWEProtocols/LWEProtocolTLS.h"
#include "LWEProtocols/LWEProtocolHTTP.h"

class LWEProtocolHttps : public LWEProtocolTLS {
public:
	enum {
		RequestBufferSize = 64
	};

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t Len);

	LWEProtocolHttps &ProcessRequests(uint32_t ProtocolID, LWProtocolManager &Manager);

	bool PushRequest(LWEHttpRequest &Request);

	bool PushResponse(LWEHttpRequest &InRequest, const char *Response, uint32_t lStatus);

	bool GetNextRequest(LWEHttpRequest &Request);

	bool GetNextFromPool(LWEHttpRequest **Request);

	LWEProtocolHttps &SetAgentString(const char *Agent);

	LWEProtocolHttps &SetServerString(const char *Server);

	LWEProtocolHttps(uint32_t HttpsProtocolID, uint32_t TLSProtocolID, LWProtocolManager *Manager, LWAllocator &Allocator, const char *CertFile, const char *KeyFile);
protected:
	char m_Agent[256];
	char m_Server[256];
	LWProtocolManager *m_Manager;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_Pool;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_OutRequests;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_InRequests;
	uint32_t m_hProtocolID;


};

#endif