#include "LWNetwork/LWSocket.h"
#include "LWCore/LWUnicode.h"
#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWTimer.h"
#include "LWPlatform/LWPlatform.h"
#include "LWNetwork/LWProtocolManager.h"
#include <iostream>

const char8_t *LWSocket::ProtocolNames[LWSocket::ProtocolCount] = { u8"https", u8"http", u8"wss", u8"ws", u8"ftp", u8"telnet", u8"ssh" };
const uint32_t LWSocket::ProtocolPorts[LWSocket::ProtocolCount] = { 443, 80, 443, 80, 20, 23, 22 };

uint32_t LWSocket::MakeIP(uint8_t First, uint8_t Second, uint8_t Third, uint8_t Fourth){
	return First << 24 | (Second << 16) | (Third << 8) | Fourth;
}

uint32_t LWSocket::MakeIP(const LWUTF8Iterator &Address) {
	uint32_t A, B, C, D;
	if (sscanf(Address(), "%d.%d.%d.%d", &A, &B, &C, &D) != 4) return 0;
	LWVerify(A >= 0 && A <= 255 && B >= 0 && B <= 255 && C >= 0 && C <= 255 && D >= 0 && D <= 255);
	return (A << 24) | (B << 16) | (C << 8) | D;
}

bool LWSocket::MakeAddress(uint32_t Address, char8_t *Buffer, uint32_t BufferLen){
	return snprintf(Buffer, BufferLen, "%d.%d.%d.%d", ((Address >> 24) & 0xFF), ((Address >> 16) & 0xFF), ((Address >> 8) & 0xFF), (Address & 0xFF))>0;
}

uint32_t LWSocket::LookUpAddress(uint32_t Address, uint32_t *IPBuffer, uint32_t IPBufferLen, char *Addresses, uint32_t AddressLen){
	char AddressBuffer[16];
	if (!MakeAddress(Address, AddressBuffer, sizeof(AddressBuffer))) return 0xFFFFFFFF;
	return LookUpAddress(AddressBuffer, IPBuffer, IPBufferLen, Addresses, AddressLen);
}

uint32_t LWSocket::DecodeURI(const LWUTF8Iterator &URI, char8_t *Buffer, uint32_t BufferSize) {
	char8_t *Last = Buffer + BufferSize;
	uint32_t o = 0;
	uint32_t HexID = 0;
	LWUTF8Iterator C = URI;
	for (; !C.AtEnd(); ++C) {
		uint32_t CodePoint = *C;
		uint32_t r = 0;
		if (CodePoint == '%') {
			if (sscanf(C.GetPosition(), "%%%2x", &HexID)) {
				r = LWUTF8Iterator::EncodeCodePoint(Buffer, (uint32_t)(uintptr_t)(Last - Buffer), HexID);
				C.Advance(2);
			}
		}
		if (!r) r = LWUTF8Iterator::EncodeCodePoint(Buffer, (uint32_t)(uintptr_t)(Last - Buffer), CodePoint);
		if (Buffer + r < Last) Buffer += r;
		else Last = Buffer;
		o += r;
	}
	if (BufferSize) {
		if (Buffer == Last) while (LWUTF8Iterator::isTrailingCodeUnit(--Buffer)) {}
		*Buffer = '\0';
	}
	return o + 1;
}

