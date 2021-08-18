#include "LWEProtocols/LWEProtocolHTTP.h"
#include <LWNetwork/LWProtocolManager.h>
#include <LWNetwork/LWSocket.h>
#include <LWEJson.h>
#include <LWELogger.h>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <iostream>
#include <zlib.h>
#include <ctime>
#include <functional>

//LWEAWSCredentials
LWEAWSCredentials::LWEAWSCredentials(const LWUTF8Iterator &AccessKey, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken, float TimeToLive) : m_TimeToLive(TimeToLive) {
	LWVerify(AccessKey.Copy(m_AccessKey, sizeof(m_AccessKey)) <= sizeof(m_AccessKey));
	LWVerify(SecretKey.Copy(m_SecretKey, sizeof(m_SecretKey)) <= sizeof(m_SecretKey));
	LWVerify(SessionToken.Copy(m_SessionToken, sizeof(m_SessionToken)) <= sizeof(m_SessionToken));
}

//LWEHttpRequestHeader:
LWEHttpRequestHeader::LWEHttpRequestHeader(uint32_t NameOffset, uint32_t NameLength, uint32_t ValueOffset, uint32_t ValueLength, uint32_t NameHash) : m_NameOffset(NameOffset), m_ValueOffset(ValueOffset), m_NameLength(NameLength), m_ValueLength(ValueLength), m_NameHash(NameHash){}


//LWEHttpRequest:
uint32_t LWEHttpRequest::MakeJSONQueryString(LWEJson &Json, char *Buffer, uint32_t BufferLen) {
	const uint32_t TBufferLen = 64 * 1024;
	char TempBuffer[TBufferLen];
	uint32_t Len = Json.GetLength();
	uint32_t o = 0;

	bool First = true;
	for (uint32_t i = 0; i < Len; i++) {
		LWEJObject *JO = Json.GetElement(i, nullptr);
		if (JO->m_Type == LWEJObject::Array || JO->m_Type == LWEJObject::Object) continue;

		if (First) o += LWUTF8I::Fmt_ns(TempBuffer, TBufferLen, o, "{}={}", JO->m_Name, JO->m_Value);
		else o += LWUTF8I::Fmt_ns(TempBuffer, TBufferLen, o, "&{}={}", JO->m_Name, JO->m_Value);
		First = false;
	}
	return LWEJson::EscapeString(TempBuffer, Buffer, BufferLen);
}

uint32_t LWEHttpRequest::MakeHTTPDate(char8_t *Buffer, uint32_t BufferLen) {
	time_t now = time(0);
	struct tm mtime = *gmtime(&now);
	return (uint32_t)strftime((char*)Buffer, BufferLen, "%a, %d %b %Y %H:%M:%S %Z", &mtime);
}

uint32_t LWEHttpRequest::MakeAMZDate(char8_t *Buffer, uint32_t BufferLen, bool IncludeSubTime) {
	time_t now = time(0);
	struct tm gtime = *gmtime(&now);
	if (IncludeSubTime) return (uint32_t)strftime((char*)Buffer, BufferLen, "%Y%m%dT%H%M%SZ", &gtime);
	return (uint32_t)strftime((char*)Buffer, BufferLen, "%Y%m%d", &gtime);
}

