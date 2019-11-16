#include "LWCore/LWText.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWByteBuffer.h"
#include <cstdarg>
#include <iostream>

bool LWText::isWhitespace(uint32_t Character) {
	return Character == 0x9 || Character == 0xA || Character == 0xB || Character == 0xC || Character == 0xD || Character == 0x20;
}

uint32_t LWText::UTF8ByteSize(uint32_t Character) {
	if (Character > 0xFFFF) return 4;
	if (Character > 0x7FF) return 3;
	if (Character > 0x7F) return 2;
	return 1;
}

uint32_t LWText::UTF8ByteSize(const uint8_t *P) {
	if ((*P & 0xF0) == 0xF0) return 4;
	if ((*P & 0xE0) == 0xE0) return 3;
	if ((*P & 0xC0) == 0xC0) return 2;
	return 1;
}

uint32_t LWText::UTF8ByteSize(const char *P) {
	return UTF8ByteSize((const uint8_t*)P);
}

const uint8_t *LWText::NextWord(const uint8_t *Position, bool First){
	const uint8_t *P = FirstCharacter(Position);
	for (; P; P = Next(P)){
		uint32_t Char = GetCharacter(P);
		if (isWhitespace(Char)) First = true;
		else if (First) return P;
	}
	return nullptr;
}

const char *LWText::NextWord(const char *Position, bool First){
	return (const char*)NextWord((const uint8_t*)Position, First);
}

uint8_t *LWText::NextWord(uint8_t *Position, bool First){
	uint8_t *P = FirstCharacter(Position);
	for (; P; P = Next(P)){
		uint32_t Char = GetCharacter(P);
		if (isWhitespace(Char)) First = true;
		else if (First) return P;
	}
	return nullptr;
}

char *LWText::NextWord(char *Position, bool First){
	return (char*)NextWord((uint8_t*)Position, First);
}

const uint8_t *LWText::PrevWord(const uint8_t *Position, const uint8_t *Start, bool First) {
	const uint8_t *P = FirstCharacter(Position);
	const uint8_t *C = P;
	bool Begin = false;
	for (; P; C = P, P = Prev(P, Start)) {
		uint32_t Char = GetCharacter(P);
		if(isWhitespace(Char)){
			if (Begin) return C;
			First = true;
			Begin = false;
		} else if (First) {
			Begin = true;
		}
	}
	if (Begin) return C;
	return nullptr;
}

const char *LWText::PrevWord(const char *Position, const char *Start, bool First) {
	return (const char*)PrevWord((const uint8_t*)Position, (const uint8_t*)Start, First);
}

uint8_t *LWText::PrevWord(uint8_t *Position, const uint8_t *Start, bool First) {
	uint8_t *P = FirstCharacter(Position);
	uint8_t *C = P;
	bool Begin = false;
	for (; P; C = P, P = Prev(P, Start)) {
		uint32_t Char = GetCharacter(P);
		if(isWhitespace(Char)){
			if (Begin) return C;
			First = true;
			Begin = false;
		} else if (First) {
			Begin = true;
		}
	}
	if (Begin) return C;
	return nullptr;
}

char *LWText::PrevWord(char *Position, const char *Start, bool First) {
	return (char*)PrevWord((uint8_t*)Position, (const uint8_t*)Start, First);
}

const uint8_t *LWText::FirstToken(const uint8_t *Position, uint32_t Token){
	const uint8_t *P = FirstCharacter(Position);
	for (; P; P = Next(P)) if (GetCharacter(P) == Token) return P;
	return nullptr;
}

const char *LWText::FirstToken(const char *Position, uint32_t Token){
	return (const char*)FirstToken((const uint8_t*)Position, Token);
}

uint8_t *LWText::FirstToken(uint8_t *Position, uint32_t Token){
	uint8_t *P = FirstCharacter(Position);
	for (; P; P = Next(P)) if (GetCharacter(P) == Token) return P;
	return nullptr;
}

char *LWText::FirstToken(char *Position, uint32_t Token){
	return (char*)FirstToken((uint8_t*)Position, Token);
}

const uint8_t *LWText::FirstTokens(const uint8_t *Position, const uint8_t *Tokens){
	const uint8_t *P = FirstCharacter(Position);
	for (; P; P=Next(P)){
		uint32_t PC = GetCharacter(P);
		for (const uint8_t *T = Tokens; T; T=Next(T)){
			if (PC == GetCharacter(T)) return P;
		}
	}
	return nullptr;
}

