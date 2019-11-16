#ifndef LWCRYPTO_H
#define LWCRYPTO_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWCrypto.h"

/*!< \brief LWCrypto is a class which encodes numerous hashing schemes, and encoding/decoding schemes. most useful this class can be used in network communication, or file verification. this is a pure abstract class, which provides only static methods. */
class LWCrypto{
public:
	/*!< \brief constructs an MD5 hash on Inbuffer, and stores the result into OutBuffer. 
		 \return the number of bytes written to OutBuffer, should always be 16.
		 \note OutBuffer must be 16 bytes wide. */
	static uint32_t HashMD5(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer);

	/*!< \brief does a simple SHA-1 hash on InBuffer, and stores the result into OutBuffer.
		 \return the number of bytes written to OutBuffer, should always be 20.
		 \note OutBuffer must be 20 bytes wide. */
	static uint32_t HashSHA1(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer);


	/*!< \brief encodes InBuffer into a base-64 encoded string.
		 \return the number of bytes to encode the string.
	*/
	static uint32_t Base64Encode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

	/*!< \brief decodes an encoded base 64 string into a binary stream into outbuffer.
		 \return the number of bytes to decode the string.
	*/
	static uint32_t Base64Decode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

};

#endif