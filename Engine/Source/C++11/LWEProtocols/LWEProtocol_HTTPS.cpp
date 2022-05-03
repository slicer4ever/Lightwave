#include "LWEProtocols/LWEProtocol_HTTPS.h"
#include <LWCore/LWLogger.h>
#include <iostream>


LWProtocol &LWEProtocol_HTTPS::Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	return LWEProtocol_TLS::Read(Socket, Manager);
}

LWProtocol &LWEProtocol_HTTPS::SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	LWEProtocol_TLS::SocketClosed(Socket, Manager);
	LWEProtocol_HTTP::SocketClosed(Socket, Manager);
	return *this;
}

LWProtocol &LWEProtocol_HTTPS::ProcessTLSData(LWRef<LWSocket> &Socket, const void *Data, uint32_t DataLen) {
	uint32_t o = 0;
	while (o < DataLen) {
		uint32_t r = ProcessHTTPReadMessage(Socket, (char8_t*)Data + o, DataLen - o, true);
		if (!LWLogCriticalIf<256>(r > 0 && r != -1, "Error occurred while parsing HTTP buffer from : '{}'.", Socket->GetRemoteAddr())) break;
		o += r;
	}
	return *this;
}

uint32_t LWEProtocol_HTTPS::Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len) {
	return LWEProtocol_TLS::Send(Socket, Buffer, Len);
}

LWEProtocol_HTTPS::LWEProtocol_HTTPS(uint32_t ProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile, const LWUTF8Iterator &KeyFile) : LWProtocol(ProtocolID), LWEProtocol_TLS(ProtocolID, Allocator, CertFile, KeyFile), LWEProtocol_HTTP(ProtocolID, Allocator) {}