uint32_t LWSocket::EncodeURI(const LWUTF8Iterator &URI, char8_t *Buffer, uint32_t BufferSize) {
	const uint32_t EncodeChars[] = { ' ', '!' };
	const uint32_t EncodeCharCnt = sizeof(EncodeChars) / sizeof(uint32_t);
	char8_t *Last = Buffer + BufferSize;
	uint32_t o = 0;
	LWUTF8Iterator C = URI;
	for (; !C.AtEnd(); ++C) {
		uint32_t CodePoint = *C;
		bool needsEncoding = false;
		for (uint32_t i = 0; i < EncodeCharCnt && !needsEncoding; i++) needsEncoding = CodePoint == EncodeChars[i];
		uint32_t r = 0;
		if (needsEncoding) r = snprintf(Buffer, (uint32_t)(uintptr_t)(Last - Buffer), "%%%x", CodePoint);
		else r = LWUTF8Iterator::EncodeCodePoint(Buffer, (uint32_t)(uintptr_t)(Last - Buffer), CodePoint);
		if (Buffer + r < Last) Buffer += r;
		else Last = Buffer;
		o += r;
	}
	if (BufferSize) {
		if (Buffer == Last) while (LWUTF8Iterator::isTrailingCodeUnit(--Buffer)) {}
		*Buffer = '\0';
	}
	return o + 1;
}

void LWSocket::SplitURI(const LWUTF8Iterator &URI, uint16_t &Port, LWUTF8Iterator &Domain, LWUTF8Iterator &Protocol, LWUTF8Iterator &Path) {
	const char8_t TokenList[] = u8":/\\";
	const char8_t ProtocolHeader[] = u8"://";
	const uint16_t DefaultPort = 80;
	LWUTF8Iterator TokenListIter = LWUTF8Iterator(TokenList, sizeof(TokenList));
	Domain = Protocol = Path = LWUTF8Iterator();
	Port = 0;
	LWUTF8Iterator P = URI;
	LWUTF8Iterator T = URI.NextTokens(TokenListIter, false);
	if (*T == ':') { //Possible protocol or port specifier.
		if (T.Compare(ProtocolHeader, 3)) { //At protocol portion:
			Protocol = LWUTF8Iterator(URI, T);
			T.Advance(3);
			P = T;
			T.AdvanceTokens(TokenListIter, false);
		}
		if (*T == ':') { //At port portion:
			Domain = LWUTF8Iterator(P, T);
			T.Advance(1);
			sscanf((const char*)T.GetPosition(), "%hu", &Port);
			P = T;
			T.AdvanceTokens(TokenListIter, false);
		}
	}
	if (!Domain.isInitialized()) Domain = LWUTF8Iterator(P, T);
	if (*T == '/' || *T == '\\') Path = LWUTF8Iterator(T.GetPosition(), URI.GetLast());

	if (!Port) { //Decode protocol to port.
		Port = DefaultPort;
		uint32_t i = Protocol.CompareLista(ProtocolCount, ProtocolNames);
		if (i != -1) Port = ProtocolPorts[i];
	}
	return;
}

uint32_t LWSocket::WriteDNSLabel(const LWUTF8Iterator &Label, char8_t *Buffer, uint32_t BufferSize) {
	uint32_t o = 0;
	char8_t *Last = Buffer + std::min<uint32_t>(BufferSize, BufferSize-1);
	LWUTF8Iterator C = Label;
	LWUTF8Iterator P = C;
	for (; !C.AtEnd(); ++C) {
		if (*C == '.') {
			if (Buffer < Last) *Buffer++ = (char8_t)P.RawDistance(C);
			o++;
			for (; P != C; ++P) {
				uint32_t r = LWUTF8Iterator::EncodeCodePoint(Buffer, (uint32_t)(uintptr_t)(Last - Buffer), *P);
				if (Buffer + r < Last) Buffer += r;
				else Last = Buffer;
				o += r;
			}
			++P;
		}
	}
	for (; P != C; ++P) {
		if (Buffer < Last) *Buffer++ = (char8_t)P.RawDistance(C);
		o++;
		uint32_t r = LWUTF8Iterator::EncodeCodePoint(Buffer, (uint32_t)(uintptr_t)(Last - Buffer), *P);
		if (Buffer + r < Last) Buffer += r;
		else Last = Buffer;
		o += r;
	}
	if (BufferSize) *Buffer = '\0';
	return o + 1;
}

