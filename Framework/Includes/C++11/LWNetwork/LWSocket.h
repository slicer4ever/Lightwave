#ifndef LWSOCKET_H
#define LWSOCKET_H
#include "LWCore/LWTypes.h"
#include "LWNetwork/LWTypes.h"

struct LWSRVRecord {
	char m_Address[64];
	uint16_t m_Port;
	uint32_t m_Priority;
	uint32_t m_Weight;
};

/*!< \brief default socket object, it encompasses the tcp/ip and udp/ip stacks with a single interface.  providing simple mechanisms to send and receive data between two devices. */
class LWSocket{
public:
	enum{
		Tcp = 0x0, /*!< \brief flag to indicate the socket object is a tcp type of socket. */
		Udp = 0x1, /*!< \brief flag to indicate the socket object is a udp type of socket. */
		Listen = 0x2, /*!< \brief flag to indicate the socket has listening capabilitys to accept and create new connections. */
		Closeable = 0x4, /*!< \brief flag to indicate the socket is closable. if this flag is set, assume the socket is no longer open. */

		UdpConnReset = 0x8, /*!< \brief flag to indicate the socket is to reset when the remote socket is unresponsive. */
		Broadcast = 0x10, /*!< \brief flag to indicate the socket should support broadcasting if possible. */
		TcpNoDelay = 0x20, /*!< \brief flag to indicate the socket should not delay tcp packets when waiting for more data. */
		ReuseAddr = 0x40, /*!< \brief flag to indicate the socket is allowed to reuse a port which is already under use. */

		BroadcastIP = 0xFFFFFFFF, /*!< \brief ip address to broadcast to the entire local network. */
		LocalIP = 0x7F000001, /*!< \brief ip address for loopback address on local device. */

		ErrConnect = 0x1, /*!< \brief error flag to indicate error in connecting to remote host. */
		ErrAddress = 0x2, /*!< \brief error flag to indicate error in retrieving the address of remote and local host. */
		ErrSocket = 0x3, /*!< \brief error flag to indicate error in creating the socket object. */
		ErrBind = 0x4, /*!< \brief error flag to indicate error in binding the socket object. */
		ErrListen = 0x5, /*!< \brief error flag to indicate error in becoming a listening object. */
		ErrGetSock = 0x6, /*!< \brief error flag to indicate error in retrieving local socket information. */
		ErrGetPeer = 0x7, /*!< \brief error flag to indicate error in retrieving remote socket information. */
		ErrCtrlFlags = 0x8, /*!< \brief error flag to indicate an error occurred while setting certain socket properties. */

		MaxBacklog = 20, /*!< \brief Max backlog for a listening socket before it begins dropping connections. */
	
		MaxProtocols=8, /*!< \brief max number of protocol data storage pointers. */
	
		 /*!< \brief id for dns record types: */
		DNS_A=1,
		DNS_NS,
		DNS_MD,
		DNS_MF,
		DNS_CNAME,
		DNS_SOA,
		DNS_MB,
		DNS_MG,
		DNS_MR,
		DNS_NULL,
		DNS_WKS,
		DNS_PTR,
		DNS_HINFO,
		DNS_MINFO,
		DNS_MX,
		DNS_TXT,
		DNS_SRV=33
	};

	static const uint32_t ProtocolCount = 7; /*!< \brief total number of protocol's lightwave is aware of. */
	static const char8_t *ProtocolNames[ProtocolCount]; /*!< \brief names of commonly used protocols. */
	static const uint32_t ProtocolPorts[ProtocolCount]; /*!< \brief default port for the commonly used protocols;

	/*!< \brief constructs an ipv4 address from a string in ipv4 notation. */
	static uint32_t MakeIP(const LWUTF8Iterator &Address);

	/*!< \brief constructs an ipv4 address from 4 independent components. */
	static uint32_t MakeIP(uint8_t First, uint8_t Second, uint8_t Third, uint8_t Fourth);

	/*!< \brief constructs an ipv4 notation string from an ip address. 
		 \return true on success, or false on failure.  a max of 16 bytes is the largest an ipv4 address will take up.
	*/
	static bool MakeAddress(uint32_t Address, char8_t *Buffer, uint32_t BufferLen);

	/*!< \brief looks up an addresses info from either an ip address or host name.
		 \param Address IP Address or host name.
		 \param IPBuffer buffer of ip addresses for all ip's that can access the associated host.
		 \param IPBufferLen the number of ip addresses that can be written to the ip buffer.
		 \param Addresses buffer to receive all addresses associated with the host, each address is seperated by a \0 character, the end of the addresses is marked with a zero length address.
		 \param AddressLen the length of the address buffer.
		 \return 0xFFFFFFFF on failure, otherwise number of ip's are returned.
	*/
	static uint32_t LookUpAddress(const LWUTF8Iterator &Address, uint32_t *IPBuffer, uint32_t IPBufferLen, char8_t *Addresses, uint32_t AddressLen);
 
