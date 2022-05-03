#include "LWCore/LWCrypto.h"
#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWUnicode.h"
#include <cstring>
#include <iostream>

uint32_t LWCrypto::NextChunk(const char **ReadBufferList, uint32_t *ReadBufferSize, uint32_t ReadBufferCount, uint32_t &ActiveBuffer, uint32_t &ActivePosition, uint32_t ChunkSize, void *Out) {
	uint32_t o = 0;
	while (ActiveBuffer != ReadBufferCount && o != ChunkSize) {
		uint32_t r = std::min<uint32_t>(ReadBufferSize[ActiveBuffer] - ActivePosition, ChunkSize - o);
		const char *ReadPos = ReadBufferList[ActiveBuffer] + ActivePosition;
		std::copy(ReadPos, ReadPos + r, (char*)Out + o);
		ActivePosition += r;
		o += r;
		if (ActivePosition == ReadBufferSize[ActiveBuffer]) {
			ActiveBuffer++;
			ActivePosition = 0;
		}
	}
	return o;
};

uint32_t LWCrypto::HashHMAC(const char *Message, uint32_t MessageLen, const char *Key, uint32_t KeyLen, LWHashHMACFunc HashFunc, uint32_t ChunkSize, void *OutBuffer) {
	static const uint32_t MaxChunkSize = 64;
	static const uint32_t MaxHashKeySize = 16;
	char I_Key[MaxChunkSize] = {};
	char O_Key[MaxChunkSize] = {};
	uint32_t Digest[MaxHashKeySize];
	assert(ChunkSize <= MaxChunkSize);
	if (KeyLen > ChunkSize) KeyLen = HashFunc(&Key, &KeyLen, 1, I_Key);
	else std::copy(Key, Key + KeyLen, I_Key);
	std::copy(I_Key, I_Key + KeyLen, O_Key);
	
	for (uint32_t i = 0; i < ChunkSize; i++) {
		I_Key[i] ^= 0x36;
		O_Key[i] ^= 0x5c;
	}

	const char *InnerList[2] = { I_Key, Message };
	const char *OuterList[2] = { O_Key, (char*)Digest };
	uint32_t InnerLens[2] = { ChunkSize, MessageLen };
	uint32_t OuterLens[2] = { ChunkSize, 0 };
	assert((OuterLens[1] = HashFunc((const char**)InnerList, InnerLens, 2, Digest)) <= MaxHashKeySize*4);
	return HashFunc((const char**)OuterList, OuterLens, 2, OutBuffer);
}

uint32_t LWCrypto::Digest(const void *Hash, uint32_t HashSize, char8_t *OutBuffer, uint32_t OutBufferLen) {
	const uint8_t *C = (const uint8_t*)Hash;
	uint32_t o = 0;
	for (uint32_t i = 0; i < HashSize; i++) o += LWUTF8I::Fmt_ns(OutBuffer, OutBufferLen, o, "{:02x}", C[i]);
	return o;
}

