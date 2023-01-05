#include "LWNetwork/LWProtocolManager.h"
#include "LWNetwork/LWProtocol.h"
#include "LWCore/LWLogger.h"
#include "LWCore/LWTimer.h"
#include <cstring>
#include <iostream>

//LWPM_AsyncSockets

LWUTF8Iterator LWPM_AsyncSockets::GetURI(void) const {
	return URI;
}

LWPM_AsyncSockets::LWPM_AsyncSockets(const LWUTF8Iterator &iURI, LWRef<LWSocket> &Result) : Result(Result) {
	iURI.Copy(URI, sizeof(URI));
}

LWPM_AsyncSockets::LWPM_AsyncSockets(LWRef<LWSocket> &Result) : Result(Result) {}
	

//LWProtocolManager:
LWProtocolManager &LWProtocolManager::SetUserData(void *UserData){
	m_UserData = UserData;
	return *this;
}

LWRef<LWSocket> LWProtocolManager::PushSocket(LWSocket &Socket){
	if (m_ActiveSocketCount >= LWProtocolManager::MaxSockets) return LWRef<LWSocket>();
	LWRef<LWSocket> Sock = m_Allocator.CreateRef<LWSocket>(std::move(Socket));
	return PushSocket(Sock);
}

LWRef<LWSocket> LWProtocolManager::PushSocket(LWRef<LWSocket> &Sock) {
	if(m_ActiveSocketCount>=LWProtocolManager::MaxSockets) return LWRef<LWSocket>();
	m_SocketLock.lock();
	m_SocketSet[m_ActiveSocketCount] = { Sock->GetHandle(), POLLIN, 0 };
	m_Sockets[m_ActiveSocketCount++] = Sock;
	m_SocketLock.unlock();
	LWRef<LWProtocol> Proto = m_Protocols[Sock->GetProtocolID()];
	if(Proto) Proto->SocketAdded(Sock, *this);
	return Sock;
}

LWRef<LWSocket> LWProtocolManager::CreateAsyncSocket(const LWUTF8Iterator &URI, uint16_t RemotePort, uint32_t Type, uint32_t Flags, uint32_t ProtocolID) {
	return CreateAsyncSocket(URI, RemotePort, Type, 0, Flags, ProtocolID);
}

LWRef<LWSocket> LWProtocolManager::CreateAsyncSocket(const LWUTF8Iterator &URI, uint16_t RemotePort, uint32_t Type, uint16_t LocalPort, uint32_t Flags, uint32_t ProtocolID) {
	LWSocketHandle Handle = LWSocketHandle();
	LWRef<LWSocket> Result = m_Allocator.CreateRef<LWSocket>(Handle, LWSocketAddr(0, LocalPort), LWSocketAddr(0, RemotePort), (Type<<LWSocket::TypeBitsOffset) | (LWSocket::S_Connecting<<LWSocket::StatusBitsOffset) | Flags, ProtocolID);
	if(!m_AsyncQueue.Push(LWPM_AsyncSockets(URI, Result))) return LWRef<LWSocket>();
	return Result;
}

LWRef<LWSocket> LWProtocolManager::CreateAsyncSocket(const LWSocketAddr &RemoteAddr, uint32_t Type, uint32_t Flags, uint32_t ProtocolID) {
	return CreateAsyncSocket(RemoteAddr, Type, 0, Flags, ProtocolID);
}

LWRef<LWSocket> LWProtocolManager::CreateAsyncSocket(const LWSocketAddr &RemoteAddr, uint32_t Type, uint16_t LocalPort, uint32_t Flags, uint32_t ProtocolID) {
	LWSocketHandle Handle = LWSocketHandle();
	LWRef<LWSocket> Result = m_Allocator.CreateRef<LWSocket>(Handle, LWSocketAddr(0, LocalPort), RemoteAddr, (Type<<LWSocket::TypeBitsOffset) | (LWSocket::S_Connecting << LWSocket::StatusBitsOffset) | Flags, ProtocolID);
	if(!m_AsyncQueue.Push(LWPM_AsyncSockets(Result))) return LWRef<LWSocket>();
	return Result;
}

LWRef<LWProtocol> LWProtocolManager::GetProtocol(uint32_t ProtocolID) {
	return m_Protocols[ProtocolID];
}

LWRef<LWSocket> LWProtocolManager::GetSocket(uint32_t Index) {
	return m_Sockets[Index];
}

void *LWProtocolManager::GetUserData(void) const{
	return m_UserData;
}

uint32_t LWProtocolManager::GetActiveSocketCount(void) const{
	return m_ActiveSocketCount;
}

