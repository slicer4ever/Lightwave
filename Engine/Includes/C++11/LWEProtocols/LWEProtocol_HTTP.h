#ifndef LWEPROTOCOL_HTTP_H
#define LWEPROTOCOL_HTTP_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWSocket.h>
#include <LWCore/LWRef.h>
#include <functional>
#include <LWEJson.h>
#include <LWCore/LWConcurrent/LWFIFO.h>
#include <shared_mutex>

struct LWEHTTPMessage;

typedef std::function<void(const LWRef<LWEHTTPMessage> &, const LWUTF8Iterator &)> LWEHTTPResponseCallback;

/*!< \brief convenient structure for holding cognito credentials from AWS service's. */
struct LWEAWSCredentials {
	char8_t m_AccessKey[64] = {};
	char8_t m_SecretKey[64] = {};
	char8_t m_SessionToken[2048] = {};
	float m_TimeToLive; //Time till expiration and credentials need to be renewed. (aws credentials sent with Expiration date in seconds since epoch, this should be subtracted from current epoch and converted to seconds to get TTL value)  This value can be trimmed down slightly so that renewal happens before credentials actually run out.

	LWEAWSCredentials(const LWUTF8Iterator &AccessKey, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken, float TimeToLive = std::numeric_limits<float>::max());

	LWEAWSCredentials() = default;
};

struct LWEHTTPMessageHeader {
	//Can't point directly to data as copying a request will then break this data.
	uint32_t m_NameOffset = 0;//Offset into Header to get name position.
	uint32_t m_ValueOffset = 0;
	uint32_t m_NameLength = 0; //Includes terminal.
	uint32_t m_ValueLength = 0;
	uint32_t m_NameHash = LWCrypto::FNV1AHash;

	LWEHTTPMessageHeader(uint32_t NameOffset, uint32_t NameLength, uint32_t ValueOffset, uint32_t ValueLength, uint32_t NameHash);

	LWEHTTPMessageHeader() = default;
};

struct LWEHTTPMessage {
	static const uint32_t BodyMaxLength = 1024 * 256; //256kb default.
	static const uint32_t ValueMaxLength = 256;
	static const uint32_t PathMaxLength = 1024;
	static const uint32_t MaxHeaderItems = 32;
	static const uint32_t MaxHeaderLength = 1024 * 2; //2kb of header data.
	LWBitField32(MethodBits, 4, 0);
	LWBitField32(ConnectionBits, 2, MethodBitsOffset+4);
	LWBitField32(CacheControlBits, 2, ConnectionBitsOffset+2);
	LWBitField32(EncodingBits, 1, CacheControlBitsOffset+2);
	LWBitField32(ContentEncodeBits, 3, EncodingBitsOffset+1);
	LWBitField32(UpgradeBits, 1, ContentEncodeBitsOffset+3); 
	LWBitField32(StatusBits, 10, UpgradeBitsOffset+1);

	static const uint32_t Method_Get = 0;
	static const uint32_t Method_Head = 1;
	static const uint32_t Method_Post = 2;
	static const uint32_t Method_Put = 3;
	static const uint32_t Method_Delete = 4;
	static const uint32_t Method_Connect = 5;
	static const uint32_t Method_Options = 6;
	static const uint32_t Method_Trace = 7;
	static const uint32_t Method_Patch = 8;

	static const uint32_t Connection_Close = 0;
	static const uint32_t Connection_KeepAlive = 1;
	static const uint32_t Connection_Upgrade = 2;

	static const uint32_t CacheControl_None = 0;//Default cache control if not indicated.
	static const uint32_t CacheControl_NoCache = 1; 
	static const uint32_t CacheControl_NoStore = 2;
	static const uint32_t CacheControl_Cache = 3;

	static const uint32_t Encode_None = 0;
	static const uint32_t Encode_Chunked = 1;

	static const uint32_t ContentEncode_Identity = 0;
	static const uint32_t ContentEncode_GZip = 1;
	static const uint32_t ContentEncode_Compress = 2;
	static const uint32_t ContentEncode_Deflate = 3;
	static const uint32_t ContentEncode_BR = 4;

	static const uint32_t Upgrade_None = 0;
	static const uint32_t Upgrade_WebSocket = 1;

	static const uint32_t HeadersRead = 0x40000000;
	static const uint32_t GenerateDate = 0x80000000;

