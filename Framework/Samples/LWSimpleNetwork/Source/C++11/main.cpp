#include "LWCore/LWText.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWPlatform/LWPlatform.h"
#include "LWNetwork/LWProtocolManager.h"
#include "LWNetwork/LWProtocol.h"
#include "LWNetwork/LWSocket.h"
#include "LWNetwork/LWPacket.h"
#include "LWNetwork/LWPacketManager.h"
#include <iostream>

class LWTelnetProtocol : public LWProtocol{
public:
	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager){
		char Buffer[256];
		LWSocket Sock;
		if (Socket.GetFlag()&LWSocket::Listen){ //Our listening socket!
			std::cout << "New connection found!" << std::endl;
			if(!Socket.Accept(Sock)){
				std::cout << "Error accepting socket: " << LWProtocolManager::GetError() << std::endl;
				return *this;
			} else{
				LWSocket::MakeAddress(Sock.GetRemoteIP(), Buffer, sizeof(Buffer));
				if(!Manager->PushSocket(Sock)){
					std::cout << "Failed to add socket!" << std::endl;
				}
				std::cout << "New connection: " << Buffer << ":" << Sock.GetRemotePort() << std::endl;
			}
		} else{
			LWSocket::MakeAddress(Socket.GetRemoteIP(), Buffer, sizeof(Buffer));
			std::cout << "Received input from: " << Buffer << ":" << Socket.GetRemotePort() << std::endl;
			uint32_t r = Socket.Receive(Buffer, sizeof(Buffer));
			if(r==0){
				std::cout << "Socket has closed the connection!" << std::endl;
				Socket.MarkClosable();
				return *this;
			}
			std::cout << "Received: '" << Buffer << "'" << std::endl;
			Socket.Send(Buffer, r);
		}
		return *this;
	}

private:
};

