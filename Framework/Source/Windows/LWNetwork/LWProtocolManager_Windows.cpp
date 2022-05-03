#include "LWNetwork/LWProtocolManager.h"
#include "LWNetwork/LWProtocol.h"
#include "LWPlatform/LWPlatform.h"
#include <LWCore/LWByteBuffer.h>
#include <algorithm>
#include <iostream>
bool LWProtocolManager::InitateNetwork(void){
	WSAData Data;
	return WSAStartup(MAKEWORD(2, 2), &Data) == 0;
}

void LWProtocolManager::TerminateNetwork(void){
	WSACleanup();
	return;
}

bool LWProtocolManager::PollSet(LWSocketPollHandle *SocketSet, uint32_t SetCnt, uint32_t Timeout) {
	if (!SetCnt) return true;
	uint32_t r = WSAPoll(SocketSet, SetCnt, Timeout);
	if (r == SOCKET_ERROR) return false;
	return true;
}

uint32_t LWProtocolManager::GetHostIPs(uint32_t *IPBuffer, uint32_t BufferSize) {
	char hostbuffer[128];
	addrinfo hint = { AI_CANONNAME | AI_RETURN_PREFERRED_NAMES, AF_INET, 0, 0, 0, nullptr, nullptr, nullptr }, *servinfo = nullptr;
	if (gethostname(hostbuffer, sizeof(hostbuffer)) == SOCKET_ERROR) return 0;
	
	if (getaddrinfo(hostbuffer, nullptr, &hint, &servinfo)) return 0;
	uint32_t i = 0;
	for (addrinfo *p = servinfo; p; p = p->ai_next, i++) {
		if (i < BufferSize) IPBuffer[i] = LWByteBuffer::MakeHost((uint32_t)((sockaddr_in*)p->ai_addr)->sin_addr.S_un.S_addr);
	}
	return i;
}

uint32_t LWProtocolManager::GetError(void){
	return WSAGetLastError();
}
