#include "LWEProtocols/LWEProtocolWebSocket.h"
#include "LWEProtocols/LWEProtocolHTTP.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWByteBuffer.h>
#include <iostream>


uint32_t LWEWebPacket::Deserialize(const char *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
	LWByteBuffer Buf((const int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned | LWByteBuffer::Network);
	
	auto ReadData = [this](LWByteBuffer &Buf) -> bool{
		uint32_t opCode = GetOp();
		uint32_t Len = m_DataLen;
		if (opCode == CONTROL_TEXT) Len--;
		uint32_t Remaining = Len - m_DataPos;
		Remaining = std::min<uint32_t>(Remaining, Buf.GetBufferSize() - Buf.GetPosition());
		m_DataPos += Buf.Read(m_Data + m_DataPos, Remaining);
		if (m_DataPos != Len) return false;
		if (opCode == CONTROL_TEXT) m_DataPos++;
		if (m_Mask) {
			
			uint32_t RemainLen = (m_DataLen - m_FramePos);
			uint32_t *uData = (uint32_t*)m_Data + (m_FramePos / 4);
			uint32_t uBytes = RemainLen / 4;
			if ((RemainLen%4) != 0) uBytes++;
			if (uBytes) {
				uint32_t FrameP = m_FramePos % 4;
				if (FrameP == 0) uData[0] ^= m_Mask;
				else if (FrameP == 1) uData[0] ^= ((m_Mask<<8) & 0xFFFFFF00);
				else if (FrameP == 2) uData[0] ^= ((m_Mask<<16) & 0xFFFF0000);
				else uData[0] ^= ((m_Mask<<24) & 0xFF000000);
				for (uint32_t i = 1; i < uBytes; i++) uData[i] ^= m_Mask;
			}
		}
		m_FramePos = m_DataPos;
		if (opCode == CONTROL_TEXT) {
			m_Data[Len] = '\0';
			m_FramePos--;
		}
		return true;
	};

	if (m_DataPos != m_DataLen) {
		ReadData(Buf);
		return Buf.GetPosition();
	}
	uint16_t Flag = Buf.Read<uint16_t>();
	bool Fin = (Flag & 0x8000) != 0;
	uint16_t opCodes = (Flag & 0xF00) >> 8;
	bool Masked = (Flag & 0x80) != 0;
	uint64_t Len = ((uint64_t)Flag) & 0x7F;
	if (Len == 126) Len = (uint64_t)Buf.Read<uint16_t>();
	else if (Len == 127) Len = Buf.Read<uint64_t>();
	uint32_t Mask = Masked ? Buf.Read<uint32_t>() : 0;
	//Order swap mask:
	m_Mask = (Mask & 0xFF) << 24 | (Mask & 0xFF00) << 8 | (Mask & 0xFF0000) >> 8 | (Mask & 0xFF000000) >> 24;
	if (opCodes == 1) m_ControlFlag |= CONTROL_TEXT;
	else if (opCodes == 2) m_ControlFlag |= CONTROL_BINARY;
	else if (opCodes == 8) m_ControlFlag |= CONTROL_CLOSED;
	else if (opCodes == 9) m_ControlFlag |= CONTROL_PING;
	else if (opCodes == 10) m_ControlFlag |= CONTROL_PONG;
	else if(opCodes != 0) return -1;
	if (Fin) m_ControlFlag |= CONTROL_FINISHED;
	opCodes = GetOp();
	if (m_DataLen) {
		uint32_t l = m_DataLen;
		l += (uint32_t)Len;
		uint32_t DLen = l % 4;
		DLen = l + (DLen == 0 ? 0 : 4 - DLen);
		char *NewData = Allocator.Allocate<char>(DLen);
		char *OldData = m_Data;
		std::copy(OldData, OldData + m_DataLen, NewData);
		m_Data = NewData;
		m_DataLen = l;
		if (opCodes == CONTROL_TEXT) m_DataPos--;
		LWAllocator::Destroy(OldData);
	} else {
		LWAllocator::Destroy(m_Data);
		m_DataLen = (uint32_t)Len;
		if (opCodes == CONTROL_TEXT) m_DataLen++;
		uint32_t DLen = m_DataLen % 4;
		DLen = m_DataLen + (DLen == 0 ? 0 : 4 - DLen);
		m_Data = Allocator.Allocate<char>(DLen);
	}
	ReadData(Buf);
	return Buf.GetPosition();
}

uint32_t LWEWebPacket::Serialize(char *Buffer, uint32_t BufferLen, bool isClient) {
	LWByteBuffer Buf((int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned | LWByteBuffer::Network);
	uint8_t ops[] = { 0, 1, 2, 8, 9, 10, 11 };
	uint8_t Flag = ((m_ControlFlag&CONTROL_FINISHED)?0x80:0) | ops[(m_ControlFlag&~CONTROL_FINISHED)];
	Buf.Write<uint8_t>(Flag);
	uint8_t subFlag = isClient?0x80:0;
	if (m_DataLen > 0xffff) {
		subFlag |= 0x7F;
		Buf.Write<uint8_t>(subFlag);
		Buf.Write<uint64_t>(((uint64_t)m_DataLen)&0xFFFFFFFF);
	} else if (m_DataLen > 125) {
		subFlag |= 0x7E;
		Buf.Write<uint8_t>(subFlag);
		Buf.Write<uint16_t>((uint16_t)m_DataLen);
	} else {
		subFlag |= (uint8_t)m_DataLen;
		Buf.Write<uint8_t>(subFlag);
	}
	if (isClient) {
		uint32_t Mask = 0;

		Buf.Write<uint32_t>(Mask);
		
		//Uncomment when doing proper masking.
		//uint32_t uMask = (Mask & 0xFF) << 24 | (Mask & 0xFF00) << 8 | (Mask & 0xFF0000) >> 8 | (Mask & 0xFF000000) >> 24;
		//uint32_t *uData = (uint32_t*)m_Data;
		//uint32_t uBytes = m_DataLen / 4 + (m_DataLen%4==0?0:1);
		//for (uint32_t i = 0; i < uBytes; i++) uData[i] ^= uMask;
		
	}
	Buf.Write<char>(m_DataLen, m_Data);
	return Buf.GetPosition();
}

uint32_t LWEWebPacket::GetOp(void) {
	return (m_ControlFlag&(CONTROL_BINARY | CONTROL_TEXT | CONTROL_CLOSED | CONTROL_PING | CONTROL_PONG));
}

void LWEWebPacket::WorkFinished(void) {
	LWAllocator::Destroy(m_Data);
	m_Data = nullptr;
	m_DataPos = 0;
	m_DataLen = 0;
	m_FramePos = 0;
	m_ControlFlag = 0;
	return;
}

bool LWEWebPacket::Finished(void) {
	return (m_ControlFlag&(CONTROL_FINISHED)) != 0;
}

LWEWebPacket &LWEWebPacket::operator=(LWEWebPacket &&Other) {
	LWAllocator::Destroy(m_Data);
	m_Data = Other.m_Data;
	m_DataLen = Other.m_DataLen;
	m_ControlFlag = Other.m_ControlFlag;
	m_WebSocket = Other.m_WebSocket;
	m_Mask = Other.m_Mask;
	m_DataPos = Other.m_DataPos;
	m_FramePos = Other.m_FramePos;
	Other.m_Data = nullptr;
	Other.m_DataLen = 0;
	Other.m_ControlFlag = 0;
	Other.m_DataPos = 0;
	Other.m_FramePos = 0;
	Other.m_Mask = 0;
	Other.m_WebSocket = nullptr;
	return *this;
}

LWEWebPacket &LWEWebPacket::operator = (const LWEWebPacket &Other) {
	m_Data = Other.m_Data;
	m_Mask = Other.m_Mask;
	m_DataPos = Other.m_DataPos;
	m_DataLen = Other.m_DataLen;
	m_FramePos = Other.m_FramePos;
	m_ControlFlag = Other.m_ControlFlag;
	m_WebSocket = Other.m_WebSocket;
	return *this;
}

LWEWebPacket::LWEWebPacket(LWEWebPacket &&Other) : m_Data(Other.m_Data), m_DataPos(Other.m_DataPos), m_FramePos(Other.m_FramePos), m_DataLen(Other.m_DataLen), m_Mask(Other.m_Mask), m_ControlFlag(Other.m_ControlFlag), m_WebSocket(Other.m_WebSocket) {
	Other.m_Data = nullptr;
	Other.m_DataPos = 0;
	Other.m_DataLen = 0;
	Other.m_FramePos = 0;
	Other.m_Mask = 0;
	Other.m_ControlFlag = 0;
	Other.m_WebSocket = nullptr;
}

LWEWebPacket::LWEWebPacket(const char *Data, uint32_t DataLen, LWAllocator &Allocator, uint32_t ControlFlag, LWEWebSocket *WebSocket) : m_DataLen(DataLen), m_ControlFlag(ControlFlag), m_WebSocket(WebSocket) {
	if (Data) {
		uint32_t DLen = DataLen % 4;
		DLen = DataLen + (DLen == 0 ? 0 : (4 - DLen));
		m_Data = Allocator.Allocate<char>(DLen);
		std::copy(Data, Data + m_DataLen, m_Data);
	}
}

LWEWebPacket::~LWEWebPacket() {
	LWAllocator::Destroy(m_Data);
}

LWEWebSocket &LWEWebSocket::SetURI(const LWUTF8Iterator &URI) {
	LWUTF8Iterator Proto, Domain, Path;
	LWSocket::SplitURI(URI, m_Port, Domain, Proto, Path);
	Domain.Copy(m_Host, sizeof(m_Host));
	Path.Copy(m_Path, sizeof(m_Path));
	return *this;
}

LWEWebSocket &LWEWebSocket::SetHost(const LWUTF8Iterator &Host) {
	Host.Copy(m_Host, sizeof(m_Host));
	return *this;
}

LWEWebSocket &LWEWebSocket::SetPath(const LWUTF8Iterator &Path) {
	Path.Copy(m_Path, sizeof(m_Path));
	return *this;
}

LWEWebSocket &LWEWebSocket::SetOrigin(const LWUTF8Iterator &Origin) {
	Origin.Copy(m_Origin, sizeof(m_Origin));
	return *this;
}

LWEWebSocket &LWEWebSocket::SetSecKey(const LWUTF8Iterator &Key) {
	Key.Copy(m_SecKey, sizeof(m_SecKey));
	return *this;
}

LWEWebSocket &LWEWebSocket::SetSecProtocols(const LWUTF8Iterator &Protocols) {
	Protocols.Copy(m_SecProtocols, sizeof(m_SecProtocols));
	return *this;
}

LWEWebSocket &LWEWebSocket::GenerateKey(uint32_t seed) {
	uint32_t Key[] = { seed, (seed + 7) * 13, (seed + 34) * 123, (~seed) * 10324 };
	uint32_t Len = LWCrypto::Base64Encode((const char*)Key, sizeof(Key), m_SecKey, sizeof(m_SecKey));
	m_SecKey[Len] = '\0';
	return *this;
}

LWUTF8Iterator LWEWebSocket::GetHost(void) const {
	return m_Host;
}

LWUTF8Iterator LWEWebSocket::GetPath(void) const {
	return m_Path;
}

LWUTF8Iterator LWEWebSocket::GetOrigin(void) const {
	return m_Origin;
}

LWUTF8Iterator LWEWebSocket::GetSecKey(void) const {
	return m_SecKey;
}

LWUTF8Iterator LWEWebSocket::GetSecProtocols(void) const {
	return m_SecProtocols;
}

uint32_t LWEWebSocket::GetConnectStatus(void) {
	return m_Flag&(CONNECTING_CLIENT | CONNECTING_SERVER | CONNECTED_CLIENT | CONNECTED_SERVER);
}

bool LWEWebSocket::IsConnected(void) {
	uint32_t lStatus = GetConnectStatus();
	return lStatus == CONNECTED_CLIENT || lStatus == CONNECTED_SERVER;
}

LWEWebSocket::LWEWebSocket(const LWUTF8Iterator &URI, const LWUTF8Iterator &Origin) {
	SetURI(URI);
	SetOrigin(Origin);
}

LWProtocol &LWEProtocolWebSocket::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	char Buffer[64 * 1024]; //64 kb buffer!
	uint32_t r = Socket.Receive(Buffer, sizeof(Buffer));
	if (r == 0xFFFFFFFF) {
		Socket.MarkClosable();
		return *this;
	}
	ProcessRead(Socket, Buffer, r);
	return *this;
}

bool LWEProtocolWebSocket::ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen) {
	const uint32_t MaxProtocolList = 32;
	LWUTF8Iterator ProtocolList[MaxProtocolList];
	char Buf[256];
	char BufB[256];
	LWEWebSocket *WebSocket = (LWEWebSocket*)Socket.GetProtocolData(m_ProtocolID);
	if (!WebSocket) {
		LWEHttpRequest Request;
		uint32_t Error = 0;
		Error = Request.Deserialize(Buffer, BufferLen)?0:1;
		Error = Error ? Error : (*Request.m_SecWebSockKey && Request.m_WebSockVersion==LWEWEBSOCKET_SUPPVER && Request.UpgradeConnection()) ? 0 : 2;
		if (!Error && *Request.m_SecWebSockProto) {
			bool ValidProtocol = false;
			if (*m_SubProtocol) {
				uint32_t ProtocolCnt = std::min<uint32_t>(Request.GetSecWebSockProto().SplitToken(ProtocolList, MaxProtocolList, ','), MaxProtocolList);
				for (uint32_t i = 0; i < ProtocolCnt && !ValidProtocol; i++) ValidProtocol = ProtocolList[i].AdvanceWord(true).Compare(m_SubProtocol);
			}
			if (!ValidProtocol) Error = 3;
		}
		if (Error) {
			if (Error == 1) fmt::print("Error deserializing websocket request.\n");
			else if (Error == 2) fmt::print("Error Headers did not include correct websocket data.\n");
			else if (Error == 3) fmt::print("Error protocol asked for is not supported: '{}'\n", Request.m_SecWebSockProto);
			Socket.MarkClosable();
			return false;
		}
		WebSocket = m_Allocator.Create<LWEWebSocket>(LWUTF8Iterator(), LWUTF8Iterator());
		WebSocket->m_Socket = &Socket;
		strlcpy(Buf, Request.m_SecWebSockKey, sizeof(Buf));
		strlcat(Buf, LWEWEBSOCKET_GUID, sizeof(Buf));
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
		Socket.SetProtocolData(m_ProtocolID, WebSocket);
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
				if (Error == 1) fmt::print("Error deserializing websocket request.\n");
				else if (Error == 2) fmt::print("Error headers did not include correct websocket data.\n");
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
			fmt::print("Error deserializing data.\n");
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

LWProtocol &LWEProtocolWebSocket::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	LWEWebSocket *WebSock = (LWEWebSocket*)Socket.GetProtocolData(m_ProtocolID);
	bool Del = true;
	if(m_WebSocketClosedCallback) Del = m_WebSocketClosedCallback(Socket, WebSock, Manager);
	if (WebSock) WebSock->m_Socket = nullptr;
	if (Del) LWAllocator::Destroy(WebSock);
	return *this;
}

LWProtocol &LWEProtocolWebSocket::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEWebSocket *WebSocket = (LWEWebSocket*)Prev.GetProtocolData(m_ProtocolID);
	New.SetProtocolData(m_ProtocolID, Prev.GetProtocolData(m_ProtocolID));
	if(WebSocket) WebSocket->m_Socket = &New;
	if (m_WebSocketChangedCallback) m_WebSocketChangedCallback(Prev, New, WebSocket, Manager);
	return *this;
}