const char *LWText::FirstTokens(const char *Position, const char *Tokens){
	return (const char*)FirstTokens((const uint8_t*)Position, (const uint8_t*)Tokens);
}

uint8_t *LWText::FirstTokens(uint8_t *Position, const uint8_t *Tokens){
	uint8_t *P = FirstCharacter(Position);
	for (; P; P=Next(P)){
		uint32_t PC = GetCharacter(P);
		for (const uint8_t *T = Tokens; T; T=Next(T)){
			if (PC == GetCharacter(T)) return P;
		}
	}
	return nullptr;
}

char *LWText::FirstTokens(char *Position, const char *Tokens){
	return (char*)FirstTokens((uint8_t*)Position, (const uint8_t*)Tokens);
}

const uint8_t *LWText::CopyToTokens(const uint8_t *Position, uint8_t *Buffer, uint32_t BufferLen, const uint8_t *Tokens){
	uint8_t *BufferLast = Buffer + BufferLen;
	uint8_t *B = Buffer;
	const uint8_t *P = Position;
	if (B) {
		bool Found = false;
		for (; *P;) {
			uint32_t PC = GetCharacter(P);
			uint32_t Size = UTF8ByteSize(PC);
			for (const uint8_t *T = Tokens; T && !Found; T = Next(T)) Found = PC == GetCharacter(T);
			if (Found) break;
			if (B + Size < BufferLast) {
				std::copy(P, P + Size, B);
				B += Size;
			}
			P += Size;
		}
		if (B == BufferLast) B--;
		*B = '\0';
	} else {
		for (; *P;) {
			uint32_t PC = GetCharacter(P);
			uint32_t Size = UTF8ByteSize(PC);
			for (const uint8_t *T = Tokens; T; T = Next(T)) {
				if (PC == GetCharacter(T)) return P;
			}
			P += Size;
		}
	}
	return P;
}

const char *LWText::CopyToTokens(const char *Position, char *Buffer, uint32_t BufferLen, const char *Tokens){
	return (const char*)CopyToTokens((const uint8_t*)Position, (uint8_t*)Buffer, BufferLen, (const uint8_t*)Tokens);
}

uint8_t *LWText::CopyToTokens(uint8_t *Position, uint8_t *Buffer, uint32_t BufferLen, const uint8_t *Tokens){
	uint8_t *BufferLast = Buffer + BufferLen;
	uint8_t *B = Buffer;
	uint8_t *P = Position;
	bool Found = false;
	if (B) {
		for (; *P;) {
			uint32_t PC = GetCharacter(P);
			uint32_t Size = UTF8ByteSize(PC);
			for (const uint8_t *T = Tokens; T && !Found; T = Next(T)) Found = PC == GetCharacter(T);
			if (Found) break;
			if (B + Size < BufferLast) {
				std::copy(P, P + Size, B);
				B += Size;
			}
			P += Size;
		}
		if (B == BufferLast) B--;
		*B = '\0';
	} else {
		for (; *P;) {
			uint32_t PC = GetCharacter(P);
			uint32_t Size = UTF8ByteSize(PC);
			for (const uint8_t *T = Tokens; T; T = Next(T)) {
				if (PC == GetCharacter(T)) return P;
			}
			P += Size;
		}
	}
	return P;
}

char *LWText::CopyToTokens(char *Position, char *Buffer, uint32_t BufferLen, const char *Tokens){
	return (char*)CopyToTokens((uint8_t*)Position, (uint8_t*)Buffer, BufferLen, (const uint8_t*)Tokens);
}


uint32_t LWText::SplitToken(const uint8_t *String, uint8_t **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, uint32_t Token) {
	uint32_t c = 0;
	uint32_t p = 0;
	LongestBuffer = 0;
	for (const uint8_t *S = String; *S;) {
		uint32_t Char = GetCharacter(S);
		uint32_t Size = UTF8ByteSize(Char);
		bool isToken = Char == Token;
		if (isToken) {
			if (p < BufferCount && BufferLen) {
				if (c >= BufferLen) BufferList[p][BufferLen - 1] = '\0';
				else BufferList[p][c] = '\0';
			}
			c++;
			p++;
			LongestBuffer = std::max<uint32_t>(c, LongestBuffer);
			c = 0;
		} else {
			c += Size;
			if (p < BufferCount && c < BufferLen) std::copy(S, S + Size, BufferList[p] + (c - Size));
		}
		S += Size;
	}
	if (p < BufferCount && BufferLen) {
		if (c >= BufferLen) BufferList[p][BufferLen - 1] = '\0';
		else BufferList[p][c] = '\0';
	}
	c++;
	p++;
	LongestBuffer = std::max<uint32_t>(c, LongestBuffer);
	return p;
}

