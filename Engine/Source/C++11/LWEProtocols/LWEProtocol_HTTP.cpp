#include "LWEProtocols/LWEProtocol_HTTP.h"
#include <LWNetwork/LWProtocolManager.h>
#include <LWNetwork/LWSocket.h>
#include <LWCore/LWLogger.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWTimer.h>
#include <LWEJson.h>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <iostream>
#include <zlib.h>
#include <time.h>
#include <cstdio>
#include <functional>

//LWEAWSCredentials
LWEAWSCredentials::LWEAWSCredentials(const LWUTF8Iterator &AccessKey, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken, float TimeToLive) : m_TimeToLive(TimeToLive) {
	LWVerify(AccessKey.Copy(m_AccessKey, sizeof(m_AccessKey)) <= sizeof(m_AccessKey));
	LWVerify(SecretKey.Copy(m_SecretKey, sizeof(m_SecretKey)) <= sizeof(m_SecretKey));
	LWVerify(SessionToken.Copy(m_SessionToken, sizeof(m_SessionToken)) <= sizeof(m_SessionToken));
}

//LWEHTTPRequestHeader:
LWEHTTPMessageHeader::LWEHTTPMessageHeader(uint32_t NameOffset, uint32_t NameLength, uint32_t ValueOffset, uint32_t ValueLength, uint32_t NameHash) : m_NameOffset(NameOffset), m_ValueOffset(ValueOffset), m_NameLength(NameLength), m_ValueLength(ValueLength), m_NameHash(NameHash){}


//LWEHTTPMessage:
const char8_t LWEHTTPMessage::StatusCodeNames[S_Count][32] = { "Continue", "Switching Protocols", "OK", "Bad Request", "Unauthorized", "Forbidden", "Not Found", "Request Timeout",  "Not Implemented", "Bad Gateway", "Internal Server Error" };
const uint32_t LWEHTTPMessage::StatusCodeMap[S_Count]  = { S_Continue, S_SwitchingProtocols, S_Ok, S_BadRequest, S_Unauthorized, S_Forbidden, S_NotFound, S_RequestTimeout, S_NotImplemented, S_BadGateway, S_InternalServerError};


uint32_t LWEHTTPMessage::MakeJSONQueryString(LWEJson &Json, char8_t *Buffer, uint32_t BufferLen) {
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

uint32_t LWEHTTPMessage::MakeHTTPDate(void *Buffer, uint32_t BufferLen) {
	time_t now = time(0);
	struct tm mtime;
#if _MSC_VER
	gmtime_s(&mtime, &now);
#else
	gmtime_r(&now, &mtime);
#endif
	return (uint32_t)strftime((char*)Buffer, BufferLen, "%a, %d %b %Y %H:%M:%S %Z", &mtime);
}

uint32_t LWEHTTPMessage::MakeAMZDate(void *Buffer, uint32_t BufferLen, bool IncludeSubTime) {
	time_t now = time(0);
	struct tm gtime;
#if _MSC_VER
	gmtime_s(&gtime, &now);
#else
	gmtime_r(&now, &gtime);
#endif
	if (IncludeSubTime) return (uint32_t)strftime((char*)Buffer, BufferLen, "%Y%m%dT%H%M%SZ", &gtime);
	return (uint32_t)strftime((char*)Buffer, BufferLen, "%Y%m%d", &gtime);
}

uint32_t LWEHTTPMessage::SerializeBody(void *Buffer, uint32_t BufferLen) {
	uint32_t Remaining = m_ContentLength - m_ChunkOffset;
	uint32_t o = 0;
	if (!Remaining) return o;
	uint32_t Length = std::min<uint32_t>(BufferLen, Remaining);
	if (GetEncodeType() == Encode_Chunked) o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "{:x}\r\n", Length);
	memcpy((char8_t *)Buffer + o, m_Body + m_ChunkOffset, Length);
	o += Length;
	m_ChunkOffset += Length;
	if (GetEncodeType() == Encode_Chunked) {
		if (m_ChunkOffset == Remaining) {
			o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "0\r\n\r\n"); //add final chunk, and then no trailing headers.
			if (!LWLogWarnIf(o <= BufferLen, "Error: Could not correctly add trailing chunk, this problem needs to be fixed.")) return std::min<uint32_t>(o, BufferLen);
		}
	}
	return o;
}

