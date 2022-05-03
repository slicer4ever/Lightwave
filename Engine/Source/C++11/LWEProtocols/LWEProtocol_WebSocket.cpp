#include "LWEProtocols/LWEProtocol_WebSocket.h"
#include "LWEProtocols/LWEProtocol_HTTP.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWTimer.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWLogger.h>
#include <iostream>

uint32_t LWEWebPacket::Deserialize(const void *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
	LWByteBuffer Buf((const int8_t*)Buffer, BufferLen, LWByteBuffer::Network);
	
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

uint32_t LWEWebPacket::Serialize(void *Buffer, uint32_t BufferLen, bool isClient) {
	LWByteBuffer Buf((int8_t*)Buffer, BufferLen, LWByteBuffer::Network);
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

uint32_t LWEWebPacket::GetOp(void) const {
	return (m_ControlFlag&(CONTROL_BINARY | CONTROL_TEXT | CONTROL_CLOSED | CONTROL_PING | CONTROL_PONG));
}

LWUTF8Iterator LWEWebPacket::AsText(void) const {
	return LWUTF8Iterator((char8_t*)m_Data, m_DataLen);
}

bool LWEWebPacket::isBinaryPacket(void) const {
	return GetOp() == CONTROL_BINARY;
}

void LWEWebPacket::WorkFinished(void) {
	m_Data = LWAllocator::Destroy(m_Data);
	m_DataPos = 0;
	m_DataLen = 0;
	m_FramePos = 0;
	m_ControlFlag = 0;
	return;
}

bool LWEWebPacket::isConnectingPacket(void) const {
	return (m_ControlFlag & CONTROL_CONNECT) != 0;
}

bool LWEWebPacket::isFinished(void) const {
	return (m_ControlFlag&CONTROL_FINISHED) != 0;
}

LWEWebPacket &LWEWebPacket::operator=(LWEWebPacket &&Other) {
	LWAllocator::Destroy(m_Data);
	m_Data = std::exchange(Other.m_Data, nullptr);
	m_WebSocket = std::exchange(Other.m_WebSocket, LWRef<LWEWebSocket>());
	m_DataLen = std::exchange(Other.m_DataLen, 0);
	m_ControlFlag = std::exchange(Other.m_ControlFlag, 0);
	m_Mask = std::exchange(Other.m_Mask, 0);
	m_DataPos = std::exchange(Other.m_DataPos, 0);
	m_FramePos = std::exchange(Other.m_FramePos, 0);
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

LWEWebPacket::LWEWebPacket(LWEWebPacket &&Other) : m_WebSocket(std::exchange(Other.m_WebSocket, LWRef<LWEWebSocket>())), m_Data(std::exchange(Other.m_Data, nullptr)), m_DataPos(std::exchange(Other.m_DataPos, 0)), m_FramePos(std::exchange(Other.m_FramePos, 0)), m_DataLen(std::exchange(Other.m_DataLen, 0)), m_Mask(std::exchange(Other.m_Mask, 0)), m_ControlFlag(std::exchange(Other.m_ControlFlag, 0)) {}

LWEWebPacket::LWEWebPacket(const void *Data, uint32_t DataLen, LWAllocator &Allocator, uint32_t ControlFlag, LWRef<LWEWebSocket> &WebSocket) : m_WebSocket(WebSocket), m_DataLen(DataLen), m_ControlFlag(ControlFlag) {
	if (Data) {
		uint32_t DLen = DataLen % 4;
		DLen = DataLen + (DLen == 0 ? 0 : (4 - DLen));
		m_Data = Allocator.Allocate<char>(DLen);
		std::copy((char*)Data, (char*)Data + m_DataLen, m_Data);
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

LWEWebSocket::LWEWebSocket(const LWUTF8Iterator &URI, const LWUTF8Iterator &Origin, LWEWebSocketReceivedCallback RecvCallback, LWEWebSocketConnectedCallback ConnCallback, LWEWebSocketClosedCallback ClosedCallback) : m_OnClosedCallback(ClosedCallback), m_OnConnectedCallback(ConnCallback), m_OnReceivedCallback(RecvCallback) {
	SetURI(URI);
	SetOrigin(Origin);
}

LWProtocol &LWEProtocol_WebSocket::Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	char Buffer[64 * 1024]; //64 kb buffer!
	uint32_t r = Socket->Receive(Buffer, sizeof(Buffer));
	if (r == 0 || r==-1) {
		Socket->MarkClosable();
		return *this;
	}
	ProcessWSReadPacket(Socket, Buffer, r);
	return *this;
}

bool LWEProtocol_WebSocket::ProcessWSReadPacket(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t BufferLen) {
	const uint32_t MaxProtocolList = 32;
	LWUTF8Iterator ProtocolList[MaxProtocolList];
	char Buf[256];
	char BufB[256];
	LWRef<LWEWebSocket> WebSocket = GetSocketDataFor(Socket, m_WebSocketMap, m_WebSocketMapMutex);
	if (!WebSocket) {
		LWEHTTPMessage Request;
		uint32_t Error = 0;
		Error = Request.DeserializeHeaders(Buffer, BufferLen, false, true)?0:1;
		LWUTF8Iterator WebSockKey = Request["sec-websocket-key"];
		Error = Error ? Error : (!WebSockKey.AtEnd() && Request.m_WebSockVersion==LWEWEBSOCKET_SUPPVER && Request.UpgradeConnection()) ? 0 : 2;
		if (!Error && !WebSockKey.AtEnd()) {
			bool ValidProtocol = false;
			if (*m_SubProtocol) {
				uint32_t ProtocolCnt = std::min<uint32_t>(Request["sec-websocket-protocol"].SplitToken(ProtocolList, MaxProtocolList, ','), MaxProtocolList);
				for (uint32_t i = 0; i < ProtocolCnt && !ValidProtocol; i++) ValidProtocol = ProtocolList[i].AdvanceWord(true).Compare(m_SubProtocol);
			}
			if (!ValidProtocol) Error = 3;
		}
		if (Error) {
			LWLogCriticalv(Error!=1, "deserializing websocket request.");
			LWLogCriticalv(Error!=2, "Headers did not include correct websocket data.");
			LWLogCriticalv<256>(Error!=3, "protocol asked for is not supported: '{}'", Request["sec-websocket-protocol"]);
			Socket->MarkClosable();
			return false;
		}
		WebSocket = m_Allocator.CreateRef<LWEWebSocket>(LWUTF8Iterator(), LWUTF8Iterator());
		WebSocket->m_Socket = Socket;
		LWUTF8I::Fmt_n(Buf, sizeof(Buf), "{}{}", WebSockKey, LWEWEBSOCKET_GUID);
		LWCrypto::HashSHA1(Buf, (uint32_t)strlen(Buf), BufB);
		//Convert to network order:
		uint32_t *uBuf = (uint32_t*)BufB; //Order swap bytes before encoding.
		for (uint32_t i = 0; i < 5; i++) uBuf[i] = LWByteBuffer::MakeNetwork(uBuf[i]);

		uint32_t Len = LWCrypto::Base64Encode(BufB, 20, WebSocket->m_SecKey, sizeof(WebSocket->m_SecKey));
		WebSocket->m_SecKey[Len] = '\0';
		WebSocket->SetSecProtocols(m_SubProtocol);
		WebSocket->SetHost(Request["host"]);
		WebSocket->SetPath(Request.m_Path);
		WebSocket->SetOrigin(Request["origin"]);
		WebSocket->m_Flag |= LWEWebSocket::CONNECTING_SERVER;
		SetSocketDataFor(Socket, WebSocket, m_WebSocketMap, m_WebSocketMapMutex);
		return PushOutPacket(nullptr, 0, WebSocket, LWEWebPacket::CONTROL_CONNECT);
	}

	if (!WebSocket->IsConnected()) {
		if (WebSocket->GetConnectStatus() == LWEWebSocket::CONNECTING_CLIENT) {
			LWEHTTPMessage Request;
			uint32_t Error = 0;
			Error = Request.DeserializeHeaders(Buffer, BufferLen, false, true) ? 0 : 1;
			LWUTF8Iterator WebSockKey = Request["sec-websocket-accept"];
			Error = Error ? Error : ((!WebSockKey.AtEnd() && Request.GetStatus() == LWEHTTPMessage::S_SwitchingProtocols) ? 0 : 2);
			//we should probably also validate the key....
			
			if (Error) {
				LWLogCriticalv(Error!=1, "deserializing websocket request.");
				LWLogCriticalv(Error!=2, "headers did not include correct websocket data.");
				Socket->MarkClosable();
				return false;
			}
			WebSocket->m_Flag = (WebSocket->m_Flag&~LWEWebSocket::CONNECTING_CLIENT) | LWEWebSocket::CONNECTED_CLIENT;
			if(WebSocket->m_OnConnectedCallback) WebSocket->m_OnConnectedCallback(WebSocket, *this);
		}
		return true;
	}
	uint32_t o = 0;
	while (o != BufferLen) {
		uint32_t Res = WebSocket->m_ActivePacket.Deserialize((char8_t*)Buffer + o, BufferLen - o, m_Allocator);
		if(!LWLogCriticalIf(Res!=-1, "Failed deserializing data.")) return false;
		o += Res;
		if (!WebSocket->m_ActivePacket.isFinished()) continue;
		if (WebSocket->m_ActivePacket.m_DataLen != WebSocket->m_ActivePacket.m_DataPos) continue;
		if (WebSocket->m_ActivePacket.GetOp() == LWEWebPacket::CONTROL_CLOSED) {
			WebSocket->m_ActivePacket.WorkFinished();
			Socket->MarkClosable();
			return true;
		} else if (WebSocket->m_ActivePacket.GetOp() == LWEWebPacket::CONTROL_PING) {
			PushOutPacket(WebSocket->m_ActivePacket.m_Data, WebSocket->m_ActivePacket.m_DataLen, WebSocket, LWEWebPacket::CONTROL_PONG);
			WebSocket->m_ActivePacket.WorkFinished();
			continue;
		}
		WebSocket->m_ActivePacket.m_WebSocket = WebSocket;
		if(WebSocket->m_OnReceivedCallback) WebSocket->m_OnReceivedCallback(WebSocket, WebSocket->m_ActivePacket, *this);
		WebSocket->m_ActivePacket.WorkFinished();
	}
	return true;
}

LWProtocol &LWEProtocol_WebSocket::SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	LWRef<LWEWebSocket> WebSock = ExchangeSocketsData(Socket, LWRef<LWEWebSocket>(), m_WebSocketMap, m_WebSocketMapMutex);
	if(WebSock) {
		if(WebSock->m_OnClosedCallback) WebSock->m_OnClosedCallback(WebSock, *this);
	}
	return *this;
}

uint32_t LWEProtocol_WebSocket::Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len) {
	uint32_t r = Socket->SendAll(Buffer, Len);
	LWLogCriticalIf<256>(r == Len, "Socket '{}' Failed to send data: {}", Socket->GetRemoteAddr(), LWProtocolManager::GetError());
	return r;
}

LWEProtocol_WebSocket &LWEProtocol_WebSocket::ProcessOutboundPackets(uint64_t lTimeout) {
	char Buffer[1024 * 64];
	LWRef<LWEWebPacket> RPack;
	uint64_t EndTime = lTimeout == 0 ? -1 : LWTimer::GetCurrent() + lTimeout;
	while (LWTimer::GetCurrent() < EndTime && m_OutPackets.PopMove(RPack)) {
		LWRef<LWEWebSocket> WebSock = RPack->m_WebSocket;
		LWRef<LWSocket> Sock = WebSock->m_Socket;
		if(!LWLogWarnIf((bool)Sock && Sock->IsValid(), "Socket closed before packet was sent, dropping packet.")) continue;

		if (!WebSock->IsConnected() && RPack->isConnectingPacket()) {
			LWEHTTPMessage Request = LWEHTTPMessage("", LWEHTTPMessage::Method_Get, LWEHTTPMessage::Connection_Upgrade|LWEHTTPMessage::Upgrade_WebSocket);
			Request.PushHeader("sec-websocket-key", WebSock->m_SecKey);
			if(*m_SubProtocol) Request.PushHeader("sec-websocket-protocol", m_SubProtocol);
			Request.m_WebSockVersion = WebSocketVersion;
			Request.SetStatus(WebSock->GetConnectStatus() == LWEWebSocket::CONNECTING_SERVER ? LWEHTTPMessage::S_SwitchingProtocols : 0);
			if (WebSock->GetConnectStatus() == LWEWebSocket::CONNECTING_CLIENT) {
				Request.PushHeader("host", WebSock->m_Host);
				Request.PushHeader("origin", WebSock->m_Origin);
				Request.SetPath(WebSock->m_Path);
				Request.m_WebSockVersion = LWEWEBSOCKET_SUPPVER;
			} else WebSock->m_Flag = (WebSock->m_Flag&~LWEWebSocket::CONNECTING_SERVER) | LWEWebSocket::CONNECTED_SERVER;
			uint32_t Len = Request.SerializeHeaders(Buffer, sizeof(Buffer), WebSock->GetConnectStatus()==LWEWebSocket::CONNECTED_CLIENT?m_UserAgent:m_Server);
			Len += Request.SerializeBody(Buffer+Len, sizeof(Buffer)-Len);
			uint32_t Res = Send(Sock, Buffer, Len); //Error reporting should occur inside send if a problem occurred.
			if(Res!=Len) {
				if(Res==0) { //We couldn't send(likely a TLS connection, so we will re-insert and try again later).
					if(!LWLogCriticalIf(m_OutPackets.PushMove(RPack), "Panic, Failed to re-insert packet.  server likely overloaded.")) Sock->MarkClosable();
				}else Sock->MarkClosable(); //Something went wrong with the socket, so we just close it.
				return *this; //let a frame. go by if this was a re-insert.
			}
		}
		if(RPack->isConnectingPacket()) continue; //discard as this was just to complete the upgrade transaction.
		if(!WebSock->IsConnected()){
			m_OutPackets.PushMove(RPack);
			return *this; //break and let some time pass before we try again.
		}
		uint32_t Len = RPack->Serialize(Buffer, sizeof(Buffer), WebSock->GetConnectStatus() == LWEWebSocket::CONNECTED_CLIENT);
		uint32_t Res = Send(Sock, Buffer, Len);
		if(Res!=Len) Sock->MarkClosable(); //Something went wrong with the socket, so we just close it. (error reporting should happen inside send.)
	}
	return *this;
}

LWRef<LWEWebSocket> LWEProtocol_WebSocket::OpenSocket(const LWUTF8Iterator &URI, LWProtocolManager &ProtocolManager, const LWUTF8Iterator &Origin, LWEWebSocketReceivedCallback RecvCallback, LWEWebSocketConnectedCallback ConnCallback, LWEWebSocketClosedCallback ClosedCallback){
	LWUTF8Iterator Proto, Domain, Path;
	uint16_t Port;
	LWSocket::SplitURI(URI, Port, Domain, Proto, Path);
	LWSocket Sock;
	uint32_t Err = LWSocket::CreateSocket(Sock, LWSocket::TCP, LWSocketAddr(Domain, Port), 0, ProtocolID);
	if(!LWLogCriticalIf<256>(Err==0, "Failed to establish connection to '{}:{}': {}", Domain, Port, Err)) return LWRef<LWEWebSocket>();
	LWRef<LWSocket> S = ProtocolManager.PushSocket(Sock);
	if(!LWLogCriticalIf((bool)S, "Failed to insert socket into protocol manager.")) return LWRef<LWEWebSocket>();
	LWRef<LWEWebSocket> WebSock = m_Allocator.CreateRef<LWEWebSocket>(URI, Origin, RecvCallback, ConnCallback, ClosedCallback);
	WebSock->m_Flag |= LWEWebSocket::CONNECTING_CLIENT;
	WebSock->GenerateKey(m_KeySeed++);
	SetSocketDataFor(S, WebSock, m_WebSocketMap, m_WebSocketMapMutex);
	WebSock->m_Socket = S;
	if(!LWLogCriticalIf<256>(PushOutPacket(nullptr, 0, WebSock, LWEWebPacket::CONTROL_CONNECT), "Failed to insert connecting packet for '{}:{}', Terminating connection.", Domain, Port)) {
		S->MarkClosable();
		return LWRef<LWEWebSocket>();
	}
	return WebSock;
}

bool LWEProtocol_WebSocket::PushOutPacket(const void *Buffer, uint32_t BufferLen, LWRef<LWEWebSocket> &Socket, uint32_t ControlFlag) {
	LWRef<LWEWebPacket> RMessage = m_Allocator.CreateRef<LWEWebPacket>(Buffer, BufferLen, m_Allocator, ControlFlag | LWEWebPacket::CONTROL_FINISHED, Socket);
	return m_OutPackets.PushMove(RMessage);
}

bool LWEProtocol_WebSocket::PushOutPacket(const LWUTF8Iterator &Text, LWRef<LWEWebSocket> &Socket) {
	return PushOutPacket(Text(), (uint32_t)Text.RawLength(), Socket, LWEWebPacket::CONTROL_TEXT);
}

LWEProtocol_WebSocket &LWEProtocol_WebSocket::SetServer(const LWUTF8Iterator &Server) {
	Server.Copy(m_Server, sizeof(m_Server));
	return *this;
}

LWEProtocol_WebSocket &LWEProtocol_WebSocket::SetUserAgent(const LWUTF8Iterator &Agent) {
	Agent.Copy(m_UserAgent, sizeof(m_UserAgent));
	return *this;
}

LWEProtocol_WebSocket &LWEProtocol_WebSocket::SetSubProtocol(const LWUTF8Iterator &SubProtocol) {
	SubProtocol.Copy(m_SubProtocol, sizeof(m_SubProtocol));
	return *this;
}

LWEProtocol_WebSocket::LWEProtocol_WebSocket(uint32_t ProtocolID, LWAllocator &Allocator) : LWProtocol(ProtocolID), m_Allocator(Allocator) {}