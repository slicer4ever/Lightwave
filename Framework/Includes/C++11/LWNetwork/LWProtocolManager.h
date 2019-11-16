#ifndef LWPROTOCOLMANAGER_H
#define LWPROTOCOLMANAGER_H
#include "LWCore/LWTypes.h"
#include "LWNetwork/LWTypes.h"
#include "LWNetwork/LWSocket.h"
#include "LWPlatform/LWPlatform.h"


/*!< \brief protocol manager, supports managements of sockets, and the associated protocols they are associated with. also contains the functions to initiate and terminate the network status for the application. */
class LWProtocolManager{
public:
	enum{
		MaxSockets = 4096, /*!< \brief defines the total max sockets a protocol manager can support at one time. */
		MaxProtocols = 128 /*!< \brief defines the total max unique protocol's the protocol manager can support at one time, this number is also the max protcol id's allowed, each protocol must be given a unique id. */
	};
	/*!< \brief initiates the network stack for the application.
		 \return true if the network could be successfully initiated, false on failure.
	*/
	static bool InitateNetwork(void);

	/*!< \brief terminates the network stack for the application. */
	static void TerminateNetwork(void);

	static bool PollSet(pollfd *SocketSet, uint32_t SetCnt, uint32_t Timeout);

	/*!< \brief returns a list of host ip's avaiable to be connected to.
		 \param IPBuffer the buffer to receive the ip's.
		 \param BufferSize the size of buffer(in uint32_t size, not byte size! i.e: the number of ip's we can write to.
		 \return the total number of ip interfaces available.
	*/
	static uint32_t GetHostIPs(uint32_t *IPBuffer, uint32_t BufferSize);

	/*!< \brief returns the last network error on the stack. */
	static uint32_t GetError(void);

	/*!< \brief sets the user data for the protocol manager. */
	LWProtocolManager &SetUserData(void *UserData);

	/*!< \brief polls all active sockets for any data to be read, and calls the relevant protocols. 
		 \param Timeout the timeout time is in milliseconds, with 0 being instant, and 0xFFFFFFFF for infinite.
		 \return true if polling was successful, false if failure.
	*/
	bool Poll(uint32_t Timeout);

	/*!< \brief pushes a socket into the protocol manager, this function now takes ownership of the socket internally via a move operation(as such operations on the original socket well be invalid), however the memory passed to this function is still owned by the application, and must be cleaned up by the application(if allocated on heap).
		 \return the socket object upon successfully being added, nullptr if the number of sockets is already exhausted.
	*/
	LWSocket *PushSocket(LWSocket &Socket);

	/*!< \brief Registers a protocol with the protocol manager.  note that the protocol manager does not own the protocol, so it is still the programs responsibility to clean up each protocol it creates. */
	LWProtocolManager &RegisterProtocol(LWProtocol *Protocol, uint32_t ProtocolID);

	/*!< \brief returns the registered protocol with the specified id.*/
	LWProtocol *GetProtocol(uint32_t ProtocolID);

	/*!< \brief returns the active socket at the specified index. */
	LWSocket *GetSocket(uint32_t Index);

	/*!< \brief returns the total number of active sockets. */
	uint32_t GetActiveSocketCount(void) const;

	/*!< \brief returns the user data for the protocol. */
	void *GetUserData(void) const;

	/*!< \brief constructs a protocol manager object. */
	LWProtocolManager();

	~LWProtocolManager();
private:
	LWSocket m_Sockets[MaxSockets];
	pollfd m_SocketSet[MaxSockets];
	LWProtocol *m_Protocols[MaxProtocols];
	void *m_UserData = nullptr;
	uint32_t m_ActiveSocketCount = 0;
};

#endif