uint32_t LWText::SplitToken(const char *String, char **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, uint32_t Token) {
	return SplitToken((const uint8_t*)String, (uint8_t**)BufferList, BufferLen, BufferCount, LongestBuffer, Token);
}

uint32_t LWText::SplitTokens(const uint8_t *String, uint8_t **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, const uint8_t *Tokens) {
	uint32_t c = 0;
	uint32_t p = 0;
	LongestBuffer = 0;
	for (const uint8_t *S = String; *S;) {
		uint32_t Char = GetCharacter(S);
		uint32_t Size = UTF8ByteSize(Char);
		bool isToken = false;
		for (const uint8_t *T = Tokens; T && !isToken; T = Next(T)) isToken = Char == GetCharacter(T);
		if (isToken) {
			if (p < BufferCount && BufferLen) {
				if (c >= BufferLen) BufferList[p][BufferLen - 1] = '\0';
				else BufferList[p][c] = '\0';
			}
			c++;
			p++;
			LongestBuffer = std::max<uint32_t>(c, LongestBuffer);
			c = 0;
		} else {
			c += Size;
			if (p < BufferCount && c < BufferLen) std::copy(S, S + Size, BufferList[p] + (c - Size));
		}
		S += Size;
	}
	if (p < BufferCount && BufferLen) {
		if (c >= BufferLen) BufferList[p][BufferLen - 1] = '\0';
		else BufferList[p][c] = '\0';
	}
	c++;
	p++;
	LongestBuffer = std::max<uint32_t>(c, LongestBuffer);
	return p;
}

uint32_t LWText::SplitTokens(const char *String, char **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, const char *Tokens) {
	return SplitTokens((const uint8_t*)String, (uint8_t**)BufferList, BufferLen, BufferCount, LongestBuffer, (const uint8_t*)Tokens);
}

uint32_t LWText::SplitWords(const uint8_t *String, uint8_t **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer) {
	uint32_t c = 0;
	uint32_t p = 0;
	bool InWord = false;
	for (const uint8_t *S = String; *S;) {
		uint32_t Char = GetCharacter(S);
		uint32_t Size = UTF8ByteSize(Char);

		bool isWhite = isWhitespace(Char);
		if (isWhite && InWord) {
			if (p < BufferCount && BufferLen) {
				if (c >= BufferLen) BufferList[p][BufferLen - 1] = '\0';
				else {
					BufferList[p][c] = '\0';
				}
			}
			c++;
			InWord = false;
			p++;
			LongestBuffer = std::max<uint32_t>(c, LongestBuffer);
			c = 0;
		} else if (!isWhite) {
			c += Size;
			if (p < BufferCount && c < BufferLen) {
				std::copy(S, S + Size, BufferList[p] + (c - Size));
			}
			InWord = true;
		}
		S += Size;
	}
	if (InWord) {
		if (p < BufferCount && BufferLen) {
			if (c >= BufferLen) BufferList[p][BufferLen - 1] = '\0';
			else BufferList[p][c] = '\0';
		}
		c++;
		p++;
	}
	LongestBuffer = std::max<uint32_t>(c, LongestBuffer);
	return p;
}

uint32_t LWText::SplitWords(const char *String, char **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer) {
	return SplitWords((const uint8_t*)String, (uint8_t**)BufferList, BufferLen, BufferCount, LongestBuffer);
}

uint32_t LWText::Copy(const uint8_t *Pos, uint32_t n, uint8_t *Buffer, uint32_t BufferLen) {
	uint8_t *BLast = Buffer + BufferLen;
	uint32_t o = 0;
	for (uint32_t i = 0; i < n && (Buffer+o)!=BLast && *Pos;) {
		*(Buffer + o++) = *Pos++;
		if ((*Pos & 0xC0) != 0x80) i++;
	}

	if (Buffer + o == BLast)o--;
	Buffer[o] = '\0';
	return o;
}

uint32_t LWText::Copy(const char *Pos, uint32_t n, char *Buffer, uint32_t BufferLen) {
	return Copy((const uint8_t*)Pos, n, (uint8_t*)Buffer, BufferLen);
}