bool TestListen(void) {
	char Buffer[256];
	char BufferB[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	int32_t TelnetConnCount = 0;
	std::cout << "Initiating TCP telnet test, open for " << TelnetConnCount << " Connections before moving on." << std::endl;
	LWSocket Listen;
	int32_t Result = LWSocket::CreateSocket(Listen, 5051, LWSocket::Tcp | LWSocket::Listen, 0);
	if (Result) {
		std::cout << "Failed to create socket with error: " << Errors[Result] << " Network: " << LWProtocolManager::GetError() << std::endl;
		return false;
	}
	LWSocket::MakeAddress(Listen.GetLocalIP(), Buffer, sizeof(Buffer));
	std::cout << "Created socket at: " << Buffer << ":" << Listen.GetLocalPort() << " waiting for TCP connections." << std::endl;

	for (int32_t i = 0; i < TelnetConnCount; i++) {
		LWSocket Recv;
		if (!Listen.Accept(Recv)) {
			std::cout << "Failed to accept incoming connection." << std::endl;
			continue;
		}
		LWSocket::MakeAddress(Recv.GetRemoteIP(), Buffer, sizeof(Buffer));
		std::cout << "Accepted socket from: " << Buffer << ":" << Recv.GetRemotePort() << std::endl;
		char ResponseA[] = "Thank you for contacting LWFramework network test, please say something.\n";
		Recv.Send(ResponseA, sizeof(ResponseA));
		Recv.Receive(Buffer, sizeof(Buffer) - 32);
		snprintf(BufferB, sizeof(BufferB), "You said: %s", Buffer);
		Recv.Send(BufferB, strlen(BufferB) + 1);
	}
	Listen.Close();
	std::cout << "finished TCP Telnet test!" << std::endl;
	return true;
}

bool TestUDP(void) {
	char Buffer[256];
	char BufferB[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	uint32_t RemoteIP = 0;
	uint16_t RemotePort = 0;
	
	std::cout << "Beginning local UDP tests!" << std::endl;
	LWSocket UDPA;
	LWSocket UDPB;
	uint32_t Result = LWSocket::CreateSocket(UDPA, 0, 0);
	if (Result) {
		std::cout << "Failed to create UDPA socket with error: " << Errors[Result] << " Network: " << LWProtocolManager::GetError() << std::endl;
		return false;
	}
	Result = LWSocket::CreateSocket(UDPB, 5052, LWSocket::Udp, 0);
	if (Result) {
		std::cout << "Failed to create UDPB socket with error: " << Errors[Result] << " Network: " << LWProtocolManager::GetError() << std::endl;
		return false;
	}
	LWSocket::MakeAddress(UDPA.GetLocalIP(), Buffer, sizeof(Buffer));
	std::cout << "UDPA created at: " << Buffer << ":" << UDPA.GetLocalPort() << std::endl;
	LWSocket::MakeAddress(UDPB.GetLocalIP(), Buffer, sizeof(Buffer));
	std::cout << "UDPB created at: " << Buffer << ":" << UDPB.GetLocalPort() << std::endl;
	std::cout << "Sending from UDPB to UDPA!" << std::endl;
	char RequestData[] = "Test Data 123ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	UDPB.Send(RequestData, sizeof(RequestData), LWSocket::LocalIP, UDPA.GetLocalPort());
	UDPA.Receive(Buffer, sizeof(Buffer), &RemoteIP, &RemotePort);
	if (LWText(Buffer) != LWText(RequestData)) {
		std::cout << "Error receiving data from UDPB: " << LWProtocolManager::GetError() << std::endl << "Data received: '" << Buffer << "'" << std::endl;
		return false;
	}
	LWSocket::MakeAddress(RemoteIP, BufferB, sizeof(BufferB));
	std::cout << "Received Data: '" << Buffer << "' From: " << BufferB << ":" << RemotePort << std::endl;
	std::cout << "Sending from UDPA to UDPB!" << std::endl;
	UDPA.Send(Buffer, strlen(Buffer) + 1, RemoteIP, RemotePort);
	Buffer[0] = '\0';//Clear our buffer.
	UDPB.Receive(Buffer, sizeof(Buffer), &RemoteIP, &RemotePort);
	if (LWText(Buffer) != LWText(RequestData)) {
		std::cout << "Error receiving data from UDPA: " << LWProtocolManager::GetError() << std::endl << "Data received: '" << Buffer << "'" << std::endl;
		return false;
	}
	LWSocket::MakeAddress(RemoteIP, BufferB, sizeof(BufferB));
	std::cout << "Received Data: '" << Buffer << "' From: " << BufferB << ":" << RemotePort << std::endl;
	uint32_t Error = LWProtocolManager::GetError();
	if (Error) {
		std::cout << "Network error detected: " << Error << std::endl;
		return false;
	}
	std::cout << "Finished udp test successfully!" << std::endl;
	return true;
}

bool TestHTML(void) {
	char LargeBuffer[16 * 1024]; //16kb
	char Buffer[256];
	char BufferB[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	char HostDomain[] = "google.com";
	const uint32_t IPBufferMaxLen = 32;
	uint32_t IPBuffer[IPBufferMaxLen];
	std::cout << "Beginning simple HTTP request." << std::endl;
	std::cout << "Looking up domain: " << HostDomain << std::endl;
	uint32_t AddrResult = LWSocket::LookUpAddress(LWText(HostDomain), IPBuffer, IPBufferMaxLen, Buffer, sizeof(Buffer));
	if (AddrResult == 0xFFFFFFFF) {
		std::cout << "No results found, potential error: " << LWProtocolManager::GetError() << std::endl;
		return false;
	}
	char *B = Buffer;
	for (uint32_t i = 0; i < AddrResult; i++) {
		LWSocket::MakeAddress(IPBuffer[i], BufferB, sizeof(BufferB));
		std::cout << i << ": " << BufferB << " - " << B << std::endl;
		if (*B != '\0') B += strlen(B) + 2;
	}
	LWSocket HSock;
	uint32_t Result = LWSocket::CreateSocket(HSock, LWText(HostDomain), 80, LWSocket::Tcp, 0);
	if (Result) {
		std::cout << "Failed to create HTTP socket with error: " << Errors[Result] << " Network: " << LWProtocolManager::GetError() << std::endl;
		return 0;
	}
	LWSocket::MakeAddress(HSock.GetLocalIP(), Buffer, sizeof(Buffer));
	LWSocket::MakeAddress(HSock.GetRemoteIP(), BufferB, sizeof(BufferB));
	std::cout << "Created HTTP Socket at: " << Buffer << ":" << HSock.GetLocalPort() << " To " << BufferB << ":" << HSock.GetRemotePort() << std::endl;
	char HTTPREQDATA[] = "GET\n";
	HSock.Send(HTTPREQDATA, sizeof(HTTPREQDATA));
	uint32_t ResLen = HSock.Receive(LargeBuffer, sizeof(LargeBuffer));
	uint32_t Error = LWProtocolManager::GetError();
	if (Error) {
		std::cout << "Network error detected: " << Error << std::endl;
		return false;
	}
	ResLen = std::min<uint32_t>(ResLen, sizeof(LargeBuffer) - 1);
	LargeBuffer[ResLen] = '\0';
	std::cout << "Received Data:" << std::endl << LargeBuffer << std::endl;
	return true;
}

bool TestTelnet(void) {
	uint32_t TelnetProtocolTime = 0; //30 seconds.
	char Buffer[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	std::cout << "Beginning telnet concurrent protocol tests for " << TelnetProtocolTime << " seconds" << std::endl;
	LWTelnetProtocol TelProto;
	LWProtocolManager TelnetProtoManager;
	TelnetProtoManager.RegisterProtocol(&TelProto, 0);
	LWSocket Listen;
	uint32_t Result = LWSocket::CreateSocket(Listen, 5051, LWSocket::Tcp | LWSocket::Listen, 0);
	if (Result) {
		std::cout << "Failed to create listening socket: " << Errors[Result] << " Network: " << LWProtocolManager::GetError() << std::endl;
		return false;
	}
	LWSocket::MakeAddress(Listen.GetLocalIP(), Buffer, sizeof(Buffer));
	std::cout << "Created listening socket at: " << Buffer << ":" << Listen.GetLocalPort() << std::endl;
	TelnetProtoManager.PushSocket(Listen);
	uint64_t Start = LWTimer::GetCurrent();
	uint64_t Freq = LWTimer::GetResolution()*TelnetProtocolTime;
	uint64_t SubFreq = LWTimer::GetResolution() * 5; //5 second interval;
	while (LWTimer::GetCurrent() < Start + Freq) {
		if (LWTimer::GetCurrent() > Start + SubFreq) {
			std::cout << SubFreq / LWTimer::GetResolution() << " Seconds have elapsed." << std::endl;
			SubFreq += LWTimer::GetResolution() * 5;
		}
		if (!TelnetProtoManager.Poll(0)) {
			std::cout << "Network error on poll: " << LWProtocolManager::GetError() << std::endl;
			return false;
		}
	}
	return true;
}

bool TestLWPacketManager(LWAllocator &Allocator) {
	char Buffer[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };

	uint32_t ProtoManagerTime = 10; //10 second
	auto Receive = [](LWPacket *Pack, LWPacketManager *) -> bool {
		std::cout << "Received packet: " << Pack->GetType() << " ID: " << Pack->GetPacketID() << std::endl;
		return false;
	};

	auto Send = [](LWPacket *Pack, LWPacketManager *Man) -> bool {
		char Buf[1024];
		char BufB[1024];
		std::cout << "Sending packet: " << Pack->GetType() << " ID: " << Pack->GetPacketID() << std::endl;
		if (Pack->GetFlag()&LWPacket::Ack) {
			LWPacket AckPack(Pack->GetPacketAckID(), nullptr, LWPacket::PacketAck, 0);
			uint32_t Len = Man->SerializePacket(&AckPack, Buf, sizeof(Buf));
			uint32_t LenB = Man->SerializePacket(Pack, BufB, sizeof(BufB));
			if (!Man->ProcessRawData(nullptr, nullptr, Buf, 8)) std::cout << "Failed to add A" << std::endl;
			if (!Man->ProcessRawData((void*)0x1, nullptr, BufB, 8)) std::cout << "Failed to add B" << std::endl;
			if (!Man->ProcessRawData(nullptr, nullptr, Buf + 8, Len - 8)) std::cout << "Failed to add C" << std::endl;
			if (!Man->ProcessRawData((void*)0x1, nullptr, BufB + 8, LenB - 8)) std::cout << "Failed to add D" << std::endl;
		}
		return true;
	};

	std::cout << "Beginning LWPacketManager tests!" << std::endl;
	LWPacketManager Manager(1024, Allocator, Allocator, Receive, Send);
	Manager.RegisterDeserialization(1, LWPacket::Deserialize);
	LWPacket *Pack = Allocator.Allocate<LWPacket>(10, nullptr, 1, LWPacket::Ack);
	uint32_t Len = Manager.SerializePacket(Pack, Buffer, sizeof(Buffer));
	std::cout << "Serialized packet, len: " << Len << std::endl;
	if (!Manager.ProcessRawData(nullptr, nullptr, Buffer, Len)) {
		std::cout << "Error processing raw data packet!" << std::endl;
	}
	Manager.PushOutPacket(Pack);
	uint64_t Start = LWTimer::GetCurrent();
	uint64_t Freq = LWTimer::GetResolution()*ProtoManagerTime;
	bool Out = false;
	while (LWTimer::GetCurrent() < Start + Freq) {
		Manager.Update(LWTimer::GetCurrent(), LWTimer::GetResolution());
		if (!Out) {
			std::cout << "Waiting for any ack packets, only 1 send packet should appear after this line:" << std::endl;
			Out = true;
		}
	}
	std::cout << "Finished LWPacketManager tests!" << std::endl;
	return true;
}

int LWMain(int, char **){
	LWAllocator_Default Allocator;
	std::cout << "Initiating Network test." << std::endl;
	if(!LWProtocolManager::InitateNetwork()){
		std::cout << "Failed to initialize network." << std::endl;
		return 0;
	}
	if (!TestListen()) std::cout << "Failed Listener test." << std::endl;
	if (!TestUDP()) std::cout << "Failed udp test." << std::endl;
	if (!TestHTML()) std::cout << "Failed html test." << std::endl;
	if (!TestTelnet()) std::cout << "Failed telnet test." << std::endl;
	if (!TestLWPacketManager(Allocator)) std::cout << "Failed LWPacketManager test." << std::endl;
	LWProtocolManager::TerminateNetwork();
	std::cout << "Finished Network test!" << std::endl;
	return 0;
}