uint32_t LWEHttpRequest::Serialize(char8_t *Buffer, uint32_t BufferLen, const LWUTF8Iterator &UserAgent) {
	char8_t Methods[][32] = { "GET", "POST" };
	char8_t Caches[][32] = { "no-cache" };
	char8_t Connections[][32] = { "close", "keep-alive",  "upgrade", "Keep-alive, upgrade" };
	char8_t Encodings[][32] = { "", "chunk" };
	char8_t ContentEncodings[][32] = { "identity", "gzip", "compress", "deflate", "br" };
	char8_t Upgrades[][32] = { "", "websocket" };
	uint32_t o = 0;
	uint32_t ConnectionBits = (m_Flag&CONNECTIONBITS) >> CONNECTIONOFFSET;
	uint32_t CacheBits = (m_Flag&CACHEBITS) >> CACHEOFFSET;
	uint32_t EncodeBits = (m_Flag&ENCODEBITS) >> ENCODEOFFSET;
	uint32_t ContentEncodeBits = (m_Flag&CONTENTENCODEBITS) >> CONTENTENCODEOFFSET;
	uint32_t UpgradeBits = (m_Flag&UPGRADEBITS) >> UPGRADEOFFSET;
	bool IsResponse = m_Status != 0;
	if (IsResponse) {
		char8_t *lStatus = "";
		if (m_Status == Ok) lStatus = "OK";
		else if (m_Status == Continue) lStatus = "Continue";
		else if (m_Status == SwitchingProtocols) lStatus = "Switching Protocols";
		else if (m_Status == lBadRequest) lStatus = "Bad Request";
		else if (m_Status == Unauthorized) lStatus = "Unauthorized";
		else if (m_Status == Forbidden) lStatus = "Forbidden";
		else if (m_Status == NotFound) lStatus = "Not Found";
		else if (m_Status == InternalServerError) lStatus = "Internal Server Error";
		else if (m_Status == NotImplemented) lStatus = "Not Implemented";
		else if (m_Status == BadGateway) lStatus = "Bad Gateway";
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "HTTP/1.1 {} {}\r\n", m_Status, lStatus);
	}else o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "{} {} HTTP/1.1\r\n", Methods[m_Flag&METHODBITS], m_Path);

	for (uint32_t i = 0; i < m_HeaderCount; i++) {
		LWEHttpRequestHeader &H = m_HeaderTable[i];
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "{}: {}\r\n", m_Header + H.m_NameOffset, m_Header + H.m_ValueOffset);
	}
	if (m_ContentLength) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Content-Length: {}\r\n", m_ContentLength);
	if (!UserAgent.AtEnd()) {
		if (IsResponse) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Server: {}\r\n", UserAgent);
		else o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "User-Agent: {}\r\n", UserAgent);
	}
	if (m_WebSockVersion && !IsResponse) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Sec-WebSocket-Version: {}\r\n", m_WebSockVersion);
	if (*Encodings[EncodeBits]) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Transfer-Encoding: {}\r\n", Encodings[EncodeBits]);
	if (*Upgrades[UpgradeBits]) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Upgrade: {}\r\n", Upgrades[UpgradeBits]);
	o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Connection: {}\r\n", Connections[ConnectionBits]);
	o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Cache-Control: {}\r\n", Caches[CacheBits]);
	
	if (ContentEncodeBits != ContentEncodeIdentity) {
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "Content-Encoding: {}\r\n", ContentEncodings[ContentEncodeBits]);
	}

	o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "\r\n");
	if (*m_Body) {
		if (m_Flag&EncodeChunked) {
			if (!m_ContentLength) m_ContentLength = (uint32_t)strlen(m_Body);
			o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "{:x}\r\n{}\r\n", m_ContentLength, m_Body);
		}else {
			o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "{}", m_Body);
		}
	}
	return o;
}

uint32_t LWEHttpRequest::GZipDecompress(const char8_t *In, uint32_t InLen, char8_t *Buffer, uint32_t BufferLen) {
	z_stream Stream;
	Stream.next_in = (Bytef*)In;
	Stream.avail_in = InLen;
	Stream.total_out = 0;
	Stream.total_in = 0;
	Stream.zalloc = Z_NULL;
	Stream.zfree = Z_NULL;
	if (inflateInit2(&Stream, 31) != Z_OK) {
		LWELogCritical<256>("Failed to start inflate.");
		return 0;
	}
	uint32_t o = 0;
	bool Finished = false;
	while (!Finished) {
		if (o >= BufferLen) {
			return o;
		}
		Stream.next_out = (Bytef*)Buffer + o;
		Stream.avail_out = BufferLen - o;
		int32_t r = inflate(&Stream, Z_SYNC_FLUSH);
		if (r == Z_OK || r == Z_STREAM_END) o = (uint32_t)Stream.total_out;
		if (r != Z_OK) {
			if (Stream.msg) LWELogCritical<256>("GZip Error: {}", Stream.msg);
			Finished = true;
		}
	}
	if (inflateEnd(&Stream) != Z_OK) {
		LWELogCritical<256>("GZip error: 'inflateEnd'");
	}
	return o;
}

