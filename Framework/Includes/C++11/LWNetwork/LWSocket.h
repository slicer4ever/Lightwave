#ifndef LWSOCKET_H
#define LWSOCKET_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWUnicodeIterator.h"
#include "LWNetwork/LWTypes.h"
#include "LWPlatform/LWPlatform.h"

struct LWSocketAddr {
	static const uint32_t IP6Addr = 0x80000000; /*!< \brief indicates the socket address is an ip6 address. set in the upper 16 bits of the port. */
	static const uint32_t Valid = 0x40000000; /*!< \brief indicates the socket address has been filled out, and is not default constructed. */

	uint32_t IP[4] = {}; /*!< \brief IP4 or IP6 container, if IP4 then the IP is placed in the 3rd index, otherwise is a 128-bit IP6 address. */
	uint32_t Port = 0; //If IP6 then the upper bit of Port is set

	//Parse's a string for IPNotation, returning true if the IP notation is detected, false if not.
	static bool ParseIP(const LWUTF8Iterator &IPNotation, uint32_t *IPBuffer, bool &bIsIP6);

	/*!< \brief turns IP address into an IPNotation string, does not perform IP6 condensation of spaces. */
	template<std::size_t Len = 40>
	LWUTF8I::C_View<Len> MakeIPNotation(void) const {
		if (IsIP6Addr()) return LWUTF8I::Fmt<Len>("{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}", 
			(uint32_t)((IP[0] >> 16) & 0xFFFF), (uint32_t)(IP[0] & 0xFFFF), 
			(uint32_t)((IP[1] >> 16) & 0xFFFF), (uint32_t)(IP[1] & 0xFFFF), 
			(uint32_t)((IP[2] >> 16) & 0xFFFF), (uint32_t)(IP[2] & 0xFFFF), 
			(uint32_t)((IP[3] >> 16) & 0xFFFF), (uint32_t)(IP[3] & 0xFFFF));
		return LWUTF8I::Fmt<Len>("{}.{}.{}.{}", 
			(uint32_t)((IP[3] >> 24) & 0xFF), 
			(uint32_t)((IP[3] >> 16) & 0xFF), 
			(uint32_t)((IP[3] >> 8) & 0xFF), 
			(uint32_t)(IP[3] & 0xFF));
	}

	/*!< \brief returns the lower 16 bits of the Port address. */
	uint16_t GetPort(void) const;

	/*!< \brief creates a hash out of this address structure. */
	uint32_t Hash(void) const;

	/*!< \brief returns true if IP6 flag is raised. */
	bool IsIP6Addr(void) const;

	/*!< \brief returns ture if IP6 flag is not raised. */
	bool IsIP4Addr(void) const;

	/*!< \brief returns true if the socket address has been filled out, otherwise false if it is not. */
	bool IsValid(void) const;

	/*!< \brif ostream operator for printing out the socket address into a readable format. */
	friend std::ostream &operator << (std::ostream &o, const LWSocketAddr &Addr) {
		const uint32_t *IP = Addr.IP;
		if (Addr.IsIP4Addr()) o << (uint32_t)((IP[3] & 0xFF000000) >> 24) << "." << (uint32_t)((IP[3] & 0xFF0000) >> 16) << "." << (uint32_t)((IP[3] & 0xFF00) >> 8) << "." << (uint32_t)(IP[3] & 0xFF) << ":" << Addr.GetPort();
		else {
			o << std::hex << "[";
			o << (uint32_t)((IP[0] >> 16) & 0xFFFF) << ":" << (uint32_t)(IP[0] & 0xFFFF) << ":";
			o << (uint32_t)((IP[1] >> 16) & 0xFFFF) << ":" << (uint32_t)(IP[1] & 0xFFFF) << ":";
			o << (uint32_t)((IP[2] >> 16) & 0xFFFF) << ":" << (uint32_t)(IP[2] & 0xFFFF) << ":";
			o << (uint32_t)((IP[3] >> 16) & 0xFFFF) << ":" << (uint32_t)(IP[3] & 0xFFFF);
			o << std::dec << "]:" << Addr.GetPort();
		}
		return o;
	}