uint32_t LWEHTTPMessage::SerializeHeaders(void *Buffer, uint32_t BufferLen, const LWUTF8Iterator &UserAgent) {
	char8_t Methods[][32] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
	char8_t Caches[][32] = { "none", "no-cache", "no-store", "cache" };
	char8_t Connections[][32] = { "close", "Keep-Alive",  "upgrade"};
	char8_t Encodings[][32] = { "", "chunk" };
	char8_t ContentEncodings[][32] = { "identity", "gzip", "compress", "deflate", "br" };
	char8_t Upgrades[][32] = { "", "websocket" };
	
	uint32_t HeaderSize = std::min<uint32_t>(m_HeaderOffset + m_HeaderCount*4/*{}: {}\r\n*/ + 1024/*Include flagged headers*/, BufferLen); //We guess how big our header will be, slightly over is preferred to detect if we will be chunking this content.
	if (m_ContentLength > BufferLen-HeaderSize) m_Flag = LWBitFieldSet(EncodingBits, m_Flag, Encode_Chunked); //if we can't send entire chunk in 1 go, then we use chunked encoding format.

	uint32_t o = 0;
	uint32_t Connection = GetConnectionState();
	uint32_t Cache = GetCacheControl();
	uint32_t Encoding = GetEncodeType();
	uint32_t ContentEncode = GetContentEncoding();
	uint32_t UpgradeState = GetUpgradeState();
	uint32_t lStatusCode = GetStatus();
	bool bIsResponse = lStatusCode !=S_Request;
	
	//if (!m_ChunkLength) { //This is the first "Chunk", we send chunk encoding if m_ContentLength>BufferLen.
	if(bIsResponse) {
		const uint32_t *StatusMap = std::find(StatusCodeMap, StatusCodeMap+S_Count, lStatusCode);
		uint32_t Index = (uint32_t)std::distance(StatusCodeMap, StatusMap);
		const char8_t *lStatusName = Index==S_Count ? "" : StatusCodeNames[Index];
		o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "HTTP/1.1 {} {}\r\n", lStatusCode, lStatusName);
	}else o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "{} {} HTTP/1.1\r\n", Methods[GetMethod()], m_Path);
	
	//Write all header's:
	for (uint32_t i = 0; i < m_HeaderCount; i++) {
		LWEHTTPMessageHeader &H = m_HeaderTable[i];
		o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "{}: {}\r\n", m_Header + H.m_NameOffset, m_Header + H.m_ValueOffset);
	}
	o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "Content-Length: {}\r\n", m_ContentLength);
	if (!UserAgent.AtEnd()) {
		if (bIsResponse) o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "Server: {}\r\n", UserAgent);
		else o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "User-Agent: {}\r\n", UserAgent);
	} 
	if (m_WebSockVersion && !bIsResponse) o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "Sec-WebSocket-Version: {}\r\n", m_WebSockVersion);
	if (*Encodings[Encoding]) o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "Transfer-Encoding: {}\r\n", Encodings[Encoding]);
	if (*Upgrades[UpgradeState]) o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "Upgrade: {}\r\n", Upgrades[UpgradeState]);
	o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "Connection: {}\r\n", Connections[Connection]);
	if(Connection==LWEHTTPMessage::Connection_KeepAlive && (m_KeepAliveTimeout>0 || m_KeepAliveMessages>0)) {
		o += LWUTF8I::Fmt_ns((char8_t*)Buffer, BufferLen, o, "Keep-Alive: timeout={}, max={}\r\n", m_KeepAliveTimeout, m_KeepAliveMessages);
	}
	if(Cache!=CacheControl_None || m_CacheMaxAge>0) {
		o += LWUTF8I::Fmt_ns((char8_t*)Buffer, BufferLen, o, "Cache-Control: ");
		if(m_CacheMaxAge>0) o += LWUTF8I::Fmt_ns((char8_t*)Buffer, BufferLen, o, "max-age={}{} ", m_CacheMaxAge, (Cache!=CacheControl_None?",":""));
		if(Cache!=CacheControl_None) o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "{}", Caches[Cache]);
		o += LWUTF8I::Fmt_ns((char8_t*)Buffer, BufferLen, o, "\r\n");
	}
	if (ContentEncode != ContentEncode_Identity)  o += LWUTF8I::Fmt_ns((char8_t*)Buffer, BufferLen, o, "Content-Encoding: {}\r\n", ContentEncodings[ContentEncode]);
	o += LWUTF8I::Fmt_ns((char8_t *)Buffer, BufferLen, o, "\r\n");
	//if(!m_ContentLength) o += LWUTF8I::Fmt_ns((char8_t*)Buffer, BufferLen, o, "\r\n");
	LWLogWarnIf<256>(o<HeaderSize, "Predicted header size: '{}' is smaller than actual header size: '{}'", HeaderSize, o); 

	m_ChunkOffset = 0; //Reset chunk offset for serializing.
	return o;
}

