#include "LWNetwork/LWProtocolManager.h"
#include "LWNetwork/LWProtocol.h"
#include "LWPlatform/LWPlatform.h"
#include <LWCore/LWByteBuffer.h>
#include <algorithm>
#include <errno.h>

bool LWProtocolManager::InitateNetwork(void) {
	return true;
}

void LWProtocolManager::TerminateNetwork(void) {

	return;
}

bool LWProtocolManager::PollSet(pollfd *SocketSet, uint32_t SetCnt, uint32_t Timeout) {
	uint32_t r = (uint32_t)poll(SocketSet, SetCnt, Timeout);
	if (r == SOCKET_ERROR) return false;
	return true;
}

uint32_t LWProtocolManager::GetHostIPs(uint32_t *IPBuffer, uint32_t BufferSize) {
	char hostbuffer[128];
	addrinfo hint = { AI_CANONNAME, AF_INET, 0, 0, 0, nullptr, nullptr, nullptr }, *servinfo = nullptr;
	if (gethostname(hostbuffer, sizeof(hostbuffer)) == SOCKET_ERROR) return 0;

	if (getaddrinfo(hostbuffer, nullptr, &hint, &servinfo)) return 0;
	uint32_t i = 0;
	for (addrinfo *p = servinfo; p; p = p->ai_next, i++) {
		if (i < BufferSize) IPBuffer[i] = LWByteBuffer::MakeHost((uint32_t)((sockaddr_in*)p->ai_addr)->sin_addr.s_addr);
	}
	return i;
}

uint32_t LWProtocolManager::GetError(void) {
	return errno;
}
/*
bool LWProtocolManager::Poll(uint32_t Timeout) {
	if (!PollSet(m_SocketSet, m_ActiveSocketCount, Timeout)) return false;
	for (uint32_t i = 0; i < m_ActiveSocketCount; i++) {
		if (m_SocketSet[i].revents&(POLLIN | POLLHUP)) {
			LWProtocol *P = m_Protocols[m_Sockets[i].GetProtocolID()];
			if (P) P->Read(m_Sockets[i], this);
			m_SocketSet[i].revents = 0;
		}
		if (m_Sockets[i].GetFlag()&LWSocket::Closeable) {
			m_Sockets[i].Close();
			m_Sockets[i] = std::move(m_Sockets[m_ActiveSocketCount - 1]);
			m_SocketSet[i] = m_SocketSet[m_ActiveSocketCount - 1];
			i--;
			m_ActiveSocketCount--;
		}
	}
	return true;
}*/