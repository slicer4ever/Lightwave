#include "LWNetwork/LWSocket.h"
#include "LWCore/LWUnicode.h"
#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWLogger.h"
#include "LWPlatform/LWPlatform.h"
#include "LWNetwork/LWProtocolManager.h"
#include <iostream>

//LWSocketAddr:
bool LWSocketAddr::ParseIP(const LWUTF8Iterator &IPNotation, uint32_t *IPBuffer, bool &bIsIP6) {
	LWUTF8Iterator ColonSplits[8];
	LWUTF8Iterator DotSplits[4];
	if (IPNotation.AtEnd()) return false;
	uint32_t ColonCnt = IPNotation.SplitToken(ColonSplits, 8, ':');

	//We want to avoid the []'s of a uri domain.
	if (*ColonSplits[0] == '[') ColonSplits[0].Advance(1);
	LWUTF8I Last = ColonSplits[ColonCnt - 1].NextEnd().Prev();
	if (*Last == ']') ColonSplits[ColonCnt - 1] = LWUTF8I(ColonSplits[ColonCnt - 1], Last);

	uint32_t DotCnt = ColonSplits[ColonCnt - 1].SplitToken(DotSplits, 4, '.');

	uint8_t *IPBytes = (uint8_t *)IPBuffer;
	uint32_t o = 0;
	uint32_t Range = DotCnt == 4 ? 16 : 18;
	auto ReadHex = [&IPBytes, &o, &ColonCnt, &Range](const LWUTF8Iterator &Value, uint32_t i)->bool {
		uint32_t hValue = 0;
		if (Value.AtEnd()) o = Range - (ColonCnt - i) * 2; //skip to end of bytes being written.
		else {
			sscanf_s(Value(), "%x", &hValue);
			*(uint16_t *)(IPBytes + o) = (uint16_t)hValue;
			o += 2;
		}
		return hValue <= 0xFFFF;
	};

	auto ReadDot = [&IPBytes, &o](const LWUTF8Iterator &Value) -> bool {
		uint32_t dValue;
		sscanf_s(Value(), "%d", &dValue);
		IPBytes[o++] = (uint8_t)dValue;
		return dValue <= 0xFF;
	};

	bIsIP6 = false;
	if (ColonCnt > 1) { //Has to be an ip6 address:
		bIsIP6 = true;
		for (uint32_t i = 0; i < ColonCnt - 1; ++i) {
			if (!ReadHex(ColonSplits[i], i)) return false;
		}
		if (DotCnt < 4) {
			if (!ReadHex(ColonSplits[ColonCnt - 1], ColonCnt - 1)) return false;
		}
		//need to swap hi/low of all bytes to be ordered right:
		for (int32_t i = 0; i < 4; i++) IPBuffer[i] = ((IPBuffer[i] >> 16) & 0xFFFF) | (IPBuffer[i] << 16);
	}
	if (DotCnt == 4) {
		o = 12;
		for (uint32_t i = 0; i < DotCnt; ++i) {
			if (!ReadDot(DotSplits[i])) return false;
		}
		//Need to swap endian order of last bytes to correct order.
		IPBuffer[3] = ((IPBuffer[3] >> 24) & 0xFF) | ((IPBuffer[3] >> 8) & 0xFF00) | ((IPBuffer[3] << 8) & 0xFF0000) | ((IPBuffer[3] << 24) & 0xFF000000);
	}
	return ColonCnt > 1 || DotCnt == 4;
}

uint16_t LWSocketAddr::GetPort(void) const {
	return (uint16_t)Port;
}

uint32_t LWSocketAddr::Hash(void) const {
	return LWCrypto::HashFNV1A((const uint8_t *)this, sizeof(LWSocketAddr));
}

uint32_t LWSocketAddr::HashIP(void) const {
	return LWCrypto::HashFNV1A((const uint8_t*)IP, sizeof(IP));
}

bool LWSocketAddr::IsIP6Addr(void) const {
	return (Port & IP6Addr) != 0;
}

bool LWSocketAddr::IsIP4Addr(void) const {
	return (Port & IP6Addr) == 0;
}

bool LWSocketAddr::IsValid(void) const {
	return (Port&Valid)!=0;
}

