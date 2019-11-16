#include "LWEProtocols/LWEProtocolWebSocketSecure.h"
#include "LWEProtocols/LWEProtocolHTTP.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWText.h>
#include <LWCore/LWByteBuffer.h>
#include <iostream>


bool LWEProtocolWebSocketSecure::ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen) {
	char Buf[256];
	char BufB[256];
	LWEWebSocket *WebSocket = (LWEWebSocket*)Socket.GetProtocolData(m_wProtocolID);
	if (!WebSocket) {
		LWEHttpRequest Request;
		uint32_t Error = 0;
		Error = Request.Deserialize(Buffer, BufferLen) ? 0 : 1;
		Error = Error ? Error : (*Request.m_SecWebSockKey && Request.m_WebSockVersion == LWEWEBSOCKET_SUPPVER && Request.UpgradeConnection()) ? 0 : 2;
		if (!Error && *Request.m_SecWebSockProto) {
			bool ValidProtocol = false;
			if (*m_SubProtocol) {
				uint32_t ProtocolLen = (uint32_t)strlen(m_SubProtocol);
				for (const char *C = Request.m_SecWebSockProto; C && !ValidProtocol; C = LWText::FirstToken(C, ',')) {
					C = LWText::NextWord(*C == ',' ? C + 1 : C, true);
					ValidProtocol = LWText::Compare(C, m_SubProtocol, ProtocolLen);
				}
			}
			if (!ValidProtocol) Error = 3;
		}
		if (Error) {
			std::cout << "Buffer:" << std::endl << Buffer << std::endl;
			if (Error == 1) std::cout << "Error deserializing websocket request." << std::endl;
			else if (Error == 2) std::cout << "Error Headers did not include correct websocket data." << std::endl;
			else if (Error == 3) std::cout << "Error protocol asked for is not supported." << std::endl;
			Socket.MarkClosable();
			return false;
		}
		WebSocket = m_Allocator.Allocate<LWEWebSocket>(nullptr, nullptr);
		WebSocket->m_Socket = &Socket;
		*Buf = '\0';
		strncat(Buf, Request.m_SecWebSockKey, sizeof(Buf));
		strncat(Buf, LWEWEBSOCKET_GUID, sizeof(Buf));
		LWCrypto::HashSHA1(Buf, (uint32_t)strlen(Buf), BufB);
		uint32_t *uBuf = (uint32_t*)BufB; //Order swap bytes before encoding.
		for (uint32_t i = 0; i < 5; i++) uBuf[i] = (uBuf[i] & 0xFF) << 24 | (uBuf[i] & 0xFF00) << 8 | (uBuf[i] & 0xFF0000) >> 8 | (uBuf[i] & 0xFF000000) >> 24;
		uint32_t Len = LWCrypto::Base64Encode(BufB, 20, WebSocket->m_SecKey, sizeof(WebSocket->m_SecKey));
		WebSocket->m_SecKey[Len] = '\0';
		WebSocket->SetSecProtocols(m_SubProtocol);
		WebSocket->SetHost(Request.m_Host);
		WebSocket->SetPath(Request.m_Path);
		WebSocket->SetOrigin(Request.m_Origin);
		WebSocket->m_Flag |= LWEWebSocket::CONNECTING_SERVER;
		Socket.SetProtocolData(m_wProtocolID, WebSocket);
		PushOutPacket(nullptr, 0, WebSocket, LWEWebPacket::CONTROL_CONNECT);
		return true;
	}
	if (!WebSocket->IsConnected()) {
		if (WebSocket->GetConnectStatus() == LWEWebSocket::CONNECTING_CLIENT) {
			LWEHttpRequest Request;
			uint32_t Error = 0;
			Error = Request.Deserialize(Buffer, BufferLen) ? 0 : 1;
			Error = Error ? Error : ((*Request.m_SecWebSockKey && Request.m_Status == LWEHttpRequest::SwitchingProtocols) ? 0 : 2);
			//we should probably also validate the key....

			if (Error) {
				std::cout << "Buffer:" << std::endl << Buffer << std::endl;
				if (Error == 1) std::cout << "Error deserializing websocket request." << std::endl;
				else if (Error == 2) std::cout << "Error headers did not include correct websocket data." << std::endl;
				Socket.MarkClosable();
				return false;
			}
			WebSocket->m_Flag = (WebSocket->m_Flag&~LWEWebSocket::CONNECTING_CLIENT) | LWEWebSocket::CONNECTED_CLIENT;
		}
		return true;
	}
	char IPBuf[32];
	LWSocket::MakeAddress(Socket.GetRemoteIP(), IPBuf, sizeof(IPBuf));
	if (BufferLen > 100) {}
	LWEWebPacket *OPack;
	uint32_t Target;
	uint32_t ReservePos;
	uint32_t o = 0;
	while (o != BufferLen) {
		uint32_t Res = WebSocket->m_ActivePacket.Deserialize(Buffer + o, BufferLen - o, m_Allocator);
		if (Res == -1) {
			std::cout << "Error deserializing data." << std::endl;
			return false;
		}
		o += Res;
		if (!WebSocket->m_ActivePacket.Finished()) continue;
		if (WebSocket->m_ActivePacket.m_DataLen != WebSocket->m_ActivePacket.m_DataPos) continue;
		if (WebSocket->m_ActivePacket.GetOp() == LWEWebPacket::CONTROL_CLOSED) {
			WebSocket->m_ActivePacket.WorkFinished();
			Socket.MarkClosable();
			return true;
		} else if (WebSocket->m_ActivePacket.GetOp() == LWEWebPacket::CONTROL_PING) {
			WebSocket->m_ActivePacket.WorkFinished();
			PushOutPacket(nullptr, 0, WebSocket, LWEWebPacket::CONTROL_PONG);
			continue;
		} else if (WebSocket->m_ActivePacket.GetOp() == LWEWebPacket::CONTROL_PONG) {
			WebSocket->m_ActivePacket.WorkFinished();
			continue;
		}
		WebSocket->m_ActivePacket.m_WebSocket = WebSocket;
		if (!m_InPackets.PushStart(&OPack, Target, ReservePos)) return false;
		*OPack = std::move(WebSocket->m_ActivePacket);
		m_InPackets.PushFinished(Target, ReservePos);
	}
	return true;
}

