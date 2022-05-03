#include "LWCore/LWTimer.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWCore/LWUnicode.h"
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
	fmt::print("Initiating TCP telnet test, open for {} Connection before moving on.\n", TelnetConnCount);
	LWSocket Listen;
	int32_t Result = LWSocket::CreateSocket(Listen, 5051, LWSocket::Tcp | LWSocket::Listen, 0);
	if (Result) {
		fmt::print("Failed to create socket with error: {} Network error: {}\n", Errors[Result], LWProtocolManager::GetError());
		return false;
	}
	LWSocket::MakeAddress(Listen.GetLocalIP(), Buffer, sizeof(Buffer));
	fmt::print("Created socket at: {}:{} waiting for TCP connections.\n", Buffer, Listen.GetLocalPort());

	for (int32_t i = 0; i < TelnetConnCount; i++) {
		LWSocket Recv;
		if (!Listen.Accept(Recv)) {
			fmt::print("Failed to accept incoming connection.\n");
			continue;
		}
		LWSocket::MakeAddress(Recv.GetRemoteIP(), Buffer, sizeof(Buffer));
		fmt::print("Accepted socket from: {}:{}\n", Buffer, Recv.GetRemotePort());
		char ResponseA[] = "Thank you for contacting LWFramework network test, please say something.\n";
		Recv.Send(ResponseA, sizeof(ResponseA));
		Recv.Receive(Buffer, sizeof(Buffer) - 32);
		snprintf(BufferB, sizeof(BufferB), "You said: %s", Buffer);
		Recv.Send(BufferB, (uint32_t)strlen(BufferB) + 1);
	}
	Listen.Close();
	fmt::print("Finished TCP Telnet test.\n");
	return true;
}

