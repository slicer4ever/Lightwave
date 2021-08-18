#ifndef LWPROTOCOL_H
#define LWPROTOCOL_H
#include "LWCore/LWTypes.h"
#include "LWNetwork/LWTypes.h"

/*!< \brief protocol is an abstract class to be derived into actual protocols for communicating across many bands of different communication. */
class LWProtocol{
public:
	/*!< \brief Read is called any time a socket is detected to have data on it. how that data is handled is upto the protocol.*/
	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager) = 0;

	/*!< \brief If a socket is a listen socket, Protocol manager will call this method when a new socket is waiting to connect to this protocol. (default implementation add's NewSocket to ProtocolManager)*/
	virtual LWProtocol &Accept(LWSocket &NewSocket, LWProtocolManager *Manager);

	/*!< \brief callback for when a socket is closed, allowing for any cleanup code to be committed. */
	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	/*!< \brief callback for when a socket changes position in the protocol manager's position, implement if underlying protocol data relys on the socket's pointer.
		 \note Prev will have already been moved into new, as such any protocol data should be accessed from New instead of from Prev.
	*/
	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);
private:

};

#endif