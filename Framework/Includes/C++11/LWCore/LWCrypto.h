#ifndef LWCRYPTO_H
#define LWCRYPTO_H
#include "LWCore/LWTypes.h"

/*!< \brief LWCrypto is a class which encodes numerous hashing schemes, and encoding/decoding schemes. most useful this class can be used in network communication, or file verification. this is a pure abstract class, which provides only static methods. */
class LWCrypto{
public:
	static const uint32_t FNV1AHash = 2166136261; /*!< \brief initial hash value for FNV1A algorithm. */

	/*!< \brief constructs an MD5 hash on Inbuffer, and stores the result into OutBuffer. 
		 \return the number of bytes written to OutBuffer, should always be 16.
		 \note OutBuffer must be 16 bytes wide. */
	static uint32_t HashMD5(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer);

	/*!< \brief does a simple SHA-1 hash on InBuffer, and stores the result into OutBuffer.
		 \return the number of bytes written to OutBuffer, should always be 20.
		 \note OutBuffer must be 20 bytes wide. */
	static uint32_t HashSHA1(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer);

	/*! \brief generates a hash for the supplied utf-8 text. this uses the fnv-1a algorithm. */
	template<class Type>
	static uint32_t HashFNV1A(const LWUnicodeIterator<Type> &Iter, uint32_t Hash = FNV1AHash);

	/*! \brief generates a hash for the supplied buffer. this uses the fnv-1a algorithm. */
	static uint32_t HashFNV1A(const uint8_t *Buffer, uint32_t BufferLen, uint32_t Hash = FNV1AHash);

	/*!< \brief encodes InBuffer into a base-64 encoded string.
		 \return the number of bytes to encode the string.
	*/
	static uint32_t Base64Encode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

	/*!< \brief decodes an encoded base 64 string into a binary stream into outbuffer.
		 \return the number of bytes to decode the string.
	*/
	static uint32_t Base64Decode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

	/*! \brief generates a crc32 value on set of data. 
		\param Data the data to work on.
		\param DataLen the amount of data to generate a crc for.
		\param CRCCurr if the crc is to be worked on over multiple pieces of data then feed the previous result of CRC32 CRCCurr to continue operation.
		\param Finished if this is the last data block set finished to true(also can pass no data with Finished set to true) to do a final operation on the crc value.
	*/
	static uint32_t CRC32(const uint8_t *Data, uint32_t DataLen, uint32_t CRCCurr=0xFFFFFFFF, bool Finished=true);
};

template<>
uint32_t LWCrypto::HashFNV1A<char8_t>(const LWUnicodeIterator<char8_t> &Iter, uint32_t Hash);


template<>
uint32_t LWCrypto::HashFNV1A<char16_t>(const LWUnicodeIterator<char16_t> &Iter, uint32_t Hash);

template<>
uint32_t LWCrypto::HashFNV1A<char32_t>(const LWUnicodeIterator<char32_t> &Iter, uint32_t Hash);
#endif