#ifndef LWEPROTOCOL_HTTP_H
#define LWEPROTOCOL_HTTP_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWSocket.h>
#include <functional>
#include <LWEJson.h>
#include <LWCore/LWConcurrent/LWFIFO.h>

struct LWEHttpRequest;

typedef std::function<void(LWEHttpRequest&, const LWUTF8Iterator &Response)> LWEHttpResponseCallback;

struct LWEHttpRequest {
	static const uint32_t BodyMaxLength = 1024 * 256; //256kb default.
	static const uint32_t HeadersMaxLength = 128;
	enum {

		GET = 0x0,
		POST = 0x1,
		METHODBITS = 0x1,

		ConnectionClose = 0x0,
		ConnectionKeepAlive = 0x2,
		ConnectionUpgrade = 0x4,
		CONNECTIONBITS = 0x6,
		CONNECTIONOFFSET = 1,

		NoCacheControl = 0x0,
		CACHEBITS = 0x8,
		CACHEOFFSET = 3,

		EncodeNone = 0x0,
		EncodeChunked = 0x10,
		ENCODEBITS = 0x10,
		ENCODEOFFSET = 4,

		ContentEncodeIdentity = 0x0,
		ContentEncodeGZip = 0x20,
		ContentEncodeCompress = 0x30,
		ContentEncodeDeflate = 0x40,
		ContentEncodeBR = 0x80,
		CONTENTENCODEBITS = 0xE0,
		CONTENTENCODEOFFSET = 5,

		UpgradeNone = 0x0,
		UpgradeWebSock = 0x100,
		UPGRADEBITS = 0x100,
		UPGRADEOFFSET = 8,

		HeadersRead = 0x100,
		ResponseReady = 0x200,

		Continue = 100,
		SwitchingProtocols = 101,
		Ok = 200,
		lBadRequest = 400,
		Unauthorized = 401,
		Forbidden = 403,
		NotFound = 404,
		InternalServerError = 500,
		NotImplemented = 501,
		BadGateway = 502
	};

	char8_t m_Body[BodyMaxLength]="";
	char8_t m_Host[HeadersMaxLength]="";
	char8_t m_Path[HeadersMaxLength]="";
	char8_t m_Origin[HeadersMaxLength]="";
	char8_t m_Authorization[HeadersMaxLength]="";
	char8_t m_ContentType[HeadersMaxLength]="";
	char8_t m_TransferEncoding[HeadersMaxLength]="";
	char8_t m_SecWebSockKey[HeadersMaxLength]="";
	char8_t m_SecWebSockProto[HeadersMaxLength]="";
	LWSocket *m_Socket = nullptr;
	void *m_UserData = nullptr;
	std::function<void(LWEHttpRequest &, const LWUTF8Iterator &)> m_Callback = nullptr;
	uint32_t m_ContentLength = 0;
	uint32_t m_ChunkLength = 0;
	uint32_t m_WebSockVersion = 0;
	uint32_t m_Status = 0;
	uint32_t m_Flag = 0;
	uint16_t m_Port = 0;

	static uint32_t MakeJSONQueryString(LWEJson &Json, char8_t *Buffer, uint32_t BufferLen);

	static uint32_t GZipDecompress(const char8_t *In, uint32_t InLen, char8_t *Buffer, uint32_t BufferLen);

	uint32_t Serialize(char8_t *Buffer, uint32_t BufferLen, const LWUTF8Iterator &UserAgent);

	bool Deserialize(const char8_t *Buffer, uint32_t Len);

	LWEHttpRequest &SetURI(const LWUTF8Iterator &URI);

	LWEHttpRequest &SetHost(const LWUTF8Iterator &Host);

	LWEHttpRequest &SetPath(const LWUTF8Iterator &Path);

	LWEHttpRequest &SetOrigin(const LWUTF8Iterator &Origin);

	LWEHttpRequest &SetWebSockKey(const LWUTF8Iterator &Key);

	LWEHttpRequest &SetWebSockProto(const LWUTF8Iterator &Protocols);

	LWEHttpRequest &SetAuthorization(const LWUTF8Iterator &Auth);

	LWEHttpRequest &SetContentType(const LWUTF8Iterator &ContentType);

	LWEHttpRequest &SetBody(const LWUTF8Iterator &Body);

	LWUTF8Iterator GetBody(void) const;

	LWUTF8Iterator GetHost(void) const;

	LWUTF8Iterator GetPath(void) const;

	LWUTF8Iterator GetOrigin(void) const;

	LWUTF8Iterator GetAuthorization(void) const;

	LWUTF8Iterator GetContentType(void) const;

	LWUTF8Iterator GetTransferEncoding(void) const;

	LWUTF8Iterator GetSecWebSockKey(void) const;

	LWUTF8Iterator GetSecWebSockProto(void) const;

	LWEHttpRequest &SetCallback(LWEHttpResponseCallback Callback);

	LWEHttpRequest &SetMethod(uint32_t Method);

	LWEHttpRequest &SetConnectionState(uint32_t ConnState);

	LWEHttpRequest &SetCacheState(uint32_t CacheState);

	uint32_t GetCacheState(void) const;

	uint32_t GetConnectionState(void) const;

	bool CloseConnection(void) const;

	bool KeepAliveConnection(void) const;

	bool UpgradeConnection(void) const;

	uint32_t GetMethod(void) const;

	uint32_t GetEncodeType(void) const;

	uint32_t GetUpgradeType(void) const;

	bool isHeadersRead(void) const;

	bool isResponseReady(void) const;

	template<class T, class Y>
	LWEHttpRequest &SetMethodCallback(T Inst, Y Method) {
		return SetCallback(std::bind(&Method, Inst, std::placeholders::_1, std::placeholders::_2));
	}

	LWEHttpRequest(const LWUTF8Iterator &URI, uint32_t Flag);

	LWEHttpRequest() = default;
};

class LWEProtocolHttp : public virtual LWProtocol {
public:
	enum {
		RequestBufferSize = 64
	};

	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual uint32_t Send(LWSocket &Socket, const char *Buffer, uint32_t Len);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t Len);

	LWEProtocolHttp &ProcessRequests(uint32_t ProtocolID, LWProtocolManager &Manager);

	LWEProtocolHttp &SetProtocolID(uint32_t ProtocolID);

	LWEProtocolHttp &SetProtocolManager(LWProtocolManager *Manager);

	bool PushRequest(LWEHttpRequest &Request);

	bool PushResponse(LWEHttpRequest &InRequest, const LWUTF8Iterator &Response, uint32_t lStatus);

	bool GetNextRequest(LWEHttpRequest &Request);

	bool GetNextFromPool(LWEHttpRequest **Request);

	LWEProtocolHttp &SetAgentString(const LWUTF8Iterator &Agent);

	LWEProtocolHttp &SetServerString(const LWUTF8Iterator &Server);

	LWEProtocolHttp(uint32_t ProtocolID, LWProtocolManager *Manager);

	LWEProtocolHttp() = default;
protected:
	char8_t m_Agent[LWEHttpRequest::HeadersMaxLength]="";
	char8_t m_Server[LWEHttpRequest::HeadersMaxLength]="";
	LWProtocolManager *m_Manager = nullptr;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_Pool;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_OutRequests;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_InRequests;
	uint32_t m_ProtocolID = 0;
};

#endif