LWProtocol &LWEProtocolWebSocketSecure::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketClosed(Socket, Manager);
	LWEWebSocket *WebSock = (LWEWebSocket*)Socket.GetProtocolData(m_wProtocolID);
	bool Del = true;
	if (m_WebSocketClosedCallback) Del = m_WebSocketClosedCallback(Socket, WebSock, Manager);
	if (WebSock) WebSock->m_Socket = nullptr;
	if (Del) LWAllocator::Destroy(WebSock);
	return *this;
}

LWProtocol &LWEProtocolWebSocketSecure::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketChanged(Prev, New, Manager);
	LWEWebSocket *WebSocket = (LWEWebSocket*)Prev.GetProtocolData(m_wProtocolID);
	New.SetProtocolData(m_wProtocolID, Prev.GetProtocolData(m_wProtocolID));
	if (WebSocket) WebSocket->m_Socket = &New;
	if (m_WebSocketChangedCallback) m_WebSocketChangedCallback(Prev, New, WebSocket, Manager);
	return *this;
}

LWProtocol &LWEProtocolWebSocketSecure::ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen) {
	ProcessRead(Socket, Data, DataLen);
	return *this;
}

LWEProtocolWebSocketSecure &LWEProtocolWebSocketSecure::ProcessOutPackets(void) {
	char Buffer[1024 * 64];
	LWEWebPacket *Pack;
	uint32_t Target;
	uint32_t ReservePos;
	while (m_OutPackets.PopStart(&Pack, Target, ReservePos)) {
		LWEWebPacket RPack = std::move(*Pack);
		m_OutPackets.PopFinshed(Target, ReservePos);
		LWEWebSocket *Sock = RPack.m_WebSocket;
		if (!Sock->IsConnected() && (RPack.m_ControlFlag&LWEWebPacket::CONTROL_CONNECT)) {
			if (!Sock->m_Socket) return *this;
			LWEHttpRequest Request;
			Request.SetWebSockKey(Sock->m_SecKey);
			Request.SetWebSockProto(m_SubProtocol);
			Request.m_Flag |= LWEHttpRequest::ConnectionUpgrade | LWEHttpRequest::UpgradeWebSock;
			Request.m_Status = Sock->GetConnectStatus() == LWEWebSocket::CONNECTING_SERVER ? LWEHttpRequest::SwitchingProtocols : 0;
			if (Sock->GetConnectStatus() == LWEWebSocket::CONNECTING_CLIENT) {
				Request.SetHost(Sock->m_Host).SetPath(Sock->m_Path).SetOrigin(Sock->m_Origin);
				Request.m_WebSockVersion = LWEWEBSOCKET_SUPPVER;
			} else Sock->m_Flag = (Sock->m_Flag&~LWEWebSocket::CONNECTING_SERVER) | LWEWebSocket::CONNECTED_SERVER;
			uint32_t Len = Request.Serialize(Buffer, sizeof(Buffer), Sock->GetConnectStatus() == LWEWebSocket::CONNECTED_CLIENT ? m_UserAgent : m_Server);
			std::cout << "Sending headers!" << std::endl;
			uint32_t Res = Send(*Sock->m_Socket, Buffer, Len);
			if(Res==-1){
				std::cout << "Error sending data." << std::endl;
				return *this;
			}
			if (!Res) {
				if (Sock->GetConnectStatus() == LWEWebSocket::CONNECTED_SERVER) Sock->m_Flag = (Sock->m_Flag&~LWEWebSocket::CONNECTING_SERVER) | LWEWebSocket::CONNECTING_SERVER;
				PushOutPacket(nullptr, 0, Sock, LWEWebPacket::CONTROL_CONNECT);
				return *this; //break and let some time pass before we try again.
			}
		}
		if (RPack.m_ControlFlag&LWEWebPacket::CONTROL_CONNECT) continue; //discard as this was just to complete the upgrade transaction.
		if (!Sock->IsConnected()) {
			if (!m_OutPackets.PushStart(&Pack, Target, ReservePos)) {
				std::cout << "Error re-inserting packet." << std::endl;
				return *this;
			}
			*Pack = std::move(RPack);
			m_OutPackets.PushFinished(Target, ReservePos);
			return *this; //break and let some time pass before we try again.
		}
		std::cout << "Sending data: " << RPack.GetOp() << " Len: " << RPack.m_DataLen << " Fin: " << RPack.m_ControlFlag << std::endl;
		LWSocket *rSock = RPack.m_WebSocket->m_Socket;
		if (!rSock) continue;
		uint32_t Len = RPack.Serialize(Buffer, sizeof(Buffer), Sock->GetConnectStatus() == LWEWebSocket::CONNECTED_CLIENT);
		std::cout << "Serialized: " << Len << std::endl;
		uint32_t Res = Send(*rSock, Buffer, Len);
		if (Res==-1){
			std::cout << "Error sending data." << std::endl;
			return *this;
		}
		if (!Res) {
			if (!m_OutPackets.PushStart(&Pack, Target, ReservePos)) {
				std::cout << "Error re-inserting packet." << std::endl;
				return *this;
			}
			*Pack = std::move(RPack);
			m_OutPackets.PushFinished(Target, ReservePos);
		}
	}
	return *this;
}


