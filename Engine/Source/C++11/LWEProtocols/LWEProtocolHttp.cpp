#include "LWEProtocols/LWEProtocolHTTP.h"
#include <LWNetwork/LWProtocolManager.h>
#include <LWNetwork/LWSocket.h>
#include <LWEJson.h>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <iostream>
#include <zlib.h>
#include <functional>

uint32_t LWEHttpRequest::MakeJSONQueryString(LWEJson &Json, char *Buffer, uint32_t BufferLen) {
	const uint32_t TBufferLen = 64 * 1024;
	char TempBuffer[TBufferLen];
	uint32_t Len = Json.GetLength();
	uint32_t o = 0;

	bool First = true;
	for (uint32_t i = 0; i < Len; i++) {
		LWEJObject *JO = Json.GetElement(i, nullptr);
		if (JO->m_Type == LWEJObject::Array || JO->m_Type == LWEJObject::Object) continue;

		if (First) o += snprintf(TempBuffer + o, TBufferLen - o, "%s=%s", JO->m_Name, JO->m_Value);
		else o += snprintf(TempBuffer + o, TBufferLen - o, "&%s=%s", JO->m_Name, JO->m_Value);
		First = false;

	}
	return LWEJson::EscapeString(TempBuffer, Buffer, BufferLen);
}

uint32_t LWEHttpRequest::Serialize(char8_t *Buffer, uint32_t BufferLen, const LWUTF8Iterator &UserAgent) {
	//std::cout << "Serializing!" << std::endl;
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
		o += snprintf((char*)Buffer + o, BufferLen - o, "HTTP/1.1 %d %s\r\n", m_Status, lStatus);
	}
	else o += snprintf((char*)Buffer + o, BufferLen - o, "%s %s HTTP/1.1\r\n", Methods[m_Flag&METHODBITS], m_Path);
	if (*m_Host && !IsResponse) o += snprintf((char*)Buffer + o, BufferLen - o, "Host: %s\r\n", m_Host);
	if (*m_Origin && !IsResponse) o += snprintf((char*)Buffer + o, BufferLen - o, "Origin: %s\r\n", m_Origin);
	if (*m_ContentType) {
		if (IsResponse) o += snprintf((char*)Buffer + o, BufferLen - o, "Content-Type: %s\r\n", m_ContentType);
		else o += snprintf((char*)Buffer + o, BufferLen - o, "Accept: %s\r\n", m_ContentType);
	}
	if (*m_Authorization) o += snprintf((char*)Buffer + o, BufferLen - o, "Authorization: %s\r\n", m_Authorization);
	if (m_ContentLength) o += snprintf((char*)Buffer + o, BufferLen - o, "Content-Length: %d\r\n", m_ContentLength);
	if (!UserAgent.AtEnd()) {
		if (IsResponse) o += snprintf((char*)Buffer + o, BufferLen - o, "Server: %s\r\n", (const char*)UserAgent());
		else o += snprintf((char*)Buffer + o, BufferLen - o, "User-Agent: %s\r\n", (const char*)UserAgent());
	}
	if (*m_SecWebSockKey) {
		if (IsResponse) o += snprintf((char*)Buffer + o, BufferLen - o, "Sec-WebSocket-Accept: %s\r\n", m_SecWebSockKey);
		else o += snprintf((char*)Buffer + o, BufferLen - o, "Sec-WebSocket-Key: %s\r\n", m_SecWebSockKey);
		o += snprintf((char*)Buffer + o, BufferLen - o, "Sec-WebSocket-Extensions: \r\n");
	}
	if (*m_SecWebSockProto) o += snprintf((char*)Buffer + o, BufferLen - o, "Sec-WebSocket-Protocol: %s\r\n", m_SecWebSockProto);
	if (m_WebSockVersion && !IsResponse) o += snprintf((char*)Buffer + o, BufferLen - o, "Sec-WebSocket-Version: %d\r\n", m_WebSockVersion);
	if (*Encodings[EncodeBits]) o += snprintf((char*)Buffer + o, BufferLen - o, "Transfer-Encoding: %s\r\n", Encodings[EncodeBits]);
	if (*Upgrades[UpgradeBits]) o += snprintf((char*)Buffer + o, BufferLen - o, "Upgrade: %s\r\n", Upgrades[UpgradeBits]);
	o += snprintf((char*)Buffer + o, BufferLen - o, "Connection: %s\r\n", Connections[ConnectionBits]);
	o += snprintf((char*)Buffer + o, BufferLen - o, "Cache-Control: %s\r\n", Caches[CacheBits]);
	if (ContentEncodeBits != ContentEncodeIdentity) {
		o += snprintf((char*)Buffer + o, BufferLen - o, "Content-Encoding: %s\r\n", ContentEncodings[ContentEncodeBits]);
	}

	o += snprintf((char*)Buffer + o, BufferLen - o, "\r\n");
	if (*m_Body) {
		if (m_Flag&EncodeChunked) {
			if (!m_ContentLength) m_ContentLength = (uint32_t)strlen(m_Body);
			o += snprintf((char*)Buffer + o, BufferLen - o, "%x\r\n%s\r\n", m_ContentLength, m_Body);
		}
		else {
			o += snprintf((char*)Buffer + o, BufferLen - o, "%s", m_Body);
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
		fmt::print("Failed to start inflate.\n");
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
			if (Stream.msg) fmt::print("GZip Error: {}\n", Stream.msg);
			Finished = true;
		}
	}
	if (inflateEnd(&Stream) != Z_OK) {
		fmt::print("GZip error: 'inflateEnd'\n");
	}
	return o;
}