const uint8_t *LWText::FirstString(const uint8_t *Position, const uint8_t *SubString){
	for(const uint8_t *P = Position; P; P++){
		bool Valid = true;
		const uint8_t *PC = P;
		for (const uint8_t *S = SubString; *S;){
			if(*PC++!=*S++){
				Valid = false;
				break;
			}
		}
		if (Valid) return P;
	}
	return nullptr;
}

const char *LWText::FirstString(const char *Position, const char *SubString){
	return (const char*)FirstString((const uint8_t*)Position, (const uint8_t*)SubString);
}

uint8_t *LWText::FirstString(uint8_t *Position, const uint8_t *SubString){
	for (uint8_t *P = Position; P; P++){
		bool Valid = true;
		uint8_t *PC = P;
		for (const uint8_t *S = SubString; *S;){
			if (*PC++ != *S++){
				Valid = false;
				break;
			}
		}
		if (Valid) return P;
	}
	return nullptr;
}

char *LWText::FirstString(char *Position, const char *SubString){
	return (char*)FirstString((uint8_t*)Position, (const uint8_t*)SubString);
}

const uint8_t *LWText::Next(const uint8_t *Position){
	const uint8_t *NextChar = Position + UTF8ByteSize(Position);
	return *NextChar ? NextChar : nullptr;
}

const char *LWText::Next(const char *Position){
	return (const char*)Next((const uint8_t*)Position);
}

uint8_t *LWText::Next(uint8_t *Position){
	uint8_t *NextChar = Position + UTF8ByteSize(Position);
	return *NextChar ? NextChar : nullptr;
}

char *LWText::Next(char *Position){
	return (char*)Next((uint8_t*)Position);
}

const uint8_t *LWText::Prev(const uint8_t *Position, const uint8_t *String){
	if (Position == String) return nullptr;
	for (const uint8_t *P = Position - 1;; P--) if ((*P & 0xC0) != 0x80) return P;
}

const char *LWText::Prev(const char *Position, const char *String){
	return (const char*)Prev((const uint8_t*)Position, (const uint8_t*)String);
}

uint8_t *LWText::Prev(uint8_t *Position, const uint8_t *String){
	if (Position == String) return nullptr;
	for (uint8_t *P = Position - 1;; P--) if ((*P & 0xC0) != 0x80) return P;
}

char *LWText::Prev(char *Position, const char *String){
	return (char*)Prev((uint8_t*)Position, (const uint8_t*)String);
}

const uint8_t *LWText::At(const uint8_t *Text, uint32_t Index) {
	const uint8_t *C = FirstCharacter(Text);
	for (; C && Index; Index--) C = Next(C);
	return C;
}

const char *LWText::At(const char *Text, uint32_t Index) {
	return (const char*)At((const uint8_t*)Text, Index);
}

uint8_t *LWText::At(uint8_t *Text, uint32_t Index) {
	uint8_t *C = FirstCharacter(Text);
	for (; C && Index; Index--) C = Next(C);
	return C;
}

char *LWText::At(char *Text, uint32_t Index) {
	return (char*)At((uint8_t*)Text, Index);
}

uint32_t LWText::GetCharacter(const uint8_t *Position){
	if ((*Position & 0xF0) == 0xF0) return (*Position & 0x7) << 18 | (*(Position + 1) & 0x3F) << 12 | (*(Position + 2) & 0x3F) << 6 | (*(Position + 3) & 0x3F);
	else if ((*Position & 0xE0) == 0xE0) return (*Position & 0xF) << 12 | (*(Position + 1) & 0x3F) << 6 | (*(Position + 2) & 0x3F);
	else if ((*Position & 0xC0) == 0xC0) return (*Position & 0x1F) << 6 | (*(Position + 1) & 0x3F);
	return *Position;
}

uint32_t LWText::TextLength(const uint8_t *Text){
	uint32_t Len = 0;
	for (; *Text; Text++) if (((*Text) & 0xC0) != 0x80) Len++;
	return Len;
}

uint32_t LWText::TextLength(const char *Position){
	return TextLength((const uint8_t*)Position);
}

uint32_t LWText::GetUTF8Size(uint32_t Char) {
	if (Char <= 0x7F) return 1;
	if (Char <= 0x7FF) return 2;
	if (Char <= 0xFFFF) return 3;
	return 4;
}