LWSocketAddr::LWSocketAddr(const LWUTF8Iterator &URI, LWUTF8Iterator &Domain, LWUTF8Iterator &Path, LWUTF8Iterator &Protocol, uint16_t DefaultPort) {
	bool bIsIP6 = false;
	uint16_t iPort = 0;
	if (!LWSocket::SplitURI(URI, iPort, Domain, Protocol, Path)) return; //Malformed URI detected.
	if (iPort == LWSocket::DefaultURIPort) iPort = DefaultPort;
	if (ParseIP(Domain, IP, bIsIP6)) {
		Port |= Valid | (bIsIP6 ? IP6Addr : 0);
	} else {
		LWSocket::LookUpAddress(Domain, this, 1, nullptr, 0);
	}
	Port |= (uint32_t)iPort;
}

LWSocketAddr::LWSocketAddr(const LWUTF8Iterator &URI, uint16_t DefaultPort) {
	LWUTF8Iterator Domain, Protocol, Path;
	bool bIsIP6 = false;
	uint16_t iPort = 0;
	if (!LWSocket::SplitURI(URI, iPort, Domain, Protocol, Path)) return; //Malformed URI detected.
	if(iPort==LWSocket::DefaultURIPort) iPort = DefaultPort;
	if (ParseIP(Domain, IP, bIsIP6)) {
		Port |= Valid | (bIsIP6 ? IP6Addr : 0);
	} else {
		LWSocket::LookUpAddress(Domain, this, 1, nullptr, 0);
	}
	Port |= (uint32_t)iPort;
}

LWSocketAddr::LWSocketAddr(uint32_t IP4, uint16_t Port) : Port(Port|Valid) {
	IP[3] = IP4;
}

LWSocketAddr::LWSocketAddr(uint32_t IP6A, uint32_t IP6B, uint32_t IP6C, uint32_t IP6D, uint16_t Port) : Port((uint32_t)Port | IP6Addr|Valid) {
	IP[0] = IP6A;
	IP[1] = IP6B;
	IP[2] = IP6C;
	IP[3] = IP6D;
}

//LWSocket:
LWBitField32Define(LWSocket::TypeBits);
LWBitField32Define(LWSocket::StatusBits);
LWBitField32Define(LWSocket::ErrorBits);

const char8_t *LWSocket::ProtocolNames[LWSocket::ProtocolCount] = { u8"https", u8"http", u8"wss", u8"ws", u8"ftp", u8"telnet", u8"ssh" };
const uint32_t LWSocket::ProtocolPorts[LWSocket::ProtocolCount] = { 443, 80, 443, 80, 20, 23, 22 };
const char8_t *LWSocket::ErrorNames[LWSocket::E_Count] = { u8"OK", u8"Connect", u8"AddressLookup", u8"Socket", u8"Bind", u8"Listen", u8"GetLocalAddress", u8"GetRemoteAddress", u8"ControlFlags" };

const uint32_t LWSocket::TCP; // \brief indicates socket is TCP ip protocol.
const uint32_t LWSocket::UDP; // \brief indicates socket is UDP ip protocol.

const uint32_t LWSocket::S_Connected; // \brief indicates socket is still in connecting phase.
const uint32_t LWSocket::S_Connecting; // \brief indicates socket is still in connecting phase.
const uint32_t LWSocket::S_Closed; // \brief indicates socket is closed.
const uint32_t LWSocket::S_Error; // \brief indicates socket is in an error state.

const uint32_t LWSocket::E_OK; /*!< \brief error flag indicating everything is ok. */
const uint32_t LWSocket::E_Connect; /*!< \brief error flag to indicate error in connecting to remote host. */
const uint32_t LWSocket::E_Address; /*!< \brief error flag to indicate error in retrieving the address of remote and local host. */
const uint32_t LWSocket::E_Socket; /*!< \brief error flag to indicate error in creating the socket object. */
const uint32_t LWSocket::E_Bind; /*!< \brief error flag to indicate error in binding the socket object. */
const uint32_t LWSocket::E_Listen; /*!< \brief error flag to indicate error in becoming a listening object. */
const uint32_t LWSocket::E_GetSock; /*!< \brief error flag to indicate error in retrieving local socket information. */
const uint32_t LWSocket::E_GetPeer; /*!< \brief error flag to indicate error in retrieving remote socket information. */
const uint32_t LWSocket::E_CtrlFlags; /*!< \brief error flag to indicate an error occurred while setting certain socket properties. */
const uint32_t LWSocket::E_Count; /*!< \brief total number of error flags avaiable. */

const uint32_t LWSocket::LocalIP4; // \brief ip4 local loopback address.
const uint32_t LWSocket::BroadcastIP4; // \brief ip4 broadcast address.