uint32_t LWEHTTPMessage::GZipDecompress(const void *In, uint32_t InLen, void *Buffer, uint32_t BufferLen) {
	z_stream Stream;
	Stream.next_in = (Bytef*)In;
	Stream.avail_in = InLen;
	Stream.total_out = 0;
	Stream.total_in = 0;
	Stream.zalloc = Z_NULL;
	Stream.zfree = Z_NULL;
	if(!LWLogCriticalIf(inflateInit2(&Stream, 31)==Z_OK, "GZip error: 'inflateInit2'")) return 0;

	uint32_t o = 0;
	bool Finished = false;
	while (!Finished) {
		if (o >= BufferLen) {
			return o;
		}
		Stream.next_out = (Bytef*)Buffer + o;
		Stream.avail_out = BufferLen - o;
		int32_t r = inflate(&Stream, Z_SYNC_FLUSH);
		if(LWLogCriticalIf<256>(r==Z_OK||r==Z_STREAM_END, "GZip error: 'inflate'({})", Stream.msg ? Stream.msg : "")) o = (uint32_t)Stream.total_out;
		else Finished = true;
	}
	LWLogCriticalIf(inflateEnd(&Stream)==Z_OK, "GZipe error: 'inflateEnd'");
	return o;
}

uint32_t LWEHTTPMessage::DeserializeBody(const void *Buffer, uint32_t Len, bool Verbose) {
	uint32_t Encoding = GetEncodeType();
	uint32_t o = 0;
	while(o<Len && (m_ChunkOffset!=m_ContentLength || Encoding==Encode_Chunked)) {
		uint32_t BodyRemaining = sizeof(m_Body) - std::min<uint32_t>(sizeof(m_Body), m_ChunkOffset);
		if(!m_ChunkLength) {
			if(Encoding==Encode_Chunked) {
				uint32_t n = 0;
				std::sscanf((char8_t *)Buffer+o, "%x%n", &m_ChunkLength, &n);
				if(!LWLogWarnIf(n!=0, "Error HTTP message did not include chunked size.")) return 0;
				if(!LWLogWarnIf(LWUTF8I((char8_t *)Buffer+o+n).Compare("\r\n", 2), "Error HTTP message is malformed.")) return 0;
				o+=n+2;
				if(!m_ChunkLength) Len = 0;
			}else m_ChunkLength = m_ContentLength ? std::min<uint32_t>(Len, m_ContentLength-m_ChunkOffset) : Len;
		}
		uint32_t ReadLen = std::min<uint32_t>(m_ChunkLength, Len-o);
		uint32_t BodyLength = std::min<uint32_t>(ReadLen, BodyRemaining);
		std::copy((char8_t *)Buffer+o, (char8_t *)Buffer+o+BodyLength, m_Body+ m_ChunkOffset);
		m_ChunkOffset+=ReadLen;
		m_ChunkLength-=ReadLen;
		o += ReadLen;
	}
	if(!Len) { //Connection was closed or terminating chunk was detected:
		if(m_ChunkLength) { //we did not finish reading the entire chunk, an error has occurred:
			return -1;
		}else {
			if(!m_ContentLength) m_ContentLength = m_ChunkLength = m_ChunkOffset;
			//else if(m_ContentLength>m_ChunkOffset) return -1; //We did not receive the full message.
		}
	}
	return o;
}