bool LWEHttpRequest::Deserialize(const char8_t *Buffer, uint32_t Len, bool Verbose) {
	char8_t Methods[][32] = { "GET", "POST" };
	char8_t NameBuffer[1024];
	char8_t ResultBuffer[1024];
	const uint32_t MethodCnt = 2;

	uint32_t o = 0;
	uint32_t k = 0;
	uint32_t i = 0;
	bool IsResponse = true;
	if (isHeadersRead()) {
		if ((m_Flag&ENCODEBITS) == EncodeChunked) {
			if (!m_ChunkLength) {
				sscanf(Buffer + o, "%1023s\r\n%n", NameBuffer, &k);
				if (k == 0) return false;
				sscanf(NameBuffer, "%x", &m_ChunkLength);
				o += k;
				if (!m_ChunkLength) {
					m_Flag |= ResponseReady;
					return true;
				}
				return Deserialize(Buffer + o, Len - o);
			} else {
				uint32_t Remain = std::min<uint32_t>(sizeof(m_Body) - m_ContentLength, std::min<uint32_t>(Len, m_ChunkLength));
				memcpy(m_Body + m_ContentLength, Buffer, Remain);
				m_ContentLength += Remain;
				m_ChunkLength -= Remain;
				o += Remain;
				if (!m_ChunkLength) o += 2;
				if (m_ContentLength == sizeof(m_Body)) m_ContentLength--;
				m_Body[m_ContentLength] = '\0';
				return o != Len ? Deserialize(Buffer + o, Len - o) : true;
			}
		} else {
			uint32_t Remain = std::min<uint32_t>((sizeof(m_Body)) - m_ChunkLength, Len);
			uint32_t ContentEncodeBits = (m_Flag&CONTENTENCODEBITS);
			std::copy(Buffer, Buffer + Remain, m_Body + m_ChunkLength);
			m_ChunkLength += Remain;
			if (!ContentEncodeBits) {
				if (m_ChunkLength == sizeof(m_Body)) m_ChunkLength--;
				m_Body[m_ChunkLength] = '\0';
			}
			if (m_ChunkLength >= m_ContentLength || m_ChunkLength >= sizeof(m_Body) - 1) {
				if (ContentEncodeBits == ContentEncodeGZip) {
					m_ContentLength = GZipDecompress(m_Body, m_ContentLength, m_Body, sizeof(m_Body));
					m_Body[m_ContentLength] = 0;
				}
				m_Flag |= ResponseReady;
			}
		}
		return true;
	}

	sscanf(Buffer + o, "HTTP/1.1 %d %255[^\r]\n%n", &m_Status, m_Path, &k);
	if (k == 0) {
		sscanf(Buffer + o, "%255s %255s HTTP/1.1\r\n%n", NameBuffer, m_Path, &k);
		if (k == 0) return false;
		IsResponse = false;
	}
	o += k;
	if (!IsResponse) {
		for (i = 0; i < MethodCnt; i++) {
			if (!strcmp(NameBuffer, Methods[i])) {
				m_Flag |= i;
				break;
			}
		}
		if (i == MethodCnt) {
			if(Verbose) LWELogCritical<256>("Unknown method: '{}'", NameBuffer);
			return false;
		}
	};
	m_HeaderCount = 0;
	m_Flag |= ConnectionKeepAlive;
	while (Buffer[o] && o < Len) {
		if (Buffer[o] == '\r' || Buffer[o] == '\n') {
			if ((m_Flag&EncodeChunked) != 0) m_ContentLength = 0;
			m_Flag |= HeadersRead;
			m_Body[0] = '\0';
			return Deserialize(Buffer + o + 2, Len - (o + 2));
		}
		k = 0;
		sscanf(Buffer + o, "%1023[^:]: %1023[^\r]%n", NameBuffer, ResultBuffer, &k);
		if (k == 0) return false;
		o += k + 2;
		LWUTF8Iterator Name = LWUTF8Iterator(NameBuffer);
		LWUTF8Iterator Value = LWUTF8Iterator(ResultBuffer);
		Name.Lower(NameBuffer, sizeof(NameBuffer)); //Lower Name.

		//Check if header is any that we convert to flags/values.
		if (Name.Compare("content-length")) m_ContentLength = atoi(Value.c_str());
		else if(Name.Compare("sec-websocket-version")) m_WebSockVersion = atoi(Value.c_str());
		else if (Name.Compare("connection")) {
			for (LWUTF8Iterator C = Value; !C.AtEnd(); C.AdvanceToken(',').Advance().AdvanceWord(true)) {
				if (C.Compare("close", 5)) m_Flag &= ~CONNECTIONBITS;
				else if (C.Compare("keep-alive", 10)) m_Flag |= ConnectionKeepAlive;
				else if (C.Compare("upgrade", 7)) m_Flag |= ConnectionUpgrade;
			}
		} else {
			//Check if header is anything where value comparison is also case-insensitive.
			uint32_t hdr = Name.CompareList("cache-control", "transfer-encoding", "content-encoding", "upgrade");
			if (hdr != -1) {
				Value.Lower(ResultBuffer, sizeof(ResultBuffer)); //lower value to pre-pare for case insensitive comparisons.
				if (hdr == 0) { //cache-control was true.
					if (Value.Compare("no-cache")) m_Flag |= NoCacheControl;
				} else if (hdr == 1) { //transfer-encoding.
					if (Value.Compare("chunked")) m_Flag |= EncodeChunked;
				} else if (hdr == 2) {
					if (Value.Compare("identity")) m_Flag |= ContentEncodeIdentity;
					else if (Value.Compare("gzip")) m_Flag |= ContentEncodeGZip;
					else if (Value.Compare("compress")) m_Flag |= ContentEncodeCompress;
					else if (Value.Compare("deflate")) m_Flag |= ContentEncodeDeflate;
					else if (Value.Compare("br")) m_Flag |= ContentEncodeBR;
				} else if (hdr == 3) {
					if (Value.Compare("websocket")) m_Flag |= UpgradeWebSock;
				}
			} else {
				if (!PushHeader(Name, Value)) {
					if (Verbose) LWELogWarn<256>("HTTP header dropped: '{}' - '{}'", Name, Value);
				}
			}
		}
	}
	return true;
}