	/*!< \brief looks up an addresses info from an IP address.
		 \param Address IP Address or host name.
		 \param IPBuffer buffer of ip addresses for all ip's that can access the associated host.
		 \param IPBufferLen the number of ip addresses that can be written to the ip buffer.
		 \param Addresses buffer to receive all addresses associated with the host, each address is seperated by a \0 character, the end of the addresses is marked with a zero length address.
		 \param AddressLen the length of the address buffer.
		 \return 0xFFFFFFFF on failure, otherwise number of ip's are returned.
	*/
	static uint32_t LookUpAddress(uint32_t Address, uint32_t *IPBuffer, uint32_t IPBufferLen, char8_t *Addresses, uint32_t AddressLen);

	/*!< \brief decode's uri with percent based encoding. 
		 \return the number of bytes needed to contain the entire decoded URI(including null terminated character).
	*/
	static uint32_t DecodeURI(const LWUTF8Iterator &URI, char8_t *Buffer, uint32_t BufferSize);

	/*!< \brief encode's uri string with percent based encoding.
	*	 \return the number of bytes needed to contain the entire encoded URI(including null terminated character).
	*/
	static uint32_t EncodeURI(const LWUTF8Iterator &URI, char8_t *Buffer, uint32_t BufferSize);

	/*!< \brief split's uri to get an iterator to the domain portion, protocol porition(if specified), and path porition.  Port is defaulted to http, but can be specified explicitly after domain, or uses the default for the specified protocol(if specified). */
	static void SplitURI(const LWUTF8Iterator &URI, uint16_t &Port, LWUTF8Iterator &Domain, LWUTF8Iterator &Protocol, LWUTF8Iterator &Path);

	/*!< \brief converts a string label into a dns formatted label. 
		 \return number of bytes written to Buffer.
	*/
	static uint32_t WriteDNSLabel(const LWUTF8Iterator &Label, char8_t *Buffer, uint32_t BufferSize);

	/*!< \brief reads back from response a dns formmated label into a named label, ResponseStart is required due to how dns performs certain compression technique's. 
		 \return number of bytes read from Response.
	*/
	static uint32_t ReadDNSLabel(const char *Response, const char *ResponseStart, char *Buffer, uint32_t BufferLen);

	/*!< \brief returns the dns servers for the associated platform. */
	static uint32_t LookUpDNSServers(uint32_t *IPBuf, uint32_t IPBufferLen);

	/*!< \brief constructs and makes a dns query, returns in response the results of the query. 
		 \param Queryname the dns query to make. (note: srv record querys look like: (i.e: _servicename._protocol.domainname).
		 \param QueryType the type of query to be made.
		 \param Buffer the buffer to receive the resulting query.
		 \param BufferLen the length of the buffer.
		 \param DNSIP if 0 then well attempt to use the primary dns server associated with the platform.
		 \return the number of bytes written to Buffer.
	*/
	static uint32_t DNSQuery(const LWUTF8Iterator &QueryName, uint16_t QueryType, char *Buffer, uint32_t BufferLen, uint32_t DNSIP=0);

	/*!< \brief constructs and makes a dns consisting of multiple query's.
		 \param QueryNames the list of query names to be made.
		 \param QueryTypes the list of query types to be made.
		 \param QueryCnt the number of querys expected.
		 \param Buffer the buffer to receive the response.
		 \param BufferLen the length of the buffer.
		 \param DNSIP if 0 then well attempt to use the primary dns server associated with the platform.
	*/
	static uint32_t DNSQuery(const LWUTF8Iterator *QueryNames, uint16_t *QueryTypes, uint32_t QueryCnt, char *Buffer, uint32_t BufferLen, uint32_t DNSIP = 0);

	/*!< \brief parses a dns response for all srv record response from a dns query call.
		 \param Response the response text to parse.
		 \param ResponseLen the length of the number of responses to parse.
		 \param RecordBuffer the buffer to receive the number of records.
		 \param RecordBufferLen the number of records that can be recoreded into recordbuffer.
		 \return the total number of records available.
	*/
	static uint32_t DNSParseSRVRecord(const char *Response, uint32_t ResponseLen, LWSRVRecord *RecordBuffer, uint32_t RecordBufferLen);

	/*!< \brief parses a dns response for all ip addresses were requested from a dns query call. */
	static uint32_t DNSParseARecord(const char *Response, uint32_t ResponseLen, uint32_t *IPBuffer, uint32_t IPBufferLen);
	