uint32_t LWEProtocolWebSocket::Send(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	uint32_t o = 0;
	for (; o < Len;) {
		int32_t r = Socket.Send(Buffer + o, Len - o);
		if (r == -1) {
			fmt::print("Error sending: {}\n", LWProtocolManager::GetError());
			return false;
		}
		o += (uint32_t)r;
	}
	return o;
}

LWEProtocolWebSocket &LWEProtocolWebSocket::ProcessOutPackets(void) {
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
			uint32_t Len = Request.Serialize(Buffer, sizeof(Buffer), Sock->GetConnectStatus()==LWEWebSocket::CONNECTED_CLIENT?m_UserAgent:m_Server);
			if (Send(*Sock->m_Socket, Buffer, Len)!=Len) {
				fmt::print("Error sending data.\n");
				return *this;
			}
		}
		if (RPack.m_ControlFlag&LWEWebPacket::CONTROL_CONNECT) continue; //discard as this was just to complete the upgrade transaction.
		if(!Sock->IsConnected()){
			if (!m_OutPackets.PushStart(&Pack, Target, ReservePos)) {
				fmt::print("Error re-inserting packet.\n");
				return *this;
			}
			*Pack = std::move(RPack);
			m_OutPackets.PushFinished(Target, ReservePos);
			return *this; //break and let some time pass before we try again.
		}
		LWSocket *rSock = RPack.m_WebSocket->m_Socket;
		if(!rSock) continue;
		uint32_t Len = RPack.Serialize(Buffer, sizeof(Buffer), Sock->GetConnectStatus() == LWEWebSocket::CONNECTED_CLIENT);
		Send(*rSock, Buffer, Len);
	}
	return *this;
}