LWEHttpRequest &LWEHttpRequest::SetURI(const LWUTF8Iterator &URI) {
	LWUTF8Iterator Proto, Domain, Path;
	LWSocket::SplitURI(URI, m_Port, Domain, Proto, Path);
	PushHeader("host", Domain);
	SetPath(Path);
	return *this;
}

bool LWEHttpRequest::PushHeader(const LWUTF8Iterator &HeaderName, const LWUTF8Iterator &HeaderValue) {
	if (m_HeaderCount >= MaxHeaderItems) return false;
	uint32_t NameOffset = m_HeaderOffset;
	uint32_t Remaining = MaxHeaderLength - m_HeaderOffset;
	uint32_t NameLen = std::min<uint32_t>(HeaderName.Lower(m_Header + m_HeaderOffset, Remaining) + 1, Remaining); //Add 1 to skip over terminal.
	uint32_t ValueLen = std::min<uint32_t>(HeaderValue.Copy(m_Header + m_HeaderOffset + NameLen, Remaining - NameLen) + 1, Remaining - NameLen);
	m_HeaderOffset += NameLen + ValueLen;
	m_HeaderTable[m_HeaderCount++] = LWEHttpRequestHeader(NameOffset, NameLen, NameOffset + NameLen, ValueLen, LWUTF8Iterator(m_Header + NameOffset, NameLen).Hash());
	return true;
}