uint32_t LWCrypto::HashMD5(const char *InBuffer, uint32_t InBufferLen, void *OutBuffer){
	char Buffer[128] = {}; //0 Initialize Buffer, buffer size is size of 2 blocks.
	uint32_t Table[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
						 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
						 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
						 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
						 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
						 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
						 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
						 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

	uint32_t Shift[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
						 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
						 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

	if (!OutBuffer) return 16;
	uint32_t *o = (uint32_t*)OutBuffer;
	o[0] = 0x67452301;
	o[1] = 0xefcdab89;
	o[2] = 0x98badcfe;
	o[3] = 0x10325476;

	//Setup last chunk's:
	uint32_t LastChunk = InBufferLen / 64 * 64;
	uint32_t LastByte = InBufferLen - LastChunk;
	std::copy(InBuffer + LastChunk, InBuffer + InBufferLen, Buffer);
	Buffer[LastByte] = '\x80';
	*((uint64_t*)Buffer + (LastByte < 56 ? 7 : 15)) = (uint64_t)InBufferLen * 8;

	for (uint32_t i = 0; i < InBufferLen+9;i+=64){
		const uint32_t *Block = (const uint32_t*)(InBuffer + i);
		if (i + 64 > InBufferLen) Block = (const uint32_t*)(Buffer + (i <= InBufferLen ? 0 : 64));
		uint32_t A = o[0];
		uint32_t B = o[1];
		uint32_t C = o[2];
		uint32_t D = o[3];
		for (uint32_t d = 0; d < 64;d++){
			uint32_t F = 0;
			uint32_t g = 0;
			if(d<16){
				F = (B&C) | ((~B)&D);
				g = d;
			} else if (d < 32){
				F = (D&B) | ((~D)&C);
				g = (5 * d + 1) % 16;
			}else if(d<48){
				F = B^C^D;
				g = (3 * d + 5) % 16;
			}else{
				F = C ^ (B | (~D));
				g = (7 * d) % 16;
			}
			uint32_t n = (A + F + Table[d] + Block[g]);
			A = D;
			D = C;
			C = B;
			B = B + LWRotateLeft(n, Shift[d]);
		}
		o[0] += A;
		o[1] += B;
		o[2] += C;
		o[3] += D;
	}
	o[0] = o[0];
	o[1] = o[1];
	o[2] = o[2];
	o[3] = o[3];
	return 16;
}

uint32_t LWCrypto::HashMD5l(const char **InBuffer, uint32_t *InBufferLen, uint32_t InBufferCount, void *OutBuffer) {
	const uint32_t ChunkSize = 64;
	char Chunk[ChunkSize];
	uint32_t *Block = (uint32_t*)Chunk;
	uint32_t Table[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
						 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
						 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
						 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
						 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
						 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
						 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
						 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

	uint32_t Shift[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
						 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
						 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

	if (!OutBuffer) return 16;
	uint32_t *o = (uint32_t*)OutBuffer;
	o[0] = 0x67452301;
	o[1] = 0xefcdab89;
	o[2] = 0x98badcfe;
	o[3] = 0x10325476;

	uint32_t ActiveBuffer = 0;
	uint32_t ActiveBufferPos = 0;

	uint32_t TotalLength = 0;
	for(uint32_t i = 0; i < TotalLength + 9;i+=64) {
		TotalLength += NextChunk(InBuffer, InBufferLen, InBufferCount, ActiveBuffer, ActiveBufferPos, ChunkSize, Chunk);
		if (i + 64 > TotalLength) {
			std::fill(Chunk + (TotalLength>i?(TotalLength - i):0), Chunk + ChunkSize, 0);
			if(i<TotalLength){
				if (i + ChunkSize != TotalLength) Chunk[TotalLength - i] = '\x80';
			} else if (i == TotalLength) Chunk[0] = '\x80';
			if (i + (ChunkSize-8) > TotalLength) *(((uint64_t*)Chunk) + 7) = (uint64_t)TotalLength * 8;
		}
		uint32_t A = o[0];
		uint32_t B = o[1];
		uint32_t C = o[2];
		uint32_t D = o[3];
		for (uint32_t d = 0; d < 64; d++) {
			uint32_t F = 0;
			uint32_t g = 0;
			if (d < 16) {
				F = (B & C) | ((~B) & D);
				g = d;
			} else if (d < 32) {
				F = (D & B) | ((~D) & C);
				g = (5 * d + 1) % 16;
			} else if (d < 48) {
				F = B ^ C ^ D;
				g = (3 * d + 5) % 16;
			} else {
				F = C ^ (B | (~D));
				g = (7 * d) % 16;
			}
			uint32_t n = (A + F + Table[d] + Block[g]);
			A = D;
			D = C;
			C = B;
			B = B + LWRotateLeft(n, Shift[d]);
		}
		o[0] += A;
		o[1] += B;
		o[2] += C;
		o[3] += D;
	}
	return 16;
}

uint32_t LWCrypto::HashHMAC_MD5(const char *InBuffer, uint32_t InBufferLen, const char *Key, uint32_t KeyLen, void *OutBuffer) {
	return HashHMAC(InBuffer, InBufferLen, Key, KeyLen, LWCrypto::HashMD5l, 64, OutBuffer);
}

uint32_t LWCrypto::HashSHA1(const char *InBuffer, uint32_t InBufferLen, void *OutBuffer){
	//From Wikipedia, version is for big endian, so data is converted to Network order, then back to host on out.
	char Buffer[128] = {}; //0 Initialize Buffer, buffer size is size is 2 in block's.
	uint32_t Block[80];
	
	if (!OutBuffer) return 20;
	uint32_t *o = (uint32_t*)OutBuffer;
	
	o[0] = 0x67452301; 
	o[1] = 0xefcdab89; 
	o[2] = 0x98badcfe; 
	o[3] = 0x10325476; 
	o[4] = 0xC3D2E1F0;

	//Setup last chunks:
	uint32_t LastChunk = InBufferLen / 64 * 64;
	uint32_t LastByte = InBufferLen - LastChunk;
	std::copy(InBuffer + LastChunk, InBuffer + InBufferLen, Buffer);
	Buffer[LastByte] = '\x80';
	*((uint64_t*)Buffer + (LastByte < 56 ? 7 : 15)) = LWByteBuffer::MakeNetwork(((uint64_t)InBufferLen * 8));

	for (uint32_t i = 0; i < InBufferLen + 9; i += 64){
		const uint32_t *Bf = (const uint32_t*)(InBuffer + i);
		if (i + 64 > InBufferLen) Bf = (const uint32_t*)(Buffer + (i <= InBufferLen ? 0 : 64));
		std::copy(Bf, Bf + 16, Block);

		uint32_t A = o[0];
		uint32_t B = o[1];
		uint32_t C = o[2];
		uint32_t D = o[3];
		uint32_t E = o[4];
		for (uint32_t d = 0; d < 16; d++) Block[d] = LWByteBuffer::MakeNetwork(Block[d]); //we need to convert the input of block to big endian representation.
		for (uint32_t d = 16; d < 80; d++){
			Block[d] = LWRotateLeft(Block[d - 3] ^ Block[d - 8] ^ Block[d - 14] ^ Block[d - 16], 1u);
		}
		for (uint32_t d = 0; d < 80; d++){
			uint32_t F = 0;
			uint32_t k = 0;
			if (d < 20){
				F = (B&C) | ((~B)&D);
				k = 0x5A827999;
			} else if (d < 40){
				F = B^C^D;
				k = 0x6ED9EBA1;
			} else if (d < 60){
				F = (B&C) | (B&D) | (C&D);
				k = 0x8F1BBCDC;
			} else{
				F = B^C^D;
				k = 0xCA62C1D6;
			}
			uint32_t n = LWRotateLeft(A, 5u) + F + E + k + Block[d];
			E = D;
			D = C;
			C = LWRotateLeft(B, 30u);
			B = A;
			A = n;
		}
		o[0] += A;
		o[1] += B;
		o[2] += C;
		o[3] += D;
		o[4] += E;
	}
	for (uint32_t i = 0; i < 5; i++) o[i] = LWByteBuffer::MakeHost(o[i]);
	return 20;
}

uint32_t LWCrypto::HashSHA1l(const char **InBuffer, uint32_t *InBufferLen, uint32_t InBufferCount, void *OutBuffer) {
	const uint32_t ChunkSize = 64;
	char Chunk[320]; //ChunkSize is 64, but the chunk is expanded upto 320 bytes.
	uint32_t *Block = (uint32_t*)Chunk;

	if (!OutBuffer) return 20;
	uint32_t *o = (uint32_t*)OutBuffer;
	o[0] = 0x67452301;
	o[1] = 0xefcdab89;
	o[2] = 0x98badcfe;
	o[3] = 0x10325476;
	o[4] = 0xC3D2E1F0;
	uint32_t ActiveBuffer = 0;
	uint32_t ActiveBufferPos = 0;
	uint32_t TotalLength = 0;

	for (uint32_t i = 0; i < TotalLength + 9; i += 64) {
		TotalLength += NextChunk(InBuffer, InBufferLen, InBufferCount, ActiveBuffer, ActiveBufferPos, ChunkSize, Chunk);
		if (i + 64 > TotalLength) {
			std::fill(Chunk + (TotalLength > i ? (TotalLength - i) : 0), Chunk + ChunkSize, 0);
			if (i < TotalLength) {
				if (i + ChunkSize != TotalLength) Chunk[TotalLength - i] = '\x80';
			} else if (i == TotalLength) Chunk[0] = '\x80';
			if (i + (ChunkSize-8) > TotalLength) *(((uint64_t*)Chunk) + 7) = LWByteBuffer::MakeNetwork((uint64_t)TotalLength * 8);
		}

		uint32_t A = o[0];
		uint32_t B = o[1];
		uint32_t C = o[2];
		uint32_t D = o[3];
		uint32_t E = o[4];
		for (uint32_t d = 0; d < 16; d++) Block[d] = LWByteBuffer::MakeNetwork(Block[d]); //we need to convert the input of block to big endian representation.
		for (uint32_t d = 16; d < 80; d++) {
			Block[d] = LWRotateLeft(Block[d - 3] ^ Block[d - 8] ^ Block[d - 14] ^ Block[d - 16], 1u);
		}
		for (uint32_t d = 0; d < 80; d++) {
			uint32_t F = 0;
			uint32_t k = 0;
			if (d < 20) {
				F = (B & C) | ((~B) & D);
				k = 0x5A827999;
			} else if (d < 40) {
				F = B ^ C ^ D;
				k = 0x6ED9EBA1;
			} else if (d < 60) {
				F = (B & C) | (B & D) | (C & D);
				k = 0x8F1BBCDC;
			} else {
				F = B ^ C ^ D;
				k = 0xCA62C1D6;
			}
			uint32_t n = LWRotateLeft(A, 5u) + F + E + k + Block[d];
			E = D;
			D = C;
			C = LWRotateLeft(B, 30u);
			B = A;
			A = n;
		}
		o[0] += A;
		o[1] += B;
		o[2] += C;
		o[3] += D;
		o[4] += E;
	}
	for (uint32_t i = 0; i < 5; i++) o[i] = LWByteBuffer::MakeHost(o[i]);
	return 20;
}

uint32_t LWCrypto::HashHMAC_SHA1(const char *InBuffer, uint32_t InBufferLen, const char *Key, uint32_t KeyLen, void *OutBuffer) {
	return HashHMAC(InBuffer, InBufferLen, Key, KeyLen, LWCrypto::HashSHA1l, 64, OutBuffer);
}

uint32_t LWCrypto::HashSHA256(const char *InBuffer, uint32_t InBufferLen, void *OutBuffer) {
	//Implementation based on pseduo-code found on https://en.wikipedia.org/wiki/SHA-2
	char Buffer[128] = {};
	uint32_t Block[64];

	const uint32_t Table[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
								   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
								   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
								   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
								   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
								   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
								   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
								   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };
	if (!OutBuffer) return 32;
	uint32_t *o = (uint32_t*)OutBuffer;
	o[0] = 0x6a09e667;
	o[1] = 0xbb67ae85;
	o[2] = 0x3c6ef372;
	o[3] = 0xa54ff53a;
	o[4] = 0x510e527f;
	o[5] = 0x9b05688c;
	o[6] = 0x1f83d9ab;
	o[7] = 0x5be0cd19;

	//Setup final chunks.
	uint32_t LastChunk = InBufferLen / 64 * 64;
	uint32_t LastByte = InBufferLen - LastChunk;
	std::copy(InBuffer + LastChunk, InBuffer + InBufferLen, Buffer);
	Buffer[LastByte] = '\x80';
	*((uint64_t*)Buffer + (LastByte < 56 ? 7 : 15)) = LWByteBuffer::MakeNetwork((uint64_t)InBufferLen * 8);

	for (uint32_t i = 0; i < InBufferLen + 9; i += 64) {
		const uint32_t *Bf = (const uint32_t*)(InBuffer+i);
		if (i + 64 > InBufferLen) Bf = (const uint32_t*)(Buffer + (i <= InBufferLen ? 0 : 64));
		std::copy(Bf, Bf + 16, Block);

		uint32_t A = o[0];
		uint32_t B = o[1];
		uint32_t C = o[2];
		uint32_t D = o[3];
		uint32_t E = o[4];
		uint32_t F = o[5];
		uint32_t G = o[6];
		uint32_t H = o[7];

		for (uint32_t d = 0; d < 16; d++) Block[d] = LWByteBuffer::MakeNetwork(Block[d]);
		for (uint32_t d = 16; d < 64; d++) {
			uint32_t s0 = LWRotateRight(Block[d - 15], 7u) ^ LWRotateRight(Block[d - 15], 18u) ^ (Block[d - 15] >> 3u);
			uint32_t s1 = LWRotateRight(Block[d - 2], 17u) ^ LWRotateRight(Block[d - 2], 19u) ^ (Block[d - 2] >> 10u);
			Block[d] = Block[d - 16] + s0 + Block[d - 7] + s1;
		}
		for (uint32_t d = 0; d < 64; d++) {
			uint32_t s1 = LWRotateRight(E, 6u) ^ LWRotateRight(E, 11u) ^ LWRotateRight(E, 25u);
			uint32_t ch = (E & F) ^ ((~E) & G);
			uint32_t T1 = H + s1 + ch + Table[d] + Block[d];
			uint32_t s0 = LWRotateRight(A, 2u) ^ LWRotateRight(A, 13u) ^ LWRotateRight(A, 22u);
			uint32_t maj = (A & B) ^ (A & C) ^ (B & C);
			uint32_t T2 = s0 + maj;

			H = G;
			G = F;
			F = E;
			E = D + T1;
			D = C;
			C = B;
			B = A;
			A = T1 + T2;
		}
		o[0] += A;
		o[1] += B;
		o[2] += C;
		o[3] += D;
		o[4] += E;
		o[5] += F;
		o[6] += G;
		o[7] += H;
	}
	for (uint32_t i = 0; i < 8; i++) o[i] = LWByteBuffer::MakeHost(o[i]);
	return 32;
}

uint32_t LWCrypto::HashSHA256l(const char **InBuffer, uint32_t *InBufferLen, uint32_t InBufferCount, void *OutBuffer) {
	//Implementation based on pseduo-code found on https://en.wikipedia.org/wiki/SHA-2
	const uint32_t ChunkSize = 64;
	char Chunk[256]; //ChunkSize is 64, but we expand out to 256 bytes.
	uint32_t *Block = (uint32_t*)Chunk;

	const uint32_t Table[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
								   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
								   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
								   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
								   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
								   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
								   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
								   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };
	if (!OutBuffer) return 32;
	uint32_t *o = (uint32_t*)OutBuffer;
	o[0] = 0x6a09e667;
	o[1] = 0xbb67ae85;
	o[2] = 0x3c6ef372;
	o[3] = 0xa54ff53a;
	o[4] = 0x510e527f;
	o[5] = 0x9b05688c;
	o[6] = 0x1f83d9ab;
	o[7] = 0x5be0cd19;

	uint32_t ActiveBuffer = 0;
	uint32_t ActiveBufferPos = 0;
	uint32_t TotalLength = 0;

	for (uint32_t i = 0; i < TotalLength + 9; i += 64) {
		TotalLength += NextChunk(InBuffer, InBufferLen, InBufferCount, ActiveBuffer, ActiveBufferPos, ChunkSize, Chunk);
		if (i + 64 > TotalLength) {
			std::fill(Chunk + (TotalLength > i ? (TotalLength - i) : 0), Chunk + ChunkSize, 0);
			if (i < TotalLength) {
				if (i + ChunkSize != TotalLength) Chunk[TotalLength - i] = '\x80';
			} else if (i == TotalLength) Chunk[0] = '\x80';
			if (i + (ChunkSize - 8) > TotalLength) *(((uint64_t*)Chunk) + 7) = LWByteBuffer::MakeNetwork((uint64_t)TotalLength * 8);
		}

		uint32_t A = o[0];
		uint32_t B = o[1];
		uint32_t C = o[2];
		uint32_t D = o[3];
		uint32_t E = o[4];
		uint32_t F = o[5];
		uint32_t G = o[6];
		uint32_t H = o[7];

		for (uint32_t d = 0; d < 16; d++) Block[d] = LWByteBuffer::MakeNetwork(Block[d]);
		for (uint32_t d = 16; d < 64; d++) {
			uint32_t s0 = LWRotateRight(Block[d - 15], 7u) ^ LWRotateRight(Block[d - 15], 18u) ^ (Block[d - 15] >> 3u);
			uint32_t s1 = LWRotateRight(Block[d - 2], 17u) ^ LWRotateRight(Block[d - 2], 19u) ^ (Block[d - 2] >> 10u);
			Block[d] = Block[d - 16] + s0 + Block[d - 7] + s1;
		}
		for (uint32_t d = 0; d < 64; d++) {
			uint32_t s1 = LWRotateRight(E, 6u) ^ LWRotateRight(E, 11u) ^ LWRotateRight(E, 25u);
			uint32_t ch = (E & F) ^ ((~E) & G);
			uint32_t T1 = H + s1 + ch + Table[d] + Block[d];
			uint32_t s0 = LWRotateRight(A, 2u) ^ LWRotateRight(A, 13u) ^ LWRotateRight(A, 22u);
			uint32_t maj = (A & B) ^ (A & C) ^ (B & C);
			uint32_t T2 = s0 + maj;

			H = G;
			G = F;
			F = E;
			E = D + T1;
			D = C;
			C = B;
			B = A;
			A = T1 + T2;
		}
		o[0] += A;
		o[1] += B;
		o[2] += C;
		o[3] += D;
		o[4] += E;
		o[5] += F;
		o[6] += G;
		o[7] += H;
	}
	for (uint32_t i = 0; i < 8; i++) o[i] = LWByteBuffer::MakeHost(o[i]);
	return 32;
}

uint32_t LWCrypto::HashHMAC_SHA256(const char *InBuffer, uint32_t InBufferLen, const char *Key, uint32_t KeyLen, void *OutBuffer) {
	return HashHMAC(InBuffer, InBufferLen, Key, KeyLen, LWCrypto::HashSHA256l, 64, OutBuffer);
}

uint32_t LWCrypto::HashFNV1A(const uint8_t *Buffer, uint32_t BufferLen, uint32_t Hash) {
	const uint32_t Prime = 16777619;
	const uint8_t *Last = Buffer + BufferLen;
	for (; Buffer != Last; ++Buffer) Hash = (Hash ^ ((uint32_t)*Buffer & 0xFF)) * Prime;
	return Hash;
}

template<>
uint32_t LWCrypto::HashFNV1A<char8_t>(const LWUnicodeIterator<char8_t> &Iter, uint32_t Hash) {
	const uint32_t Prime = 16777619;
	const char8_t *P = Iter();
	const char8_t *L = Iter.GetLast();
	for (; P != L && *P; ++P) Hash = (Hash ^ ((uint32_t)*P & 0xFF)) * Prime;
	return Hash;
}


template<>
uint32_t LWCrypto::HashFNV1A<char16_t>(const LWUnicodeIterator<char16_t> &Iter, uint32_t Hash) {
	const uint32_t Prime = 16777619;
	const char16_t *P = Iter();
	const char16_t *L = Iter.GetLast();
	for (; P != L && *P; ++P) Hash = (Hash ^ ((uint32_t)*P & 0xFFFF)) * Prime;
	return Hash;
}

template<>
uint32_t LWCrypto::HashFNV1A<char32_t>(const LWUnicodeIterator<char32_t> &Iter, uint32_t Hash) {
	const uint32_t Prime = 16777619;
	const char32_t *P = Iter();
	const char32_t *L = Iter.GetLast();
	for (; P != L && *P; ++P) Hash = (Hash ^ *P) * Prime;
	return Hash;
}


uint32_t LWCrypto::Base64Encode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen){
	if (!OutBuffer) return (InBufferLen + 2) / 3 * 4;
	char CodeTable[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
	char *OutLast = OutBuffer + OutBufferLen;
	for (uint32_t i = 0; i < InBufferLen;i+=3){
		uint32_t iA = InBuffer[i];
		uint32_t iB = (i + 1) < InBufferLen ? InBuffer[i + 1] : 0;
		uint32_t iC = (i + 2) < InBufferLen ? InBuffer[i + 2] : 0;

		uint32_t oA = (iA & 0xFC) >> 2;
		uint32_t oB = ((iA & 0x3) << 4) | ((iB & 0xF0) >> 4);
		uint32_t oC = (iB & 0xF) << 2 | (iC & 0xC0) >> 6;
		uint32_t oD = (iC & 0x3F);
		if (OutBuffer != OutLast) *OutBuffer++ = CodeTable[oA];
		if (OutBuffer != OutLast) *OutBuffer++ = CodeTable[oB];
		if (OutBuffer != OutLast) *OutBuffer++ = (i + 1) < InBufferLen ? CodeTable[oC] : '=';
		if (OutBuffer != OutLast) *OutBuffer++ = (i + 2) < InBufferLen ? CodeTable[oD] : '=';


	}
	return (InBufferLen + 2) / 3 * 4;
}

uint32_t LWCrypto::Base64Decode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen){
	if (!OutBuffer) return (InBufferLen + 3) / 4 * 3;
	char CodeTable[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
	char *OutLast = OutBuffer + OutBufferLen;
	for (uint32_t i = 0; i < InBufferLen;i+=4){
		
		uintptr_t iA = (uintptr_t)(strchr(CodeTable, InBuffer[i]) - CodeTable);
		uintptr_t iB = (uintptr_t)(strchr(CodeTable, InBuffer[i+1]) - CodeTable);
		uintptr_t iC = (uintptr_t)(strchr(CodeTable, InBuffer[i+2]));
		uintptr_t iD = (uintptr_t)(strchr(CodeTable, InBuffer[i+3]));
		iC = iC == 0 ? iC : (iC - (uintptr_t)CodeTable);
		iD = iD == 0 ? iD : (iD - (uintptr_t)CodeTable);
		if (OutBuffer != OutLast) *OutBuffer++ = (char)((iA << 2) | ((iB & 0x30) >> 4));
		if (OutBuffer != OutLast) *OutBuffer++ = (char)(((iB & 0x0F) << 4) | ((iC & 0x3C) >> 2));
		if (OutBuffer != OutLast) *OutBuffer++ = (char)(((iC & 0x03) << 6) | iD);
	}
	return (InBufferLen + 3) / 4 * 3;
}

uint32_t LWCrypto::HexEncode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen) {
	uint32_t o = 0;
	for (uint32_t i = 0; i < InBufferLen; i++) o += LWUTF8I::Fmt_ns(OutBuffer, OutBufferLen, o, "{:02x}", (uint8_t)InBuffer[i]);
	return o;
}

uint32_t LWCrypto::HexDecode(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer, uint32_t OutBufferLen) {
	uint32_t o = 0;
	auto Value = [](uint8_t C, uint32_t BitOffset) -> char {
		if (C >= '0' && C <= '9') return (C - '0') << BitOffset;
		else if (C >= 'a' && C <= 'f') return (C - 'a' + 10) << BitOffset;
		else if (C >= 'A' && C <= 'F') return (C - 'A' + 10) << BitOffset;
		return 0;
	};

	for (uint32_t i = 0; i < InBufferLen; i += 2, ++o) {
		if (o < OutBufferLen) OutBuffer[o] = Value((uint8_t)InBuffer[i], 4) | Value((uint8_t)InBuffer[i + 1], 0);
	}
	return o;
}

uint32_t LWCrypto::CRC32(const uint8_t *Data, uint32_t DataLen, uint32_t CRCCurr, bool Finished) {
	constexpr uint32_t crc32Table[256] = {
	   0x00000000u, 0x77073096u, 0xee0e612cu, 0x990951bau, 0x076dc419u, 0x706af48fu, 0xe963a535u, 0x9e6495a3u,
	   0x0edb8832u, 0x79dcb8a4u, 0xe0d5e91eu, 0x97d2d988u, 0x09b64c2bu, 0x7eb17cbdu, 0xe7b82d07u, 0x90bf1d91u,
	   0x1db71064u, 0x6ab020f2u, 0xf3b97148u, 0x84be41deu, 0x1adad47du, 0x6ddde4ebu, 0xf4d4b551u, 0x83d385c7u,
	   0x136c9856u, 0x646ba8c0u, 0xfd62f97au, 0x8a65c9ecu, 0x14015c4fu, 0x63066cd9u, 0xfa0f3d63u, 0x8d080df5u,
	   0x3b6e20c8u, 0x4c69105eu, 0xd56041e4u, 0xa2677172u, 0x3c03e4d1u, 0x4b04d447u, 0xd20d85fdu, 0xa50ab56bu,
	   0x35b5a8fau, 0x42b2986cu, 0xdbbbc9d6u, 0xacbcf940u, 0x32d86ce3u, 0x45df5c75u, 0xdcd60dcfu, 0xabd13d59u,
	   0x26d930acu, 0x51de003au, 0xc8d75180u, 0xbfd06116u, 0x21b4f4b5u, 0x56b3c423u, 0xcfba9599u, 0xb8bda50fu,
	   0x2802b89eu, 0x5f058808u, 0xc60cd9b2u, 0xb10be924u, 0x2f6f7c87u, 0x58684c11u, 0xc1611dabu, 0xb6662d3du,
	   0x76dc4190u, 0x01db7106u, 0x98d220bcu, 0xefd5102au, 0x71b18589u, 0x06b6b51fu, 0x9fbfe4a5u, 0xe8b8d433u,
	   0x7807c9a2u, 0x0f00f934u, 0x9609a88eu, 0xe10e9818u, 0x7f6a0dbbu, 0x086d3d2du, 0x91646c97u, 0xe6635c01u,
	   0x6b6b51f4u, 0x1c6c6162u, 0x856530d8u, 0xf262004eu, 0x6c0695edu, 0x1b01a57bu, 0x8208f4c1u, 0xf50fc457u,
	   0x65b0d9c6u, 0x12b7e950u, 0x8bbeb8eau, 0xfcb9887cu, 0x62dd1ddfu, 0x15da2d49u, 0x8cd37cf3u, 0xfbd44c65u,
	   0x4db26158u, 0x3ab551ceu, 0xa3bc0074u, 0xd4bb30e2u, 0x4adfa541u, 0x3dd895d7u, 0xa4d1c46du, 0xd3d6f4fbu,
	   0x4369e96au, 0x346ed9fcu, 0xad678846u, 0xda60b8d0u, 0x44042d73u, 0x33031de5u, 0xaa0a4c5fu, 0xdd0d7cc9u,
	   0x5005713cu, 0x270241aau, 0xbe0b1010u, 0xc90c2086u, 0x5768b525u, 0x206f85b3u, 0xb966d409u, 0xce61e49fu,
	   0x5edef90eu, 0x29d9c998u, 0xb0d09822u, 0xc7d7a8b4u, 0x59b33d17u, 0x2eb40d81u, 0xb7bd5c3bu, 0xc0ba6cadu,
	   0xedb88320u, 0x9abfb3b6u, 0x03b6e20cu, 0x74b1d29au, 0xead54739u, 0x9dd277afu, 0x04db2615u, 0x73dc1683u,
	   0xe3630b12u, 0x94643b84u, 0x0d6d6a3eu, 0x7a6a5aa8u, 0xe40ecf0bu, 0x9309ff9du, 0x0a00ae27u, 0x7d079eb1u,
	   0xf00f9344u, 0x8708a3d2u, 0x1e01f268u, 0x6906c2feu, 0xf762575du, 0x806567cbu, 0x196c3671u, 0x6e6b06e7u,
	   0xfed41b76u, 0x89d32be0u, 0x10da7a5au, 0x67dd4accu, 0xf9b9df6fu, 0x8ebeeff9u, 0x17b7be43u, 0x60b08ed5u,
	   0xd6d6a3e8u, 0xa1d1937eu, 0x38d8c2c4u, 0x4fdff252u, 0xd1bb67f1u, 0xa6bc5767u, 0x3fb506ddu, 0x48b2364bu,
	   0xd80d2bdau, 0xaf0a1b4cu, 0x36034af6u, 0x41047a60u, 0xdf60efc3u, 0xa867df55u, 0x316e8eefu, 0x4669be79u,
	   0xcb61b38cu, 0xbc66831au, 0x256fd2a0u, 0x5268e236u, 0xcc0c7795u, 0xbb0b4703u, 0x220216b9u, 0x5505262fu,
	   0xc5ba3bbeu, 0xb2bd0b28u, 0x2bb45a92u, 0x5cb36a04u, 0xc2d7ffa7u, 0xb5d0cf31u, 0x2cd99e8bu, 0x5bdeae1du,
	   0x9b64c2b0u, 0xec63f226u, 0x756aa39cu, 0x026d930au, 0x9c0906a9u, 0xeb0e363fu, 0x72076785u, 0x05005713u,
	   0x95bf4a82u, 0xe2b87a14u, 0x7bb12baeu, 0x0cb61b38u, 0x92d28e9bu, 0xe5d5be0du, 0x7cdcefb7u, 0x0bdbdf21u,
	   0x86d3d2d4u, 0xf1d4e242u, 0x68ddb3f8u, 0x1fda836eu, 0x81be16cdu, 0xf6b9265bu, 0x6fb077e1u, 0x18b74777u,
	   0x88085ae6u, 0xff0f6a70u, 0x66063bcau, 0x11010b5cu, 0x8f659effu, 0xf862ae69u, 0x616bffd3u, 0x166ccf45u,
	   0xa00ae278u, 0xd70dd2eeu, 0x4e048354u, 0x3903b3c2u, 0xa7672661u, 0xd06016f7u, 0x4969474du, 0x3e6e77dbu,
	   0xaed16a4au, 0xd9d65adcu, 0x40df0b66u, 0x37d83bf0u, 0xa9bcae53u, 0xdebb9ec5u, 0x47b2cf7fu, 0x30b5ffe9u,
	   0xbdbdf21cu, 0xcabac28au, 0x53b39330u, 0x24b4a3a6u, 0xbad03605u, 0xcdd70693u, 0x54de5729u, 0x23d967bfu,
	   0xb3667a2eu, 0xc4614ab8u, 0x5d681b02u, 0x2a6f2b94u, 0xb40bbe37u, 0xc30c8ea1u, 0x5a05df1bu, 0x2d02ef8du,
	};

	for (uint32_t i = 0; i < DataLen; i++) {
		CRCCurr = crc32Table[(uint8_t)(Data[i] ^ CRCCurr)] ^ CRCCurr >> 8;
	}
	if (Finished) CRCCurr ^= 0xFFFFFFFF;
	return CRCCurr;
}