	static const uint32_t ResponseFlags = ConnectionBits | CacheControlBits | UpgradeBits;

	//Status codes:
	static const uint32_t S_Request = 0; //Special code that indicates this is not a response, but a request.
	static const uint32_t S_Continue = 100;
	static const uint32_t S_SwitchingProtocols = 101;
	static const uint32_t S_Ok = 200;
	static const uint32_t S_BadRequest = 400;
	static const uint32_t S_Unauthorized = 401;
	static const uint32_t S_Forbidden = 403;
	static const uint32_t S_NotFound = 404;
	static const uint32_t S_RequestTimeout = 408;
	static const uint32_t S_InternalServerError = 500;
	static const uint32_t S_NotImplemented = 501;
	static const uint32_t S_BadGateway = 502;

	static const uint32_t S_Count = 13; //Number of status's this implementation understand.
	static const uint32_t S_Fallback = S_BadRequest; //If we receive a status code we don't understand.
	static const uint32_t S_DomainNoResponse = 600; //Custom code returned to requests if domain could not be found/would not respond to connection.

	static const char8_t StatusCodeNames[S_Count][32]; //Named codes
	static const uint32_t StatusCodeMap[S_Count]; //Map of names to status codes.

	char8_t m_Body[BodyMaxLength]= {};
	char8_t m_Path[PathMaxLength] = u8"/";
	char8_t m_Header[MaxHeaderLength]={};
	LWEHTTPMessageHeader m_HeaderTable[MaxHeaderItems];
	
	void *m_UserData = nullptr;
	LWEHTTPResponseCallback m_Callback = nullptr;
	LWRef<LWSocket> m_Socket;
	uint32_t m_ContentLength = 0;
	uint32_t m_ChunkOffset = 0; //Offset used when serializing/deserializing data.
	uint32_t m_ChunkLength = 0; //Length of the chunk when deserializing.
	uint32_t m_WebSockVersion = 0;
	uint32_t m_HeaderCount = 0;
	uint32_t m_HeaderOffset = 0; //Offset into HeaderData to store more data.
	uint32_t m_KeepAliveTimeout = 0;
	uint32_t m_KeepAliveMessages = 0;
	uint32_t m_CacheMaxAge = 0; //how long response can be cache'd for. (Value > 0 will write into headers).
	uint32_t m_Flag = 0;
	uint16_t m_Port = 0;

	static uint32_t MakeJSONQueryString(LWEJson &Json, char8_t *Buffer, uint32_t BufferLen);

	static uint32_t GZipDecompress(const void *In, uint32_t InLen, void *Buffer, uint32_t BufferLen);

	//Generates an HTTP standard date header and writes it into buffer of current time.
	static uint32_t MakeHTTPDate(void *Buffer, uint32_t BufferLen);

	//Generates an AMZ formatted date in utc time, IncludeSubTime will format with HHMMSS as well.
	static uint32_t MakeAMZDate(void *Buffer, uint32_t BufferLen, bool IncludeSubTime);

	//Returns amount of bytes serializing headers.
	uint32_t SerializeHeaders(void *Buffer, uint32_t BufferLen, const LWUTF8Iterator &UserAgent);

	//Returns amount of bytes serializing body.
	uint32_t SerializeBody(void *Buffer, uint32_t BufferLen);

	//Returns amount of bytes read from Buffer.
	uint32_t DeserializeHeaders(const void *Buffer, uint32_t Len, bool bIsTrailingHeaders = false, bool Verbose = false);

	//Returns amount of bytes read from buffer.
	uint32_t DeserializeBody(const void *Buffer, uint32_t Len, bool Verbose = false);

	LWEHTTPMessage &SetURI(const LWUTF8Iterator &URI);

	//Note HeaderName will be lowered before insertion, their is also no check if header duplicates occur.
	bool PushHeader(const LWUTF8Iterator &HeaderName, const LWUTF8Iterator &HeaderValue);

	/* \brief Will generate AWS4 signature for this request, this function should be called after the body and all other headers have been added, this function adds the header Authorization and x-amz-date headers.
	   \note this function inspects the host address to extract the region, and service being targeted, if neither are present false will be returned. if SessionToken is not empty, it will be added as x-amz
	   
	   */
	bool GenerateAWS4Auth(const LWUTF8Iterator &AccessKeyID, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken = LWUTF8Iterator());