uint32_t LWSocket::ReadDNSLabel(const char *Response, const char *ResponseStart, char *Buffer, uint32_t BufferLen) {
	auto ReadLabel = [](const char *R, char *Buffer, uint32_t BufferLen, bool PrependDot, uint32_t *BufferWrite)->uint32_t {
		uint32_t o = 0;
		uint32_t i = 0;
		uint8_t Cnt = (uint8_t)R[i++];
		*BufferWrite = 0;
		if (!Cnt) return 0;
		if (PrependDot) {
			if (o < BufferLen) Buffer[o] = '.';
			o++;
		}
		for (; i<=Cnt; i++) {
			if (o < BufferLen) Buffer[o] = R[i];
			o++;
		}

		*BufferWrite = o;
		return i;
	};
	uint32_t i = 0;
	uint32_t o = 0;
	uint32_t bw = 0;
	uint8_t Cnt = 0;
	do {
		Cnt = (uint8_t)Response[i];
		if ((Cnt & 0xC0) == 0xC0) {
			uint16_t Offset = 0;
			LWByteBuffer::ReadNetwork<uint16_t>(&Offset, (const int8_t*)Response+i);
			i += 2;
			Offset &= ~0xC000;
			uint32_t n = 0;
			uint8_t SubCnt = 0;
			do {
				SubCnt = ReadLabel(ResponseStart + Offset + n, Buffer + o, BufferLen - o, n != 0, &bw);
				n += SubCnt;
				o += bw;
			} while (SubCnt);
			return i;
		} else {
			Cnt = ReadLabel(Response + i, Buffer + o, BufferLen-o, i != 0, &bw);
			i += Cnt;
			o += bw;
		}
	} while (Cnt);
	if (o < BufferLen) Buffer[o++] = '\0';
	return i+1;
}

uint32_t LWSocket::DNSQuery(const LWUTF8Iterator &QueryName, uint16_t QueryType, char *Buffer, uint32_t BufferLen, uint32_t DNSIP) {
	const uint64_t TimeOutFreq = LWTimer::GetResolution() * 2;
	char IPBuf[16];
	char Query[1024];
	pollfd Set[1];
	uint16_t Port = 53;
	LWByteBuffer Buf((int8_t*)Query, sizeof(Query), LWByteBuffer::Network | LWByteBuffer::BufferNotOwned);
	
	if (!DNSIP) {
		if (!LookUpDNSServers(&DNSIP, 1)) return 0;
	}
	LWSocket::MakeAddress(DNSIP, IPBuf, sizeof(IPBuf));
	LWSocket DNSSock;
	int32_t Err = LWSocket::CreateSocket(DNSSock, LWSocket::Udp, 0);
	if(Err){
		fmt::print("Failed to make socket: {}\n", Err);
		return 0;
	}
	Set[0].fd = DNSSock.GetSocketDescriptor();
	Set[0].events = POLLIN;
	Set[0].revents = 0;
	uint32_t o = 0;
	o += Buf.Write<uint16_t>(0);
	o += Buf.Write<uint16_t>(0x100);
	o += Buf.Write<uint16_t>(1);
	o += Buf.Write<uint16_t>(0);
	o += Buf.Write<uint16_t>(0);
	o += Buf.Write<uint16_t>(0);
	
	uint32_t Len = WriteDNSLabel(QueryName, Query + Buf.GetPosition(), Buf.GetBufferSize() - Buf.GetPosition());
	Buf.OffsetPosition(Len);
	o += Len;
	o += Buf.Write<uint16_t>(QueryType);
	o += Buf.Write<uint16_t>(1);
	DNSSock.Send(Query, o, DNSIP, Port);

	uint64_t lCurrent = LWTimer::GetCurrent();

	while (LWTimer::GetCurrent() < lCurrent + TimeOutFreq && LWProtocolManager::PollSet(Set, 1, 0)) {
		
		if (Set[0].revents&(POLLIN | POLLHUP)) {
			uint32_t res = DNSSock.Receive(Buffer, BufferLen, &DNSIP, &Port);
			return res;
		}
	}
	fmt::print("DNS Query timed out.\n");
	return 0;
}