const uint32_t LWSocket::Listening; // \brief indicates socket listens for incoming connections, and accepts them automatically.
const uint32_t LWSocket::UDPConnReset; // \brief indicates if a udp socket should reset when it's remote connection notifies it is closed.
const uint32_t LWSocket::TcpNoDelay; // \brief indicates if a tcp socket should not delay batch data when sending, and instead send immediately.
const uint32_t LWSocket::ReuseAddr; // \brief indicates socket is safe to bind to an already bound port.
const uint32_t LWSocket::BroadcastAble; // \brief indicates socket should be made to broadcast if possible.
const uint32_t LWSocket::Closed; // \brief indicates the socket is to be closed, and should no longer be accessed.
const uint32_t LWSocket::AsyncFlags; // \brief indicates sockets flags that should be preserved when asyncing requests are made.

const uint32_t LWSocket::DefaultBacklog; //Default backlog for listen sockets.
const uint32_t LWSocket::DefaultURIPort; //Default URI port when parsing a URI that doesn't include a uri or a protocol.
const uint32_t LWSocket::DefaultDNSPort; //Default dns port when one isn't specified. 

const uint32_t LWSocket::ProtocolCount; /*!< \brief total number of protocol's lightwave is aware of. */

const uint32_t LWSocket::DNS_A;
const uint32_t LWSocket::DNS_NS;
const uint32_t LWSocket::DNS_MD;
const uint32_t LWSocket::DNS_MF;
const uint32_t LWSocket::DNS_CNAME;
const uint32_t LWSocket::DNS_SOA;
const uint32_t LWSocket::DNS_MB;
const uint32_t LWSocket::DNS_MG;
const uint32_t LWSocket::DNS_MR;
const uint32_t LWSocket::DNS_NULL;
const uint32_t LWSocket::DNS_WKS;
const uint32_t LWSocket::DNS_PTR;
const uint32_t LWSocket::DNS_HINFO;
const uint32_t LWSocket::DNS_MINFO;
const uint32_t LWSocket::DNS_MX;
const uint32_t LWSocket::DNS_TXT;
const uint32_t LWSocket::DNS_SRV;

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
	const uint32_t EncodeChars[] = { ' ', '!', '[', ']' };
	const uint32_t EncodeCharCnt = sizeof(EncodeChars) / sizeof(uint32_t);
	const uint16_t DefaultPort = 80;
	LWUTF8I Protocol, Domain, Path;
	uint16_t Port = 0;
	if (!SplitURI(URI, Port, Domain, Protocol, Path)) return 0;
	uint32_t o = 0;
	if (!Protocol.AtEnd()) o += LWUTF8I::Fmt_ns(Buffer, BufferSize, o, "{}://", Protocol);
	if (!Domain.AtEnd()) {
		if (Port == DefaultPort) o += LWUTF8I::Fmt_ns(Buffer, BufferSize, o, "{}", Domain);
		else o += LWUTF8I::Fmt_ns(Buffer, BufferSize, o, "{}:{}", Domain, Port);
	}
	if (Path.AtEnd()) return o;
	if (*Path != '/') o += LWUTF8I::Fmt_ns(Buffer, BufferSize, o, "/");
	LWUTF8Iterator C = Path;
	for (; !C.AtEnd(); ++C) {
		uint32_t CodePoint = *C;
		bool needsEncoding = false;
		for (uint32_t i = 0; i < EncodeCharCnt && !needsEncoding; i++) needsEncoding = CodePoint == EncodeChars[i];
		uint32_t r = 0;
		if (needsEncoding) o += LWUTF8I::Fmt_ns(Buffer, BufferSize, o, "%{:x}", CodePoint);
		else o += LWUTF8I::EncodeCodePoint_s(Buffer, BufferSize, o, CodePoint);
	}
	return o + 1;
}

