#include "LWNetwork/LWProtocol.h"
#include "LWNetwork/LWProtocolManager.h"
#include "LWCore/LWLogger.h"

bool LWProtocol::Accept(LWRef<LWSocket> &Listener, LWSocket &NewSocket, LWProtocolManager &Manager) {
	return true;
}

LWProtocol &LWProtocol::SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	return *this;
}

LWProtocol &LWProtocol::SocketAdded(LWRef<LWSocket> &NewSocket, LWProtocolManager &Manager) {
	return *this;
}

LWProtocol::LWProtocol(uint32_t ProtocolID) : ProtocolID(ProtocolID){}