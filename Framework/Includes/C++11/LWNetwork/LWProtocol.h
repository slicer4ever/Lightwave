#ifndef LWPROTOCOL_H
#define LWPROTOCOL_H
#include "LWCore/LWTypes.h"
#include "LWNetwork/LWSocket.h"
#include "LWNetwork/LWTypes.h"
#include "LWCore/LWRef.h"
#include <shared_mutex>
#include <unordered_map>

/*!< \brief protocol is an abstract class to be derived into actual protocols for communicating across many bands of different communication. */
class LWProtocol{
public:
	/*!< \brief helper function for mapping protocol specific data to this socket, using the socket's hash as a key for unordered_maps. returns the data if found, otherwise null. */
	template<class Type>
	static bool GetSocketDataFor(Type &Result, LWRef<LWSocket> &Socket, std::unordered_map<uint32_t, Type> &ProtocolSocketMap, std::shared_mutex &Mutex) {
		uint32_t Hash = Socket->Hash();
		Mutex.lock_shared();
		auto Iter = ProtocolSocketMap.find(Hash);
		bool bResult = Iter!=ProtocolSocketMap.end();
		if(bResult) Result = Iter->second;
		Mutex.unlock_shared();
		return bResult;
	}

	/*!< \brief helper function for mapping protocol specific data to this socket, using the socket's hash as a key for unordered_maps. returns the data if found, otherwise null. */
	template<class Type>
	static Type GetSocketDataFor(LWRef<LWSocket> &Socket, std::unordered_map<uint32_t, Type> &ProtocolSocketMap, std::shared_mutex &Mutex) {
		Type Result;
		GetSocketDataFor(Result, Socket, ProtocolSocketMap, Mutex);
		return Result;
	}

	/*!< \brief helper function for mapping protocol specific data to this socket, using the socket's hash as a key for unordered_map.  set's the data for the socket. */
	template<class Type>
	static const Type &SetSocketDataFor(LWRef<LWSocket> &Socket, const Type &Data, std::unordered_map<uint32_t, Type> &ProtocolSocketMap, std::shared_mutex &Mutex) {
		uint32_t Hash = Socket->Hash();
		Mutex.lock();
		ProtocolSocketMap.insert_or_assign(Hash, Data);
		Mutex.unlock();
		return Data;
	}

	/*!< \brief helper function for mapping protocol specific data to this socket, using the socket's hash as a key for unordered_map. insert's data at the socket key, returning the previous data(or null if it was not there). */
	template<class Type>
	static Type ExchangeSocketsData(LWRef<LWSocket> &Socket, const Type &NewData, std::unordered_map<uint32_t, Type> &ProtocolSocketMap, std::shared_mutex &Mutex) {
		uint32_t Hash = Socket->Hash();
		Type Result = Type();
		Mutex.lock();
		auto Iter = ProtocolSocketMap.find(Hash);
		if(Iter==ProtocolSocketMap.end()) ProtocolSocketMap.emplace(Hash, NewData);
		else Result = std::exchange(Iter->second, NewData);
		Mutex.unlock();
		return Result;
	}

	/*!< \brief Read is called any time a socket is detected to have data on it. how that data is handled is upto the protocol.*/
	virtual LWProtocol &Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) = 0;

	/*!< \brief if a listener accepts a new socket, this function is called, returning false will drop the socket, true keeps it. (default implementation add's NewSocket to ProtocolManager)*/
	virtual bool Accept(LWRef<LWSocket> &Listener, LWSocket &NewSocket, LWProtocolManager &Manager);

	/*!< \brief callback for when a socket is closed, allowing for any cleanup code to be committed. */
	virtual LWProtocol &SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager);

	/*!< \brief called whenever a new socket is added to the protocol manager, this allows a protocol to capture both sockets added by Accept, and sockets added by the program. */
	virtual LWProtocol &SocketAdded(LWRef<LWSocket> &NewSocket, LWProtocolManager &Manager);

	/*!< \brief constructs protocol object. */
	LWProtocol(uint32_t ProtocolID);

	uint32_t ProtocolID = 0;
private:

};

#endif