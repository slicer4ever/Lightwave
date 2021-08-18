#ifndef LWEPROTOCOL_HTTP_H
#define LWEPROTOCOL_HTTP_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWSocket.h>
#include <functional>
#include <LWEJson.h>
#include <LWCore/LWConcurrent/LWFIFO.h>

struct LWEHttpRequest;

typedef std::function<void(LWEHttpRequest&, const LWUTF8Iterator &)> LWEHttpResponseCallback;

/*!< \brief convient structure for holding cognito credentials from AWS service's. */
struct LWEAWSCredentials {
	char8_t m_AccessKey[64] = {};
	char8_t m_SecretKey[64] = {};
	char8_t m_SessionToken[2048] = {};
	float m_TimeToLive; //Time till expiration and credentials need to be renewed. (aws credentials sent with Expiration date in seconds since epoch, this should be subtracted from current epoch and converted to seconds to get TTL value)  This value can be trimmed down slightly so that renewal happens before credentials actually run out.

	LWEAWSCredentials(const LWUTF8Iterator &AccessKey, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken, float TimeToLive = std::numeric_limits<float>::max());

	LWEAWSCredentials() = default;
};

struct LWEHttpRequestHeader {
	//Can't point directly to data as copying a request will then break this data.
	uint32_t m_NameOffset = 0;//Offset into Header to get name position.
	uint32_t m_ValueOffset = 0;
	uint32_t m_NameLength = 0; //Includes terminal.
	uint32_t m_ValueLength = 0;
	uint32_t m_NameHash = LWCrypto::FNV1AHash;

	LWEHttpRequestHeader(uint32_t NameOffset, uint32_t NameLength, uint32_t ValueOffset, uint32_t ValueLength, uint32_t NameHash);

	LWEHttpRequestHeader() = default;
};

struct LWEHttpRequest {
	static const uint32_t BodyMaxLength = 1024 * 256; //256kb default.
	static const uint32_t ValueMaxLength = 256;
	static const uint32_t PathMaxLength = 1024;
	static const uint32_t MaxHeaderItems = 32;
	static const uint32_t MaxHeaderLength = 1024 * 8; //8kb of header data.
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
		GenerateDate = 0x400, //Flag passed in to creation to generate the date.

		Continue = 100,
		SwitchingProtocols = 101,
		Ok = 200,
		lBadRequest = 400,
		Unauthorized = 401,
		Forbidden = 403,
		NotFound = 404,
		InternalServerError = 500,
		NotImplemented = 501,
		BadGateway = 502,
		DomainNotFound = 10000
	};

	char8_t m_Body[BodyMaxLength]="";
	char8_t m_Path[PathMaxLength] = "/";
	char8_t m_Header[MaxHeaderLength]="";
	LWEHttpRequestHeader m_HeaderTable[MaxHeaderItems];
	
	LWSocket *m_Socket = nullptr;
	void *m_UserData = nullptr;
	LWEHttpResponseCallback m_Callback = nullptr;
	uint32_t m_ContentLength = 0;
	uint32_t m_ChunkLength = 0;
	uint32_t m_WebSockVersion = 0;
	uint32_t m_HeaderCount = 0;
	uint32_t m_HeaderOffset = 0; //Offset into HeaderData to store more data.
	uint32_t m_Status = 0;
	uint32_t m_Flag = 0;
	uint16_t m_Port = 0;

	static uint32_t MakeJSONQueryString(LWEJson &Json, char8_t *Buffer, uint32_t BufferLen);

	static uint32_t GZipDecompress(const char8_t *In, uint32_t InLen, char8_t *Buffer, uint32_t BufferLen);

	//Generates an HTTP standard date header and writes it into buffer of current time.
	static uint32_t MakeHTTPDate(char8_t *Buffer, uint32_t BufferLen);

	//Generates an AMZ formatted date in utc time, IncludeSubTime will format with HHMMSS as well.
	static uint32_t MakeAMZDate(char8_t *Buffer, uint32_t BufferLen, bool IncludeSubTime);

	uint32_t Serialize(char8_t *Buffer, uint32_t BufferLen, const LWUTF8Iterator &UserAgent);

	bool Deserialize(const char8_t *Buffer, uint32_t Len, bool Verbose = false);

	LWEHttpRequest &SetURI(const LWUTF8Iterator &URI);

	//Note HeaderName will be lowered before insertion, their is also no check if header duplicates occur.
	bool PushHeader(const LWUTF8Iterator &HeaderName, const LWUTF8Iterator &HeaderValue);

	/* \brief Will generate AWS4 signature for this request, this function should be called after the body and all other headers have been added, this function adds the header Authorization and x-amz-date headers.
	   \note this function inspects the host address to extract the region, and service being targeted, if neither are present false will be returned. if SessionToken is not empty, it will be added as x-amz
	   
	   */
	bool GenerateAWS4Auth(const LWUTF8Iterator &AccessKeyID, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken = LWUTF8Iterator());

	/*!< \brief convience function for interacting with aws api-gateway services, automatically fills in the accesskey, secretkey, and session token. */
	bool GenerateAWS4Auth(const LWEAWSCredentials &Credentials);

	LWEHttpRequest &SetPath(const LWUTF8Iterator &Path);

	LWEHttpRequest &SetBody(const LWUTF8Iterator &Body);

	//Same as GetHeader, but with more convenient interface.  HeaderName should be all lower-case.
	LWUTF8Iterator operator[](const LWUTF8Iterator &HeaderName) const;

	LWUTF8Iterator GetBody(void) const;

	//Search's the header table for the specified header, note HeaderName should be all lower case. returns the value of that header.
	LWUTF8Iterator GetHeader(const LWUTF8Iterator &HeaderName) const;
	
	LWUTF8Iterator GetPath(void) const;
	
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

	template<class Type, class CB>
	LWEHttpRequest &SetMethodCallback(CB Method, Type Obj) {
		return SetCallback(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2));
	}

	LWEHttpRequest(const LWUTF8Iterator &URI, uint32_t Flag, void *UserData = nullptr, LWEHttpResponseCallback Callback = nullptr);

	template<class Type, class CB>
	LWEHttpRequest(const LWUTF8Iterator &URI, uint32_t Flag, void *UserData, CB Method, Type Obj) : LWEHttpRequest(URI, Flag, UserData, std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2)) {}

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

	//Process Request will automatically create sockets for requests which do not have an associated socket.
	LWEProtocolHttp &ProcessRequests(uint32_t ProtocolID, LWProtocolManager &Manager);

	LWEProtocolHttp &SetProtocolID(uint32_t ProtocolID);

	bool PushRequest(LWEHttpRequest &Request);

	bool PushResponse(LWEHttpRequest &InRequest, const LWUTF8Iterator &Response, uint32_t lStatus);

	bool GetNextRequest(LWEHttpRequest &Request);

	bool GetNextFromPool(LWEHttpRequest **Request);

	LWEProtocolHttp &SetAgentString(const LWUTF8Iterator &Agent);

	LWEProtocolHttp &SetServerString(const LWUTF8Iterator &Server);

	LWEProtocolHttp(uint32_t ProtocolID);

	LWEProtocolHttp() = default;
protected:
	char8_t m_Agent[LWEHttpRequest::ValueMaxLength]="";
	char8_t m_Server[LWEHttpRequest::ValueMaxLength]="";
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_Pool;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_OutRequests;
	LWConcurrentFIFO<LWEHttpRequest, RequestBufferSize> m_InRequests;
	uint32_t m_ProtocolID = 0;
};

#endif

