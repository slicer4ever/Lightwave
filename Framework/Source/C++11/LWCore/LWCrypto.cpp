#include "LWCore/LWCrypto.h"
#include "LWCore/LWByteBuffer.h"
#include <cstring>
#include <iostream>
uint32_t LWCrypto::HashMD5(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer){
	char Buffer[64];
	uint32_t Table[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
						 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
						 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
						 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
						 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
						 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
						 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
						 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

	uint32_t Shift[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
						 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
						 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };
	uint32_t A = 0x67452301;
	uint32_t B = 0xefcdab89;
	uint32_t C = 0x98badcfe; 
	uint32_t D = 0x10325476;
	for (uint32_t i = 0; i < InBufferLen+9;i+=64){
		const uint32_t *Buf = (const uint32_t*)(InBuffer + i);
		if(i+64>InBufferLen){
			Buf = (const uint32_t*)Buffer;
			std::memset(Buffer, 0, sizeof(Buffer));
			if (i < InBufferLen){
				std::memcpy(Buffer, InBuffer + i, sizeof(char)*(InBufferLen - i));
				if (i + 64 != InBufferLen) Buffer[InBufferLen - i] = '\x80';
			} else if (i == InBufferLen) Buffer[0] = '\x80';
			if (i + 56 >= InBufferLen)	*(((uint64_t*)Buffer) + 7) = InBufferLen * 8;
		}

		uint32_t tA = A;
		uint32_t tB = B;
		uint32_t tC = C;
		uint32_t tD = D;
		for (uint32_t d = 0; d < 64;d++){
			uint32_t F = 0;
			uint32_t g = 0;
			if(d<16){
				F = (tB&tC) | ((~tB)&tD);
				g = d;
			} else if (d < 32){
				F = (tD&tB) | ((~tD)&tC);
				g = (5 * d + 1) % 16;
			}else if(d<48){
				F = tB^tC^tD;
				g = (3 * d + 5) % 16;
			}else{
				F = tC ^ (tB | (~tD));
				g = (7 * d) % 16;
			}
			uint32_t n = (tA + F + Table[d] + Buf[g]);
			tA = tD;
			tD = tC;
			tC = tB;
			tB = tB + ((n << Shift[d]) | (n >> (32 - Shift[d])));
		}
		A += tA;
		B += tB;
		C += tC;
		D += tD;
	}
	if(OutBuffer){
		*(uint32_t*)OutBuffer         = (A << 24) | (A >> 24) | ((A & 0xFF00) << 8) | ((A & 0xFF0000) >> 8);
		*(((uint32_t*)OutBuffer) + 1) = (B << 24) | (B >> 24) | ((B & 0xFF00) << 8) | ((B & 0xFF0000) >> 8);
		*(((uint32_t*)OutBuffer) + 2) = (C << 24) | (C >> 24) | ((C & 0xFF00) << 8) | ((C & 0xFF0000) >> 8);
		*(((uint32_t*)OutBuffer) + 3) = (D << 24) | (D >> 24) | ((D & 0xFF00) << 8) | ((D & 0xFF0000) >> 8);
	}
	return 16;
}

uint32_t LWCrypto::HashSHA1(const char *InBuffer, uint32_t InBufferLen, char *OutBuffer){
	char Buffer[320];

	auto OrderSwap = [](uint32_t Value)->uint32_t {
		return Value;
		//return (Value & 0xFF) << 24 | (Value & 0xFF00) << 8 | (Value & 0xFF0000) >> 8 | (Value & 0xFF000000) >> 24;
	};

	uint32_t A = 0x67452301;
	uint32_t B = 0xefcdab89;
	uint32_t C = 0x98badcfe;
	uint32_t D = 0x10325476;
	uint32_t E = 0xC3D2E1F0;
	for (uint32_t i = 0; i < InBufferLen + 9; i += 64){
		if (i + 64 > InBufferLen){
			std::memset(Buffer, 0, sizeof(Buffer));
			if (i < InBufferLen){
				std::memcpy(Buffer, InBuffer + i, sizeof(char)*(InBufferLen - i));
				if (i + 64 != InBufferLen) Buffer[InBufferLen - i] = '\x80';
			} else if (i == InBufferLen) Buffer[0] = '\x80';
			if (i + 56 >= InBufferLen){
				uint64_t Len = LWByteBuffer::MakeBig(InBufferLen * 8);
				*(((uint32_t*)Buffer) + 14) = (uint32_t)(Len >> 32);
				*(((uint32_t*)Buffer) + 15) = (uint32_t)Len;
			}
		} else std::memcpy(Buffer, InBuffer + i, sizeof(char) * 64);
		uint32_t *Buf = (uint32_t*)Buffer;
		for (uint32_t d = 0; d < 16; d++) Buf[d] = LWByteBuffer::MakeBig(Buf[d]);
		uint32_t tA = A;
		uint32_t tB = B;
		uint32_t tC = C;
		uint32_t tD = D;
		uint32_t tE = E;
		for (uint32_t d = 16; d < 80; d++){
			uint32_t n = (Buf[d - 3] ^ Buf[d - 8] ^ Buf[d - 14] ^ Buf[d - 16]);
			Buf[d] = (n << 1) | (n >> 31);
		}
		for (uint32_t d = 0; d < 80; d++){
			uint32_t F = 0;
			uint32_t k = 0;
			if (d < 20){
				F = (tB&tC) | ((~tB)&tD);
				k = 0x5A827999;
			} else if (d < 40){
				F = tB^tC^tD;
				k = 0x6ED9EBA1;
			} else if (d < 60){
				F = (tB&tC) | (tB&tD) | (tC&tD);
				k = 0x8F1BBCDC;
			} else{
				F = tB^tC^tD;
				k = 0xCA62C1D6;
			}
			uint32_t n = ((tA << 5) | (tA >> 27)) + F + tE + k + Buf[d];
			tE = tD;
			tD = tC;
			tC = (tB << 30) | (tB >> 2);
			tB = tA;
			tA = n;
		}
		A += tA;
		B += tB;
		C += tC;
		D += tD;
		E += tE;
	}
	if (OutBuffer){
		*(uint32_t*)OutBuffer = OrderSwap(A);
		*(((uint32_t*)OutBuffer) + 1) = OrderSwap(B);
		*(((uint32_t*)OutBuffer) + 2) = OrderSwap(C);
		*(((uint32_t*)OutBuffer) + 3) = OrderSwap(D);
		*(((uint32_t*)OutBuffer) + 4) = OrderSwap(E);
	}
	return 20;
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