uint32_t LWText::GetCharacter(const char *Position){
	return GetCharacter((const uint8_t*)Position);
}

uint32_t LWText::MakeUTF8To16(const uint8_t *Text, uint16_t *Buffer, uint32_t BufferLen){
	uint32_t i = 0;
	for (const uint8_t *P = FirstCharacter(Text); P; P = Next(P), i++){
		if(i<BufferLen) Buffer[i] = (uint16_t)GetCharacter(P);
	}
	if (i < BufferLen) Buffer[i] = 0;
	return i;
}

uint32_t LWText::MakeUTF8To32(const uint8_t *Text, uint32_t *Buffer, uint32_t BufferLen){
	uint32_t i = 0;
	for (const uint8_t *P = FirstCharacter(Text); P; P = Next(P), i++){
		if (i < BufferLen) Buffer[i] = GetCharacter(P);
	}
	if (i < BufferLen) Buffer[i] = 0;
	return i;
}

uint32_t LWText::MakeUTF16To8(const uint16_t *Text, uint8_t *Buffer, uint32_t BufferLen){
	uint32_t i = 0;
	for (const uint16_t *P = Text; *P; P++) {
		if (*P <= 0x7F){
			if (i < BufferLen) Buffer[i] = (uint8_t)*P;
			i++;
		}else if(*P<=0x7FF){
			if (i + 1 < BufferLen){
				Buffer[i + 0] = (uint8_t)(((*P >> 6) & 0x1F) | 0xC0);
				Buffer[i + 1] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 2;
		}else if(*P<=0xFFFF){
			if (i + 2 < BufferLen){
				Buffer[i + 0] = (uint8_t)(((*P >> 12) & 0xF) | 0xE0);
				Buffer[i + 1] = (uint8_t)(((*P >> 6) & 0x3F) | 0x80);
				Buffer[i + 2] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 3;
		}
	}
	if (i < BufferLen) Buffer[i] = '\0';
	return i + 1;
}

uint32_t LWText::MakeUTF32To8(const uint32_t *Text, uint8_t *Buffer, uint32_t BufferLen){
	uint32_t i = 0;
	for (const uint32_t *P = Text; *P; P++){
		if (*P <= 0x7F){
			if (i < BufferLen) Buffer[i] = (uint8_t)*P;
			i++;
		} else if (*P <= 0x7FF){
			if (i + 1 < BufferLen){
				Buffer[i + 0] = (uint8_t)(((*P >> 6) & 0x1F) | 0xC0);
				Buffer[i + 1] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 2;
		} else if (*P <= 0xFFFF){
			if (i + 2 < BufferLen){
				Buffer[i + 0] = (uint8_t)(((*P >> 12) & 0xF) | 0xE0);
				Buffer[i + 1] = (uint8_t)(((*P >> 6) & 0x3F) | 0x80);
				Buffer[i + 2] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 3;
		}else if(*P<=0x1FFFFF){
			if (i + 3 < BufferLen){
				Buffer[i + 0] = (uint8_t)(((*P >> 18) & 0x7) | 0xF0);
				Buffer[i + 1] = (uint8_t)(((*P >> 12) & 0x3F) | 0x80);
				Buffer[i + 2] = (uint8_t)(((*P >> 6) & 0x3F) | 0x80);
				Buffer[i + 3] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 4;
		}
	}
	if (i < BufferLen) Buffer[i] = '\0';
	return i+1;
}

uint32_t LWText::MakeUTF32To8(const uint32_t *Text, uint32_t CharCnt, uint8_t *Buffer, uint32_t BufferLen) {
	uint32_t i = 0;
	for (const uint32_t *P = Text; CharCnt && *P; P++, CharCnt--) {
		if (*P <= 0x7F) {
			if (i < BufferLen) Buffer[i] = (uint8_t)*P;
			i++;
		} else if (*P <= 0x7FF) {
			if (i + 1 < BufferLen) {
				Buffer[i + 0] = (uint8_t)(((*P >> 6) & 0x1F) | 0xC0);
				Buffer[i + 1] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 2;
		} else if (*P <= 0xFFFF) {
			if (i + 2 < BufferLen) {
				Buffer[i + 0] = (uint8_t)(((*P >> 12) & 0x1F) | 0xC0);
				Buffer[i + 1] = (uint8_t)(((*P >> 6) & 0x3F) | 0x80);
				Buffer[i + 2] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 3;
		} else if (*P <= 0x1FFFFF) {
			if (i + 3 < BufferLen) {
				Buffer[i + 0] = (uint8_t)(((*P >> 18) & 0x1F) | 0xC0);
				Buffer[i + 1] = (uint8_t)(((*P >> 12) & 0x3F) | 0x80);
				Buffer[i + 2] = (uint8_t)(((*P >> 6) & 0x3F) | 0x80);
				Buffer[i + 3] = (uint8_t)(((*P) & 0x3F) | 0x80);
			}
			i += 4;
		}
	}
	if (i < BufferLen) Buffer[i] = '\0';
	return i + 1;
}

const uint8_t *LWText::FirstCharacter(const uint8_t *Text){
	return Text ? *Text ? Text : nullptr : nullptr;
}

const char *LWText::FirstCharacter(const char *Text){
	return (const char*)FirstCharacter((const uint8_t*)Text);
}

uint8_t *LWText::FirstCharacter(uint8_t *Text){
	return Text ? *Text ? Text : nullptr : nullptr;
}

char *LWText::FirstCharacter(char *Text){
	return (char*)FirstCharacter((uint8_t*)Text);
}

bool LWText::Compare(const char *StrA, const char *StrB) {
	for (; *StrA || *StrB;) if (*StrA++ != *StrB++) return false;
	return true;
}

bool LWText::Compare(const LWText &StrA, const LWText &StrB) {
	return Compare((const char*)StrA.GetCharacters(), (const char*)StrB.GetCharacters());
}

bool LWText::Compare(const char *StrA, const char *StrB, uint32_t Count) {
	for (; (*StrA || *StrB) && Count; Count--) if (*StrA++ != *StrB++) return false; 
	return true;
}

uint32_t LWText::CompareMultiple(const char *StrA, uint32_t CompareCount, ...) {
	va_list lst;
	va_start(lst, CompareCount);
	for (uint32_t i = 0; i < CompareCount; i++) {
		const char *S = va_arg(lst, const char*);
		if (LWText::Compare(StrA, S)) {
			va_end(lst);
			return i;
		}
	}
	va_end(lst);
	return 0xFFFFFFFF;
}

uint32_t LWText::CompareMultiplen(const char *StrA, uint32_t CharCount, uint32_t CompareCount, ...) {
	va_list lst;
	va_start(lst, CompareCount);
	for (uint32_t i = 0; i < CompareCount; i++) {
		const char *S = va_arg(lst, const char*);
		if (LWText::Compare(StrA, S, CharCount)) {
			va_end(lst);
			return i;
		}
	}
	va_end(lst);
	return 0xFFFFFFFF;
}

uint32_t LWText::CompareMultiplea(const char *Str, uint32_t CompareCount, const char **Strings) {
	for (uint32_t i = 0; i < CompareCount; i++) {
		if (LWText::Compare(Str, Strings[i])) return i;
	}
	return 0xFFFFFFFF;
}

uint32_t LWText::CompareMultiplena(const char *Str, uint32_t CharCount, uint32_t CompareCount, const char **Strings) {
	for (uint32_t i = 0; i < CompareCount; i++) {
		if (LWText::Compare(Str, Strings[i], CharCount)) return i;
	}
	return 0xFFFFFFFF;
}

LWText &LWText::Set(const uint8_t *Text){
	LWAllocator *Alloc = m_TextBuffer ? LWAllocator::GetAllocator(m_TextBuffer) : nullptr;
	uint8_t *P = m_TextBuffer;
	uint8_t *T = m_TextBuffer;
	uint32_t Len = (uint32_t)std::strlen((const char*)Text) + 1;
	const uint8_t *R = Text;
	uint32_t NewBufferLen = m_BufferLength;
	if (Alloc && Len > m_BufferLength) {
		T = Alloc->AllocateArray<uint8_t>(Len);
		R = T;
		NewBufferLen = Len;
	}
	if (T) {
		std::memcpy(T, Text, sizeof(uint8_t)*Len);
		R = T;
	}
	uint32_t NewTextLen = TextLength(R);
	uint32_t NewHash = MakeHash(R);
	m_TextBuffer = T;
	m_ReadBuffer = R;
	m_TextLength = NewTextLen;
	m_BufferLength = NewBufferLen;
	m_Hash = NewHash;
	if (P != T) {
		LWAllocator::Destroy(P);
	}
	return *this;
}

LWText &LWText::Set(const char *Text){
	return Set((const uint8_t*)Text);
}

LWText &LWText::Set(const uint8_t *Text, LWAllocator &Allocator) {
	uint32_t Len = (uint32_t)std::strlen((const char*)Text) + 1;
	uint8_t *N = m_TextBuffer;
	const uint8_t *R = N;
	uint32_t NewBufferLen = m_BufferLength;
	if (Len > m_BufferLength) {
		N = Allocator.AllocateArray<uint8_t>(Len);
		R = N;
		NewBufferLen = Len;
	}
	uint8_t *P = m_TextBuffer;
	std::memcpy(N, Text, sizeof(uint8_t)*Len);
	uint32_t NewLen = TextLength(R);
	uint32_t NewHash = MakeHash(R);
	m_TextBuffer = N;
	m_ReadBuffer = R;
	m_BufferLength = NewBufferLen;
	m_TextLength = NewLen;
	m_Hash = NewHash;
	if (P != N) {
		LWAllocator::Destroy(P);
	}
	return *this;
}

LWText &LWText::Set(const char *Text, LWAllocator &Allocator){
	return Set((const uint8_t*)Text, Allocator);
}

LWText &LWText::Setf(const uint8_t *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text, lst);
	va_end(lst);
	return Set(Buffer);
}

LWText &LWText::Setf(const char *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), Text, lst);
	va_end(lst);
	return Set(Buffer);
}

LWText &LWText::Setfa(LWAllocator &Allocator, const uint8_t *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text, lst);
	va_end(lst);
	return Set(Buffer, Allocator);
}

LWText &LWText::Setfa(LWAllocator &Allocator, const char *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), Text, lst);
	va_end(lst);
	return Set(Buffer, Allocator);
}

LWText &LWText::Append(const uint8_t *Text){
	if (!m_TextBuffer) return *this;
	LWAllocator *Alloc = LWAllocator::GetAllocator(m_TextBuffer);
	
	uint8_t *N = m_TextBuffer;
	uint8_t *P = m_TextBuffer;
	uint32_t Len = (uint32_t)std::strlen((const char*)Text)+1;
	uint32_t CLen = (uint32_t)std::strlen((const char*)m_TextBuffer);
	uint32_t NewBufferLen = m_BufferLength;
	if (CLen + Len > m_BufferLength) {
		N = Alloc->AllocateArray<uint8_t>(CLen + Len);
		memcpy(N, m_TextBuffer, sizeof(uint8_t)*CLen);
		NewBufferLen = CLen + Len;
	}
	memcpy(N + CLen, Text, sizeof(uint8_t)*Len);
	uint32_t NewLen = TextLength(N);
	uint32_t NewHash = MakeHash(N);
	m_TextBuffer = N;
	m_ReadBuffer = N;
	m_BufferLength = NewBufferLen;
	m_TextLength = NewLen;
	m_Hash = NewHash;
	if(P!=N) LWAllocator::Destroy(P);
	return *this;
}

LWText &LWText::Append(const char *Text){
	return Append((const uint8_t*)Text);
}

LWText &LWText::Appendf(const uint8_t *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text, lst);
	va_end(lst);
	return Append(Buffer);
}

