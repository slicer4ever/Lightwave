#include "LWEProtocols/LWEProtocolHTTP.h"
#include <LWNetwork/LWProtocolManager.h>
#include <LWNetwork/LWSocket.h>
#include <LWCore/LWText.h>
#include <LWEJson.h>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <iostream>
#include <zlib.h>
#include <functional>

uint32_t LWEHttpRequest::Escape(const char *In, char *Buffer, uint32_t BufferLen) {
	char ValidChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~";
	uint32_t o = 0;
	char *B = Buffer;
	char *L = B + BufferLen;
	for (const char *C = In; *C; C++) {
		uint32_t i = 0;
		for (; ValidChars[i] && *C != ValidChars[i]; i++) {}
		if (ValidChars[i]) {
			if (B != L) *B++ = *C;
			o++;
		}
		else {
			uint32_t Val = ((uint32_t)*C) & 0xFF;
			uint32_t k = snprintf(B, (uint32_t)(uintptr_t)(L - B), "%%%X", Val);
			B += k;
			if (B > L) B = L;
			o += k;
		}
	}
	if (B != L) *B = '\0';
	o++;
	return o;
}

uint32_t LWEHttpRequest::EscapeURI(const char *In, char *Buffer, uint32_t BufferLen) {
	char ValidChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_.!~*'();,/?&=+$#";
	uint32_t o = 0;
	char *B = Buffer;
	char *L = B + BufferLen;
	for (const char *C = In; *C; C++) {
		uint32_t i = 0;
		for (; ValidChars[i] && *C != ValidChars[i]; i++) {}
		if (ValidChars[i]) {
			if (B != L) *B++ = *C;
			o++;
		}
		else {
			uint32_t Val = ((uint32_t)*C) & 0xFF;
			uint32_t k = snprintf(B, (uint32_t)(uintptr_t)(L - B), "%%%X", Val);
			B += k;
			if (B > L) B = L;
			o += k;
		}
	}
	if (B != L) *B = '\0';
	o++;
	return o;
}

uint32_t LWEHttpRequest::UnEscape(const char *In, char *Buffer, uint32_t BufferLen) {
	uint32_t o = 0;
	char *B = Buffer;
	char *L = B + BufferLen;
	for (const char *C = In; *C; C++) {
		if (*C == '%') {
			uint32_t Val = 0;
			if (sscanf(C + 1, "%2X", &Val) != 1) return 0;
			if (B != L) *B++ = (char)Val;
			o++;
			C += 2;
		}
		else {
			if (B != L) *B++ = *C;
			o++;
		}
	}
	if (B != L) *B = '\0';
	o++;
	return o;
}

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
	return EscapeURI(TempBuffer, Buffer, BufferLen);
}