uint32_t LWSocket::DNSQuery(const LWUTF8Iterator *QueryNames, uint16_t *QueryTypes, uint32_t QueryCnt, char *Buffer, uint32_t BufferLen, uint32_t DNSIP) {
	const uint64_t TimeOutFreq = LWTimer::GetResolution() * 2;
	char IPBuf[16];
	char Query[1024];
	pollfd Set[1];
	uint16_t Port = 53;
	LWByteBuffer Buf((int8_t*)Query, sizeof(Query), LWByteBuffer::Network | LWByteBuffer::BufferNotOwned);

	if (!DNSIP) {
		if (!LookUpDNSServers(&DNSIP, 1)) return 0;
	}
	LWSocket::MakeAddress(DNSIP, IPBuf, sizeof(IPBuf));
	fmt::print("Querying server: {}\n", IPBuf);
	LWSocket DNSSock;
	int32_t Err = LWSocket::CreateSocket(DNSSock, LWSocket::Udp, 0);
	if(Err) {
		fmt::print("Failed to make socket: {}\n", Err);
		return 0;
	}
	Set[0].fd = DNSSock.GetSocketDescriptor();
	Set[0].events = POLLIN;
	Set[0].revents = 0;
	uint32_t o = 0;
	o += Buf.Write<uint16_t>(2);
	o += Buf.Write<uint16_t>(0x100);
	o += Buf.Write<uint16_t>((uint16_t)QueryCnt);
	o += Buf.Write<uint16_t>(0);
	o += Buf.Write<uint16_t>(0);
	o += Buf.Write<uint16_t>(0);
	for (uint32_t i = 0; i < QueryCnt; i++) {
		uint32_t Len = WriteDNSLabel(QueryNames[i], Query + Buf.GetPosition(), Buf.GetBufferSize() - Buf.GetPosition());
		Buf.OffsetPosition(Len);
		o += Len;
		o += Buf.Write<uint16_t>(QueryTypes[i]);
		o += Buf.Write<uint16_t>(1);
	}
	DNSSock.Send(Query, o, DNSIP, Port);

	uint64_t lCurrent = LWTimer::GetCurrent();

	while (LWProtocolManager::PollSet(Set, 1, 0) && LWTimer::GetCurrent() < lCurrent + TimeOutFreq) {
		if (Set[0].revents&(POLLIN | POLLHUP)) {
			uint32_t res = DNSSock.Receive(Buffer, BufferLen, &DNSIP, &Port);
			return res;
		}
	}
	fmt::print("DNS Query timed out.\n");
	return 0;
}

