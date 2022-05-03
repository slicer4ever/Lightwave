#ifndef LWPROTOCOLMANAGER_H
#define LWPROTOCOLMANAGER_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWRef.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWConcurrent/LWFIFO.h"
#include "LWNetwork/LWTypes.h"
#include "LWNetwork/LWSocket.h"
#include "LWPlatform/LWPlatform.h"
#include <unordered_map>
#include <mutex>

/*!< \brief structure holding the URI request the socket is making to. */
struct LWPM_AsyncSockets {
	char8_t URI[64] = {}; //URI to connect to, if URI is blank then attempts to only do handshake with Result's remoteaddr.
	LWRef<LWSocket> Result;

	LWUTF8Iterator GetURI(void) const;

	LWPM_AsyncSockets(const LWUTF8Iterator &URI, LWRef<LWSocket> &Result);

	//Result RemoteAddr is used instead of a URI
	LWPM_AsyncSockets(LWRef<LWSocket> &Result);

	LWPM_AsyncSockets() = default;
};

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

	static bool PollSet(LWSocketPollHandle *SocketSet, uint32_t SetCnt, uint32_t Timeout);

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
		 \param lCurrentTime the current time, used to timeout sockets which have been waiting too long without sending/receiving new data.
		 \return true if polling was successful, false if failure.
	*/
	bool Poll(uint32_t PollTimeout, uint64_t lCurrentTime);

	/*!< \brief processes the async queue list. Timeout is in high resolution time, if timeout is 0 then time is considered infinite, otherwise returns at specified time when possible. */
	void ProcessAsyncSockets(bool Verbose = true, uint64_t lTimeout = 0, uint64_t lCurrentTime = 0);

	/*!< \brief pushes a socket into the protocol manager, this function now takes ownership of the socket internally via a move operation(as such operations on the original socket well be invalid), however the memory passed to this function is still owned by the application, and must be cleaned up by the application(if allocated on heap).
		 \return the socket object upon successfully being added, nullptr if the number of sockets is already exhausted.
	*/
	LWRef<LWSocket> PushSocket(LWSocket &Socket);

	/*!< \brief create's an async socket, initial socket status will be set to connecting, protocols should be async aware of possible incoming socket. 
		 \note: If URI contains a port, it will be used instead of RemotePort.
	*/
	LWRef<LWSocket> CreateAsyncSocket(const LWUTF8Iterator &URI, uint16_t RemotePort, uint32_t Type, uint16_t LocalPort, uint32_t Flags, uint32_t ProtocolID);

	/*!< \brief create's an async socket, initial socket status will be set to connecting, protocols should be async aware of possible incoming sockets. */
	LWRef<LWSocket> CreateAsyncSocket(const LWUTF8Iterator &URI, uint16_t RemotePort, uint32_t Type, uint32_t Flags, uint32_t ProtocolID);

	/*!< \brief create's an async socket without doing a URI address lookup, and only doing the handshake. */
	LWRef<LWSocket> CreateAsyncSocket(const LWSocketAddr &RemoteAddr, uint32_t Type, uint16_t LocalPort, uint32_t Flags, uint32_t ProtocolID);

	/*!< \brief create's an async socket without doing a URI address lookup, and only doing the handshake. */
	LWRef<LWSocket> CreateAsyncSocket(const LWSocketAddr &RemoteAddr, uint32_t Type, uint32_t Flags, uint32_t ProtocolID);

	/*!< \brief constructs a protocol which will be processed internally. */
	template<class Type, typename ...Args>
	LWRef<Type> CreateProtocol(Args&&... Arg) {
		LWRef<Type> P = m_Allocator.CreateRef<Type>(std::forward<Args>(Arg)...);
		m_Protocols[P->ProtocolID] = P;
		return P;
	}

	/*!< \brief returns the registered protocol with the specified id.*/
	LWRef<LWProtocol> GetProtocol(uint32_t ProtocolID);

	/*!< \brief returns the active socket at the specified index. */
	LWRef<LWSocket> GetSocket(uint32_t Index);

	/*!< \brief returns the total number of active sockets. */
	uint32_t GetActiveSocketCount(void) const;

	/*!< \brief returns the user data for the protocol. */
	void *GetUserData(void) const;

	/*!< \brief constructs a protocol manager object. */
	LWProtocolManager(LWAllocator &Allocator);

	~LWProtocolManager();
private:
	LWRef<LWSocket> PushSocket(LWRef<LWSocket> &Sock);

	LWAllocator &m_Allocator;
	LWConcurrentFIFO<LWPM_AsyncSockets, MaxSockets> m_AsyncQueue;
	LWSocketPollHandle m_SocketSet[MaxSockets];
	LWRef<LWSocket> m_Sockets[MaxSockets];
	LWRef<LWProtocol> m_Protocols[MaxProtocols];
	void *m_UserData = nullptr;
	uint32_t m_ActiveSocketCount = 0;
	std::mutex m_SocketLock;
};

#endif