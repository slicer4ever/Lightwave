#include "LWNetwork/LWProtocolManager.h"
#include "LWNetwork/LWProtocol.h"
#include <cstring>
#include <iostream>

LWProtocolManager &LWProtocolManager::SetUserData(void *UserData){
	m_UserData = UserData;
	return *this;
}

LWSocket *LWProtocolManager::PushSocket(LWSocket &Socket){
	if (m_ActiveSocketCount >= LWProtocolManager::MaxSockets) return nullptr;
	m_SocketSet[m_ActiveSocketCount].fd = Socket.GetSocketDescriptor();
	m_SocketSet[m_ActiveSocketCount].events = POLLIN;
	m_SocketSet[m_ActiveSocketCount].revents = 0;
	m_Sockets[m_ActiveSocketCount] = std::move(Socket);
	m_ActiveSocketCount++;
	return &m_Sockets[m_ActiveSocketCount-1];
}

LWProtocolManager &LWProtocolManager::RegisterProtocol(LWProtocol *Protocol, uint32_t ProtocolID){
	m_Protocols[ProtocolID] = Protocol;
	return *this;
}

LWProtocol *LWProtocolManager::GetProtocol(uint32_t ProtocolID) {
	return m_Protocols[ProtocolID];
}

LWSocket *LWProtocolManager::GetSocket(uint32_t Index) {
	return m_Sockets + Index;
}

void *LWProtocolManager::GetUserData(void) const{
	return m_UserData;
}

uint32_t LWProtocolManager::GetActiveSocketCount(void) const{
	return m_ActiveSocketCount;
}

bool LWProtocolManager::Poll(uint32_t Timeout) {
	LWProtocol *P = nullptr;
	if (!PollSet(m_SocketSet, m_ActiveSocketCount, Timeout)) return false;
	for (uint32_t i = 0; i < m_ActiveSocketCount; i++) {
		if (m_SocketSet[i].revents&(POLLIN | POLLHUP)) {
			P = m_Protocols[m_Sockets[i].GetProtocolID()];
			if (m_Sockets[i].isListener()) {
				LWSocket NewSock;
				if (!m_Sockets[i].Accept(NewSock, m_Sockets[i].GetProtocolID())) {
					fmt::print("Error occurred while accepting new socket.\n");
				} else if (P) P->Accept(NewSock, this);
			} else if (P) P->Read(m_Sockets[i], this);
			m_SocketSet[i].revents = 0;
		}
		if(m_Sockets[i].isClosable()) {
			P = m_Protocols[m_Sockets[i].GetProtocolID()];
			if (P) P->SocketClosed(m_Sockets[i], this);
			m_Sockets[i].Close();
			m_Sockets[i] = std::move(m_Sockets[m_ActiveSocketCount - 1]);
			m_SocketSet[i] = m_SocketSet[m_ActiveSocketCount - 1];
			if ((i+1)!=m_ActiveSocketCount) {
				LWProtocol *NP = m_Protocols[m_Sockets[i].GetProtocolID()];
				if (NP) NP->SocketChanged(m_Sockets[m_ActiveSocketCount - 1], m_Sockets[i], this);
			}
			i--;
			m_ActiveSocketCount--;
		}
	}
	return true;
}

LWProtocolManager::LWProtocolManager(){
	std::fill(m_Protocols, m_Protocols + MaxProtocols, nullptr);
}

LWProtocolManager::~LWProtocolManager() {
	for (uint32_t i = 0; i < m_ActiveSocketCount; i++) {
		LWProtocol *P = m_Protocols[m_Sockets[i].GetProtocolID()];
		if (P) P->SocketClosed(m_Sockets[i], this);
		m_Sockets[i].Close();
	}
}