uint32_t LWSocket::DNSParseSRVRecord(const char *Response, uint32_t ResponseLen, LWSRVRecord *RecordBuffer, uint32_t RecordBufferLen) {
	LWByteBuffer Buffer((const int8_t*)Response, ResponseLen, LWByteBuffer::Network | LWByteBuffer::BufferNotOwned);
	char SubBuffer[256];
	uint16_t ID = Buffer.Read<uint16_t>();
	uint16_t Flag = Buffer.Read<uint16_t>();
	uint16_t QCnt = Buffer.Read<uint16_t>();
	uint16_t ACnt = Buffer.Read<uint16_t>();
	uint16_t AthCnt = Buffer.Read<uint16_t>();
	uint16_t AddCnt = Buffer.Read<uint16_t>();
	//skip question records!
	for (uint32_t i = 0; i < QCnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.OffsetPosition(Len);
		uint16_t QType = Buffer.Read<uint16_t>();
		uint16_t QClass = Buffer.Read<uint16_t>();
	}
	uint32_t Cnt = 0;
	//parse answer records!
	for (uint32_t i = 0; i < ACnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.OffsetPosition(Len);
		uint16_t RType = Buffer.Read<uint16_t>();
		uint16_t RClass = Buffer.Read<uint16_t>();
		uint32_t RTTL = Buffer.Read<uint32_t>();
		uint16_t RLen = Buffer.Read<uint16_t>();
		if (RType != DNS_SRV) {
			Buffer.OffsetPosition(RLen);
			continue;
		}
		uint16_t Priority = Buffer.Read<uint16_t>();
		uint16_t Weight = Buffer.Read<uint16_t>();
		uint16_t Port = Buffer.Read<uint16_t>();
		Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.OffsetPosition(Len);
		if (Cnt < RecordBufferLen) {
			RecordBuffer[Cnt].m_Port = Port;
			RecordBuffer[Cnt].m_Weight = (uint32_t)Weight;
			RecordBuffer[Cnt].m_Priority = (uint32_t)Priority;
			strlcpy(RecordBuffer[Cnt].m_Address, SubBuffer, sizeof(RecordBuffer[Cnt].m_Address));
		}
		Buffer.OffsetPosition(RLen);
		Cnt++;
	}
	return Cnt;
}

uint32_t LWSocket::DNSParseARecord(const char *Response, uint32_t ResponseLen, uint32_t *IPBuffer, uint32_t IPBufferLen) {
	LWByteBuffer Buffer((const int8_t*)Response, ResponseLen, LWByteBuffer::Network | LWByteBuffer::BufferNotOwned);
	char SubBuffer[256];
	uint16_t ID = Buffer.Read<uint16_t>();
	uint16_t Flag = Buffer.Read<uint16_t>();
	uint16_t QCnt = Buffer.Read<uint16_t>();
	uint16_t ACnt = Buffer.Read<uint16_t>();
	uint16_t AthCnt = Buffer.Read<uint16_t>();
	uint16_t AddCnt = Buffer.Read<uint16_t>();
	for (uint32_t i = 0; i < QCnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.OffsetPosition(Len);
		uint16_t QType = Buffer.Read<uint16_t>();
		uint16_t QClass = Buffer.Read<uint16_t>();
	}
	uint32_t Cnt = 0;
	//parse answer records!
	for (uint32_t i = 0; i < ACnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.OffsetPosition(Len);
		uint16_t RType = Buffer.Read<uint16_t>();
		uint16_t RClass = Buffer.Read<uint16_t>();
		uint32_t RTTL = Buffer.Read<uint32_t>();
		uint16_t RLen = Buffer.Read<uint16_t>();
		if (RType != DNS_A) {
			Buffer.OffsetPosition(RLen);
			continue;
		}
		if (Cnt < IPBufferLen) IPBuffer[Cnt] = Buffer.Read<uint32_t>();
		Cnt++;
	}
	return Cnt;
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, uint32_t Flag, uint32_t ProtocolID){
	return CreateSocket(Socket, 0, (Flag|LWSocket::Udp)&~LWSocket::Tcp, ProtocolID);
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, uint32_t RemoteIP, uint16_t RemotePort, uint32_t Flag, uint32_t ProtocolID){
	return CreateSocket(Socket, RemoteIP, RemotePort, 0, Flag, ProtocolID);
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, const LWUTF8Iterator &Address, uint16_t RemotePort, uint32_t Flag, uint32_t ProtocolID){
	return CreateSocket(Socket, Address, RemotePort, 0, Flag, ProtocolID);
}

LWSocket &LWSocket::SetUserData(void *UserData){
	m_UserData = UserData;
	return *this;
}

LWSocket &LWSocket::SetProtocolData(uint32_t ProtocolID, void *ProtocolData){
	m_ProtocolData[ProtocolID] = ProtocolData;
	return *this;
}

bool LWSocket::Accept(LWSocket &Result) const{
	return Accept(Result, m_ProtocolID);
}

