#ifndef LWEPROTOCOL_HTTP_H
#define LWEPROTOCOL_HTTP_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWSocket.h>
#include <functional>
#include <LWEJson.h>
#include <LWCore/LWConcurrent/LWFIFO.h>

struct LWEHttpRequest {
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

	char m_Body[1024 * 256];
	char m_Host[128];
	char m_Path[128];
	char m_Origin[128];
	char m_Authorization[128];
	char m_ContentType[128];
	char m_TransferEncoding[128];
	char m_SecWebSockKey[128];
	char m_SecWebSockProto[128];
	LWSocket *m_Socket;
	void *m_UserData;
	std::function<void(LWEHttpRequest &, const char *)> m_Callback;
	uint32_t m_ContentLength;
	uint32_t m_ChunkLength;
	uint32_t m_WebSockVersion;
	uint32_t m_Status;
	uint32_t m_Flag;
	uint16_t m_Port;

	static uint32_t Escape(const char *In, char *Buffer, uint32_t BufferLen);

	static uint32_t EscapeURI(const char *In, char *Buffer, uint32_t BufferLen);

	static uint32_t UnEscape(const char *In, char *buffer, uint32_t BufferLen);

	static uint32_t MakeJSONQueryString(LWEJson &Json, char *Buffer, uint32_t BufferLen);

	static uint32_t GZipDecompress(const char *In, uint32_t InLen, char *Buffer, uint32_t BufferLen);

	/*!< \brief parses a uri and seperates it into 3 components: Hostname address of the domain, Path asked for on that domain, and leading protocol specefied, such that https://example.com/abc.html would get seperated into host: example.com protocol: https path: /abc.html, also supports specifying the port (i.e: https://example.com:100/abc.com well return the port of 100.
		 \param URI the uri to be parsed.
		 \param HostBuffer the buffer to store the host component.
		 \param HostBufferSize the length of the buffer for storing the host component.
		 \param HostBufferLen the actual length of the host buffer component(may be null).
		 \param PathBuffer the buffer to store the path component.
		 \param PathBufferSize the length of the buffer for storing the path component.
		 \param PathBufferLen the actual length of the path buffer component(may be null).
		 \param ProtocolBuffer the buffer to store the protocol component.
		 \param ProtocolBufferSize the length of the buffer for storing the protocol component.
		 \param ProtocolBufferLen the actual length of the protocol buffer component(may be null).
		 \return the port, either deduced from the protocol(http=80, https=443, ws=80, wss=443) or as specified in the uri, otherwise 0 if unknown protocol.
	*/

	static uint16_t ParseURI(const char *URI, char *HostBuffer, uint32_t HostBufferSize, uint32_t *HostBufferLen, char *PathBuffer, uint32_t PathBufferSize, uint32_t *PathBufferLen, char *ProtocolBuffer, uint32_t ProtocolBufferSize, uint32_t *ProtocolBufferLen);

	uint32_t Serialize(char *Buffer, uint32_t BufferLen, const char *UserAgent);

	bool Deserialize(const char *Buffer, uint32_t Len);

	LWEHttpRequest &SetURI(const char *URI);

	LWEHttpRequest &SetURIf(const char *Fmt, ...);

	LWEHttpRequest &SetHost(const char *Host);

	LWEHttpRequest &SetHostf(const char *Fmt, ...);

	LWEHttpRequest &SetPath(const char *Path);

	LWEHttpRequest &SetPathf(const char *Fmt, ...);

	LWEHttpRequest &SetOrigin(const char *Origin);

	LWEHttpRequest &SetOriginf(const char *Fmt, ...);

	LWEHttpRequest &SetWebSockKey(const char *Key);

	LWEHttpRequest &SetWebSockKeyf(const char *Fmt, ...);

	LWEHttpRequest &SetWebSockProto(const char *Protocols);

	LWEHttpRequest &SetWebSockProtof(const char *Fmt, ...);

	LWEHttpRequest &SetAuthorization(const char *Auth);

	LWEHttpRequest &SetAuthorizationf(const char *Fmt, ...);

	LWEHttpRequest &SetContentType(const char *ContentType);

	LWEHttpRequest &SetContentTypef(const char *Fmt, ...);

	LWEHttpRequest &SetBody(const char *Body);

	LWEHttpRequest &SetBodyf(const char *Fmt, ...);

	LWEHttpRequest &SetCallback(std::function<void(LWEHttpRequest &, const char*)> Callback);

	LWEHttpRequest &SetMethod(uint32_t Method);

	LWEHttpRequest &SetConnectionState(uint32_t ConnState);

	LWEHttpRequest &SetCacheState(uint32_t CacheState);

	uint32_t GetCacheState(void);

	uint32_t GetConnectionState(void);

	bool CloseConnection(void);

	bool KeepAliveConnection(void);

	bool UpgradeConnection(void);

	uint32_t GetMethod(void);

	uint32_t GetEncodeType(void);

	uint32_t GetUpgradeType(void);

	template<class T, class Y>
	LWEHttpRequest &SetMethodCallback(T Inst, Y Method) {
		return SetCallback(std::bind(&Method, Inst, std::placeholders::_1, std::placeholders::_2));
	}

	LWEHttpRequest(const char *URI, uint32_t Flag);

	LWEHttpRequest();
};

class LWEProtocolHttp : public LWProtocol {
public:
	enum {
		RequestBufferSize = 64
	};

	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	uint32_t Send(LWSocket &Socket, const char *Buffer, uint32_t Len);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t Len);

	LWEProtocolHttp &ProcessRequests(uint32_t ProtocolID, LWProtocolManager &Manager);

	LWEProtocolHttp &SetProtocolID(uint32_t ProtocolID);

	LWEProtocolHttp &SetProtocolManager(LWProtocolManager *Manager);

	bool PushRequest(LWEHttpRequest &Request);

	bool PushResponse(LWEHttpRequest &InRequest, const char *Response, uint32_t lStatus);

	bool GetNextRequest(LWEHttpRequest &Request);

	bool GetNextFromPool(LWEHttpRequest **Request);

	LWEProtocolHttp &SetAgentString(const char *Agent);

	LWEProtocolHttp &SetServerString(const char *Server);

	LWEProtocolHttp(uint32_t ProtocolID, LWProtocolManager *Manager);

	LWEProtocolHttp();
protected:
	char m_Agent[256];
	char m_Server[256];
	LWProtocolManager *m_Manager;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_Pool;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_OutRequests;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_InRequests;
	uint32_t m_ProtocolID;
};

#endif