	/*!< \brief convience function for interacting with aws api-gateway services, automatically fills in the accesskey, secretkey, and session token. */
	bool GenerateAWS4Auth(const LWEAWSCredentials &Credentials);

	LWEHTTPMessage &SetPath(const LWUTF8Iterator &Path);

	//Returns amount of bytes body takes up now, can be used to check message was correctly stored.
	uint32_t SetBody(const LWUTF8Iterator &Body);

	//Copies binary data into buffer(returns amount of bytes copied(in case BufferLen>sizeof(m_Body)).
	uint32_t SetBody(const void *Buffer, uint32_t BufferLen);

	//Set's the content length to min(Length, sizeof(Body)), returning the result.  ChunkLength is set actual length, and IsContentTooLarge can be used to verify the length was within the body size.
	uint32_t SetContentLength(uint32_t Length);

	//Same as GetHeader, but with more convenient interface.  HeaderName should be all lower-case.
	LWUTF8Iterator operator[](const LWUTF8Iterator &HeaderName) const;

	LWUTF8Iterator GetBodyAsString(void) const;

	//Search's the header table backwards for the specified header(this way repeat headers will return the last version), note HeaderName should be all lower case. returns the value of that header.
	LWUTF8Iterator GetHeader(const LWUTF8Iterator &HeaderName) const;
	
	LWUTF8Iterator GetPath(void) const;
	
	LWEHTTPMessage &SetCallback(LWEHTTPResponseCallback Callback);

	LWEHTTPMessage &SetMethod(uint32_t Method);

	LWEHTTPMessage &SetConnectionState(uint32_t ConnState);

	LWEHTTPMessage &SetCacheControl(uint32_t CacheControl);

	LWEHTTPMessage &SetEncoding(uint32_t Encoding);

	LWEHTTPMessage &SetContentEncoding(uint32_t ContentEncode);

	LWEHTTPMessage &SetUpgradeState(uint32_t Upgrade);

	LWEHTTPMessage &SetStatus(uint32_t lStatus);

	//if seconds > 0, set's the Keep-Alive connection flag.
	LWEHTTPMessage &SetKeepAlive(uint32_t Seconds, uint32_t Messages);

	uint32_t GetCacheControl(void) const;

	uint32_t GetConnectionState(void) const;

	uint32_t GetContentEncoding(void) const;

	bool CloseConnection(void) const;

	bool KeepAliveConnection(void) const;

	bool UpgradeConnection(void) const;

	uint32_t GetMethod(void) const;

	uint32_t GetEncodeType(void) const;

	uint32_t GetUpgradeState(void) const;

	uint32_t GetStatus(void) const;

	//Returns true if m_ChunkOffset==m_ContentLength && m_ContentLength>0
	bool isFinished(void) const;

	//Returns true if m_ContentLength==0 && Encoding==Chunked, the server will close any connection deemed variable sized(and send a bad request response), but clients can continue until server has closed the connection.
	bool isVariableSized(void) const;

	bool isHeadersRead(void) const;

	bool isContentTooLarge(void) const; //Detected content size is too large for this message to encapsulate. (Future implementation should accomondate any sized messages)