uint32_t LWEHttpRequest::Serialize(char *Buffer, uint32_t BufferLen, const char *UserAgent) {
	//std::cout << "Serializing!" << std::endl;
	char Methods[][32] = { "GET", "POST" };
	char Caches[][32] = { "no-cache" };
	char Connections[][32] = { "close", "keep-alive",  "upgrade", "Keep-alive, upgrade" };
	char Encodings[][32] = { "", "chunk" };
	char ContentEncodings[][32] = { "identity", "gzip", "compress", "deflate", "br" };
	char Upgrades[][32] = { "", "websocket" };
	uint32_t o = 0;
	uint32_t ConnectionBits = (m_Flag&CONNECTIONBITS) >> CONNECTIONOFFSET;
	uint32_t CacheBits = (m_Flag&CACHEBITS) >> CACHEOFFSET;
	uint32_t EncodeBits = (m_Flag&ENCODEBITS) >> ENCODEOFFSET;
	uint32_t ContentEncodeBits = (m_Flag&CONTENTENCODEBITS) >> CONTENTENCODEOFFSET;
	uint32_t UpgradeBits = (m_Flag&UPGRADEBITS) >> UPGRADEOFFSET;
	bool IsResponse = m_Status != 0;
	if (IsResponse) {
		char *lStatus = "";
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
		o += snprintf(Buffer + o, BufferLen - o, "HTTP/1.1 %d %s\r\n", m_Status, lStatus);
	}
	else o += snprintf(Buffer + o, BufferLen - o, "%s %s HTTP/1.1\r\n", Methods[m_Flag&METHODBITS], m_Path);
	if (*m_Host && !IsResponse) o += snprintf(Buffer + o, BufferLen - o, "Host: %s\r\n", m_Host);
	if (*m_Origin && !IsResponse) o += snprintf(Buffer + o, BufferLen - o, "Origin: %s\r\n", m_Origin);
	if (*m_ContentType) {
		if (IsResponse) o += snprintf(Buffer + o, BufferLen - o, "Content-Type: %s\r\n", m_ContentType);
		else o += snprintf(Buffer + o, BufferLen - o, "Accept: %s\r\n", m_ContentType);
	}
	if (*m_Authorization) o += snprintf(Buffer + o, BufferLen - o, "Authorization: %s\r\n", m_Authorization);
	if (m_ContentLength) o += snprintf(Buffer + o, BufferLen - o, "Content-Length: %d\r\n", m_ContentLength);
	if (UserAgent && *UserAgent) {
		if (IsResponse) o += snprintf(Buffer + o, BufferLen - o, "Server: %s\r\n", UserAgent);
		else o += snprintf(Buffer + o, BufferLen - o, "User-Agent: %s\r\n", UserAgent);
	}
	if (*m_SecWebSockKey) {
		if (IsResponse) o += snprintf(Buffer + o, BufferLen - o, "Sec-WebSocket-Accept: %s\r\n", m_SecWebSockKey);
		else o += snprintf(Buffer + o, BufferLen - o, "Sec-WebSocket-Key: %s\r\n", m_SecWebSockKey);
		o += snprintf(Buffer + o, BufferLen - o, "Sec-WebSocket-Extensions: \r\n");
	}
	if (*m_SecWebSockProto) o += snprintf(Buffer + o, BufferLen - o, "Sec-WebSocket-Protocol: %s\r\n", m_SecWebSockProto);
	if (m_WebSockVersion && !IsResponse) o += snprintf(Buffer + o, BufferLen - o, "Sec-WebSocket-Version: %d\r\n", m_WebSockVersion);
	if (*Encodings[EncodeBits]) o += snprintf(Buffer + o, BufferLen - o, "Transfer-Encoding: %s\r\n", Encodings[EncodeBits]);
	if (*Upgrades[UpgradeBits]) o += snprintf(Buffer + o, BufferLen - o, "Upgrade: %s\r\n", Upgrades[UpgradeBits]);
	o += snprintf(Buffer + o, BufferLen - o, "Connection: %s\r\n", Connections[ConnectionBits]);
	o += snprintf(Buffer + o, BufferLen - o, "Cache-Control: %s\r\n", Caches[CacheBits]);
	if (ContentEncodeBits != ContentEncodeIdentity) {
		o += snprintf(Buffer + o, BufferLen - o, "Content-Encoding: %s\r\n", ContentEncodings[ContentEncodeBits]);
	}

	o += snprintf(Buffer + o, BufferLen - o, "\r\n");
	if (*m_Body) {
		if (m_Flag&EncodeChunked) {
			if (!m_ContentLength) m_ContentLength = (uint32_t)strlen(m_Body);
			o += snprintf(Buffer + o, BufferLen - o, "%x\r\n%s\r\n", m_ContentLength, m_Body);
		}
		else {
			o += snprintf(Buffer + o, BufferLen - o, "%s", m_Body);
		}
	}
	return o;
}

uint32_t LWEHttpRequest::GZipDecompress(const char *In, uint32_t InLen, char *Buffer, uint32_t BufferLen) {
	z_stream Stream;
	Stream.next_in = (Bytef*)In;
	Stream.avail_in = InLen;
	Stream.total_out = 0;
	Stream.total_in = 0;
	Stream.zalloc = Z_NULL;
	Stream.zfree = Z_NULL;
	if (inflateInit2(&Stream, 31) != Z_OK) {
		std::cout << "Failed to start inflate." << std::endl;
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
			if (Stream.msg) std::cout << "error: " << Stream.msg << std::endl;
			Finished = true;
		}
	}
	if (inflateEnd(&Stream) != Z_OK) {
		std::cout << "Failed finish inflating." << std::endl;
	}
	return o;
}