LWEWebSocket *LWEProtocolWebSocket::OpenSocket(const LWUTF8Iterator &URI, uint32_t ProtocolID, const LWUTF8Iterator &Origin){
	LWUTF8Iterator Proto, Domain, Path;
	uint16_t Port;
	LWSocket::SplitURI(URI, Port, Domain, Proto, Path);
	LWSocket Sock;
	uint32_t Err = LWSocket::CreateSocket(Sock, Domain, Port, LWSocket::Tcp, ProtocolID);
	if(Err) {
		fmt::print("Error establishing connection to '{}:{}': {}\n", Domain, Port, Err);
		return nullptr;
	}
	LWSocket *S = m_Manager->PushSocket(Sock);
	if (!S) {
		fmt::print("Error inserting socket into manager.\n");
		return nullptr;
	}
	LWEWebSocket *WebSock = m_Allocator.Create<LWEWebSocket>(URI, Origin);
	WebSock->m_Flag |= LWEWebSocket::CONNECTING_CLIENT;
	WebSock->GenerateKey(m_KeySeed++);
	S->SetProtocolData(m_ProtocolID, WebSock);
	WebSock->m_Socket = S;	
	if (!PushOutPacket(nullptr, 0, WebSock, LWEWebPacket::CONTROL_CONNECT)) {
		fmt::print("Error could not create initial connecting packet for websocket.\n");
		S->MarkClosable();
		return nullptr;
	}
	return WebSock;
}