bool LWSocket::SplitURI(const LWUTF8Iterator &URI, uint16_t &Port, LWUTF8Iterator &Domain, LWUTF8Iterator &Protocol, LWUTF8Iterator &Path) {
	const char8_t TokenList[] = u8":/\\[";
	const char8_t ProtocolHeader[] = u8"://";
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
	}
	if (*T == '[') { //At Domain IP Notation
		P = T;
		T.AdvanceToken(']');
		if (T.AtEnd()) return false; //URI is malformed.
		Domain = LWUTF8Iterator(P, T + 1);
		P = T;
		T.AdvanceTokens(TokenListIter, false);
	}
	if (*T == ':') { //At Port portion:
		if (!Domain.isInitialized()) Domain = LWUTF8Iterator(P, T);
		T.Advance(1);
		sscanf_s((const char *)T.GetPosition(), "%hu", &Port);
		P = T;
		T.AdvanceTokens(TokenListIter, false);
	}
	if (!Domain.isInitialized()) Domain = LWUTF8Iterator(P, T);
	if (*T == '/' || *T == '\\') Path = LWUTF8Iterator(T.GetPosition(), URI.GetLast());

	if (!Port) { //Decode protocol to port.
		Port = DefaultURIPort;
		uint32_t i = Protocol.CompareLista(ProtocolCount, ProtocolNames);
		if (i != -1) Port = ProtocolPorts[i];
	}
	return true;
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

uint32_t LWSocket::DNSQuery(const LWUTF8Iterator &QueryName, uint16_t QueryType, void *Buffer, uint32_t BufferLen, const LWSocketAddr &DNSAddr) {
	const uint64_t TimeOutFreq = LWTimer::GetResolution() * 2;
	char Query[1024];
	LWSocketPollHandle Set[1];
	LWByteBuffer Buf((int8_t*)Query, sizeof(Query), LWByteBuffer::Network);
	LWSocketAddr Addr = DNSAddr;
	if(!Addr.IsValid()) {
		if(!LookUpDNSServers(&Addr, 0)) return 0;
	}
	LWSocket DNSSock;
	int32_t Err = LWSocket::CreateSocket(DNSSock, LWSocket::UDP, 0);
	if(!LWLogWarnIf<128>(Err==0, "Failed to make socket: {}", Err)) return 0;

	Set[0].fd = DNSSock.GetHandle();
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
	Buf.Seek(Len);
	o += Len;
	o += Buf.Write<uint16_t>(QueryType);
	o += Buf.Write<uint16_t>(1);
	DNSSock.Send(Query, o, Addr);

	uint64_t lCurrent = LWTimer::GetCurrent();

	while (LWTimer::GetCurrent() < lCurrent + TimeOutFreq && LWProtocolManager::PollSet(Set, 1, 0)) {
		
		if (Set[0].revents&(POLLIN | POLLHUP)) {
			uint32_t res = DNSSock.Receive(Buffer, BufferLen, Addr);
			return res;
		}
	}
	LWLogWarn("DNS Query timed out.");
	return 0;
}

uint32_t LWSocket::DNSQuery(const LWUTF8Iterator *QueryNames, uint16_t *QueryTypes, uint32_t QueryCnt, void *Buffer, uint32_t BufferLen, const LWSocketAddr &DNSAddr) {
	const uint64_t TimeOutFreq = LWTimer::GetResolution() * 2;
	char Query[1024];
	LWSocketPollHandle Set[1];
	LWSocketAddr Addr = DNSAddr;
	LWByteBuffer Buf((int8_t*)Query, sizeof(Query), LWByteBuffer::Network);
	if(!Addr.IsValid()) {
		if(!LookUpDNSServers(&Addr, 1)) return 0;
	}
	LWLogEvent<256>("Querying DNS server: {}", Addr);
	LWSocket DNSSock;
	int32_t Err = LWSocket::CreateSocket(DNSSock, LWSocket::UDP, 0);
	if(!LWLogWarnIf<256>(Err==0, "Failed to make socket: {}", Err)) return 0;
	
	Set[0].fd = DNSSock.GetHandle();
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
		Buf.Seek(Len);
		o += Len;
		o += Buf.Write<uint16_t>(QueryTypes[i]);
		o += Buf.Write<uint16_t>(1);
	}
	DNSSock.Send(Query, o, Addr);

	uint64_t lCurrent = LWTimer::GetCurrent();

	while (LWProtocolManager::PollSet(Set, 1, 0) && LWTimer::GetCurrent() < lCurrent + TimeOutFreq) {
		if (Set[0].revents&(POLLIN | POLLHUP)) {
			uint32_t res = DNSSock.Receive(Buffer, BufferLen, Addr);
			return res;
		}
	}
	LWLogEvent("DNS Query timed out.");
	return 0;
}