uint16_t LWEHttpRequest::ParseURI(const char *URI, char *HostBuffer, uint32_t HostBufferSize, uint32_t *HostBufferLen, char *PathBuffer, uint32_t PathBufferSize, uint32_t *PathBufferLen, char *ProtocolBuffer, uint32_t ProtocolBufferSize, uint32_t *ProtocolBufferLen) {
	const char *F = URI;
	char Port[32];
	char *Pt = Port;
	char *PtL = Port + sizeof(Port);
	char *H = HostBuffer;
	char *HL = HostBuffer + HostBufferSize;
	char *Pa = PathBuffer;
	char *PaL = PathBuffer + PathBufferSize;
	char *Pr = ProtocolBuffer;
	char *PrL = ProtocolBuffer + ProtocolBufferSize;
	bool WriteProtocol = true;
	bool WritePath = false;
	bool WritePort = false;
	uint32_t Hs = 0;
	uint32_t Pas = 0;
	uint32_t Prs = 0;
	Port[0] = '\0';
	//skip leading //'s
	for (; *F; F++) if (*F != '/') break;
	for (; *F; F++) {
		if (WritePath) {
			Pas++;
			if (Pa != PaL) *Pa++ = *F;
			continue;
		}
		if (*F == ':') {
			if (WriteProtocol) {
				if (*(F + 1) == '/') {
					WriteProtocol = false;
					Hs = 0;
					H = HostBuffer;
					if (H != HL) *H = '\0';
					//skip any //'s
					for (F++; *F; F++) if (*F != '/') break;
					F -= 1;
				} else {
					Pr = ProtocolBuffer;
					if (Pr != PrL) *Pr = '\0';
					Prs = 0;
					WritePort = true;
				}
			} else WritePort = true;
			continue;
		}
		if (*F == '/') {
			if (WriteProtocol) {
				Pr = ProtocolBuffer;
				if (Pr != PrL) *Pr = '\0';
				Prs = 0;
			}
			if (H != HL) *H = '\0';
			if (Pt == PtL) Pt--;
			*Pt = '\0';
			WritePath = true;
			F--;
			continue;
		}
		if (WritePort) {
			if (Pt != PtL) *Pt++ = *F;
		}
		if (WriteProtocol) {
			Prs++;
			if (Pr != PrL) *Pr++ = *F;
		}
		if (!WritePort) {
			Hs++;
			if (H != HL) *H++ = *F;
		}
	}
	if (H) {
		if (H == HL) *(H - 1) = '\0';
		else *H = '\0';
	}
	if (Pa) {
		if (Pa == PaL) *(Pa - 1) = '\0';
		else *Pa = '\0';
	}
	if (Pr) {
		if (Pr == PrL) *(PrL - 1) = '\0';
		else *Pr = '\0';
	}
	if (HostBufferLen) *HostBufferLen = Hs;
	if (PathBufferLen) *PathBufferLen = Pas;
	if (ProtocolBufferLen) *ProtocolBufferLen = Prs;
	uint16_t PortNbr = 0;
	if (*Port) PortNbr = (uint16_t)atoi(Port);
	else {
		if (ProtocolBuffer) {
			if (!_stricmp(ProtocolBuffer, "https")) PortNbr = 443;
			else if (!_stricmp(ProtocolBuffer, "http")) PortNbr = 80;
			else if (!_stricmp(ProtocolBuffer, "wss")) PortNbr = 443;
			else if (!_stricmp(ProtocolBuffer, "ws")) PortNbr = 80;
		}
	}
	return PortNbr;
}

bool LWEHttpRequest::Deserialize(const char *Buffer, uint32_t Len) {
	char Methods[][32] = { "GET", "POST" };
	char NameBuffer[1024];
	char ResultBuffer[1024];
	const uint32_t MethodCnt = 2;

	uint32_t o = 0;
	uint32_t k = 0;
	uint32_t i = 0;
	bool IsResponse = true;
	if (m_Flag&HeadersRead) {
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
			}
			else {
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
		}
		else {
			uint32_t Remain = std::min<uint32_t>((sizeof(m_Body)) - m_ChunkLength, Len);
			uint32_t ContentEncodeBits = (m_Flag&CONTENTENCODEBITS);
			memcpy(m_Body + m_ChunkLength, Buffer, Remain);
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
			std::cout << "Unknown method: '" << NameBuffer << "'" << std::endl;
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
		if (!_stricmp(NameBuffer, "Host")) strncat(m_Host, ResultBuffer, sizeof(m_Host));
		else if (!_stricmp(NameBuffer, "Content-Type")) strncat(m_ContentType, ResultBuffer, sizeof(m_ContentType));
		else if (!_stricmp(NameBuffer, "Accept")) strncat(m_ContentType, ResultBuffer, sizeof(m_ContentType));
		else if (!_stricmp(NameBuffer, "Authorization")) strncat(m_Authorization, ResultBuffer, sizeof(m_Authorization));
		else if (!_stricmp(NameBuffer, "Content-Length")) m_ContentLength = atoi(ResultBuffer);
		else if (!_stricmp(NameBuffer, "Accept-Encoding")) {}
		else if (!_stricmp(NameBuffer, "Origin")) strncat(m_Origin, ResultBuffer, sizeof(m_Origin));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Key")) strncat(m_SecWebSockKey, ResultBuffer, sizeof(m_SecWebSockKey));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Accept")) strncat(m_SecWebSockKey, ResultBuffer, sizeof(m_SecWebSockKey));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Protocol")) strncat(m_SecWebSockProto, ResultBuffer, sizeof(m_SecWebSockProto));
		else if (!_stricmp(NameBuffer, "Sec-WebSocket-Version")) m_WebSockVersion = atoi(ResultBuffer);
		else if (!_stricmp(NameBuffer, "Cache-Control")) {
			if (!_stricmp(ResultBuffer, "no-cache")) m_Flag |= NoCacheControl;
		}else if (!_stricmp(NameBuffer, "Connection")) {
			for (const char *C = ResultBuffer; C; C = LWText::FirstToken(C, ',')) {
				C = LWText::NextWord(C == ResultBuffer ? C : (C + 1), true);
				if (!_strnicmp(C, "close", 5)) m_Flag &= ~CONNECTIONBITS;
				else if (!_strnicmp(C, "keep-alive", 10)) m_Flag |= ConnectionKeepAlive;
				else if (!_strnicmp(C, "upgrade", 7)) m_Flag |= ConnectionUpgrade;
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
			std::cout << "header: '" << NameBuffer << "' - '" << ResultBuffer << "'" << std::endl;
		}

	}
	return true;
}

