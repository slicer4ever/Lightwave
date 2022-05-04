#include "LWNetwork/LWSocket.h"
#include "LWNetwork/LWProtocolManager.h"
#include "LWCore/LWUnicode.h"
#include "LWCore/LWByteBuffer.h"
#include "LWPlatform/LWPlatform.h"
#include <arpa/inet.h>
#include <resolv.h>
#include <iostream>

sockaddr_in ToSockAddr(const LWSocketAddr &Addr) {
	sockaddr_in Result = { AF_INET, LWByteBuffer::MakeNetwork(Addr.GetPort()), 0, {0} };
	Result.sin_addr.s_addr = LWByteBuffer::MakeNetwork(Addr.IP[3]);
	return Result;
}

LWSocketAddr FromSockAddr(const sockaddr_in &Addr) {
	return LWSocketAddr(LWByteBuffer::MakeHost((uint32_t)Addr.sin_addr.s_addr), LWByteBuffer::MakeHost(Addr.sin_port));
}


uint32_t LWSocket::LookUpAddress(const LWUTF8Iterator &Address, LWSocketAddr *AddrBuffer, uint32_t AddrBufferLen, char *Addresses, uint32_t AddressLen) {
	addrinfo hint = { AI_CANONNAME, AF_INET, 0, 0, 0, nullptr, nullptr, nullptr }, *servinfo = nullptr;
	if (getaddrinfo(*Address.c_str<256>(), nullptr, &hint, &servinfo)) return 0xFFFFFFFF;
	unsigned int i = 0;
	char *AL = Addresses + AddressLen;
	for (addrinfo *p = servinfo; p; p = p->ai_next, i++) {
		if (i < AddrBufferLen) AddrBuffer[i] = FromSockAddr(*(sockaddr_in *)p->ai_addr);
		if (p->ai_canonname) {
			char *c = p->ai_canonname;
			for (; Addresses != AL && c; Addresses++, c++) *Addresses = *c;
		}
		if (Addresses != AL) {
			*Addresses = 0;
			Addresses++;
		}
	}
	if (Addresses != AL) {
		*Addresses = 0;
		Addresses++;
	}
	freeaddrinfo(servinfo);
	return i;
}

uint32_t LWSocket::LookUpDNSServers(LWSocketAddr *AddrBuffer, uint32_t AddrBufferLen) {
	res_init();
	uint32_t Cnt = 0;
	for (; Cnt < _res.nscount; Cnt++) {
		auto addr = _res.nsaddr_list[Cnt];
		if (Cnt < AddrBufferLen) AddrBuffer[Cnt] = FromSockAddr(*(sockaddr_in*)&addr);
	}
	return Cnt;
}

uint32_t LWSocket::CreateSocket(LWSocket &Socket, uint32_t Type, const LWSocketAddr &RemoteAddr, uint16_t LocalPort, uint32_t Flag, uint32_t ProtocolID) {
	sockaddr_in sBindAddr = ToSockAddr(LWSocketAddr(0, LocalPort));
	sockaddr_in sConnectAddr = ToSockAddr(RemoteAddr);
	socklen_t AddrSize = sizeof(sockaddr_in);
	uint32_t UPDNoConnReset = (Flag & LWSocket::UDPConnReset) == 0;
	uint32_t SockBroadcast = (Flag & LWSocket::BroadcastAble) != 0;
	uint32_t iTcpNoDelay = (Flag & LWSocket::TcpNoDelay) != 0;
	uint32_t iReuseAddr = (Flag & LWSocket::ReuseAddr) != 0;

	LWSocketHandle Handle = socket(AF_INET, Type == LWSocket::UDP ? SOCK_DGRAM : SOCK_STREAM, Type == LWSocket::UDP ? IPPROTO_UDP : IPPROTO_TCP);

	if (Handle == INVALID_SOCKET) return LWSocket::E_Socket;
	if (Type == LWSocket::UDP) {
		//if (ioctl(Handle, UDP_CONNRESET, (u_long *)&UPDNoConnReset)<0) return LWSocket::E_CtrlFlags;
		if (setsockopt(Handle, SOL_SOCKET, SO_BROADCAST, (char *)&SockBroadcast, sizeof(SockBroadcast))) return LWSocket::E_CtrlFlags;
	} else {
		if (setsockopt(Handle, IPPROTO_TCP, TCP_NODELAY, (char *)&iTcpNoDelay, sizeof(iTcpNoDelay))) return LWSocket::E_CtrlFlags;
	}

	if (setsockopt(Handle, SOL_SOCKET, SO_REUSEADDR, (char *)&iReuseAddr, sizeof(iReuseAddr))) return LWSocket::E_CtrlFlags;
	if (bind(Handle, (sockaddr *)&sBindAddr, AddrSize)) return LWSocket::E_Bind;
	if (RemoteAddr.IsValid()) {
		if (connect(Handle, (sockaddr *)&sConnectAddr, AddrSize)) return LWSocket::E_Connect;
		if (getpeername(Handle, (sockaddr *)&sConnectAddr, &AddrSize)) return LWSocket::E_GetPeer;
	}
	if (Flag & LWSocket::Listening) {
		if (listen(Handle, DefaultBacklog)) return LWSocket::E_Listen;
	}
	if (getsockname(Handle, (sockaddr *)&sBindAddr, &AddrSize)) return LWSocket::E_GetSock;

	Socket = LWSocket(Handle, FromSockAddr(sBindAddr), FromSockAddr(sConnectAddr), Flag | Type, ProtocolID);
	return 0;
}