	//Parse's a URI and performs a look up on the address if the Domain is not an IP notation.  DefaultPort is used if the URI does not provide a port of it's own.
	LWSocketAddr(const LWUTF8Iterator &URI, LWUTF8Iterator &Domain, LWUTF8Iterator &Path, LWUTF8Iterator &Protocol, uint16_t DefaultPort = 0);

	//Parse's a URI, does not return any of the URI information if that is not desirable. DefaultPort is used if the URI does not provide a port of it's own.
	LWSocketAddr(const LWUTF8Iterator &URI, uint16_t DefaultPort = 0);

	/*!< \brief constructs an IP4 address. */
	LWSocketAddr(uint32_t IP4, uint16_t Port);

	/*!< \brief constructs an IP6 address from the 4 32 bit components. */
	LWSocketAddr(uint32_t IP6A, uint32_t IP6B, uint32_t IP6C, uint32_t IP6D, uint16_t Port);

	/*!< \brief default constructed address, Valid is not raised when default constructed. */
	LWSocketAddr() = default;
};

struct LWSRVRecord {
	char m_Address[64];
	uint16_t m_Port;
	uint32_t m_Priority;
	uint32_t m_Weight;
};

/*!< \brief default socket object, it encompasses the tcp/ip and udp/ip stacks with a single interface.  providing simple mechanisms to send and receive data between two devices. */
class LWSocket{
public:
	LWBitField32(TypeBits, 1, 0);
	LWBitField32(StatusBits, 2, TypeBitsOffset+1);
	LWBitField32(ErrorBits, 4, StatusBitsOffset+2);

	static const uint32_t TCP = 0; // \brief indicates socket is TCP ip protocol.
	static const uint32_t UDP = 1; // \brief indicates socket is UDP ip protocol.

	static const uint32_t S_Connected = 0; // \brief indicates socket is still in connecting phase.
	static const uint32_t S_Connecting = 1; // \brief indicates socket is still in connecting phase.
	static const uint32_t S_Closed = 2; // \brief indicates socket is closed.
	static const uint32_t S_Error = 3; // \brief indicates socket is in an error state.

	static const uint32_t E_OK = 0; /*!< \brief error flag indicating everything is ok. */
	static const uint32_t E_Connect = 1; /*!< \brief error flag to indicate error in connecting to remote host. */
	static const uint32_t E_Address = 2; /*!< \brief error flag to indicate error in retrieving the address of remote and local host. */
	static const uint32_t E_Socket = 3; /*!< \brief error flag to indicate error in creating the socket object. */
	static const uint32_t E_Bind = 4; /*!< \brief error flag to indicate error in binding the socket object. */
	static const uint32_t E_Listen = 5; /*!< \brief error flag to indicate error in becoming a listening object. */
	static const uint32_t E_GetSock = 6; /*!< \brief error flag to indicate error in retrieving local socket information. */
	static const uint32_t E_GetPeer = 7; /*!< \brief error flag to indicate error in retrieving remote socket information. */
	static const uint32_t E_CtrlFlags = 8; /*!< \brief error flag to indicate an error occurred while setting certain socket properties. */
	static const uint32_t E_Count = 9; /*!< \brief total number of error flags avaiable. */

	static const uint32_t LocalIP4 = 0x7F000001; // \brief ip4 local loopback address.
	static const uint32_t BroadcastIP4 = 0xFFFFFFFF; // \brief ip4 broadcast address.

	static const uint32_t Listening = 0x100; // \brief indicates socket listens for incoming connections, and accepts them automatically.
	static const uint32_t UDPConnReset = 0x200; // \brief indicates if a udp socket should reset when it's remote connection notifies it is closed.
	static const uint32_t TcpNoDelay = 0x400; // \brief indicates if a tcp socket should not delay batch data when sending, and instead send immediately.
	static const uint32_t ReuseAddr = 0x800; // \brief indicates socket is safe to bind to an already bound port.
	static const uint32_t BroadcastAble = 0x1000; // \brief indicates socket should be made to broadcast if possible.
	static const uint32_t Closed = 0x2000; // \brief indicates the socket is to be closed, and should no longer be accessed.
	static const uint32_t AsyncFlags = Listening | UDPConnReset | TcpNoDelay | ReuseAddr | BroadcastAble | Closed; // \brief indicates sockets flags that should be preserved when asyncing requests are made.