LWEHttpRequest &LWEHttpRequest::SetURI(const char *URI) {
	char ProtocolBuffer[256];
	m_Port = ParseURI(URI, m_Host, sizeof(m_Host), nullptr, m_Path, sizeof(m_Path), nullptr, ProtocolBuffer, sizeof(ProtocolBuffer), nullptr);
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetURIf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetURI(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetHost(const char *Host) {
	*m_Host = '\0';
	strncat(m_Host, Host, sizeof(m_Host));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetHostf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetHost(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetPath(const char *Path) {
	*m_Path = '\0';
	strncat(m_Path, Path, sizeof(m_Path));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetPathf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetPath(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetOrigin(const char *Origin) {
	*m_Origin = '\0';
	strncat(m_Origin, Origin, sizeof(m_Origin));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetOriginf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetOrigin(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetWebSockKey(const char *Key) {
	*m_SecWebSockKey = '\0';
	strncat(m_SecWebSockKey, Key, sizeof(m_SecWebSockKey));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetWebSockKeyf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetWebSockKey(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetWebSockProto(const char *Protocols) {
	*m_SecWebSockProto = '\0';
	strncat(m_SecWebSockProto, Protocols, sizeof(m_SecWebSockProto));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetWebSockProtof(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetWebSockProto(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetAuthorization(const char *Auth) {
	*m_Authorization = '\0';
	strncat(m_Authorization, Auth, sizeof(m_Authorization));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetAuthorizationf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetAuthorization(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetContentType(const char *ContentType) {
	*m_ContentType = '\0';
	strncat(m_ContentType, ContentType, sizeof(m_ContentType));
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetContentTypef(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetContentType(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetBody(const char *Body) {
	*m_Body = '\0';
	strncat(m_Body, Body, sizeof(m_Body));
	m_ContentLength = (uint32_t)strlen(m_Body);
	return *this;
}

LWEHttpRequest &LWEHttpRequest::SetBodyf(const char *Fmt, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);

	return SetBody(Buffer);
}

LWEHttpRequest &LWEHttpRequest::SetCallback(std::function<void(LWEHttpRequest &, const char *)> Callback) {
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

uint32_t LWEHttpRequest::GetCacheState(void) {
	return m_Flag&CACHEBITS;
}

uint32_t LWEHttpRequest::GetConnectionState(void) {
	return m_Flag&CONNECTIONBITS;
}

bool LWEHttpRequest::CloseConnection(void){
	return (m_Flag&CONNECTIONBITS) == 0;
}

bool LWEHttpRequest::KeepAliveConnection(void){
	return (m_Flag&ConnectionKeepAlive) != 0;
}

bool LWEHttpRequest::UpgradeConnection(void) {
	return (m_Flag&ConnectionUpgrade) != 0;
}

uint32_t LWEHttpRequest::GetMethod(void) {
	return m_Flag&METHODBITS;
}

uint32_t LWEHttpRequest::GetEncodeType(void) {
	return m_Flag&ENCODEBITS;
}

uint32_t LWEHttpRequest::GetUpgradeType(void) {
	return m_Flag&UPGRADEBITS;
}

LWEHttpRequest::LWEHttpRequest(const char *URI, uint32_t Flag) : m_Socket(nullptr), m_Flag(Flag), m_Status(0), m_Callback(nullptr), m_UserData(nullptr), m_ContentLength(0), m_ChunkLength(0), m_Port(80), m_WebSockVersion(0) {
	m_Host[0] = '\0';
	m_Path[0] = '\0';
	m_Authorization[0] = '\0';
	m_ContentType[0] = '\0';
	m_Body[0] = '\0';
	m_Origin[0] = '\0';
	m_SecWebSockKey[0] = '\0';
	m_SecWebSockProto[0] = '\0';
	SetURI(URI);
}

LWEHttpRequest::LWEHttpRequest() : m_Socket(nullptr), m_Flag(0), m_Status(0), m_Callback(nullptr), m_UserData(nullptr), m_ContentLength(0), m_ChunkLength(0), m_Port(80), m_WebSockVersion(0) {
	m_Host[0] = m_Path[0] = m_Authorization[0] = m_ContentType[0] = m_Body[0] = m_Origin[0] = m_SecWebSockKey[0] = m_SecWebSockProto[0] = '\0';
}


LWProtocol &LWEProtocolHttp::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	char Buffer[1024 * 64];
	if(Socket.GetFlag()&LWSocket::Listen){
		LWSocket Acc;
		if (!Socket.Accept(Acc, Socket.GetProtocolID())) {
			std::cout << "Error accepting socket!" << std::endl;
			return *this;
		}
		Manager->PushSocket(Acc);
		return *this;
	}
	int32_t r = Socket.Receive(Buffer, sizeof(Buffer));
	if (r <= 0) {
		Socket.MarkClosable();
		return *this;
	}
	Buffer[r] = '\0';
	if (!ProcessRead(Socket, Buffer, r)) {
		std::cout << "Error parsing HTTP buffer." << std::endl;
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
			std::cout << "Error sending: " << Socket.GetSocketDescriptor() << " " << std::endl;
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
		std::cout << "Error deserializing response." << std::endl;
		return false;
	}
	if (Insert) {
		if (!GetNextFromPool(&Req)) {
			std::cout << "Could not get a request to write to from pool." << std::endl;
			return false;
		}
		*Req = IReq;
		Socket.SetProtocolData(m_ProtocolID, Req);
	}
	if (Req->m_Flag&LWEHttpRequest::ResponseReady) {
		if (Req->m_Status == 0) {
			if (!m_InRequests.Push(*Req, &Req)) {
				std::cout << "Failed to insert into request." << std::endl;
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
			uint32_t Error = LWSocket::CreateSocket(Sock, LWText(Request->m_Host), Request->m_Port, (uint32_t)LWSocket::Tcp, ProtocolID);
			if (Error) {
				std::cout << "Error connecting to: '" << Request->m_Host << ":" << Request->m_Port << "' " << Error << std::endl;
				continue;
			}
			Request->m_Socket = Manager.PushSocket(Sock);
			//std::cout << "Making socket: " << Request->m_Socket->GetSocketDescriptor() << std::endl;
			Request->m_Socket->SetProtocolData(m_ProtocolID, Request);
			if (!Request->m_Socket) {
				std::cout << "Error inserting socket." << std::endl;
				continue;
			}
		}

		uint32_t Res = Send(*Request->m_Socket, Buffer, Len);
		if (Res == -1) {
			std::cout << "Error sending request." << std::endl;
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

bool LWEProtocolHttp::PushResponse(LWEHttpRequest &InRequest, const char *Response, uint32_t lStatus) {
	LWEHttpRequest Request = InRequest;
	Request.m_Status = lStatus;
	Request.SetBody(Response);
	return m_OutRequests.Push(Request);
}

LWEProtocolHttp &LWEProtocolHttp::SetAgentString(const char *Agent) {
	m_Agent[0] = '\0';
	strncat(m_Agent, Agent, sizeof(m_Agent));
	return *this;
}

LWEProtocolHttp &LWEProtocolHttp::SetServerString(const char *Server) {
	m_Server[0] = '\0';
	strncat(m_Server, Server, sizeof(m_Server));
	return *this;
}

LWEProtocolHttp::LWEProtocolHttp(uint32_t ProtocolID, LWProtocolManager *Manager) : LWProtocol(), m_ProtocolID(ProtocolID), m_Manager(m_Manager) {
	m_Agent[0] = '\0';
	m_Server[0] = '\0';
}

LWEProtocolHttp::LWEProtocolHttp() : LWProtocol(), m_ProtocolID(0), m_Manager(nullptr) {
	m_Agent[0] = '\0';
	m_Server[0] = '\0';
}