LWText &LWText::Appendf(const char *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), Text, lst);
	va_end(lst);
	return Append(Buffer);
}

LWText &LWText::PreAppend(const uint8_t *Text) {
	if (!m_TextBuffer) return *this;
	LWAllocator *Alloc = LWAllocator::GetAllocator(m_TextBuffer);

	uint8_t *N = m_TextBuffer;
	uint8_t *P = m_TextBuffer;
	uint32_t Len = (uint32_t)std::strlen((const char*)Text);
	uint32_t CLen = (uint32_t)std::strlen((const char*)m_TextBuffer)+1;
	uint32_t NewBufferLen = m_BufferLength;
	if (CLen + Len > m_BufferLength) {
		N = Alloc->AllocateArray<uint8_t>(CLen + Len);
		NewBufferLen = CLen + Len;
	}
	memcpy(N, Text, sizeof(uint8_t)*Len);
	memcpy(N + Len, m_TextBuffer, sizeof(uint8_t)*CLen);
	uint32_t NewLen = TextLength(N);
	uint32_t NewHash = MakeHash(N);
	m_TextBuffer = N;
	m_ReadBuffer = N;
	m_BufferLength = NewBufferLen;
	m_TextLength = NewLen;
	m_Hash = NewHash;
	if (P != N) LWAllocator::Destroy(P);
	return *this;
}