bool LWEProtocolWebSocket::PushOutPacket(const char *Buffer, uint32_t BufferLen, LWEWebSocket *Socket, uint32_t ControlFlag) {
	LWEWebPacket *Pack;
	uint32_t Target;
	uint32_t ReservePos;
	if (!m_OutPackets.PushStart(&Pack, Target, ReservePos)) return false;
	*Pack = LWEWebPacket(Buffer, BufferLen, m_Allocator, ControlFlag | LWEWebPacket::CONTROL_FINISHED, Socket);
	m_OutPackets.PushFinished(Target, ReservePos);
	return true;
}

bool LWEProtocolWebSocket::GetNextPacket(LWEWebPacket &Packet) {
	LWEWebPacket *Pack;
	uint32_t Target;
	uint32_t ReservePos;
	if (!m_InPackets.PopStart(&Pack, Target, ReservePos)) return false;
	Packet = std::move(*Pack);
	m_InPackets.PopFinshed(Target, ReservePos);
	return true;
}

LWEProtocolWebSocket &LWEProtocolWebSocket::SetServer(const LWUTF8Iterator &Server) {
	Server.Copy(m_Server, sizeof(m_Server));
	return *this;
}

LWEProtocolWebSocket &LWEProtocolWebSocket::SetUserAgent(const LWUTF8Iterator &Agent) {
	Agent.Copy(m_UserAgent, sizeof(m_UserAgent));
	return *this;
}

LWEProtocolWebSocket &LWEProtocolWebSocket::SetSubProtocol(const LWUTF8Iterator &SubProtocol) {
	SubProtocol.Copy(m_SubProtocol, sizeof(m_SubProtocol));
	return *this;
}

LWEProtocolWebSocket &LWEProtocolWebSocket::SetWebSocketClosedCallback(LWEWebSocketClosedCallback WebSocketClosedCallback) {
	m_WebSocketClosedCallback = WebSocketClosedCallback;
	return *this;
}

LWEProtocolWebSocket &LWEProtocolWebSocket::SetWebSocketChangedCallback(LWEWebSocketChangedCallback WebSocketChangedCallback) {
	m_WebSocketChangedCallback = WebSocketChangedCallback;
	return *this;
}

LWEProtocolWebSocket::LWEProtocolWebSocket(uint32_t ProtocolID, LWAllocator &Allocator, LWProtocolManager *Manager) : m_ProtocolID(ProtocolID), m_Allocator(Allocator), m_Manager(Manager) {}