bool LWEHttpRequest::Deserialize(const char8_t *Buffer, uint32_t Len) {
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

	sscanf(Buffer + o, "HTTP/1.1 %d %127[^\r]\n%n", &m_Status, m_Path, &k);
	if (k == 0) {
		sscanf(Buffer + o, "%255s %127s HTTP/1.1\r\n%n", NameBuffer, m_Path, &k);
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
			fmt::print("Unknown method: '{}'\n", NameBuffer);
			return false;
		}
	}
	*m_Host = *m_ContentType = *m_Authorization = *m_Path = *m_TransferEncoding = *m_Origin = *m_SecWebSockKey = *m_SecWebSockProto = '\0';
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
		if (!_stricmp(NameBuffer, "Host")) strlcpy(m_Host, ResultBuffer, sizeof(m_Host));
		else if (!_stricmp(NameBuffer, "Content-Type")) strlcpy(m_ContentType, ResultBuffer, sizeof(m_ContentType));
		else if (!_stricmp(NameBuffer, "Accept")) strlcpy(m_ContentType, ResultBuffer, sizeof(m_ContentType));
		else if (!_stricmp(NameBuffer, "Authorization")) strlcpy(m_Authorization, ResultBuffer, sizeof(m_Authorization));
		else if (!_stricmp(NameBuffer, "Content-Length")) m_ContentLength = atoi(ResultBuffer);
		else if (!_stricmp(NameBuffer, "Accept-Encoding")) {}
		else if (!_stricmp(NameBuffer, "Origin")) strlcpy(m_Origin, ResultBuffer, sizeof(m_Origin));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Key")) strlcpy(m_SecWebSockKey, ResultBuffer, sizeof(m_SecWebSockKey));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Accept")) strlcpy(m_SecWebSockKey, ResultBuffer, sizeof(m_SecWebSockKey));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Protocol")) strlcpy(m_SecWebSockProto, ResultBuffer, sizeof(m_SecWebSockProto));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Version")) m_WebSockVersion = atoi(ResultBuffer);
		else if (!_stricmp(NameBuffer, "Cache-Control")) {
			if (!_stricmp(ResultBuffer, "no-cache")) m_Flag |= NoCacheControl;
		}else if (!_stricmp(NameBuffer, "Connection")) {
			for (LWUTF8Iterator C = ResultBuffer; !C.AtEnd(); C.AdvanceToken(',').Advance().AdvanceWord(true)) {
				if (C.Compare("close", 5)) m_Flag &= ~CONNECTIONBITS;
				else if (C.Compare("keep-alive", 10)) m_Flag |= ConnectionKeepAlive;
				else if (C.Compare("upgrade", 7)) m_Flag |= ConnectionUpgrade;
			}
		}else if (!_stricmp(NameBuffer, "Transfer-Encoding")) {
			if (!_stricmp(ResultBuffer, "chunked")) m_Flag |= EncodeChunked;
		} else if (!_stricmp(NameBuffer, "Content-Encoding")) {
			if (!_stricmp(ResultBuffer, "identity")) m_Flag |= ContentEncodeIdentity;
			else if (!_stricmp(ResultBuffer, "gzip")) m_Flag |= ContentEncodeGZip;
			else if (!_stricmp(ResultBuffer, "compress")) m_Flag |= ContentEncodeCompress;
			else if (!_stricmp(ResultBuffer, "deflate")) m_Flag |= ContentEncodeDeflate;
			else if (!_stricmp(ResultBuffer, "br")) m_Flag |= ContentEncodeBR;
		}else if(!_stricmp(NameBuffer, "Upgrade")){
			if (!_stricmp(ResultBuffer, "websocket")) m_Flag |= UpgradeWebSock;
		}else {
			fmt::print("Header: '{}' - '{}'\n", NameBuffer, ResultBuffer);
		}

	}
	return true;
}