	static const uint32_t DefaultBacklog = 20; //Default backlog for listen sockets.
	static const uint32_t DefaultURIPort = 80; //Default URI port when parsing a URI that doesn't include a uri or a protocol.
	static const uint32_t DefaultDNSPort = 53; //Default dns port when one isn't specified. 

	static const uint32_t ProtocolCount = 7; /*!< \brief total number of protocol's lightwave is aware of. */
	static const char8_t *ProtocolNames[ProtocolCount]; /*!< \brief names of commonly used protocols. */
	static const uint32_t ProtocolPorts[ProtocolCount]; /*!< \brief default port for the commonly used protocols; */

	static const char8_t *ErrorNames[E_Count]; /*!< \brief names of all socket errors LWSocket can return. */

	/*!< \brief id for dns record types: */
	static const uint32_t DNS_A = 1;
	static const uint32_t DNS_NS = 2;
	static const uint32_t DNS_MD = 3;
	static const uint32_t DNS_MF = 4;
	static const uint32_t DNS_CNAME = 5;
	static const uint32_t DNS_SOA = 6;
	static const uint32_t DNS_MB = 7;
	static const uint32_t DNS_MG = 8;
	static const uint32_t DNS_MR = 9;
	static const uint32_t DNS_NULL = 10;
	static const uint32_t DNS_WKS = 11;
	static const uint32_t DNS_PTR = 12;
	static const uint32_t DNS_HINFO = 13;
	static const uint32_t DNS_MINFO = 14;
	static const uint32_t DNS_MX = 15;
	static const uint32_t DNS_TXT = 16;
	static const uint32_t DNS_SRV = 33;

	/*!< \brief looks up an addresses info from either an ip address or host name.
		 \param Address IP Address or host name
		 \param AddrBuffer buffer of ip addresses associated with the host name.
		 \param AddrBufferLen the length of buffer for receiving address's.
		 \param IPBuffer buffer of ip addresses for all ip's that can access the associated host.
		 \param IPBufferLen the number of ip addresses that can be written to the ip buffer.
		 \param Addresses buffer to receive all addresses associated with the host, each address is seperated by a \0 character, the end of the addresses is marked with a zero length address.
		 \param AddressLen the length of the address buffer.
		 \return 0xFFFFFFFF on failure, otherwopise number of ip's are returned.
	*/
	static uint32_t LookUpAddress(const LWUTF8Iterator &Address, LWSocketAddr *AddrBuffer, uint32_t AddrBufferLen, char8_t *Addresses, uint32_t AddressLen);
 
	/*!< \brief decode's uri with percent based encoding. 
		 \return the number of bytes needed to contain the entire decoded URI(including null terminated character).
	*/
	static uint32_t DecodeURI(const LWUTF8Iterator &URI, char8_t *Buffer, uint32_t BufferSize);

	/*!< \brief encode's uri string with percent based encoding.
	*	 \return the number of bytes needed to contain the entire encoded URI(including null terminated character).
	*/
	static uint32_t EncodeURI(const LWUTF8Iterator &URI, char8_t *Buffer, uint32_t BufferSize);

	/*!< \brief split's uri to get an iterator to the domain portion, protocol porition(if specified), and path porition.  Port is defaulted to http, but can be specified explicitly after domain, or uses the default for the specified protocol(if specified). 
		 \return flase if an invalid URI is detected, otherwise true.
	*/
	static bool SplitURI(const LWUTF8Iterator &URI, uint16_t &Port, LWUTF8Iterator &Domain, LWUTF8Iterator &Protocol, LWUTF8Iterator &Path);

	/*!< \brief converts a string label into a dns formatted label. 
		 \return number of bytes written to Buffer.
	*/
	static uint32_t WriteDNSLabel(const LWUTF8Iterator &Label, char8_t *Buffer, uint32_t BufferSize);

	/*!< \brief reads back from response a dns formmated label into a named label, ResponseStart is required due to how dns performs certain compression technique's. 
		 \return number of bytes read from Response.
	*/
	static uint32_t ReadDNSLabel(const char *Response, const char *ResponseStart, char *Buffer, uint32_t BufferLen);
	/*!< \brief returns the dns servers for the associated platform. */

	static uint32_t LookUpDNSServers(LWSocketAddr *AddrBuffer, uint32_t AddrBufferLen);