uint32_t LWSocket::Receive(void *Buffer, uint32_t BufferLen) {
	uint32_t Len = recv(m_Handle, (char *)Buffer, BufferLen, 0);
	if (Len != -1) m_RecvBytes += Len;
	return Len;
}

uint32_t LWSocket::Receive(void *Buffer, uint32_t BufferLen, LWSocketAddr &RemoteAddr) {
	sockaddr_in rAddr;
	socklen_t rAddrLen = sizeof(rAddr);
	uint32_t Len = recvfrom(m_Handle, (char *)Buffer, BufferLen, 0, (sockaddr *)&rAddr, &rAddrLen);
	RemoteAddr = FromSockAddr(rAddr);
	if (Len != -1) m_RecvBytes += Len;
	return Len;
}

uint32_t LWSocket::Send(const void *Buffer, uint32_t BufferLen) {
	uint32_t Len = send(m_Handle, (const char *)Buffer, BufferLen, 0);
	if (Len != -1) m_SentBytes += Len;
	return Len;
}

uint32_t LWSocket::Send(const void *Buffer, uint32_t BufferLen, const LWSocketAddr &RemoteAddr) {
	sockaddr_in rAddr = ToSockAddr(RemoteAddr);
	uint32_t Len = sendto(m_Handle, (const char *)Buffer, BufferLen, 0, (sockaddr *)&rAddr, sizeof(rAddr));
	if (Len != -1) m_SentBytes += Len;
	return Len;
}

bool LWSocket::Accept(LWSocket &Result, uint32_t ProtocolID) const {
	sockaddr_in rAddr, lAddr;
	socklen_t rAddrLen = sizeof(rAddr);
	socklen_t lAddrLen = sizeof(lAddr);
	uint32_t SockID = (uint32_t)accept(m_Handle, nullptr, nullptr);

	uint32_t TcpNoDelay = (m_Flags & LWSocket::TcpNoDelay) ? true : false;
	if (setsockopt(SockID, IPPROTO_TCP, TCP_NODELAY, (char *)&TcpNoDelay, sizeof(TcpNoDelay))) return false;

	if (getsockname(SockID, (sockaddr *)&lAddr, &lAddrLen)) return false;
	if (getpeername(SockID, (sockaddr *)&rAddr, &rAddrLen)) return false;
	Result = LWSocket(SockID, FromSockAddr(lAddr), FromSockAddr(rAddr), m_Flags & ~LWSocket::Listening, ProtocolID);
	return true;
}

LWSocket &LWSocket::Close(void) {
	m_Flags = LWBitFieldSet(StatusBits, m_Flags, S_Closed);
	if (m_Handle) close(m_Handle);
	m_Handle = 0;
	return *this;
}

LWSocket::~LWSocket() {
	Close();
}