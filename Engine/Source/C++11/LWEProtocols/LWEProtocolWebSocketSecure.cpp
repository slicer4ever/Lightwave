#include "LWEProtocols/LWEProtocolWebSocketSecure.h"
#include "LWEProtocols/LWEProtocolHTTP.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWByteBuffer.h>
#include <iostream>

LWProtocol &LWEProtocolWebSocketSecure::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	return LWEProtocolTLS::Read(Socket, Manager);
}

LWProtocol &LWEProtocolWebSocketSecure::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketClosed(Socket, Manager);
	LWEProtocolWebSocket::SocketClosed(Socket, Manager);
	return *this;
}

LWProtocol &LWEProtocolWebSocketSecure::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketChanged(Prev, New, Manager);
	LWEProtocolWebSocket::SocketChanged(Prev, New, Manager);
	return *this;
}

LWProtocol &LWEProtocolWebSocketSecure::ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen) {
	ProcessRead(Socket, Data, DataLen);
	return *this;
}

uint32_t LWEProtocolWebSocketSecure::Send(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	return LWEProtocolTLS::Send(Socket, Buffer, Len);
}

LWEProtocolWebSocketSecure::LWEProtocolWebSocketSecure(uint32_t ProtocolID, uint32_t TLSProtocolID, LWAllocator &Allocator, LWProtocolManager *Manager, const LWUTF8Iterator &CertFile, const LWUTF8Iterator &KeyFile) : LWEProtocolTLS(TLSProtocolID, Allocator, CertFile, KeyFile), LWEProtocolWebSocket(ProtocolID, Allocator, Manager) {}