bool LWEHttpRequest::GenerateAWS4Auth(const LWUTF8Iterator &AccessKeyID, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken) {
	const uint32_t TokenAPIID = 0;
	const uint32_t TokenService = 1;
	const uint32_t TokenRegion = 2;

	char8_t Methods[][5] = { "GET", "POST" };
	LWUTF8Iterator HostTokens[5];
	LWUTF8Iterator PathTokens[2];
	char8_t FullTimeBuffer[128];
	char8_t SubTimeBuffer[128];
	char8_t Algorithm[] = "AWS4-HMAC-SHA256";
	uint32_t BodyHash[8];
	uint32_t CanoicalHash[8];
	uint32_t SignatureHashA[8];
	uint32_t SignatureHashB[8];
	char8_t CanonicalRequest[1024 * 8];
	char8_t StringToSign[1024 * 8];
	char8_t Authorization[1024 * 8];
	char8_t AWSKey[512];
	char8_t Signature[65];
	uint32_t CanonicalLength = 0;
	uint32_t StringToSignLength = 0;
	uint32_t AuthLength = 0;
	//First seperate host into tokens to get Service+Region.
	LWUTF8Iterator Host = GetHeader("host");
	LWUTF8Iterator Path = LWUTF8Iterator(m_Path);
	if (Host.AtEnd()) return false;
	if (Host.SplitToken(HostTokens, 5, '.') != 5) return false;
	uint32_t FullTimeLen = MakeAMZDate(FullTimeBuffer, sizeof(FullTimeBuffer), true);
	uint32_t SubTimeLen = MakeAMZDate(SubTimeBuffer, sizeof(SubTimeBuffer), false);
	PushHeader("x-amz-date", FullTimeBuffer);
	if (!SessionToken.AtEnd()) PushHeader("x-amz-security-token", SessionToken);
	Path.SplitToken(PathTokens, 2, '?');

	uint32_t MethodID = GetMethod();

	//Build CanonicalRequest string. TODO: Actually escape URI+Querry correctly.
	CanonicalLength += LWUTF8I::Fmt_ns(CanonicalRequest, sizeof(CanonicalRequest), CanonicalLength, "{}\n", Methods[MethodID]);
	CanonicalLength += LWUTF8I::Fmt_ns(CanonicalRequest, sizeof(CanonicalRequest), CanonicalLength, "{}\n", PathTokens[0]); //CanonicalURI TODO: Escape URI correctly
	CanonicalLength += LWUTF8I::Fmt_ns(CanonicalRequest, sizeof(CanonicalRequest), CanonicalLength, "{}\n", PathTokens[1]); //CanonicalQuery TODO: Escape Query.
	
	//Add headers(we'll try with just host+x-amz-date signed):
	CanonicalLength += LWUTF8I::Fmt_ns(CanonicalRequest, sizeof(CanonicalRequest), CanonicalLength, "host:{}\n", Host);
	CanonicalLength += LWUTF8I::Fmt_ns(CanonicalRequest, sizeof(CanonicalRequest), CanonicalLength, "x-amz-date:{}\n\n", FullTimeBuffer);
	CanonicalLength += LWUTF8I::Fmt_ns(CanonicalRequest, sizeof(CanonicalRequest), CanonicalLength, "host;x-amz-date\n");
	CanonicalLength += LWCrypto::Digest(BodyHash, LWCrypto::HashSHA256(m_Body, m_ContentLength, BodyHash), CanonicalRequest + CanonicalLength, sizeof(CanonicalRequest) - CanonicalLength); //Possible overflow here, need to fix later.
	
	//Build the string to sign:
	StringToSignLength += LWUTF8I::Fmt_ns(StringToSign, sizeof(StringToSign), StringToSignLength, "{}\n{}\n", Algorithm, FullTimeBuffer); //Algorithm+Time
	StringToSignLength += LWUTF8I::Fmt_ns(StringToSign, sizeof(StringToSign), StringToSignLength, "{}/{}/{}/aws4_request\n", SubTimeBuffer, HostTokens[TokenRegion], HostTokens[TokenService]);
	StringToSignLength += LWCrypto::Digest(CanoicalHash, LWCrypto::HashSHA256(CanonicalRequest, CanonicalLength, CanoicalHash), StringToSign + StringToSignLength, sizeof(StringToSign) - StringToSignLength); //Possible overflow here, need to fix later.

	uint32_t AWSKeyLen = LWUTF8I::Fmt_ns(AWSKey, sizeof(AWSKey), 0, "AWS4{}", SecretKey);
	LWCrypto::HashHMAC_SHA256(SubTimeBuffer, SubTimeLen, AWSKey, AWSKeyLen, SignatureHashA);
	LWCrypto::HashHMAC_SHA256(HostTokens[TokenRegion].c_str(), (uint32_t)HostTokens[TokenRegion].RawRemaining(), (char*)SignatureHashA, 32, SignatureHashB);
	LWCrypto::HashHMAC_SHA256(HostTokens[TokenService].c_str(), (uint32_t)HostTokens[TokenService].RawRemaining(), (char*)SignatureHashB, 32, SignatureHashA);
	LWCrypto::HashHMAC_SHA256("aws4_request", 12, (char*)SignatureHashA, 32, SignatureHashB);
	LWCrypto::Digest(SignatureHashA, LWCrypto::HashHMAC_SHA256(StringToSign, StringToSignLength, (char*)SignatureHashB, 32, SignatureHashA), Signature, sizeof(Signature));

	AuthLength += LWUTF8I::Fmt_ns(Authorization, sizeof(Authorization), AuthLength, "{} Credential={}/{}/{}/{}/aws4_request, SignedHeaders=host;x-amz-date, Signature={}", Algorithm, AccessKeyID, SubTimeBuffer, HostTokens[TokenRegion], HostTokens[TokenService], Signature);
	PushHeader("Authorization", Authorization);
	return true;
}