LWText &LWText::PreAppend(const char *Text){
	return PreAppend((const uint8_t*)Text);
}

LWText &LWText::PreAppendf(const uint8_t *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text, lst);
	va_end(lst);
	return PreAppend(Buffer);
}

LWText &LWText::PreAppendf(const char *Text, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, Text);
	vsnprintf(Buffer, sizeof(Buffer), Text, lst);
	va_end(lst);
	return PreAppend(Buffer);
}

LWText &LWText::operator=(LWText &&Other){
	LWAllocator::Destroy(m_TextBuffer);
	m_TextBuffer = Other.m_TextBuffer;
	m_ReadBuffer = Other.m_ReadBuffer;
	m_BufferLength = Other.m_BufferLength;
	m_TextLength = Other.m_TextLength;
	m_Hash = Other.m_Hash;
	Other.m_TextBuffer = nullptr;
	return *this;
}

bool LWText::operator==(const LWText &Rhs) const{
	return m_Hash == Rhs.m_Hash;
}

bool LWText::operator==(const uint8_t *Rhs) const{
	return m_Hash == MakeHash(Rhs);
}

bool LWText::operator==(const char *Rhs) const{
	return m_Hash == MakeHash(Rhs);
}

bool LWText::operator!=(const LWText &Rhs) const{
	return !(*this == Rhs);
}