LWSocket &LWSocket::operator = (LWSocket &&Other){
	m_UserData = Other.m_UserData;
	std::copy(Other.m_ProtocolData, Other.m_ProtocolData + MaxProtocols, m_ProtocolData);
	std::fill(Other.m_ProtocolData, Other.m_ProtocolData + MaxProtocols, nullptr);
	m_SocketID = Other.m_SocketID;
	m_ProtocolID = Other.m_ProtocolID;
	m_SentBytes = Other.m_SentBytes;
	m_RecvBytes = Other.m_RecvBytes;
	m_LocalIP = Other.m_LocalIP;
	m_RemoteIP = Other.m_RemoteIP;
	m_Flag = Other.m_Flag;
	m_RemotePort = Other.m_RemotePort;
	m_LocalPort = Other.m_LocalPort;
	Other.m_UserData = nullptr;
	Other.m_SocketID = 0;
	return *this;
}

LWSocket &LWSocket::MarkClosable(void){
	m_Flag |= LWSocket::Closeable;
	return *this;
}

bool LWSocket::isListener(void) const {
	return (m_Flag & Listen) != 0;
}

bool LWSocket::isClosable(void) const {
	return (m_Flag & Closeable) != 0;
}

uint32_t LWSocket::GetSocketDescriptor(void) const{
	return m_SocketID;
}

uint32_t LWSocket::GetProtocolID(void) const{
	return m_ProtocolID;
}

uint32_t LWSocket::GetLocalIP(void) const{
	return m_LocalIP;
}

uint16_t LWSocket::GetLocalPort(void) const{
	return m_LocalPort;
}

uint32_t LWSocket::GetRemoteIP(void) const{
	return m_RemoteIP;
}

uint16_t LWSocket::GetRemotePort(void) const{
	return m_RemotePort;
}

uint32_t LWSocket::GetFlag(void) const{
	return m_Flag;
}

uint32_t LWSocket::GetSentBytes(void) const {
	return m_SentBytes;
}

uint32_t LWSocket::GetRecvBytes(void) const {
	return m_RecvBytes;
}

void *LWSocket::GetUserData(void) const{
	return m_UserData;
}

void *LWSocket::GetProtocolData(uint32_t ProtocolID) const{
	return m_ProtocolData[ProtocolID];
}

LWSocket::LWSocket() : m_UserData(nullptr), m_SocketID(0), m_Flag(0) {
	memset(m_ProtocolData, 0, sizeof(void*)*MaxProtocols);
}


LWSocket::LWSocket(LWSocket &&Other) : m_UserData(Other.m_UserData), m_SocketID(Other.m_SocketID), m_ProtocolID(Other.m_ProtocolID), m_SentBytes(Other.m_SentBytes), m_RecvBytes(Other.m_RecvBytes), m_LocalIP(Other.m_LocalIP), m_RemoteIP(Other.m_RemoteIP), m_Flag(Other.m_Flag), m_RemotePort(Other.m_RemotePort), m_LocalPort(Other.m_LocalPort){
	std::copy(Other.m_ProtocolData, Other.m_ProtocolData + MaxProtocols, m_ProtocolData);
	std::fill(Other.m_ProtocolData, Other.m_ProtocolData + MaxProtocols, nullptr);
	Other.m_UserData = nullptr;
	Other.m_SocketID = 0;
}

LWSocket::LWSocket(uint32_t SocketID, uint32_t ProtocolID, uint32_t LocalIP, uint16_t LocalPort, uint32_t RemoteIP, uint16_t RemotePort, uint32_t Flag) : m_UserData(nullptr), m_SocketID(SocketID), m_ProtocolID(ProtocolID), m_LocalIP(LocalIP), m_RemoteIP(RemoteIP), m_Flag(Flag), m_RemotePort(RemotePort), m_LocalPort(LocalPort) {
	std::fill(m_ProtocolData, m_ProtocolData + MaxProtocols, nullptr);
}