	template<class Type, class CB>
	LWEHTTPMessage &SetMethodCallback(CB Method, Type Obj) {
		return SetCallback(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2));
	}

	//Construct a response, copying needed data from incoming request.
	LWEHTTPMessage(const LWEHTTPMessage &Request, const LWUTF8Iterator &Body, uint32_t StatusCode);

	LWEHTTPMessage(const LWEHTTPMessage &Request, const void *Buffer, uint32_t BufferLen, uint32_t StatusCode);
	
	//Create text response message.
	LWEHTTPMessage(const LWUTF8Iterator &Host, const LWUTF8Iterator &Body, uint32_t StatusCode, uint32_t Flags, void *UserData = nullptr, LWEHTTPResponseCallback Callback = nullptr);

	template<class Type, class CB>
	LWEHTTPMessage(const LWUTF8Iterator &Host, const LWUTF8Iterator &Body, uint32_t StatusCode, uint32_t Flags, void *UserData, CB Method, Type Obj) : LWEHTTPMessage(Host, Body, StatusCode, Flags, UserData, std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2)) {}

	//Create binary response message.
	LWEHTTPMessage(const LWUTF8Iterator &Host, const void *Buffer, uint32_t BufferLen, uint32_t StatusCode, uint32_t Flags, void *UserData = nullptr, LWEHTTPResponseCallback Callback = nullptr);

	template<class Type, class CB>
	LWEHTTPMessage(const LWUTF8Iterator &Host, const void *Buffer, uint32_t BufferLen, uint32_t StatusCode, uint32_t Flags, void *UserData, CB Method, Type Obj) : LWEHTTPMessage(Host, Buffer, BufferLen, StatusCode, Flags, std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2)) {}

	LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Method, uint32_t Flags, void *UserData = nullptr, LWEHTTPResponseCallback Callback = nullptr);

	template<class Type, class CB>
	LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Methd, uint32_t Flags, void *UserData, CB Method, Type Obj) : LWEHTTPMessage(URI, Methd, Flags, UserData, std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2)) {}

	LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Method, uint32_t Flags, const LWUTF8Iterator &Body, void *UserData = nullptr, LWEHTTPResponseCallback Callback = nullptr);

	template<class Type, class CB>
	LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Methd, uint32_t Flags, const LWUTF8Iterator &Body, void *UserData, CB Method, Type Obj) : LWEHTTPMessage(URI, Methd, Flags, Body, UserData, std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2)) {}

	LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Method, uint32_t Flags, const void *Buffer, uint32_t BufferLen, void *UserData = nullptr, LWEHTTPResponseCallback Callback = nullptr);

	template<class Type, class CB>
	LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Methd, uint32_t Flags, const void *Buffer, uint32_t BufferLen, void *UserData, CB Method, Type Obj) : LWEHTTPMessage(URI, Methd, Flags, Buffer, BufferLen, UserData, std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2)) {}

	LWEHTTPMessage() = default;
};

class LWEProtocol_HTTP : public virtual LWProtocol {
public:
	enum {
		RequestBufferSize = 1024
	};

	virtual LWProtocol &Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual LWProtocol &SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	virtual uint32_t Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len);

	//Processes message from queue if possible, returns number of bytes read, or returns -1 on error.
	uint32_t ProcessHTTPReadMessage(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len, bool Verbose);

	//Process Request will automatically create sockets for requests which do not have an associated socket if Timeout(HighFrequencyClock) is non 0 then well return once time has elapsed, otherwise only returns once queue has been emptied..
	LWEProtocol_HTTP &ProcessOutboundMessages(LWProtocolManager &Manager, bool Verbose, uint64_t lTimeout = 0);

	bool PushOutMessage(const LWEHTTPMessage &Message);

	bool GetNextRequest(LWRef<LWEHTTPMessage> &Requests);

	LWEProtocol_HTTP &SetAgentString(const LWUTF8Iterator &Agent);

	LWEProtocol_HTTP &SetServerString(const LWUTF8Iterator &Server);

	//Timeout set to sockets before they will be automatically terminated.
	LWEProtocol_HTTP &SetTimeoutPeriod(uint64_t lTimeoutPeriod);

	uint64_t GetTotalInboundMessages(void) const;

	uint64_t GetTotalOutboundMessages(void) const;

	uint32_t GetHTTPRecvBytes(void) const;

	uint32_t GetHTTPSentBytes(void) const;

	LWEProtocol_HTTP(uint32_t ProtocolID, LWAllocator &Allocator);
protected:
	LWAllocator &m_Allocator;
	char8_t m_Agent[LWEHTTPMessage::ValueMaxLength]={};
	char8_t m_Server[LWEHTTPMessage::ValueMaxLength]={};
	LWConcurrentFIFO<LWRef<LWEHTTPMessage>, RequestBufferSize> m_OutMessages;
	LWConcurrentFIFO<LWRef<LWEHTTPMessage>, RequestBufferSize> m_InRequests; 
	std::unordered_map<uint32_t, LWRef<LWEHTTPMessage>> m_MessageMap;
	std::shared_mutex m_MessageMapMutex;
	uint64_t m_TimeoutPeriod = -1;
	uint64_t m_TotalInMessages = 0;
	uint64_t m_TotalOutMessages = 0;
	uint32_t m_HTTPBytesRecv = 0;
	uint32_t m_HTTPBytesSent = 0;

};

#endif

