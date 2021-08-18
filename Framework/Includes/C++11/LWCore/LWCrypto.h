#ifndef LWCRYPTO_H
#define LWCRYPTO_H
#include "LWCore/LWTypes.h"

/*!< \brief typedef for HMAC hashing function, since all HMAC hash's use the same overall algorithm, the method passed in is the actual hashing algorithm. */
typedef std::function<uint32_t(const char **, uint32_t *, uint32_t, void *)> LWHashHMACFunc;

/*!< \brief LWCrypto is a class which encodes numerous hashing schemes, and encoding/decoding schemes. most useful this class can be used in network communication, or file verification. this is a pure abstract class, which provides only static methods. */
class LWCrypto{
public:
	static const uint32_t FNV1AHash = 2166136261; /*!< \brief initial hash value for FNV1A algorithm. */

	/*!< \brief convience function for taking multiple pieces of data and concating them together into a single continuous chunk, all multi-part hash functions use this function. 
		 \param ActiveBuffer the current index of the read buffer being read.
		 \param ActivePosition position into the current buffer being read.
		 \return the number of bytes upto ChunkSize, 0 if no data remains to read.
		 \note Out must be at least ChunkSize in size.
	*/
	static uint32_t NextChunk(const char **ReadBufferList, uint32_t *ReadBufferSize, uint32_t ReadBufferCount, uint32_t &ActiveBuffer, uint32_t &ActivePosition, uint32_t ChunkSize, void *Out);

	/*!< \brief generic HMAC function that all convenience function's use, this version is not a multi-message version. 
	*	 \param ChunkSize size of chunks for the Hashing function
	*	 \param HashSize size in bytes of the hash, OutBuffer should be at least this size.
		 \note Key's and hash size's are currently limited to function's with a chunksize of 64, if another chunksize is needed, this function will need to be modified. 
	*/
	static uint32_t HashHMAC(const char *Message, uint32_t MessageLen, const char *Key, uint32_t KeyLen, LWHashHMACFunc HashFunc, uint32_t ChunkSize, void *OutBuffer);

	/*!< \brief generate's a string digest of the hash as a hex string.
		 \return number of bytes to contain the digest(excluding null character), this is essentially HashSize*2. */
	static uint32_t Digest(const void *Hash, uint32_t HashSize, char8_t *OutBuffer, uint32_t OutBufferLen);

	/*!< \brief constructs an MD5 hash on Inbuffer, and stores the result into OutBuffer. 
		 \return the number of bytes written to OutBuffer, should always be 16.
		 \note OutBuffer must be 16 bytes wide. */
	static uint32_t HashMD5(const char *InBuffer, uint32_t InBufferLen, void *OutBuffer);

	/*!< \brief constructs an MD5 hash on a list of InBuffer's, and stores the result into OutBuffer. */
	static uint32_t HashMD5l(const char **InBuffer, uint32_t *InBufferLen, uint32_t InBufferCount, void *OutBuffer);

	/*!< \brief constructs an MD5 hash on InBuffer with Key and stores the result into OutBuffer.
		 \return the number of bytes written to OutBuffer, should always be 16.
		 \note OutBuffer must be 16 bytes wide. */
	static uint32_t HashHMAC_MD5(const char *InBuffer, uint32_t InBufferLen, const char *Key, uint32_t KeyLen, void *OutBuffer);

	/*!< \brief does a simple SHA-1 hash on InBuffer, and stores the result into OutBuffer.
		 \return the number of bytes written to OutBuffer, should always be 20.
		 \note OutBuffer must be 20 bytes wide. */
	static uint32_t HashSHA1(const char *InBuffer, uint32_t InBufferLen, void *OutBuffer);

	/*!< \brief constructs a SHA-1 hash on a list of InBuffer's, and stores the result into OutBuffer. */
	static uint32_t HashSHA1l(const char **InBuffer, uint32_t *InBufferLen, uint32_t InBufferCount, void *OutBuffer);

	/*!< \brief constructs an SHA1 hash on InBuffer with Key and stores the result into OutBuffer.
		 \return the number of bytes written to OutBuffer, should always be 16.
		 \note OutBuffer must be 16 bytes wide. */
	static uint32_t HashHMAC_SHA1(const char *InBuffer, uint32_t InBufferLen, const char *Key, uint32_t KeyLen, void *OutBuffer);

	/*!< \brief does a simple SHA-2 (SHA-256 digest) has on InBuffer and stores the result into OutBuffer.
	*	 \return the number of bytes written to Outbuffer, should always be 20.
	*	 \note OutBuffer must be 20 bytes wide. */
	static uint32_t HashSHA256(const char *InBuffer, uint32_t InBufferLen, void *OutBuffer);

	/*!< \brief constructs a SHA-256 hash on a list of InBuffer's, and stores the result into OutBuffer. */
	static uint32_t HashSHA256l(const char **InBuffer, uint32_t *InBufferLen, uint32_t InBufferCount, void *OutBuffer);

	/*!< \brief constructs an SHA256 hash on InBuffer with Key and stores the result into OutBuffer.
		 \return the number of bytes written to OutBuffer, should always be 16.
		 \note OutBuffer must be 16 bytes wide. */
	static uint32_t HashHMAC_SHA256(const char *InBuffer, uint32_t InBufferLen, const char *Key, uint32_t KeyLen, void *OutBuffer);

	/*! \brief generates a 32 bit hash for the supplied utf-8 text. this uses the fnv-1a algorithm. */
	template<class Type>
	static uint32_t HashFNV1A(const LWUnicodeIterator<Type> &Iter, uint32_t Hash = FNV1AHash);

	/*! \brief generates a 32 bit hash for the supplied buffer. this uses the fnv-1a algorithm. */
	static uint32_t HashFNV1A(const uint8_t *Buffer, uint32_t BufferLen, uint32_t Hash = FNV1AHash);

	/*!< \brief encodes InBuffer into a base-64 encoded string.
		 \return the number of bytes to encode the string.
	*/
	static uint32_t Base64Encode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

	/*!< \brief decodes an encoded base 64 string into a binary stream into outbuffer.
		 \return the number of bytes to decode the string.
	*/
	static uint32_t Base64Decode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);


	/*!< \brief encodes InBuffer into a hex encoded string.
	*	 \return the number of bytes to encode the string(excluding the null character).
	*/
	static uint32_t HexEncode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

	/*!< \brief decodes an encoded hex string.
	*	 \return the number of bytes to decode the string.
	*/
	static uint32_t HexDecode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen);

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