bool LWEHttpRequest::GenerateAWS4Auth(const LWEAWSCredentials &Credentials) {
	return GenerateAWS4Auth(Credentials.m_AccessKey, Credentials.m_SecretKey, Credentials.m_SessionToken);
}

LWEHttpRequest &LWEHttpRequest::SetPath(const LWUTF8Iterator &Path) {
	if (Path.AtEnd()) strcpy(m_Path, "/");
	else Path.Copy(m_Path, sizeof(m_Path));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetBody(const LWUTF8Iterator &Body) {
	m_ContentLength = Body.Copy(m_Body, sizeof(m_Body)) - 1;//-1 to Remove null char.
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetCallback(LWEHttpResponseCallback Callback) {
	m_Callback = Callback;
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetMethod(uint32_t Method) {
	m_Flag = (m_Flag&~METHODBITS) | Method;
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetConnectionState(uint32_t ConnState) {
	m_Flag = (m_Flag&~CONNECTIONBITS) | ConnState;
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetCacheState(uint32_t CacheState) {
	m_Flag = (m_Flag&~CACHEBITS) | CacheState;
	return *this;
}

LWUTF8Iterator LWEHttpRequest::operator [](const LWUTF8Iterator &HeaderName) const {
	return GetHeader(HeaderName);
}

LWUTF8Iterator LWEHttpRequest::GetBody(void) const {
	return m_Body;
}

LWUTF8Iterator LWEHttpRequest::GetPath(void) const {
	return m_Path;
}

LWUTF8Iterator LWEHttpRequest::GetHeader(const LWUTF8Iterator &HeaderName) const {
	uint32_t Hash = HeaderName.Hash();
	for (uint32_t i = 0; i < m_HeaderCount; i++) {
		const LWEHttpRequestHeader &H = m_HeaderTable[i];
		if (Hash == H.m_NameHash) return LWUTF8Iterator(m_Header + H.m_ValueOffset, H.m_ValueLength);
	}
	return LWUTF8Iterator();
}

uint32_t LWEHttpRequest::GetCacheState(void) const {
	return m_Flag&CACHEBITS;
}

uint32_t LWEHttpRequest::GetConnectionState(void) const {
	return m_Flag&CONNECTIONBITS;
}

bool LWEHttpRequest::CloseConnection(void) const {
	return (m_Flag&CONNECTIONBITS) == 0;
}

bool LWEHttpRequest::KeepAliveConnection(void) const{
	return (m_Flag&ConnectionKeepAlive) != 0;
}

bool LWEHttpRequest::UpgradeConnection(void) const {
	return (m_Flag&ConnectionUpgrade) != 0;
}

uint32_t LWEHttpRequest::GetMethod(void) const {
	return m_Flag&METHODBITS;
}

uint32_t LWEHttpRequest::GetEncodeType(void) const {
	return m_Flag&ENCODEBITS;
}

uint32_t LWEHttpRequest::GetUpgradeType(void) const {
	return m_Flag&UPGRADEBITS;
}

bool LWEHttpRequest::isHeadersRead(void) const {
	return (m_Flag & HeadersRead) != 0;
}

bool LWEHttpRequest::isResponseReady(void) const {
	return (m_Flag & ResponseReady) != 0;
}

LWEHttpRequest::LWEHttpRequest(const LWUTF8Iterator &URI, uint32_t Flag, void *UserData, LWEHttpResponseCallback Callback) : m_Flag(Flag), m_UserData(UserData), m_Callback(Callback) {
	char8_t Date[LWEHttpRequest::ValueMaxLength];
	SetURI(URI);
	if (Flag & GenerateDate) {
		MakeHTTPDate(Date, sizeof(Date));
		PushHeader("date", Date);
	}
}

//LWEProtocolHttp:
LWProtocol &LWEProtocolHttp::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	char Buffer[1024 * 64];
	int32_t r = Socket.Receive(Buffer, sizeof(Buffer)-1);
	if (r <= 0) {
		Socket.MarkClosable();
		return *this;
	}
	Buffer[r] = '\0';
	if (!ProcessRead(Socket, Buffer, r)) {
		LWELogCritical<256>("parsing HTTP buffer.");
	}
	return *this;
}

LWProtocol &LWEProtocolHttp::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEHttpRequest *Req = (LWEHttpRequest*)New.GetProtocolData(m_ProtocolID);
	if (Req) Req->m_Socket = &New;
	return *this;
}

uint32_t LWEProtocolHttp::Send(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	uint32_t o = 0;
	for (; o < Len;) {
		int32_t r = Socket.Send(Buffer + o, Len - o);
		if (r == -1) {
			LWELogCritical<256>("Socket {} Error: {}", Socket.GetSocketDescriptor(), LWProtocolManager::GetError());
			return 1;
		}
		o += (uint32_t)r;
	}
	return o;
}

bool LWEProtocolHttp::ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	LWEHttpRequest *Req = (LWEHttpRequest*)Socket.GetProtocolData(m_ProtocolID);
	LWEHttpRequest IReq;
	bool Insert = false;
	if (!Req) {
		Req = &IReq;
		Req->m_Socket = &Socket;
		Insert = true;
	}
	if (!Req->Deserialize(Buffer, Len)) {
		LWELogCritical<256>("deserializing response.");
		return false;
	}
	if (Insert) {
		if (!GetNextFromPool(&Req)) {
			LWELogCritical<256>("Could not get a request to write to from pool.");
			return false;
		}
		*Req = IReq;
		Socket.SetProtocolData(m_ProtocolID, Req);
	}
	if(Req->isResponseReady()) {
		if (Req->m_Status == 0) {
			if (!m_InRequests.Push(*Req, &Req)) {
				LWELogCritical<256>("Failed to insert response into request queue.");
				return false;
			}
			Socket.SetProtocolData(m_ProtocolID, Req);
		}
		if (Req->m_Callback) Req->m_Callback(*Req, Req->m_Body);
		if (Req->CloseConnection() && Req->m_Status != 0) {
			Socket.MarkClosable();
		}

	}
	return true;
}

LWEProtocolHttp &LWEProtocolHttp::ProcessRequests(uint32_t ProtocolID, LWProtocolManager &Manager) {
	char Buffer[1024 * 64];
	LWEHttpRequest *Request;
	while (m_OutRequests.Pop(&Request)) {
		LWSocket Sock;
		bool IsResponse = Request->m_Status != 0;
		uint32_t Len = Request->Serialize(Buffer, sizeof(Buffer), IsResponse ? m_Agent : m_Server);
		if (!Len) continue;
		if (!Request->m_Socket) {
			uint32_t Error = LWSocket::CreateSocket(Sock, (*Request)["host"], Request->m_Port, LWSocket::Tcp, ProtocolID);
			if (Error) {
				LWELogCritical<256>("'{}:{}' Error: {}", (*Request)["host"], Request->m_Port, Error);
				//Create DomainNotFound Response:
				LWEHttpRequest ErrorRes = LWEHttpRequest();
				ErrorRes.m_Status = LWEHttpRequest::DomainNotFound;
				ErrorRes.m_UserData = Request->m_UserData;
				if (Request->m_Callback) Request->m_Callback(ErrorRes, LWUTF8Iterator());
				continue;
			}
			Request->m_Socket = Manager.PushSocket(Sock);
			//std::cout << "Making socket: " << Request->m_Socket->GetSocketDescriptor() << std::endl;
			Request->m_Socket->SetProtocolData(m_ProtocolID, Request);
			if (!Request->m_Socket) {
				LWELogCritical<256>("inserting socket.");
				continue;
			}
		}
		uint32_t Res = Send(*Request->m_Socket, Buffer, Len);
		if (Res == -1) {
			LWELogCritical<256>("sending request.");
			continue;
		} else if (Res == 0) { //We couldn't send(likely a TLS connection, so we will re-insert and try again later).
			LWEHttpRequest *NewRequest;
			if (!m_OutRequests.Push(*Request, &NewRequest)) {
				Request->m_Socket->MarkClosable();
			} else {
				NewRequest->m_Socket->SetProtocolData(m_ProtocolID, NewRequest);
				return *this; //Connection is not ready, so we put it on the back of the list and wait until it is ready.
			}
		}
		if (Request->CloseConnection() && Request->m_Status!=0) Request->m_Socket->MarkClosable();
	}
	return *this;
}

LWEProtocolHttp &LWEProtocolHttp::SetProtocolID(uint32_t ProtocolID) {
	m_ProtocolID = ProtocolID;
	return *this;
}

bool LWEProtocolHttp::GetNextRequest(LWEHttpRequest &Request) {
	return m_InRequests.Pop(Request);
}

bool LWEProtocolHttp::GetNextFromPool(LWEHttpRequest **Request) {
	LWEHttpRequest Req;
	if (!m_Pool.Push(Req, Request)) return false;
	return m_Pool.Pop(Req);
}

bool LWEProtocolHttp::PushRequest(LWEHttpRequest &Request) {
	LWEHttpRequest Req = Request;
	return m_OutRequests.Push(Req);
}

bool LWEProtocolHttp::PushResponse(LWEHttpRequest &InRequest, const LWUTF8Iterator &Response, uint32_t lStatus) {
	LWEHttpRequest Request = InRequest;
	Request.m_Status = lStatus;
	Request.SetBody(Response);
	return m_OutRequests.Push(Request);
}

LWEProtocolHttp &LWEProtocolHttp::SetAgentString(const LWUTF8Iterator &Agent) {
	Agent.Copy(m_Agent, sizeof(m_Agent));
	return *this;
}

LWEProtocolHttp &LWEProtocolHttp::SetServerString(const LWUTF8Iterator &Server) {
	Server.Copy(m_Server, sizeof(m_Server));
	return *this;
}

LWEProtocolHttp::LWEProtocolHttp(uint32_t ProtocolID) : LWProtocol(), m_ProtocolID(ProtocolID) {}