LWEHttpRequest &LWEHttpRequest::SetURI(const LWUTF8Iterator &URI) {
	LWUTF8Iterator Proto, Domain, Path;
	LWSocket::SplitURI(URI, m_Port, Domain, Proto, Path);
	Domain.Copy(m_Host, sizeof(m_Host));
	Path.Copy(m_Path, sizeof(m_Path));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetHost(const LWUTF8Iterator &Host) {
	Host.Copy(m_Host, sizeof(m_Host));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetPath(const LWUTF8Iterator &Path) {
	Path.Copy(m_Path, sizeof(m_Path));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetOrigin(const LWUTF8Iterator &Origin) {
	Origin.Copy(m_Origin, sizeof(m_Origin));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetWebSockKey(const LWUTF8Iterator &Key) {
	Key.Copy(m_SecWebSockKey, sizeof(m_SecWebSockKey));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetWebSockProto(const LWUTF8Iterator &Protocols) {
	Protocols.Copy(m_SecWebSockProto, sizeof(m_SecWebSockProto));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetAuthorization(const LWUTF8Iterator &Auth) {
	Auth.Copy(m_Authorization, sizeof(m_Authorization));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetContentType(const LWUTF8Iterator &ContentType) {
	ContentType.Copy(m_ContentType, sizeof(m_ContentType));
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

LWUTF8Iterator LWEHttpRequest::GetBody(void) const {
	return m_Body;
}

LWUTF8Iterator LWEHttpRequest::GetHost(void) const {
	return m_Host;
}

LWUTF8Iterator LWEHttpRequest::GetPath(void) const {
	return m_Path;
}

LWUTF8Iterator LWEHttpRequest::GetOrigin(void) const {
	return m_Origin;
}

LWUTF8Iterator LWEHttpRequest::GetAuthorization(void) const {
	return m_Authorization;
}

LWUTF8Iterator LWEHttpRequest::GetContentType(void) const {
	return m_ContentType;
}

LWUTF8Iterator LWEHttpRequest::GetTransferEncoding(void) const {
	return m_TransferEncoding;
}

LWUTF8Iterator LWEHttpRequest::GetSecWebSockKey(void) const {
	return m_SecWebSockKey;
}

LWUTF8Iterator LWEHttpRequest::GetSecWebSockProto(void) const {
	return m_SecWebSockProto;
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

LWEHttpRequest::LWEHttpRequest(const LWUTF8Iterator &URI, uint32_t Flag) : m_Flag(Flag) {
	SetURI(URI);
}


LWProtocol &LWEProtocolHttp::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	char Buffer[1024 * 64];
	int32_t r = Socket.Receive(Buffer, sizeof(Buffer)-1);
	if (r <= 0) {
		Socket.MarkClosable();
		return *this;
	}
	Buffer[r] = '\0';
	if (!ProcessRead(Socket, Buffer, r)) {
		fmt::print("Error parsing HTTP buffer.\n");
	}
	return *this;
}

LWProtocol &LWEProtocolHttp::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEHttpRequest *Req = (LWEHttpRequest*)Prev.GetProtocolData(m_ProtocolID);
	if (Req) Req->m_Socket = &New;
	return *this;
}

uint32_t LWEProtocolHttp::Send(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	for (uint32_t o = 0; o < Len;) {
		int32_t r = Socket.Send(Buffer + o, Len - o);
		if (r == -1) {
			fmt::print("Socket {} Error: {}\n", Socket.GetSocketDescriptor(), LWProtocolManager::GetError());
			return 1;
		}
		o += (uint32_t)r;
	}
	return 0;
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
		fmt::print("Error deserializing response.\n");
		return false;
	}
	if (Insert) {
		if (!GetNextFromPool(&Req)) {
			fmt::print("Could not get a request to write to from pool.\n");
			return false;
		}
		*Req = IReq;
		Socket.SetProtocolData(m_ProtocolID, Req);
	}
	if(Req->isResponseReady()) {
		if (Req->m_Status == 0) {
			if (!m_InRequests.Push(*Req, &Req)) {
				fmt::print("Failed to insert response into request queue.\n");
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
			uint32_t Error = LWSocket::CreateSocket(Sock, Request->m_Host, Request->m_Port, LWSocket::Tcp, ProtocolID);
			if (Error) {
				fmt::print("'{}:{}' Error: {}\n", Request->GetHost(), Request->m_Port, Error);
				continue;
			}
			Request->m_Socket = Manager.PushSocket(Sock);
			//std::cout << "Making socket: " << Request->m_Socket->GetSocketDescriptor() << std::endl;
			Request->m_Socket->SetProtocolData(m_ProtocolID, Request);
			if (!Request->m_Socket) {
				fmt::print("Error inserting socket.\n");
				continue;
			}
		}

		uint32_t Res = Send(*Request->m_Socket, Buffer, Len);
		if (Res == -1) {
			fmt::print("Error sending request.\n");
			continue;
		}
		if (Request->CloseConnection()) Request->m_Socket->MarkClosable();
	}
	return *this;
}

LWEProtocolHttp &LWEProtocolHttp::SetProtocolID(uint32_t ProtocolID) {
	m_ProtocolID = ProtocolID;
	return *this;
}

LWEProtocolHttp &LWEProtocolHttp::SetProtocolManager(LWProtocolManager *Manager) {
	m_Manager = Manager;
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

LWEProtocolHttp::LWEProtocolHttp(uint32_t ProtocolID, LWProtocolManager *Manager) : LWProtocol(), m_ProtocolID(ProtocolID), m_Manager(m_Manager) {}
