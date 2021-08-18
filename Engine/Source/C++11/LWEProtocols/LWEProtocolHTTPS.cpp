#include "LWEProtocols/LWEProtocolHTTPS.h"
#include <iostream>


LWProtocol &LWEProtocolHttps::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	return LWEProtocolTLS::Read(Socket, Manager);
}

LWProtocol &LWEProtocolHttps::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketChanged(Prev, New, Manager);
	LWEProtocolHttp::SocketChanged(Prev, New, Manager);
	return *this;
}

LWProtocol &LWEProtocolHttps::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketClosed(Socket, Manager);
	LWEProtocolHttp::SocketClosed(Socket, Manager);
	return *this;
}

LWProtocol &LWEProtocolHttps::ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen) {
	ProcessRead(Socket, Data, DataLen);
	return *this;
}

uint32_t LWEProtocolHttps::Send(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	return LWEProtocolTLS::Send(Socket, Buffer, Len);
}

LWEProtocolHttps::LWEProtocolHttps(uint32_t HttpsProtocolID, uint32_t TLSProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile, const LWUTF8Iterator &KeyFile) : LWEProtocolTLS(TLSProtocolID, Allocator, CertFile, KeyFile), LWEProtocolHttp(HttpsProtocolID) {}