bool LWText::operator!=(const uint8_t *Rhs) const{
	return !(*this == Rhs);
}

bool LWText::operator!=(const char *Rhs) const{
	return !(*this == Rhs);
}

LWText &LWText::operator+=(const LWText &Rhs){
	return Append(Rhs.GetCharacters());
}

LWText &LWText::operator+=(const uint8_t *Rhs){
	return Append(Rhs);
}

LWText &LWText::operator+=(const char *Rhs){
	return Append(Rhs);
}

LWText &LWText::operator=(const LWText &Other){
	Set(Other.GetCharacters(), *Other.GetAllocator());
	return *this;
}

std::ostream &operator<<(std::ostream &o, const LWText &Text) {
	o << (const char*)Text.GetCharacters();
	return o;
}

uint32_t LWText::GetLength(void) const{
	return m_TextLength;
}

uint32_t LWText::GetBufferLength(void) const{
	return m_BufferLength;
}

uint32_t LWText::GetHash(void) const{
	return m_Hash;
}

const uint8_t *LWText::GetCharacters(void) const{
	return m_ReadBuffer;
}

LWAllocator *LWText::GetAllocator(void) const{
	return m_TextBuffer?LWAllocator::GetAllocator(m_TextBuffer):nullptr;
}

LWText::LWText(LWText &&Other) : m_TextBuffer(Other.m_TextBuffer), m_ReadBuffer(Other.m_ReadBuffer), m_TextLength(Other.m_TextLength), m_BufferLength(Other.m_BufferLength), m_Hash(Other.m_Hash){
	Other.m_TextBuffer = nullptr;
}

LWText::LWText(const LWText &Other) : m_TextBuffer(nullptr), m_ReadBuffer(nullptr), m_TextLength(0), m_BufferLength(0), m_Hash(0){
	Set(Other.GetCharacters(), *Other.GetAllocator());
}

LWText::LWText(const uint8_t *Text) : m_TextBuffer(nullptr), m_ReadBuffer(nullptr), m_TextLength(0), m_BufferLength(0), m_Hash(0){
	Set(Text);
}

LWText::LWText(const char *Text) : m_TextBuffer(nullptr), m_ReadBuffer(nullptr), m_TextLength(0), m_BufferLength(0), m_Hash(0){
	Set(Text);
}

LWText::LWText(const uint8_t *Text, LWAllocator &Allocator) : m_TextBuffer(nullptr), m_ReadBuffer(nullptr), m_TextLength(0), m_BufferLength(0), m_Hash(0){
	Set(Text, Allocator);
}

LWText::LWText(const char *Text, LWAllocator &Allocator) : m_TextBuffer(nullptr), m_ReadBuffer(nullptr), m_TextLength(0), m_BufferLength(0), m_Hash(0){
	Set(Text, Allocator);
}

LWText::LWText() : m_TextBuffer(nullptr), m_ReadBuffer(nullptr), m_TextLength(0), m_BufferLength(0), m_Hash(0){}

LWText::~LWText(){
	LWAllocator::Destroy(m_TextBuffer);
}
