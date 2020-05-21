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