uint32_t LWEHTTPMessage::DeserializeHeaders(const void *Buffer, uint32_t Len, bool bIsTrailingHeaders, bool Verbose) {
	char8_t Methods[][32] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
	char8_t NameBuffer[1024];
	char8_t ResultBuffer[1024];

	const uint32_t MethodCnt = sizeof(Methods)/sizeof(Methods[0]);

	uint32_t StatusCode = GetStatus();
	uint32_t HTTPMajorVersion = 0;
	uint32_t HTTPMinorVersion = 0;
	uint32_t o = 0;
	uint32_t n = 0;

	//try to read primary header(might have already been read, so we can safely skip it if necessary):
	std::sscanf((char8_t *)Buffer + o, "HTTP/%d.%d %d %1023[^\r]\n%n", &HTTPMajorVersion, &HTTPMinorVersion, &StatusCode, NameBuffer, &n);
	if (n == 0) {
		std::sscanf((char8_t *)Buffer + o, "%255s %1023s HTTP/%d.%d\r\n%n", NameBuffer, m_Path, &HTTPMajorVersion, &HTTPMinorVersion, &n);
		if(n!=0) {
			uint32_t MethodType = LWUTF8I(NameBuffer).CompareLista(MethodCnt, Methods);
			if(!LWLogWarnIfv<256>(MethodType<MethodCnt, Verbose, "HTTP Message has unknown method: '{}'", NameBuffer)) return 0;
			SetMethod(MethodType);
		}
	}else {
		const uint32_t *StatusMap = std::find(StatusCodeMap, StatusCodeMap+S_Count, StatusCode);
		uint32_t Index = (uint32_t)std::distance(StatusCodeMap, StatusMap);
		if(LWLogWarnIfv<256>(Index!=S_Count, Verbose, "HTTP Message has unknown(or unhandled) status code: '{}'", NameBuffer)){ //This is not a fail error, just a warning.
			if(!LWLogWarnIfv<256>(LWUTF8I(NameBuffer).Compare(StatusCodeNames[Index]), Verbose, "HTTP Message status code name does not match code sent, got '{}' for {}", NameBuffer, StatusCode)) return 0;
		}
		SetStatus(StatusCode);
	}
	o += n;
	while (((char8_t *)Buffer)[o] && o < Len) {
		if(LWUTF8I((char8_t *)Buffer+o).Compare("\r\n", 2)) {
			m_Flag |= HeadersRead;
			if(!bIsTrailingHeaders) {
				m_ChunkOffset = m_ChunkLength = 0;
				m_Body[0] = '\0';
			}
			return o+2;
		}
		n = 0;
		std::sscanf((char8_t *)Buffer + o, "%1023[^:]: %1023[^\r]%n", NameBuffer, ResultBuffer, &n);
		if (!LWLogWarnIfv(n>0, Verbose, "Message headers were cut off.")) return o;
		if (!LWLogWarnIfv(LWUTF8I((char8_t *)Buffer+o+n).Compare("\r\n", 2), Verbose, "Message headers were not valid.")) return 0;
		o += n+2; //+2 to skip \r\n.
		//Check that \r\n follow the header:

		LWUTF8Iterator Name = LWUTF8Iterator(NameBuffer);
		LWUTF8Iterator Value = LWUTF8Iterator(ResultBuffer);
		Name.Lower(NameBuffer, sizeof(NameBuffer)); //Lower Name.

		//Check if header is any that we convert to flags/values.
		if (Name.Compare("content-length")) m_ContentLength = atoi(Value.c_str());
		else if(Name.Compare("sec-websocket-version")) m_WebSockVersion = atoi(Value.c_str());
		else if (Name.Compare("connection")) {
			for (LWUTF8Iterator C = Value; !C.AtEnd(); C.AdvanceToken(',').Advance().AdvanceWord(true)) {
				if (C.Compare("close", 5)) SetConnectionState(Connection_Close);
				else if (C.Compare("keep-alive", 10)) SetConnectionState(Connection_KeepAlive);
				else if (C.Compare("upgrade", 7)) SetConnectionState(Connection_Upgrade);
			}
		} else {
			//Check if header is anything where value comparison is also case-insensitive.
			uint32_t hdr = Name.CompareList("cache-control", "transfer-encoding", "content-encoding", "upgrade");
			if (hdr != -1) {
				Value.Lower(ResultBuffer, sizeof(ResultBuffer)); //lower value to pre-pare for case insensitive comparisons.
				if (hdr == 0) { //cache-control:
					if (Value.HasSubString("no-store")) SetCacheControl(CacheControl_NoStore);
					else if (Value.HasSubString("no-cache")) SetCacheControl(CacheControl_NoCache);
					else if (Value.HasSubString("cache")) SetCacheControl(CacheControl_Cache);
					LWUTF8Iterator MaxAgeSubStr = Value.NextSubString("max-age=");
					if(!MaxAgeSubStr.AtEnd()) {
						LWUTF8Iterator AgeValue = Value.Advance(8);
						m_CacheMaxAge = (uint32_t)atoi(Value.c_str());
					}
				} else if (hdr == 1) { //transfer-encoding:
					if (Value.Compare("chunked")) SetEncoding(Encode_Chunked);
				} else if (hdr == 2) { //content-encoding:
					if (Value.Compare("identity")) SetContentEncoding(ContentEncode_Identity);
					else if (Value.Compare("gzip")) SetContentEncoding(ContentEncode_GZip);
					else if (Value.Compare("compress")) SetContentEncoding(ContentEncode_Compress);
					else if (Value.Compare("deflate")) SetContentEncoding(ContentEncode_Deflate);
					else if (Value.Compare("br")) SetContentEncoding(ContentEncode_BR);
				} else if (hdr == 3) { //upgrade:
					if (Value.Compare("websocket")) SetContentEncoding(Upgrade_WebSocket);
				}
			} else {
				LWLogWarnIfv<256>(PushHeader(Name, Value), Verbose, "HTTP header dropped: '{}' - '{}'", Name, Value);
			}
		}
	}
	return o;
}

LWEHTTPMessage &LWEHTTPMessage::SetURI(const LWUTF8Iterator &URI) {
	if(URI.AtEnd()) return *this;
	LWUTF8Iterator Proto, Domain, Path;
	LWSocket::SplitURI(URI, m_Port, Domain, Proto, Path);
	PushHeader("host", Domain);
	SetPath(Path);
	return *this;
}

bool LWEHTTPMessage::PushHeader(const LWUTF8Iterator &HeaderName, const LWUTF8Iterator &HeaderValue) {
	if (m_HeaderCount >= MaxHeaderItems) return false;
	uint32_t NameOffset = m_HeaderOffset;
	uint32_t Remaining = MaxHeaderLength - m_HeaderOffset;
	uint32_t NameLen = std::min<uint32_t>(HeaderName.Lower(m_Header + m_HeaderOffset, Remaining) + 1, Remaining); //Add 1 to skip over terminal.
	uint32_t ValueLen = std::min<uint32_t>(HeaderValue.Copy(m_Header + m_HeaderOffset + NameLen, Remaining - NameLen) + 1, Remaining - NameLen);
	m_HeaderOffset += NameLen + ValueLen;
	m_HeaderTable[m_HeaderCount++] = LWEHTTPMessageHeader(NameOffset, NameLen, NameOffset + NameLen, ValueLen, LWUTF8Iterator(m_Header + NameOffset, NameLen).Hash());
	return true;
}