LWEWebSocket *LWEProtocolWebSocketSecure::OpenSocket(const char *URI, uint32_t ProtocolID, const char *Origin) {
	char Host[256];
	char Path[256];
	char Protocol[256];
	uint16_t Port = LWEHttpRequest::ParseURI(URI, Host, sizeof(Host), nullptr, Path, sizeof(Path), nullptr, Protocol, sizeof(Protocol), nullptr);
	LWSocket Sock;
	uint32_t Err = LWSocket::CreateSocket(Sock, Host, Port, LWSocket::Tcp, ProtocolID);
	if (Err) {
		std::cout << "Error creating socket: " << Err << std::endl;
		return nullptr;
	}
	LWSocket *S = m_Manager->PushSocket(Sock);
	LWEWebSocket *WebSock = m_Allocator.Allocate<LWEWebSocket>(URI, Origin);
	WebSock->m_Flag |= LWEWebSocket::CONNECTING_CLIENT;
	WebSock->GenerateKey(m_KeySeed++);
	S->SetProtocolData(m_wProtocolID, WebSock);
	WebSock->m_Socket = S;

	PushOutPacket(nullptr, 0, WebSock, LWEWebPacket::CONTROL_CONNECT);
	return WebSock;
}

bool LWEProtocolWebSocketSecure::PushOutPacket(const char *Buffer, uint32_t BufferLen, LWEWebSocket *Socket, uint32_t ControlFlag) {
	LWEWebPacket *Pack;
	uint32_t Target;
	uint32_t ReservePos;
	if (!m_OutPackets.PushStart(&Pack, Target, ReservePos)) return false;
	*Pack = LWEWebPacket(Buffer, BufferLen, m_Allocator, ControlFlag | LWEWebPacket::CONTROL_FINISHED, Socket);
	m_OutPackets.PushFinished(Target, ReservePos);
	return true;
}