uint32_t LWSocket::DNSParseSRVRecord(const char *Response, uint32_t ResponseLen, LWSRVRecord *RecordBuffer, uint32_t RecordBufferLen) {
	//This function needs to be upgraded to LWUnicodeIterator
	LWByteBuffer Buffer((const int8_t*)Response, ResponseLen, LWByteBuffer::Network);
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
		Buffer.Seek(Len);
		uint16_t QType = Buffer.Read<uint16_t>();
		uint16_t QClass = Buffer.Read<uint16_t>();
	}
	uint32_t Cnt = 0;
	//parse answer records!
	for (uint32_t i = 0; i < ACnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.Seek(Len);
		uint16_t RType = Buffer.Read<uint16_t>();
		uint16_t RClass = Buffer.Read<uint16_t>();
		uint32_t RTTL = Buffer.Read<uint32_t>();
		uint16_t RLen = Buffer.Read<uint16_t>();
		if (RType != DNS_SRV) {
			Buffer.Seek(RLen);
			continue;
		}
		uint16_t Priority = Buffer.Read<uint16_t>();
		uint16_t Weight = Buffer.Read<uint16_t>();
		uint16_t Port = Buffer.Read<uint16_t>();
		Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.Seek(Len);
		if (Cnt < RecordBufferLen) {
			RecordBuffer[Cnt].m_Port = Port;
			RecordBuffer[Cnt].m_Weight = (uint32_t)Weight;
			RecordBuffer[Cnt].m_Priority = (uint32_t)Priority;
			LWUTF8I(SubBuffer).Copy(RecordBuffer[Cnt].m_Address, sizeof(RecordBuffer[Cnt].m_Address));
		}
		Buffer.Seek(RLen);
		Cnt++;
	}
	return Cnt;
}

uint32_t LWSocket::DNSParseARecord(const char *Response, uint32_t ResponseLen, LWSocketAddr *AddrBuffer, uint32_t AddrBufferSize) {
	LWByteBuffer Buffer((const int8_t*)Response, ResponseLen, LWByteBuffer::Network);
	char SubBuffer[256];
	uint16_t ID = Buffer.Read<uint16_t>();
	uint16_t Flag = Buffer.Read<uint16_t>();
	uint16_t QCnt = Buffer.Read<uint16_t>();
	uint16_t ACnt = Buffer.Read<uint16_t>();
	uint16_t AthCnt = Buffer.Read<uint16_t>();
	uint16_t AddCnt = Buffer.Read<uint16_t>();
	for (uint32_t i = 0; i < QCnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.Seek(Len);
		uint16_t QType = Buffer.Read<uint16_t>();
		uint16_t QClass = Buffer.Read<uint16_t>();
	}
	uint32_t Cnt = 0;
	//parse answer records!
	for (uint32_t i = 0; i < ACnt; i++) {
		uint32_t Len = ReadDNSLabel((const char*)Buffer.GetReadBuffer() + Buffer.GetPosition(), Response, SubBuffer, sizeof(SubBuffer));
		Buffer.Seek(Len);
		uint16_t RType = Buffer.Read<uint16_t>();
		uint16_t RClass = Buffer.Read<uint16_t>();
		uint32_t RTTL = Buffer.Read<uint32_t>();
		uint16_t RLen = Buffer.Read<uint16_t>();
		if (RType != DNS_A) {
			Buffer.Seek(RLen);
			continue;
		}
		if (Cnt < AddrBufferSize) AddrBuffer[Cnt] = LWSocketAddr(Buffer.Read<uint32_t>(), 0);
		Cnt++;
	}
	return Cnt;
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, uint32_t Flag, uint32_t ProtocolID){
	return CreateSocket(Socket, LWSocket::UDP, LWSocketAddr(), 0, Flag, ProtocolID);
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, uint32_t Type, uint16_t Port, uint32_t Flag, uint32_t ProtocolID) {
	return CreateSocket(Socket, Type, LWSocketAddr(), Port, Flag, ProtocolID);
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, uint32_t Type, const LWSocketAddr &RemoteAddr, uint32_t Flag, uint32_t ProtocolID){
	return CreateSocket(Socket, Type, RemoteAddr, 0, Flag, ProtocolID);
}

LWSocket &LWSocket::SetUserData(void *UserData){
	m_UserData = UserData;
	return *this;
}

LWSocket &LWSocket::SetTimeoutPeriod(uint64_t lCurrentTime, uint64_t TimeoutPeriod) {
	if(!lCurrentTime) lCurrentTime = LWTimer::GetCurrent();
	m_TimeoutTime = lCurrentTime+TimeoutPeriod;
	return *this;
}

