#include "LWNetwork/LWProtocol.h"
#include "LWNetwork/LWProtocolManager.h"

LWProtocol &LWProtocol::Accept(LWSocket &NewSocket, LWProtocolManager *Manager) {
	if (!Manager->PushSocket(NewSocket)) {
		fmt::print("Error inserting new socket to protocol manager!.\n");
	}
	return *this;
}

LWProtocol &LWProtocol::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	return *this;
}

LWProtocol &LWProtocol::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	return *this;
}