bool TestUDP(void) {
	char Buffer[256];
	char BufferB[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	uint32_t RemoteIP = 0;
	uint16_t RemotePort = 0;
	fmt::print("Beginning local UDP tests.\n");
	LWSocket UDPA;
	LWSocket UDPB;
	uint32_t Result = LWSocket::CreateSocket(UDPA, 0, 0);
	if (Result) {
		fmt::print("Failed to cread UDP.A socket with error: {} Network error: {}\n", Errors[Result], LWProtocolManager::GetError());
		return false;
	}
	Result = LWSocket::CreateSocket(UDPB, 5052, LWSocket::Udp, 0);
	if (Result) {
		fmt::print("Failed to cread UDP.B socket with error: {} Network error: {}\n", Errors[Result], LWProtocolManager::GetError());
		return false;
	}
	LWSocket::MakeAddress(UDPA.GetLocalIP(), Buffer, sizeof(Buffer));
	fmt::print("UDP.A create at: {}:{}\n", Buffer, UDPA.GetLocalPort());
	LWSocket::MakeAddress(UDPB.GetLocalIP(), Buffer, sizeof(Buffer));
	fmt::print("UDP.B create at: {}:{}\n", Buffer, UDPB.GetLocalPort());
	fmt::print("Sending from UDP.B to UDP.A.\n");
	char RequestData[] = "Test Data 123ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	UDPB.Send(RequestData, sizeof(RequestData), LWSocket::LocalIP, UDPA.GetLocalPort());
	UDPA.Receive(Buffer, sizeof(Buffer), &RemoteIP, &RemotePort);
	if(!LWUTF8I(Buffer).Compare(RequestData)){
		fmt::print("Error receiving data from UDPB: {}\nData received: '{}'\n", LWProtocolManager::GetError(), Buffer);
		return false;
	}
	LWSocket::MakeAddress(RemoteIP, BufferB, sizeof(BufferB));
	fmt::print("Received data: '{}' From: {}:{}\n", Buffer, BufferB, RemotePort);
	fmt::print("Sending from UDP.A to UDP.B.\n");
	UDPA.Send(Buffer, (uint32_t)strlen(Buffer) + 1, RemoteIP, RemotePort);
	Buffer[0] = '\0';//Clear our buffer.
	UDPB.Receive(Buffer, sizeof(Buffer), &RemoteIP, &RemotePort);
	if (!LWUTF8I(Buffer).Compare(RequestData)) {
		fmt::print("Error receiving data from UDP.A: {}\nData received: '{}'\n", LWProtocolManager::GetError(), Buffer);
		return false;
	}
	LWSocket::MakeAddress(RemoteIP, BufferB, sizeof(BufferB));
	fmt::print("Received Data: '{}' From: {}:{}\n", Buffer, BufferB, RemotePort);
	uint32_t Error = LWProtocolManager::GetError();
	if (Error) {
		fmt::print("Network error dected: {}\n", Error);
		return false;
	}
	fmt::print("Finished udp test successfully.\n");
	return true;
}

bool TestHTML(void) {
	char LargeBuffer[16 * 1024]; //16kb
	char Buffer[256];
	char BufferB[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	char8_t HostDomain[] = "google.com";
	const uint32_t IPBufferMaxLen = 32;
	uint32_t IPBuffer[IPBufferMaxLen];
	fmt::print("Beginning simple HTTP request.\n");
	fmt::print("Looking up domain: {}\n", HostDomain);
	uint32_t AddrResult = LWSocket::LookUpAddress(HostDomain, IPBuffer, IPBufferMaxLen, Buffer, sizeof(Buffer));
	if (AddrResult == -1) {
		fmt::print("No results found: {}\n", LWProtocolManager::GetError());
		return false;
	}
	char *B = Buffer;
	for (uint32_t i = 0; i < AddrResult; i++) {
		LWSocket::MakeAddress(IPBuffer[i], BufferB, sizeof(BufferB));
		std::cout << i << ": " << BufferB << " - " << B << std::endl;
		if (*B != '\0') B += strlen(B) + 2;
	}
	LWSocket HSock;
	uint32_t Result = LWSocket::CreateSocket(HSock, HostDomain, 80, LWSocket::Tcp, 0);
	if (Result) {
		fmt::print("Failed to create HTTP socket with error: {} Network error: {}\n", Errors[Result], LWProtocolManager::GetError());
		return 0;
	}
	LWSocket::MakeAddress(HSock.GetLocalIP(), Buffer, sizeof(Buffer));
	LWSocket::MakeAddress(HSock.GetRemoteIP(), BufferB, sizeof(BufferB));
	fmt::print("Created HTTP socket at: {}:{} To {}:{}\n", Buffer, HSock.GetLocalPort(), BufferB, HSock.GetRemotePort());
	char HTTPREQDATA[] = "GET / HTTP 1.1\n\n";
	HSock.Send(HTTPREQDATA, sizeof(HTTPREQDATA));
	uint32_t ResLen = HSock.Receive(LargeBuffer, sizeof(LargeBuffer));
	uint32_t Error = LWProtocolManager::GetError();
	if (Error) {
		fmt::print("Network error detected: {}\n", Error);
		return false;
	}
	ResLen = std::min<uint32_t>(ResLen, sizeof(LargeBuffer) - 1);
	LargeBuffer[ResLen] = '\0';
	fmt::print("Received Data: {}\n{}\n", ResLen, LargeBuffer);
	return true;
}

bool TestTelnet(void) {
	uint32_t TelnetProtocolTime = 0; //30 seconds.
	char Buffer[256];
	char Errors[][32] = { "None", "Connect", "Address", "Socket", "Bind", "Listen", "GetSock", "GetPeer", "CtrlFlag" };
	fmt::print("Beginning telnet concurrent protocol tests for {}s.\n", TelnetProtocolTime);
	LWTelnetProtocol TelProto;
	LWProtocolManager TelnetProtoManager;
	TelnetProtoManager.RegisterProtocol(&TelProto, 0);
	LWSocket Listen;
	uint32_t Result = LWSocket::CreateSocket(Listen, 5051, LWSocket::Tcp | LWSocket::Listen, 0);
	if (Result) {
		fmt::print("Failed to create listening socket: {} Network error: {}\n", Errors[Result], LWProtocolManager::GetError());
		return false;
	}
	LWSocket::MakeAddress(Listen.GetLocalIP(), Buffer, sizeof(Buffer));
	fmt::print("Created listening socket at: {}:{}\n", Buffer, Listen.GetLocalPort());
	TelnetProtoManager.PushSocket(Listen);
	uint64_t Start = LWTimer::GetCurrent();
	uint64_t Freq = LWTimer::GetResolution()*TelnetProtocolTime;
	uint64_t SubFreq = LWTimer::GetResolution() * 5; //5 second interval;
	while (LWTimer::GetCurrent() < Start + Freq) {
		if (LWTimer::GetCurrent() > Start + SubFreq) {
			fmt::print("{}s have elapsed.\n", SubFreq / LWTimer::GetResolution());
			SubFreq += LWTimer::GetResolution() * 5;
		}
		if (!TelnetProtoManager.Poll(0)) {
			fmt::print("Network error on poll: {}\n", LWProtocolManager::GetError());
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
		fmt::print("Received packet: {} ID: {}\n", Pack->GetType(), Pack->GetPacketID());
		return false;
	};

	auto Send = [](LWPacket *Pack, LWPacketManager *Man) -> bool {
		char Buf[1024];
		char BufB[1024];
		fmt::print("Sending packet: {} ID: {}\n", Pack->GetType(), Pack->GetPacketID());
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

	fmt::print("Testing LWPacket Manager.\n");
	LWPacketManager Manager(1024, Allocator, Allocator, Receive, Send);
	Manager.RegisterDeserialization(1, LWPacket::Deserialize);
	LWPacket *Pack = Allocator.Create<LWPacket>(10, nullptr, 1, LWPacket::Ack);
	uint32_t Len = Manager.SerializePacket(Pack, Buffer, sizeof(Buffer));
	fmt::print("Serialized packet: {}\n", Len);
	if (!Manager.ProcessRawData(nullptr, nullptr, Buffer, Len)) {
		fmt::print("Error processing raw data packet.\n");
	}
	Manager.PushOutPacket(Pack);
	uint64_t Start = LWTimer::GetCurrent();
	uint64_t Freq = LWTimer::GetResolution()*ProtoManagerTime;
	bool Out = false;
	while (LWTimer::GetCurrent() < Start + Freq) {
		Manager.Update(LWTimer::GetCurrent(), LWTimer::GetResolution());
		if (!Out) {
			fmt::print("Waiting for any ack packets, only 1 send packet should appear after this line:\n");
			Out = true;
		}
	}
	fmt::print("Finished LWPacketManager tests.\n");
	return true;
}

int32_t LWMain(int32_t, LWUTF8Iterator *){
	LWAllocator_Default Allocator;
	fmt::print("Initiating Network tests.\n");
	if(!LWProtocolManager::InitateNetwork()){
		fmt::print("Failed to initialize network.\n");
		return 0;
	}
	if (!TestListen()) fmt::print("Failed 'TestList'.\n");
	else if (!TestUDP()) fmt::print("Failed 'TestUDP'.\n");
	else if (!TestHTML()) fmt::print("Failed 'TestHTML'.\n");
	else if (!TestTelnet()) fmt::print("Failed 'TestTelnet'.\n");
	else if (!TestLWPacketManager(Allocator)) fmt::print("Failed 'TestLWPacketManager'.\n");
	LWProtocolManager::TerminateNetwork();
	fmt::print("Finished Network tests.\n");
	return 0;
}