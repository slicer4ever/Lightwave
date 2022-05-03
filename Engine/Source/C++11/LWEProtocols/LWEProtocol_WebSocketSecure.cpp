#include "LWEProtocols/LWEProtocol_WebSocketSecure.h"
#include "LWEProtocols/LWEProtocol_HTTP.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWByteBuffer.h>
#include <iostream>

LWProtocol &LWEProtocol_WebSocketSecure::Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	return LWEProtocol_TLS::Read(Socket, Manager);
}

LWProtocol &LWEProtocol_WebSocketSecure::SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	LWEProtocol_TLS::SocketClosed(Socket, Manager);
	LWEProtocol_WebSocket::SocketClosed(Socket, Manager);
	return *this;
}

LWProtocol &LWEProtocol_WebSocketSecure::ProcessTLSData(LWRef<LWSocket> &Socket, const void *Data, uint32_t DataLen) {
	ProcessWSReadPacket(Socket, Data, DataLen);
	return *this;
}

uint32_t LWEProtocol_WebSocketSecure::Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len) {
	return LWEProtocol_TLS::Send(Socket, Buffer, Len);
}

LWEProtocol_WebSocketSecure::LWEProtocol_WebSocketSecure(uint32_t ProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile, const LWUTF8Iterator &KeyFile) : LWProtocol(ProtocolID), LWEProtocol_TLS(ProtocolID, Allocator, CertFile, KeyFile), LWEProtocol_WebSocket(ProtocolID, Allocator) {}