	/*!< \brief constructs and makes a dns query, returns in response the results of the query. 
		 \param Queryname the dns query to make. (note: srv record querys look like: (i.e: _servicename._protocol.domainname).
		 \param QueryType the type of query to be made.
		 \param Buffer the buffer to receive the resulting query.
		 \param BufferLen the length of the buffer.
		 \param DNSAddr Address of the DNS to use, if IP is 0 in the address field, then well attempt to use the primary dns server associated with the platform.
		 \return the number of bytes written to Buffer.
	*/
	static uint32_t DNSQuery(const LWUTF8Iterator &QueryName, uint16_t QueryType, void *Buffer, uint32_t BufferLen, const LWSocketAddr &DNSAddr = LWSocketAddr());

	/*!< \brief constructs and makes a dns consisting of multiple query's.
		 \param QueryNames the list of query names to be made.
		 \param QueryTypes the list of query types to be made.
		 \param QueryCnt the number of querys expected.
		 \param Buffer the buffer to receive the response.
		 \param BufferLen the length of the buffer.
		 \param DNSAddr Address of the DNS to use, if IP is 0 in the address field, then well attempt to use the primary dns server associated with the platform.
	*/
	static uint32_t DNSQuery(const LWUTF8Iterator *QueryNames, uint16_t *QueryTypes, uint32_t QueryCnt, void *Buffer, uint32_t BufferLen, const LWSocketAddr &DNSAddr = LWSocketAddr());

	/*!< \brief parses a dns response for all srv record response from a dns query call.
		 \param Response the response text to parse.
		 \param ResponseLen the length of the number of responses to parse.
		 \param RecordBuffer the buffer to receive the number of records.
		 \param RecordBufferLen the number of records that can be recoreded into recordbuffer.
		 \return the total number of records available.
	*/
	static uint32_t DNSParseSRVRecord(const char *Response, uint32_t ResponseLen, LWSRVRecord *RecordBuffer, uint32_t RecordBufferLen);

	/*!< \brief parses a dns response for all ip addresses were requested from a dns query call. */
	static uint32_t DNSParseARecord(const char *Response, uint32_t ResponseLen, LWSocketAddr *AddrBuffer, uint32_t AddrBufferSize);
	

	/*!< \brief constructs a udp socket on a random port. 
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a tcp or udp socket to bind to the specified port.
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t Type, uint16_t Port, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a socket on a random port, and attempts to connect to the specified address on the specified port. 
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t Type, const LWSocketAddr &RemoteAddr, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a socket on a specified port, and attempts to connect to the specified address on the specified port.
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t Type, const LWSocketAddr &RemoteAddr, uint16_t LocalPort, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief sets the user data associated with the socket. */
	LWSocket &SetUserData(void *UserData);

	/*!< \brief sets the timeout period where no active(or if not updated by the protocol will trigger a timeout.) 
		 \note passing 0 to lCurrentTime will have it make a call to LWTimer::GetCurrentTime().
	*/
	LWSocket &SetTimeoutPeriod(uint64_t lCurrentTime, uint64_t TimeoutPeriod);

	/*!< \brief if the socket is able to listen for incoming connections, then this method will wait for an incoming connection, upon receiving one, it will fill out the result sockets data. 
		 \return true on success, or false on failure.
	*/
	bool Accept(LWSocket &Result) const;

	/*!< \brief if the socket is able to listen for incoming connections, then this method will wait for an incoming connection, upon receiving one, it will fill out the result sockets data.  it also fills out the protocolID with the specified protocol.
		 \return true on success, or false on failure.
	*/
	bool Accept(LWSocket &Result, uint32_t ProtocolID) const;

	/*!< \brief receives an amount of data upto the bufferlen.
		 \return the amount of data written to buffer.
	*/
	uint32_t Receive(void *Buffer, uint32_t BufferLen);

	/*!< \brief receives an amount of data upto the bufferlen, and also returns the senders ip and port to return data to.
		 \return the amount of data written to buffer, or 0xFFFFFFFF if the connection has been closed from the other end.
	*/
	uint32_t Receive(void *Buffer, uint32_t BufferLen, LWSocketAddr &RemoteAddr);

	/*!< \brief sends an amount of data to the remote device, may not send all data in one step, so be sure to check the amount sent.
		 \return the amount of data sent, or 0xFFFFFFFF if the connection has been closed from the other end..
	*/
	uint32_t Send(const void *Buffer, uint32_t BufferLen);