int32_t LWProtocolManager::Poll(uint32_t Timeout, uint64_t lCurrentTime) {

	auto HandleClose = [this, &lCurrentTime](uint32_t i, LWRef<LWSocket> &Socket, LWRef<LWProtocol> &SocketProto)->bool {
		if (!Socket->IsClosable(lCurrentTime)) return false;
		if (SocketProto) SocketProto->SocketClosed(Socket, *this);
		m_SocketLock.lock();
		std::swap(m_SocketSet[i], m_SocketSet[m_ActiveSocketCount - 1]);
		std::swap(m_Sockets[i], m_Sockets[m_ActiveSocketCount - 1]);
		m_Sockets[m_ActiveSocketCount-1]->Close();
		m_Sockets[m_ActiveSocketCount-1].Release();
		m_ActiveSocketCount--;
		m_SocketLock.unlock();
		return true;
	};

	auto HandleListener = [this](LWRef<LWSocket> &Socket, LWRef<LWProtocol> &SocketProto)->bool {
		if (!LWLogCriticalIf(m_ActiveSocketCount < MaxSockets, "Error: Max sockets protocol manager can manage exceeded.")) return false;
		LWSocket NewSock;
		if (!LWLogCriticalIf(Socket->Accept(NewSock, Socket->GetProtocolID()), "Error: Accepting new socket failed.")) return false;
		if(SocketProto) {
			if(!SocketProto->Accept(Socket, NewSock, *this)) return true;
		} 
		PushSocket(NewSock);
		return true;
	};

	auto HandleRead = [this](LWRef<LWSocket> &Socket, LWRef<LWProtocol> &SocketProto) -> bool {
		if (SocketProto) SocketProto->Read(Socket, *this);
		return true;
	};

	if(!PollSet(m_SocketSet, m_ActiveSocketCount, 0)) return -1; //something went wrong polling.
	uint32_t Count = m_ActiveSocketCount;
	int32_t ReadCount = 0;
	for (uint32_t i = 0; i < Count; ++i) {
		LWRef<LWSocket> Sock = m_Sockets[i];
		if (HandleClose(i, Sock, m_Protocols[Sock->GetProtocolID()])) {
			i--;
			Count--;
		}else if (m_SocketSet[i].revents & (POLLIN | POLLHUP)) {
			if (Sock->IsListener()) HandleListener(Sock, m_Protocols[Sock->GetProtocolID()]);
			else HandleRead(Sock, m_Protocols[Sock->GetProtocolID()]);
			m_SocketSet[i].revents = 0;
			ReadCount++;
		}
	}
	return ReadCount;
}

void LWProtocolManager::ProcessAsyncSockets(bool Verbose, uint64_t lTimeout, uint64_t lCurrentTime) {
	if(!lCurrentTime) lCurrentTime = LWTimer::GetCurrent();
	uint64_t EndTime = lTimeout==0?-1:lCurrentTime+lTimeout;
	LWPM_AsyncSockets ASock;
	while(LWTimer::GetCurrent()<EndTime && m_AsyncQueue.PopMove(ASock)) {
		LWSocketAddr RemoteAddr = ASock.Result->GetRemoteAddr();
		if(*ASock.URI) RemoteAddr = LWSocketAddr(ASock.URI, RemoteAddr.GetPort());

		uint32_t Err = LWSocket::CreateSocket(*ASock.Result, ASock.Result->GetType(), RemoteAddr, ASock.Result->GetLocalAddr().GetPort(), ASock.Result->GetFlags()&LWSocket::AsyncFlags, ASock.Result->GetProtocolID());
		if(!LWLogWarnIfv<256>(Err==0, Verbose, "Socket connection to '{}' failed: '{}' {}", ASock.GetURI(), LWSocket::ErrorNames[Err], GetError())) {
			ASock.Result->SetError(Err);
			continue;
		}
		if(!LWLogWarnIfv<256>((bool)PushSocket(ASock.Result), Verbose, "Socket connection to '{}' could not be added to protocol manager.", ASock.GetURI())) {
			ASock.Result->Close();
		}
	}
	return;
}

LWProtocolManager::LWProtocolManager(LWAllocator &Allocator) : m_Allocator(Allocator) {}

LWProtocolManager::~LWProtocolManager() {
	for (uint32_t i = 0; i < m_ActiveSocketCount; i++) {
		if(m_Protocols[m_Sockets[i]->GetProtocolID()]) m_Protocols[m_Sockets[i]->GetProtocolID()]->SocketClosed(m_Sockets[i], *this);
		m_Sockets[i].Release();
	}
}