bool LWEHTTPMessage::GenerateAWS4Auth(const LWUTF8Iterator &AccessKeyID, const LWUTF8Iterator &SecretKey, const LWUTF8Iterator &SessionToken) {
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

bool LWEHTTPMessage::GenerateAWS4Auth(const LWEAWSCredentials &Credentials) {
	return GenerateAWS4Auth(Credentials.m_AccessKey, Credentials.m_SecretKey, Credentials.m_SessionToken);
}

LWEHTTPMessage &LWEHTTPMessage::SetPath(const LWUTF8Iterator &Path) {
	if (Path.AtEnd()) strcpy(m_Path, "/");
	else Path.Copy(m_Path, sizeof(m_Path));
	return *this;
}

uint32_t LWEHTTPMessage::SetBody(const LWUTF8Iterator &Body) {
	if(Body.AtEnd()) {
		m_Body[0] = '\0';
		return SetContentLength(0);
	}
	return SetContentLength(Body.Copy(m_Body, sizeof(m_Body))-1);
}

uint32_t LWEHTTPMessage::SetBody(const void *Buffer, uint32_t BufferLen) {
	uint32_t Len = SetContentLength(BufferLen);
	std::copy((const char*)Buffer, (const char*)Buffer+Len, m_Body);
	return Len;
}

uint32_t LWEHTTPMessage::SetContentLength(uint32_t Length) {
	m_ChunkLength = Length;
	m_ContentLength = std::min<uint32_t>(Length, sizeof(m_Body));
	return m_ContentLength;
}

LWEHTTPMessage &LWEHTTPMessage::SetCallback(LWEHTTPResponseCallback Callback) {
	m_Callback = Callback;
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetMethod(uint32_t Method) {
	m_Flag = LWBitFieldSet(MethodBits, m_Flag, Method);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetConnectionState(uint32_t ConnState) {
	m_Flag = LWBitFieldSet(ConnectionBits, m_Flag, ConnState);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetCacheControl(uint32_t CacheControl) {
	m_Flag = LWBitFieldSet(CacheControlBits, m_Flag, CacheControl);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetEncoding(uint32_t Encoding) {
	m_Flag = LWBitFieldSet(EncodingBits, m_Flag, Encoding);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetContentEncoding(uint32_t ContentEncode) {
	m_Flag = LWBitFieldSet(ContentEncodeBits, m_Flag, ContentEncode);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetUpgradeState(uint32_t Upgrade) {
	m_Flag = LWBitFieldSet(UpgradeBits, m_Flag, Upgrade);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetStatus(uint32_t lStatus) {
	m_Flag = LWBitFieldSet(StatusBits, m_Flag, lStatus);
	return *this;
}

LWEHTTPMessage &LWEHTTPMessage::SetKeepAlive(uint32_t Seconds, uint32_t Messages) {
	m_KeepAliveTimeout = Seconds;
	m_KeepAliveMessages = Messages;
	if(m_KeepAliveTimeout>0) SetConnectionState(Connection_KeepAlive);
	return *this;
}

LWUTF8Iterator LWEHTTPMessage::operator [](const LWUTF8Iterator &HeaderName) const {
	return GetHeader(HeaderName);
}

LWUTF8Iterator LWEHTTPMessage::GetBodyAsString(void) const {
	return m_Body;
}

LWUTF8Iterator LWEHTTPMessage::GetPath(void) const {
	return m_Path;
}

LWUTF8Iterator LWEHTTPMessage::GetHeader(const LWUTF8Iterator &HeaderName) const {
	uint32_t Hash = HeaderName.Hash();
	for(uint32_t i=m_HeaderCount-1;i<m_HeaderCount;--i) { //search backwards as we 
		const LWEHTTPMessageHeader &H = m_HeaderTable[i];
		if (Hash == H.m_NameHash) return LWUTF8Iterator(m_Header + H.m_ValueOffset, H.m_ValueLength);
	}
	return LWUTF8Iterator();
}

uint32_t LWEHTTPMessage::GetCacheControl(void) const {
	return LWBitFieldGet(CacheControlBits, m_Flag);
}

uint32_t LWEHTTPMessage::GetConnectionState(void) const {
	return LWBitFieldGet(ConnectionBits, m_Flag);
}

uint32_t LWEHTTPMessage::GetContentEncoding(void) const {
	return LWBitFieldGet(ContentEncodeBits, m_Flag);
}

uint32_t LWEHTTPMessage::GetMethod(void) const {
	return LWBitFieldGet(MethodBits, m_Flag);
}

uint32_t LWEHTTPMessage::GetEncodeType(void) const {
	return LWBitFieldGet(EncodingBits, m_Flag);
}

uint32_t LWEHTTPMessage::GetUpgradeState(void) const {
	return LWBitFieldGet(UpgradeBits, m_Flag);
}

uint32_t LWEHTTPMessage::GetStatus(void) const {
	return LWBitFieldGet(StatusBits, m_Flag);
}

bool LWEHTTPMessage::CloseConnection(void) const {
	return GetConnectionState()==Connection_Close;
}

bool LWEHTTPMessage::KeepAliveConnection(void) const{
	return GetConnectionState()==Connection_KeepAlive;
}

bool LWEHTTPMessage::UpgradeConnection(void) const {
	return GetConnectionState()==Connection_Upgrade;
}

bool LWEHTTPMessage::isFinished(void) const {
	return m_ContentLength && m_ChunkOffset==m_ContentLength;
}

bool LWEHTTPMessage::isVariableSized(void) const {
	return m_ContentLength==0 && GetEncodeType()==Encode_Chunked;
}

bool LWEHTTPMessage::isHeadersRead(void) const {
	return (m_Flag & HeadersRead) != 0;
}

bool LWEHTTPMessage::isContentTooLarge(void) const {
	return m_ContentLength>sizeof(m_Body);
}

LWEHTTPMessage::LWEHTTPMessage(const LWEHTTPMessage &Request, const LWUTF8Iterator &Body, uint32_t StatusCode) : LWEHTTPMessage(StatusCode==S_Request ? "" : Request["host"], 0, (Request.m_Flag&ResponseFlags) | (StatusCode<<StatusBitsOffset), Request.m_UserData, Request.m_Callback) {
	m_Socket = Request.m_Socket;
	m_WebSockVersion = Request.m_WebSockVersion;
	m_Port = Request.m_Port;
	SetBody(Body);
}

LWEHTTPMessage::LWEHTTPMessage(const LWEHTTPMessage &Request, const void *Buffer, uint32_t BufferLen, uint32_t StatusCode) : LWEHTTPMessage(StatusCode == S_Request ? "" : Request["host"], 0, (Request.m_Flag&ResponseFlags) | (StatusCode << StatusBitsOffset), Request.m_UserData, Request.m_Callback) {
	m_Socket = Request.m_Socket;
	m_WebSockVersion = Request.m_WebSockVersion;
	m_Port = Request.m_Port;
	SetBody(Buffer, BufferLen);
}

LWEHTTPMessage::LWEHTTPMessage(const LWUTF8Iterator &Host, const LWUTF8Iterator &Body, uint32_t StatusCode, uint32_t Flags, void *UserData, LWEHTTPResponseCallback Callback) : LWEHTTPMessage(Host, 0, Flags | (StatusCode<<StatusBitsOffset), UserData, Callback) {
	SetBody(Body);
}

LWEHTTPMessage::LWEHTTPMessage(const LWUTF8Iterator &Host, const void *Buffer, uint32_t BufferLen, uint32_t StatusCode, uint32_t Flags, void *UserData, LWEHTTPResponseCallback Callback) : LWEHTTPMessage(Host, 0, Flags | (StatusCode<<StatusBitsOffset), UserData, Callback) {
	SetBody(Buffer, BufferLen);
}

LWEHTTPMessage::LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Method, uint32_t Flags, void *UserData, LWEHTTPResponseCallback Callback) : m_Flag((Method<<MethodBitsOffset) | Flags), m_UserData(UserData), m_Callback(Callback) {
	char8_t Date[LWEHTTPMessage::ValueMaxLength];
	SetURI(URI);
	if (m_Flag & GenerateDate) {
		MakeHTTPDate(Date, sizeof(Date));
		PushHeader("date", Date);
	}
}

LWEHTTPMessage::LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Method, uint32_t Flags, const LWUTF8Iterator &Body, void *UserData, LWEHTTPResponseCallback Callback) : LWEHTTPMessage(URI, Method, Flags, UserData, Callback) {
	SetBody(Body);
}

LWEHTTPMessage::LWEHTTPMessage(const LWUTF8Iterator &URI, uint32_t Method, uint32_t Flags, const void *Buffer, uint32_t BufferLen, void *UserData, LWEHTTPResponseCallback Callback) : LWEHTTPMessage(URI, Method, Flags, UserData, Callback) {
	SetBody(Buffer, BufferLen);
}

//LWEProtocolHttp:
LWProtocol &LWEProtocol_HTTP::Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	char Buffer[1024 * 64]; //64KB buffer to read from.
	uint32_t r = Socket->Receive(Buffer, sizeof(Buffer)-1);
	if (r == 0 || r==-1) {
		if(r==0) ProcessHTTPReadMessage(Socket, nullptr, 0, true);
		Socket->MarkClosable();
		return *this;
	}
	m_HTTPBytesRecv += r;
	Buffer[r] = '\0';

	uint32_t o = 0;
	while(o<r) {
		uint32_t res = ProcessHTTPReadMessage(Socket, Buffer+o, r-o, true);
		if(!LWLogCriticalIf<256>(res>0 && res!=-1, "Error occurred while parsing HTTP buffer from : '{}'.", Socket->GetRemoteAddr())) break;
		o += res;
	}
	return *this;
}

LWProtocol &LWEProtocol_HTTP::SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	LWRef<LWEHTTPMessage> Message = ExchangeSocketsData(Socket, LWRef<LWEHTTPMessage>(), m_MessageMap, m_MessageMapMutex);
	if(Message) {
		Message->m_Socket = LWRef<LWSocket>();
		Message->SetStatus(LWEHTTPMessage::S_RequestTimeout);
		if(Message->m_Callback) Message->m_Callback(Message, "");
	}
	return *this;
}

uint32_t LWEProtocol_HTTP::Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len) {
	uint32_t r = Socket->SendAll(Buffer, Len);
	m_HTTPBytesSent += r;
	if(!LWLogCriticalIf<256>(r==Len, "Socket '{}' Error: {}", Socket->GetLocalAddr(), LWProtocolManager::GetError())) return -1;
	return r;
}

uint32_t LWEProtocol_HTTP::ProcessHTTPReadMessage(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len, bool Verbose) {
	if(m_TimeoutPeriod!=-1) Socket->SetTimeoutPeriod(0, m_TimeoutPeriod);
	uint32_t o = 0;
	LWRef<LWEHTTPMessage> Message = GetSocketDataFor(Socket, m_MessageMap, m_MessageMapMutex);
	if(!Message){
		if(!Len) return 0; //Socket was closed when no message was on it.
		Message = SetSocketDataFor(Socket, m_Allocator.CreateRef<LWEHTTPMessage>(), m_MessageMap, m_MessageMapMutex);
	}
	Message->m_Socket = Socket;

	uint32_t Result = 0;
	if(!Message->isHeadersRead()) {
		Result = Message->DeserializeHeaders((char*)Buffer+o, Len-o, false, Verbose);
		if(!LWLogCriticalIfv<256>(Result!=0, Verbose, "HTTP Message nothing was read from buffer headers on connection {}.", Socket->GetRemoteAddr())) return 0;
		o += Result;
	}
	if(Message->isHeadersRead()) {
		Result = Message->DeserializeBody((char *)Buffer+o, Len-o, Verbose);
		if(!LWLogCriticalIfv<256>(Result!=-1, Verbose, "HTTP Message Deserializing error occurred on connection {}.", Socket->GetRemoteAddr())) return -1;
		o += Result;
		if(Message->isFinished() || !Len || (Len-o==0 && Message->GetStatus()==LWEHTTPMessage::S_Request)) { //Len==0 means connection had been closed, or if a request that no data remains.
			//If chunked, we check for any trailing headers:
			if(Message->GetEncodeType()==LWEHTTPMessage::Encode_Chunked && Len-o>0) {
				Result = Message->DeserializeHeaders((char *)Buffer+o, Len-o, true, Verbose);
				o += Result;
			}
			m_TotalInMessages++;
			uint32_t StatusCode = Message->GetStatus();
			if(StatusCode==LWEHTTPMessage::S_Request) {
				if(!LWLogCriticalIfv(m_InRequests.Push(Message), Verbose, "HTTP message pool has been exhausted, server likely overloaded.")) return o;
			}
			if(Message->m_Callback) Message->m_Callback(Message, Message->GetBodyAsString());
			if(Message->CloseConnection() && StatusCode!=LWEHTTPMessage::S_Request) Socket->MarkClosable();
			SetSocketDataFor(Socket, LWRef<LWEHTTPMessage>(), m_MessageMap, m_MessageMapMutex);
			return o;
		}else if(!LWLogCriticalIfv<256>(Result!=0 || (Len-o)==0, Verbose, "HTTP Message nothing was read from buffer body on connection {}, {}.", Socket->GetRemoteAddr(), Len-o)) return 0;
	}
	return o;
}

LWEProtocol_HTTP &LWEProtocol_HTTP::ProcessOutboundMessages(LWProtocolManager &Manager, bool Verbose, uint64_t lTimeout) {
	char Buffer[1024 * 64]; //64KB buffer. 
	uint64_t EndTime = lTimeout==0?-1:LWTimer::GetCurrent()+lTimeout;
	LWRef<LWEHTTPMessage> Message;
	while (LWTimer::GetCurrent() < EndTime && m_OutMessages.PopMove(Message)) {
		bool IsResponse = Message->GetStatus() != LWEHTTPMessage::S_Request;
		LWRef<LWSocket> Socket = Message->m_Socket;
		if (!Socket && !IsResponse) {
			LWUTF8Iterator Host = (*Message)["host"];
			Socket = Message->m_Socket = Manager.CreateAsyncSocket(Host, Message->m_Port, LWSocket::TCP, 0, ProtocolID);
			if (!LWLogCriticalIfv((bool)Socket, Verbose, "Protocolmanager exceeded supported sockets.")) continue;
		}
		uint32_t lStatus = Socket->GetStatus();
		if(!IsResponse) {
			if(lStatus==LWSocket::S_Connecting) { //add message back into queue, we are still waiting on the initial handshake to finish:
				if(!LWLogCriticalIfv<256>(m_OutMessages.PushMove(Message), Verbose, "Failed to reinsert message into outbound messages.")) Socket->MarkClosable();
				return *this; //We don't want to end up in a loop on the same message, so we'll let a frame go by before trying again.
			}else if(lStatus==LWSocket::S_Error || lStatus==LWSocket::S_Closed) {
				LWLogEventv<256>(Verbose, "Failed to connect to domain '{}'.", (*Message)["host"]);
				//Domain closed the connection before a response was received, so we send back DomainNoResponse.
				Message->SetStatus(LWEHTTPMessage::S_DomainNoResponse);
				Message->SetBody(nullptr, 0);
				if(Message->m_Callback) Message->m_Callback(Message, "");
			}
		}else if (!LWLogWarnIfv((bool)Socket && Socket->IsValid(), Verbose, "Socket was closed before response was ready, discarding message.")) continue;
		if(m_TimeoutPeriod!=-1) Socket->SetTimeoutPeriod(0, m_TimeoutPeriod);

		uint32_t o = Message->SerializeHeaders(Buffer, sizeof(Buffer), IsResponse ? m_Server : m_Agent);
		if(!LWLogWarnIfv(o<sizeof(Buffer), Verbose, "Serializing headers was larger than buffer, dropping message.")) {
			Socket->MarkClosable();
			continue;
		}
		do {
			o += Message->SerializeBody(Buffer+o, sizeof(Buffer)-o);
			uint32_t Res = Send(Socket, Buffer, o);
			if(!LWLogCriticalIfv<256>(Res!=-1, Verbose, "Error occurred while sending to socket: '{}'", Socket->GetRemoteAddr())) continue;
			else if(Res==0) { //We couldn't send(likely a TLS connection, so we will re-insert and try again later).
				if(!LWLogCriticalIfv<256>(m_OutMessages.PushMove(Message), Verbose, "Failed to reinsert message into outbound messages.")) {
					Socket->MarkClosable();
				}else return *this; //Connection is not ready, so we put it on the back of the list and wait until it's ready.
			}
			o = 0;
		}while(Message->m_ChunkOffset< Message->m_ChunkLength);
		m_TotalOutMessages++;
		if(Message->GetStatus()==LWEHTTPMessage::S_Request) {
			SetSocketDataFor(Socket, Message, m_MessageMap, m_MessageMapMutex);
		}else if(Message->CloseConnection()){
			Socket->MarkClosable();
		}
	}
	return *this;
}

bool LWEProtocol_HTTP::GetNextRequest(LWRef<LWEHTTPMessage> &Requests) {
	return m_InRequests.PopMove(Requests);
}

bool LWEProtocol_HTTP::PushOutMessage(const LWEHTTPMessage &Message) {
	LWRef<LWEHTTPMessage> RMessage = m_Allocator.CreateRef<LWEHTTPMessage>(Message);
	return m_OutMessages.PushMove(RMessage);
}

LWEProtocol_HTTP &LWEProtocol_HTTP::SetAgentString(const LWUTF8Iterator &Agent) {
	Agent.Copy(m_Agent, sizeof(m_Agent));
	return *this;
}

LWEProtocol_HTTP &LWEProtocol_HTTP::SetServerString(const LWUTF8Iterator &Server) {
	Server.Copy(m_Server, sizeof(m_Server));
	return *this;
}

LWEProtocol_HTTP &LWEProtocol_HTTP::SetTimeoutPeriod(uint64_t lTimeoutPeriod) {
	m_TimeoutPeriod = lTimeoutPeriod;
	return *this;
}

uint64_t LWEProtocol_HTTP::GetTotalInboundMessages(void) const {
	return m_TotalInMessages;
}

uint64_t LWEProtocol_HTTP::GetTotalOutboundMessages(void) const {
	return m_TotalOutMessages;
}

uint32_t LWEProtocol_HTTP::GetHTTPRecvBytes(void) const {
	return m_HTTPBytesRecv;
}

uint32_t LWEProtocol_HTTP::GetHTTPSentBytes(void) const {
	return m_HTTPBytesSent;
}

LWEProtocol_HTTP::LWEProtocol_HTTP(uint32_t ProtocolID, LWAllocator &Allocator) : LWProtocol(ProtocolID), m_Allocator(Allocator) {}