bool LWSocket::Accept(LWSocket &Result) const{
	return Accept(Result, m_ProtocolID);
}

uint32_t LWSocket::SendAll(const void *Buffer, uint32_t BufferLen) {
	uint32_t o = 0;
	while(o<BufferLen) {
		uint32_t r = Send((const char*)Buffer+o, BufferLen-o);
		if(r==-1) return r;
		o += r;
	}
	return o;
}

uint32_t LWSocket::SendAll(const void *Buffer, uint32_t BufferLen, const LWSocketAddr &RemoteAddr) {
	uint32_t o = 0;
	while(o<BufferLen) {
		uint32_t r = Send((const char*)Buffer+o, BufferLen-o, RemoteAddr);
		if(r==-1) return r;
		o += r;
	}
	return o;
}

LWSocket &LWSocket::SetError(uint32_t Err) {
	m_Flags = (m_Flags & (~(ErrorBits | StatusBits))) | (Err << ErrorBitsOffset) | (Err ? (S_Error << StatusBitsOffset) : (m_Flags & StatusBits));
	return *this;
}

LWSocket &LWSocket::operator = (LWSocket &&Other) noexcept {
	m_UserData = std::exchange(Other.m_UserData, nullptr);
	m_Handle = std::exchange(Other.m_Handle, LWSocketHandle());
	m_LocalAddr = Other.m_LocalAddr;
	m_RemoteAddr = Other.m_RemoteAddr;
	m_TimeoutTime = Other.m_TimeoutTime;
	m_ProtocolID = Other.m_ProtocolID;
	m_SentBytes = Other.m_SentBytes;
	m_RecvBytes = Other.m_RecvBytes;
	m_Flags = Other.m_Flags;
	return *this;
}

LWSocket::operator bool() const {
	return IsValid();
}

LWSocket &LWSocket::MarkClosable(void){
	m_Flags |= LWSocket::Closed;
	return *this;
}

bool LWSocket::IsListener(void) const {
	return (m_Flags & Listening) != 0;
}

bool LWSocket::IsClosable(uint64_t lCurrentTime) const {
	return (m_Flags & Closed) != 0 || lCurrentTime>=m_TimeoutTime;;
}

uint32_t LWSocket::GetProtocolID(void) const{
	return m_ProtocolID;
}

LWSocketAddr LWSocket::GetLocalAddr(void) const {
	return m_LocalAddr;
}

LWSocketAddr LWSocket::GetRemoteAddr(void) const {
	return m_RemoteAddr;
}

uint32_t LWSocket::GetFlags(void) const{
	return m_Flags;
}

uint32_t LWSocket::GetType(void) const {
	return LWBitFieldGet(TypeBits, m_Flags);
}

uint32_t LWSocket::GetError(void) const {
	return LWBitFieldGet(ErrorBits, m_Flags);
}

uint32_t LWSocket::GetStatus(void) const {
	return LWBitFieldGet(StatusBits, m_Flags);
}

bool LWSocket::IsValid(void) const {
	uint32_t lStatus = GetStatus();
	return lStatus==S_Connected || lStatus==S_Connecting;
}

uint32_t LWSocket::Hash(void) const {
	return m_LocalAddr.Hash();
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

LWSocketHandle LWSocket::GetHandle(void) const {
	return m_Handle;
}

bool LWSocket::IsTimedout(uint64_t lCurrentTime) const {
	return lCurrentTime>=m_TimeoutTime;
}

LWSocket::LWSocket(LWSocket &&Other) noexcept : m_LocalAddr(Other.m_LocalAddr), m_RemoteAddr(Other.m_RemoteAddr), m_TimeoutTime(Other.m_TimeoutTime), m_ProtocolID(Other.m_ProtocolID), m_SentBytes(Other.m_SentBytes), m_RecvBytes(Other.m_RecvBytes), m_Flags(Other.m_Flags) {
	m_Handle = std::exchange(Other.m_Handle, LWSocketHandle());
	m_UserData = std::exchange(Other.m_UserData, nullptr);
}

LWSocket::LWSocket(LWSocketHandle Handle, const LWSocketAddr &LocalAddr, const LWSocketAddr &RemoteAddr, uint32_t Flag, uint32_t ProtocolID) : m_Handle(Handle), m_LocalAddr(LocalAddr), m_RemoteAddr(RemoteAddr), m_ProtocolID(ProtocolID), m_Flags(Flag){}