	/*!< \brief constructs a udp socket on a random port. 
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a tcp or udp socket to bind to the specified port.
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint16_t Port, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a tcp socket on a random port, and attempts to connect to the specified address on the specified port. 
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t RemoteIP, uint16_t RemotePort, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a tcp socket on a specified port, and attempts to connect to the specified address on the specified port.
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, uint32_t RemoteIP, uint16_t RemotePort, uint16_t LocalPort, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a tcp socket on a random port, and attempts to connect to the specified address(either ip notation, or host specified) on the specified port.
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, const LWUTF8Iterator &Address, uint16_t RemotePort, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief constructs a tcp socket on a specified port, and attempts to connect to the specified address(either ip notation, or host specified) on the specified port.
		 \return 0 on success, an error code if creation fails.
	*/
	static uint32_t CreateSocket(LWSocket &Socket, const LWUTF8Iterator &Address, uint16_t RemotePort, uint16_t LocalPort, uint32_t Flag, uint32_t ProtocolID);

	/*!< \brief sets the user data associated with the socket. */
	LWSocket &SetUserData(void *UserData);

	/*!< \brief sets the protocol data associated with the socket for the specified id. this data is separate from user data as each protocol might require knowledge of diffrent states the socket is in, where the user data is application level data that is associated with the individual connection. */
	LWSocket &SetProtocolData(uint32_t ProtocolID, void *ProtocolData);

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
	uint32_t Receive(char *Buffer, uint32_t BufferLen) const;

	/*!< \brief receives an amount of data upto the bufferlen, and also returns the senders ip and port to return data to.
		 \return the amount of data written to buffer, or 0xFFFFFFFF if the connection has been closed from the other end.
	*/
	uint32_t Receive(char *Buffer, uint32_t BufferLen, uint32_t *RemoteIP, uint16_t *RemotePort) const;

	/*!< \brief sends an amount of data to the remote device, may not send all data in one step, so be sure to check the amount sent.
		 \return the amount of data sent, or 0xFFFFFFFF if the connection has been closed from the other end..
	*/
	uint32_t Send(const char *Buffer, uint32_t BufferLen) const;

	/*!< \brief sends an amount of data to a specified remote device, may not send all data in one step, so be sure to check the amount sent.
		 \return the amount of data sent.
	*/
	uint32_t Send(char *Buffer, uint32_t BufferLen, uint32_t RemoteIP, uint16_t RemotePort) const;

	/*!< \brief move operator. */
	LWSocket &operator = (LWSocket &&Other);

	/*!< \brief marks the socket closable, so that the owning protocol manager can freely dispose of the socket when necessary. */
	LWSocket &MarkClosable(void);

	/*!< \brief returns true if the socket was created with the listen flag. */
	bool isListener(void) const;

	/*!< \brief returns true if the socket has been raised to be closed. */
	bool isClosable(void) const;

	/*!< \brief actually closes the socket if able to do so. */
	LWSocket &Close(void);

	/*!< \brief returns the underlying socket descriptor. */
	uint32_t GetSocketDescriptor(void) const;

	/*!< \brief returns the protocol id the socket is associated with. */
	uint32_t GetProtocolID(void) const;

	/*!< \brief returns the local ip address(this is likely to be the local network address that the device is using to connect to the remote address). */
	uint32_t GetLocalIP(void) const;

	/*!< \brief returns the local port the socket is bound to. */
	uint16_t GetLocalPort(void) const;

	/*!< \brief if the socket is a tcp type, then this returns the remote devices ip address. */
	uint32_t GetRemoteIP(void) const;

	/*!< \brief if the socket is a tcp type, then this returns the remote devices connected port. */
	uint16_t GetRemotePort(void) const;

	/*!< \brief returns the flags associated with the socket. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns the user data associated with the socket. */
	void *GetUserData(void) const;

	/*!< \brief returns protocol specific data for the specified id. */
	void *GetProtocolData(uint32_t ProtocolID) const;

	/*!< \brief move construct. */
	LWSocket(LWSocket &&Other);

	/*!< \brief default constructor, constructs a blank object. */
	LWSocket(void);

	/*!< \brief constructs an socket object. you should not have to call this method directly. */
	LWSocket(uint32_t SocketID, uint32_t ProtocolID, uint32_t LocalIP, uint16_t LocalPort, uint32_t RemoteIP, uint16_t RemotePort, uint32_t Flag);

	/*!< \brief destroys the socket object, and closing the connection if a tcp socket. */
	~LWSocket();
private:

	void *m_UserData = nullptr;
	void *m_ProtocolData[MaxProtocols];
	uint32_t m_SocketID = 0;
	uint32_t m_ProtocolID = 0;
	uint32_t m_LocalIP = 0;
	uint32_t m_RemoteIP = 0;
	uint32_t m_Flag = 0;
	uint16_t m_RemotePort = 0;
	uint16_t m_LocalPort = 0;

};


#endif