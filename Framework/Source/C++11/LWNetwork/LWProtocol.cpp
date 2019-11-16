#include "LWNetwork/LWProtocol.h"

LWProtocol &LWProtocol::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	return *this;
}

LWProtocol &LWProtocol::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	return *this;
}