	/*!< \brief does not return until all data has been sent(returning BufferLen), or an error has occurred(returning -1(0xFFFFFFFF))*/
	uint32_t SendAll(const void *Buffer, uint32_t BufferLen);

	/*!< \brief sends an amount of data to a specified remote device, may not send all data in one step, so be sure to check the amount sent.
		 \return the amount of data sent.
	*/
	uint32_t Send(const void *Buffer, uint32_t BufferLen, const LWSocketAddr &RemoteAddr);

	/*!< \brief  does not return until all data has been sent(returning BufferLen), or an error has occurred(returning -1(0xFFFFFFFF)) */
	uint32_t SendAll(const void *Buffer, uint32_t BufferLen, const LWSocketAddr &RemoteAddr);

	/*!< \brief if Err is not E_Ok, then set's the error and changes that status to S_Error. */
	LWSocket &SetError(uint32_t Err);

	/*!< \brief move operator. */
	LWSocket &operator = (LWSocket &&Other) noexcept;

	/*!< \brief explicit bool which return IsValid(). */
	explicit operator bool(void) const;

	/*!< \brief marks the socket closable, so that the owning protocol manager can freely dispose of the socket when necessary. */
	LWSocket &MarkClosable(void);

	/*!< \brief returns true if the socket was created with the listen flag. */
	bool IsListener(void) const;

	/*!< \brief returns if the underlying context has not been open yet. */
	bool IsValid(void) const;
	
	/*!< \brief returns the current status of the socket. */
	uint32_t GetStatus(void) const;
	
	/*!< \brief returns the current error of the socket. */
	uint32_t GetError(void) const;

	/*!< \brief returns true if the socket has been raised to be closed, or CurrentTime has passed Socket's timeout time. */
	bool IsClosable(uint64_t lCurrentTime) const;

	/*!< \brief actually closes the socket if able to do so. */
	LWSocket &Close(void);

	/*!< \brief returns the protocol id the socket is associated with. */
	uint32_t GetProtocolID(void) const;

	/*!< \brief returns the local address this socket is bound to. */
	LWSocketAddr GetLocalAddr(void) const;

	/*!< \brief returns the remote address this socket is bound to. */
	LWSocketAddr GetRemoteAddr(void) const;

	/*!< \brief returns a 32-bit hash of this socket for map's. */
	uint32_t Hash(void) const;

	/*!< \brief returns the flags associated with the socket. */
	uint32_t GetFlags(void) const;

	/*!< \brief returns the type of the socket(UDP, or TCP). */
	uint32_t GetType(void) const;

	/*!< \brief returns the number of bytes which the socket has sent over it's life time. */
	uint32_t GetSentBytes(void) const;

	/*!< \brief returns the number of bytes which the socket has recevied over it's life time. */
	uint32_t GetRecvBytes(void) const;

	/*!< \brief returns the user data associated with the socket. */
	void *GetUserData(void) const;

	/*!< \brief returns true if CurrentTime>=m_TimeoutTime. */
	bool IsTimedout(uint64_t lCurrentTime) const;

	/*!< \brief returns the handle representation for the underlying platform.  this function should not need to be called by most applications. */
	LWSocketHandle GetHandle(void) const;

	/*!< \brief move construct. */
	LWSocket(LWSocket &&Other) noexcept;

	/*!< \brief default constructor, constructs a blank object. */
	LWSocket(void) = default;

	/*!< \brief constructs an socket object. you should not have to call this method directly. */
	LWSocket(LWSocketHandle Handle, const LWSocketAddr &LocalAddr, const LWSocketAddr &RemoteAddr, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief destroys the socket object, and closing the connection if a tcp socket. */
	~LWSocket();
private:

	void *m_UserData = nullptr;
	LWSocketHandle m_Handle = LWSocketHandle();
	LWSocketAddr m_LocalAddr;
	LWSocketAddr m_RemoteAddr;
	uint64_t m_TimeoutTime = -1; //Time until this socket should timeout, to prevent people from keeping a socket open indefinitely on some protocols.
	uint32_t m_ProtocolID = 0;
	uint32_t m_SentBytes = 0;
	uint32_t m_RecvBytes = 0;
	uint32_t m_Flags = 0;

};


#endif