bool LWEProtocolWebSocketSecure::GetNextPacket(LWEWebPacket &Packet) {
	LWEWebPacket *Pack;
	uint32_t Target;
	uint32_t ReservePos;
	if (!m_InPackets.PopStart(&Pack, Target, ReservePos)) return false;
	Packet = std::move(*Pack);
	m_InPackets.PopFinshed(Target, ReservePos);
	return true;
}

LWEProtocolWebSocketSecure &LWEProtocolWebSocketSecure::SetServer(const char *Server) {
	*m_Server = '\0';
	strncat(m_Server, Server, sizeof(m_Server));
	return *this;
}

LWEProtocolWebSocketSecure &LWEProtocolWebSocketSecure::SetUserAgent(const char *Agent) {
	*m_UserAgent = '\0';
	strncat(m_UserAgent, Agent, sizeof(m_UserAgent));
	return *this;
}

LWEProtocolWebSocketSecure &LWEProtocolWebSocketSecure::SetSubProtocol(const char *SubProtocol) {
	*m_SubProtocol = '\0';
	strncat(m_SubProtocol, SubProtocol, sizeof(m_SubProtocol));
	return *this;
}

LWEProtocolWebSocketSecure &LWEProtocolWebSocketSecure::SetWebSocketClosedCallback(std::function<bool(LWSocket &, LWEWebSocket*, LWProtocolManager*)> WebSocketClosedCallback) {
	m_WebSocketClosedCallback = WebSocketClosedCallback;
	return *this;
}

LWEProtocolWebSocketSecure &LWEProtocolWebSocketSecure::SetWebSocketChangedCallback(std::function<void(LWSocket &, LWSocket &, LWEWebSocket *, LWProtocolManager *)> WebSocketChangedCallback) {
	m_WebSocketChangedCallback = WebSocketChangedCallback;
	return *this;
}

LWEProtocolWebSocketSecure::LWEProtocolWebSocketSecure(uint32_t ProtocolID, uint32_t TLSProtocolID, LWAllocator &Allocator, LWProtocolManager *Manager, const char *CertFile, const char *KeyFile) : LWEProtocolTLS(TLSProtocolID, Allocator, CertFile, KeyFile), m_wProtocolID(ProtocolID), m_Allocator(Allocator), m_Manager(Manager), m_KeySeed(0) {
	*m_Server = *m_UserAgent = *m_SubProtocol = '\0';
	m_WebSocketClosedCallback = nullptr;
	m_WebSocketChangedCallback = nullptr;
}