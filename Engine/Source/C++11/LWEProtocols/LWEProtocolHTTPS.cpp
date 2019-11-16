#include "LWEProtocols/LWEProtocolHTTPS.h"
#include <LWCore/LWText.h>
#include <iostream>

LWProtocol &LWEProtocolHttps::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketChanged(Prev, New, Manager);
	New.SetProtocolData(m_hProtocolID, Prev.GetProtocolData(m_hProtocolID));
	return *this;
}

LWProtocol &LWEProtocolHttps::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	LWEProtocolTLS::SocketClosed(Socket, Manager);
	
	return *this;
}

LWProtocol &LWEProtocolHttps::ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen) {
	ProcessRead(Socket, Data, DataLen);
	return *this;
}

bool LWEProtocolHttps::ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t Len) {
	LWEHttpRequest *Req = (LWEHttpRequest*)Socket.GetProtocolData(m_hProtocolID);
	LWEHttpRequest IReq;
	bool Insert = false;
	if (!Req) {
		Req = &IReq;
		Req->m_Socket = &Socket;
		Insert = true;
	}
	if (!Req->Deserialize(Buffer, Len)) {
		std::cout << "Error deserializing response." << std::endl;
		return false;
	}
	if (Insert) {
		if (!GetNextFromPool(&Req)) {
			std::cout << "Could not get a request to write to from pool." << std::endl;
			return false;
		}
		*Req = IReq;
		Socket.SetProtocolData(m_hProtocolID, Req);
	}
	if (Req->m_Flag&LWEHttpRequest::ResponseReady) {
		if (Req->m_Status == 0) {
			if (!m_InRequests.Push(*Req, &Req)) {
				std::cout << "Failed to insert into request." << std::endl;
				return false;
			}
			Socket.SetProtocolData(m_hProtocolID, Req);
		}
		if (Req->m_Callback) Req->m_Callback(*Req, Req->m_Body);
		if (Req->CloseConnection() && Req->m_Status != 0) {
			Socket.MarkClosable();
		}

	}
	return true;
}

LWEProtocolHttps &LWEProtocolHttps::ProcessRequests(uint32_t ProtocolID, LWProtocolManager &Manager) {
	char Buffer[1024 * 64];
	LWEHttpRequest *Request;
	while (m_OutRequests.Pop(&Request)) {
		LWSocket Sock;
		bool IsResponse = Request->m_Status != 0;
		uint32_t Len = Request->Serialize(Buffer, sizeof(Buffer), IsResponse ? m_Agent : m_Server);
		if (!Len) continue;
		if (!Request->m_Socket) {
			uint32_t Error = LWSocket::CreateSocket(Sock, LWText(Request->m_Host), Request->m_Port, (uint32_t)LWSocket::Tcp, LWEProtocolTLS::m_ProtocolID);
			if (Error) {
				std::cout << "Error connecting to: '" << Request->m_Host << ":" << Request->m_Port << "' " << Error << std::endl;
				continue;
			}
			Request->m_Socket = Manager.PushSocket(Sock);
			//std::cout << "Making socket: " << Request->m_Socket->GetSocketDescriptor() << std::endl;
			Request->m_Socket->SetProtocolData(m_hProtocolID, Request);
			if (!Request->m_Socket) {
				std::cout << "Error inserting socket." << std::endl;
				continue;
			}
		}
		uint32_t Res = LWEProtocolTLS::Send(*Request->m_Socket, Buffer, Len);
		if (Res == -1) {
			Request->m_Socket->MarkClosable();
			continue;
		} else if (Res == 0) {
			LWEHttpRequest *NewRequest;
			if (!m_OutRequests.Push(*Request, &NewRequest)) {
				std::cout << "Failed to create new request." << std::endl;
				continue;
			}
			//Allow a read operation to occur, so we'll break and come back later.
			Request->m_Socket->SetProtocolData(m_hProtocolID, NewRequest);
			return *this; //Connection is not ready, so we put it on the back of the list and wait until it is ready.
		}
		if (Request->CloseConnection()) Request->m_Socket->MarkClosable();
	}
	return *this;
}

bool LWEProtocolHttps::GetNextRequest(LWEHttpRequest &Request) {
	return m_InRequests.Pop(Request);
}

bool LWEProtocolHttps::GetNextFromPool(LWEHttpRequest **Request) {
	LWEHttpRequest Req;
	if (!m_Pool.Push(Req, Request)) return false;
	return m_Pool.Pop(Req);
}

bool LWEProtocolHttps::PushRequest(LWEHttpRequest &Request) {
	LWEHttpRequest Req = Request;
	return m_OutRequests.Push(Req);
}

bool LWEProtocolHttps::PushResponse(LWEHttpRequest &InRequest, const char *Response, uint32_t lStatus) {
	LWEHttpRequest Request = InRequest;
	Request.m_Status = lStatus;
	Request.SetBody(Response);
	return m_OutRequests.Push(Request);
}

LWEProtocolHttps &LWEProtocolHttps::SetAgentString(const char *Agent) {
	m_Agent[0] = '\0';
	strncat(m_Agent, Agent, sizeof(m_Agent));
	return *this;
}

LWEProtocolHttps &LWEProtocolHttps::SetServerString(const char *Server) {
	m_Server[0] = '\0';
	strncat(m_Server, Server, sizeof(m_Server));
	return *this;
}

LWEProtocolHttps::LWEProtocolHttps(uint32_t HttpsProtocolID, uint32_t TLSProtocolID, LWProtocolManager *ProtoManager, LWAllocator &Allocator, const char *CertFile, const char *KeyFile) : LWEProtocolTLS(TLSProtocolID, Allocator, CertFile, KeyFile), m_hProtocolID(HttpsProtocolID), m_Manager(ProtoManager) {
	m_Agent[0] = '\0';
	m_Server[0] = '\0';
}
