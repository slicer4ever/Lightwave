#ifndef LWUNICODE_ITERATOR_H
#define LWUNICODE_ITERATOR_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWCrypto.h"
#include <iostream>
#include <cstdarg>

/*!< \brief LWUTFC_View is for giving to c api's a static null terminated copy of the iterator. */
template<class Type, std::size_t Len>
struct LWUTFC_View {
	Type m_Data[Len];
	//const Type *m_ReadData = m_Data;
	uint32_t m_DataLen = 0;

	/*!< \brief returns the underlying data for giving to the c-api(since these c-api's almost universally take a const char *, we return a const char *). */
	const char *operator*(void) const {
		return (const char *)m_Data;
	}

	// \brief hash's the c-view string by calling the UnicodeIterators hash function on it. */
	uint32_t Hash(void) const {
		return LWUnicodeIterator<Type>(m_Data).Hash();
	}

	/*!< \brief returns an iterator to C_View data. */
	LWUnicodeIterator<Type> operator()(void) const {
		return LWUnicodeIterator<Type>(m_Data);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWUTFC_View<Type, Len> &cView) {
		return o << LWUnicodeIterator<Type>(cView);
	}

	/*!< \brief constructs the c string copy of the iterator range, if Iterator ends on a null character then ReadData will point to the iterator current position instead. */
	LWUTFC_View(const LWUnicodeIterator<Type> &Iterator) {
		uint32_t L = Iterator.Copy(m_Data, Len);
		//if (L) m_ReadData = (*(Iterator() + (L - 1))) == 0 ? Iterator() : m_Data;
		m_DataLen = std::min<uint32_t>(L, Len);
	}

	/*!< \brief constructs a formated string into data. */
	template<typename ...Args>
	LWUTFC_View(const Type *Fmt, Args ...Pack) {
		m_DataLen = std::min<uint32_t>((uint32_t)fmt::format_to_n((char *)m_Data, Len - 1, Fmt, Pack...).size, Len - 1); //Truncate string if it exceeds our buffer.
		m_Data[m_DataLen++] = '\0'; //Add null character.
	}

	/*!< \brief default constructor for c_view. */
	LWUTFC_View() = default;
};

/*!< \brief Unicode UTF iterator, iterates over codepoints for utf-8, utf-16, and utf-32.  String should be validated first with Create, otherwise if constructed directly the application must ensure the utf is valid.  utf-8 is specialized as char8_t, utf-16 is char16_t, and utf-32 is char32_t.  no other specializations were created. */
template<class Type>
class LWUnicodeIterator {
public:
	static const uint32_t EmptyHash = LWCrypto::FNV1AHash;

	static const uint32_t MaxCodePoints = 0x10FFFF;

	/*!< \brief forwards fmt arguments to C_View constructor. */
	template<uint32_t Len, typename ...Args>
	static LWUTFC_View<Type, Len> Fmt(const Type *Fmt, Args ...Pack) {
		return LWUTFC_View<Type, Len>(Fmt, Pack...);
	}

	/*!< \brief fmt's into Buffer, returning the length of bytes needed to hold the formatted string(-1 as null terminator is not included), automatically add's \0 if buffer is not null. */
	template<typename ...Args>
	static uint32_t Fmt_n(Type *Buffer, uint32_t BufferLen, const Type *Fmt, Args ...Pack) {
		Type *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
		uint32_t Len = (uint32_t)fmt::format_to_n((char*)Buffer, BufferLen, Fmt, Pack...).size;
		Type *B = std::min<Type*>(Buffer + Len, BL);
		if (BufferLen) *B = '\0';
		return Len;
	}

	/*!< \brief fmt's into buffer at offset if offset is not past the buffer len, this is a safe way to serialize data as this function will not write pass the buffer, and will place a \0 if buffer is not null. */
	template<typename ...Args>
	static uint32_t Fmt_ns(Type *Buffer, uint32_t BufferLen, uint32_t BufferOffset, const Type *Fmt, Args ...Pack) {
		uint32_t Remain = BufferOffset >= BufferLen ? 0 : BufferLen - BufferOffset;
		return Fmt_n(Buffer + BufferOffset, Remain, Fmt, Pack...);
	}

	/*!< \brief returns the number of units(Type's) required to occupy this codepoint. */
	static uint32_t CodePointUnitSize(uint32_t CodePoint);
	
	/*!< \brief returns the number of unit's the current position is expected to be occupying. */
	static uint32_t CodePointUnitSize(const Type *Pos);

	/*!< \brief create's a codepoint at the specified position. */
	static uint32_t DecodeCodePoint(const Type *Pos);

	/*!< \brief encode's a codepoint into Buffer if BufferSize can contain the codepoint.
	*	 \return the number of unit's needed to store the codepoint. */
	static uint32_t EncodeCodePoint(Type *Buffer, uint32_t BufferSize, uint32_t CodePoint);

	/*!< \brief encode's a codepoint at the specified offset of the bufferSize, and adds a null terminator to the buffer if BufferOffset+CodePoitnSize < BufferSize.
	*	 \return the number of unit's needed to store the codepoint(without the terminator).
	*/
	static uint32_t EncodeCodePoint_s(Type *Buffer, uint32_t BufferSize, uint32_t BufferOffset, uint32_t CodePoint) {
		uint32_t Remaining = BufferSize - std::min<uint32_t>(BufferSize, BufferOffset);
		uint32_t r = EncodeCodePoint(Buffer + BufferOffset, Remaining, CodePoint);
		if (Remaining > r) Buffer[BufferOffset + r] = '\0';
		else if (Remaining) Buffer[BufferOffset] = '\0';
		return r;
	}

	/*!< \brief iterates over the iterator and checks that no malformed utf is detected. */
	static bool ValidateIterator(const LWUnicodeIterator<Type> &Iter, uint32_t &Length, uint32_t &RawLength) {
		LWUnicodeIterator<Type> C = Iter;
		while (C.ValidateAdvance()) C.Advance();
		Length = Iter.Distance(C);
		RawLength = Iter.RawDistance(C) + 1; //Include null.
		return C.AtEnd();
	}

	/*!< \brief iterates over buffer and validates the utf-8 string is not malformed in anyway, if it detects malformed false is return.  also counts the number of codepoints in Length, and the raw buffer length(+1) that is needed to store the string Buffer points to.  
		 \note BufferSize should be the space allocated for buffer, it does not need to be the strlen(Buffer), ValidateString will automatically detect a null terminated string.  if a non null terminated string is passed then the iterator will stop when it reach's BufferLength instead.
	*/
	static bool ValidateString(const Type *Buffer, uint32_t BufferSize, uint32_t &Length, uint32_t &RawLength) {
		if (!BufferSize) return true;
		return ValidateIterator({ Buffer, BufferSize }, Length, RawLength);
	}

	/*!< \brief iterates over a nul terminated heap buffer, and validates the utf-8 string is not malformed in anyway. */
	template<size_t Len>
	static bool ValidateString(const Type(&Buffer)[Len], uint32_t &Length, uint32_t &RawLength) {
		return ValidateIterator({ Buffer }, Length, RawLength);
	}

	/*!< \brief iterates over a nul terminated buffer and validates the utf-8 string is not malformed in anyway, if it detects malformed false is returned.  also counts the number of codepoints in length, and raw buffer length(+1) that is needed to store the string buffer points to. */
	static bool ValidateString(const Type *Buffer, uint32_t &Length, uint32_t &RawLength) {
		return ValidateIterator({ Buffer }, Length, RawLength);
	}
	
	/*!< \brief simple check that codepoint is inside the valid codepoint range of unicode. */
	static bool ValidateCodePoint(uint32_t CodePoint) {
		return CodePoint < MaxCodePoints;
	}

	/*!< \brief safe way to create an iterator from a c_str(must be casted from char to char8_t, or converted to relevant type).  */
	static bool Create(LWUnicodeIterator<Type> &Res, const Type *Buffer, uint32_t BufferSize, uint32_t &Length, uint32_t &RawLength) {
		if (!ValidateString(Buffer, BufferSize, Length, RawLength)) return false;
		Res = LWUnicodeIterator<Type>(Buffer, RawLength);
		return true;
	}

	/*!< \brief safe way to create an iterator from a c_str(must be casted from char to char8_t, or converted to relevant type).  */
	template<size_t Len>
	static bool Create(LWUnicodeIterator<Type> &Res, const Type(&Buffer)[Len], uint32_t &Length, uint32_t &RawLength) {
		if (!ValidateString(Buffer, Length, RawLength)) return false;
		Res = LWUnicodeIterator<Type>(Buffer, RawLength);
		return true;
	}

	/*!< \brief safe way to create an iterator from a c_str(must be casted from char to char8_t, or converted to relevant type).  */
	static bool Create(LWUnicodeIterator<Type> &Res, const Type *Buffer, uint32_t &Length, uint32_t &RawLength) {
		if (!ValidateString(Buffer, Length, RawLength)) return false;
		Res = LWUnicodeIterator<Type>(Buffer, RawLength);
		return true;
	}

	/*!< \brief count's the number of lines in source. */
	static uint32_t CountLines(const LWUnicodeIterator<Type> &Source) {
		uint32_t Line = 0;
		for (LWUnicodeIterator<Type> C = Source; !C.AtEnd(); ++C) {
			if (C.isLineBreak()) ++Line;
		}
		return Line;
	}

	/*!< \brief count's the number of word's in source. */
	static uint32_t CountWords(const LWUnicodeIterator<Type> &Source) {
		bool InWord = false;
		uint32_t Words = 0;
		for (LWUnicodeIterator<Type> C = Source; !C.AtEnd(); ++C) {
			if (C.isWhitespace()) InWord = false;
			else if (!InWord) {
				Words++;
				InWord = true;
			}
		}
		return Words;
	}

	/*!< \brief count's the number of substring's in source. */
	static uint32_t CountSubStrings(const LWUnicodeIterator<Type> &Source, const LWUnicodeIterator<Type> &SubString) {
		uint32_t SubStrings = 0;
		for (LWUnicodeIterator<Type> C = Source; !C.AtEnd(); ++C) {
			if (C.isSubString(SubString)) ++SubStrings;
		}
		return SubStrings;
	}

	/*!< \brief count's the number of token's in source. */
	static uint32_t CountToken(const LWUnicodeIterator<Type> &Source, uint32_t Token) {
		uint32_t TokenCnt = 0;
		for (LWUnicodeIterator<Type> C = Source; !C.AtEnd(); ++C) {
			if (C.isToken(Token)) ++TokenCnt;
		}
		return TokenCnt;
	}

	/*!< \brief count's the number of tokenlist's in source upto Pos. */
	static uint32_t CountTokens(const LWUnicodeIterator<Type> &Source, const LWUnicodeIterator<Type> &TokenList) {
		uint32_t TokenCnt = 0;
		for (LWUnicodeIterator<Type> C = Source; !C.AtEnd(); ++C) {
			if (C.isTokens(TokenList)) ++TokenCnt;
		}
		return TokenCnt;
	}

	/*!< \brief returns true if the codepoint is a whitespace. */
	static bool isWhitespace(uint32_t CodePoint) {
		//extended list from: https://www.compart.com/en/unicode/category/Zs & https://jkorpela.fi/chars/spaces.html
		return (CodePoint >= 0x9 && CodePoint <= 0xD) || CodePoint == 0x20 || CodePoint == 0xA0 || CodePoint == 0x1680 || (CodePoint >= 0x2001 && CodePoint <= 0x200B) || CodePoint == 0x202F || CodePoint == 0x205F || CodePoint == 0x3000 || CodePoint == 0xFEFF;
	}

	/*!< \brief returns true if the codepoint is a linebreak character.(\n). */
	static bool isLineBreak(uint32_t CodePoint) {
		return CodePoint == '\n';
	}

	/*!< \brief returns true if the codepoint is a line ending character(\r or \n). */
	static bool isLineEnd(uint32_t CodePoint) {
		return CodePoint == '\n' || CodePoint == '\r';
	}

	/*!< \brief explicit bool conversion which returns true if not at end. */
	explicit operator bool(void) const {
		return !AtEnd();
	}

	/*!< \brief returns if Pos is currently at the start of a codepoint, or in the middle of one. */
	static bool isLeadCodeUnit(const Type *Pos);

	/*!< \brief returns if Pos is currently in the middle of a codepoint. */
	static bool isTrailingCodeUnit(const Type *Pos);

	/*! \brief constructs a utfX string from the current encoding(starting at the current position), where X is based on TargetType(char8_t, char16_t, char32_t). returns the size of the needed buffer(including null character) / sizeof(TargetType). */
	template<class TargetType>
	uint32_t MakeUTF(TargetType *Buffer, uint32_t BufferSize) const {
		uint32_t o = 0;
		LWUnicodeIterator<Type> Iter = *this;
		TargetType *B = Buffer;
		TargetType *BL = B + std::min<uint32_t>(BufferSize - 1, BufferSize);
		for (; !Iter.AtEnd(); ++Iter) {
			uint32_t r = LWUnicodeIterator<TargetType>::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), *Iter);
			B = std::min<TargetType*>(B + r, BL);
			o += r;
		}
		if (BufferSize) *B = '\0';
		return o + 1;
	}

	/*!< \brief lower's characters and copy's result into Buffer, note that this function only encompasses the English A-Z = a-z, the full unicode case matching is not yet supported. */
	uint32_t Lower(Type *Buffer, uint32_t BufferSize) const {
		Type *B = Buffer;
		Type *BL = B + std::min<uint32_t>(BufferSize, BufferSize - 1);
		LWUnicodeIterator<Type> Iter = *this;
		uint32_t o = 0;
		for (; !Iter.AtEnd(); ++Iter) {
			uint32_t CP = *Iter;
			if (CP >= 'A' && CP <= 'Z') CP += 32;
			uint32_t r = EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), CP);
			B = std::min<Type*>(B + r, BL);
			o += r;
		}
		if (BufferSize) *B = '\0';
		return o + 1;
	}

	/*!< \brief Upper's characters and copy's result into Buffer, note that this function only encompasses the English a-z = A-Z, the full unicode case matching is not yet supported. */
	uint32_t Upper(Type *Buffer, uint32_t BufferSize) const {
		Type *B = Buffer;
		Type *BL = B + std::min<uint32_t>(BufferSize, BufferSize - 1);
		LWUnicodeIterator<Type> Iter = *this;
		uint32_t o = 0;
		for (; !Iter.AtEnd(); ++Iter) {
			uint32_t CP = *Iter;
			if (CP >= 'a' && CP <= 'z') CP -= 32;
			uint32_t r = EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), CP);
			B = std::min<Type*>(B + r, BL);
			o += r;
		}
		if (BufferSize) *B = '\0';
		return o + 1;
	}
	

	/*!< \brief Concat's this iterator to the end of buffer
		 \return size of buffer(per type) needed to contain that concat'd string. */
	uint32_t Concat(Type *Buffer, uint32_t BufferSize) {
		LWUnicodeIterator<Type> Iter = LWUnicodeIterator<Type>(Buffer, BufferSize);
		uint32_t Len = (uint32_t)Iter.RawLength();
		return Len + Copy(Buffer+Len, BufferSize-std::min<uint32_t>(Len, BufferSize));
	}


	/*!< \brief Concat's this iterator to the end of buffer
	*	 \param Count the number of codepoints wanted to be copy.
		 \return size of buffer(per type) needed to contain that concat'd string. */
	uint32_t Concat(Type *Buffer, uint32_t BufferSize, uint32_t Count) {
		LWUnicodeIterator<Type> Iter = LWUnicodeIterator<Type>(Buffer, BufferSize);
		uint32_t Len = (uint32_t)Iter.RawLength();
		return Len + Copy(Buffer + Len, BufferSize - std::min<uint32_t>(Len, BufferSize), Count);
	}

	/*!< \brief Concat's this iterator to the end of buffer
		 \return size of buffer(per type) needed to contain that concat'd string. */
	uint32_t Concat(Type *Buffer, uint32_t BufferSize, const LWUnicodeIterator<Type> &Other) {
		LWUnicodeIterator<Type> Iter = LWUnicodeIterator<Type>(Buffer, BufferSize);
		uint32_t Len = (uint32_t)Iter.RawRemaining();
		return Len + Copy(Buffer + Len, BufferSize - std::min<uint32_t>(Len, BufferSize), Other);
	}

	/*!< \brief copy's from current position to Count codepoints.
	*	 \param Count the number of codepoints wanted to be copy.
	*	 \return size of buffer(per Type) needed to contain the string. */
	uint32_t Copy(Type *Buffer, uint32_t BufferSize, uint32_t Count) const {
		Type *Last = Buffer + std::min<uint32_t>(BufferSize, BufferSize-1);
		const Type *P = m_Position;
		const Type *PL = m_Last;
		for (; P != PL && *P && Count--;) {
			uint32_t r = CodePointUnitSize(P);
			if (Buffer + r <= Last) {
				for (uint32_t i = 0; i < r; i++) *Buffer++ = P[i];
			} else Buffer = Last;
			P += r;
		}
		if (BufferSize) *Buffer = '\0';
		return (uint32_t)(uintptr_t)(P - m_Position) + 1;
	}

	/*!< \brief copy's this iterator out to buffer, this is necessary for 3rd party api's that can't receive the iterator and need a fresh pointer. */
	uint32_t Copy(Type *Buffer, uint32_t BufferSize) const {
		Type *Last = Buffer + std::min<uint32_t>(BufferSize, BufferSize - 1);
		const Type *P = m_Position;
		const Type *PL = m_Last;
		for (; P != PL && *P;) {
			uint32_t r = CodePointUnitSize(P);
			if (Buffer + r <= Last) {
				for (uint32_t i = 0; i < r; i++) *Buffer++ = P[i];
			} else Buffer = Last;
			P += r;
		}
		if (BufferSize) *Buffer = '\0';
		return (uint32_t)(uintptr_t)(P - m_Position) + 1;
	}


	/*!< \brief copy's from position to Other iterator position.  asserts if Others position is not inside of this's range(or if other is behind this, then this must be inside other's range). */
	uint32_t Copy(Type *Buffer, uint32_t BufferSize, const LWUnicodeIterator<Type> &Other) const {
		if (m_Position > Other.m_Position) return Other.Copy(Buffer, BufferSize, *this);
		uint32_t o = 0;
		Type *Last = Buffer + std::min<uint32_t>(BufferSize, BufferSize-1);
		const Type *P = m_Position;
		const Type *OP = Other.m_Position;
		LWVerify(Other.m_Position >= m_First && Other.m_Position <= m_Last);
		for (; P != OP; P++, o++) {
			if (Buffer < Last) *Buffer++ = *P;
		}
		if (BufferSize) *Buffer = '\0';
		return o+1;
	}

	/*!< \brief returns the CodePoint position is currently on. */
	uint32_t CodePoint(void) const {
		return DecodeCodePoint(m_Position);
	}

	/*!< \brief returns if the current codepoint is a whitespace. */
	bool isWhitespace(void) const {
		return isWhitespace(CodePoint());
	}

	/*!< \brief returns if the current codepoint is a linebreak. */
	bool isLineBreak(void) const {
		return isLineBreak(CodePoint());
	}

	/*!< \brief returns if the current codepoint is a line ending codepoint. */
	bool isLineEnd(void) const {
		return isLineEnd(CodePoint());
	}

	/*!< \brief returns true if the current codepoint is the specified token. */
	bool isToken(uint32_t Token) const {
		return CodePoint() == Token;
	}

	/*!< \brief returns true if the current codepoint is the start of the substring.*/
	bool isSubString(const LWUnicodeIterator<Type> &SubString) {
		LWUnicodeIterator<Type> C = *this;
		LWUnicodeIterator<Type> N = SubString;
		bool isEqual = true;
		while (isEqual && !N.AtEnd()) isEqual = *C++ == *N++;
		return isEqual;
	}

	/*!< \brief returns true if the current codepoint is at the end of the substring. */
	bool isrSubString(const LWUnicodeIterator<Type> &SubString) {
		LWUnicodeIterator<Type> C = *this;
		LWUnicodeIterator<Type> N = --(SubString.NextEnd());
		bool isEqual = true;
		for (; isEqual && !N.AtStart() && !C.AtStart();) isEqual = *C-- == *N--;
		return isEqual && *C == *N && N.AtStart();
	}

	/*!< \brief returns true if the current codepoint is any of the tokens in the tokenlist. */
	bool isTokens(const LWUnicodeIterator<Type> &TokenList) {
		LWUnicodeIterator<Type> C = *this;
		LWUnicodeIterator<Type> T = TokenList;
		for (; !T.AtEnd();) {
			if (*C == *T++) return true;
		}
		return false;
	}

	/*!< \brief calculates the CodePoint distance between two iterators, if O is to the left of this, then the result will be negative. */
	int32_t Distance(const LWUnicodeIterator<Type> &O) const {
		return O.m_Index - m_Index;
	}

	/*!< \brief calculates the raw distance between two iterators, both iterators must have the same First pointer for this to work correctly. returns negative if O is to the left of this. */
	int32_t RawDistance(const LWUnicodeIterator<Type> &O) const {
		return (int32_t)(intptr_t)(O.m_Position - m_Position);
	}

	/*!< \brief simple dereference to get the CodePoint at the current position. */
	uint32_t operator *(void) const {
		return DecodeCodePoint(m_Position);
	}

	/*!< \brief advance's the iterator by one codepoint. */
	LWUnicodeIterator<Type> &operator++() {
		return Advance();
	}

	/*!< \brief advance's the iterator by one codepoint, returns the previous position. */
	LWUnicodeIterator<Type> operator++(int) {
		LWUnicodeIterator<Type> C = *this;
		Advance();
		return C;
	}

	/*!< \brief advance's the iterator by Dis codepoints.  Iterator will not advance past Last. */
	LWUnicodeIterator<Type> &operator+=(uint32_t Dis) {
		return Advance(Dis);
	}

	/*!< \brief returns a new iterator by Dis codepoints.  Iterator will not be past Last. */
	LWUnicodeIterator<Type> operator+(uint32_t Dis) const {
		return Next(Dis);
	}

	/*!< \brief reverse's the iterator by one codepoint, will not go behind First. */
	LWUnicodeIterator<Type> &operator--() {
		return rAdvance();
	}

	/*!< \brief reverse's the iterator by one codepoint, returns the previous position. */
	LWUnicodeIterator<Type> operator--(int) {
		LWUnicodeIterator<Type> N = *this;
		rAdvance();
		return N;
	}

	/*!< \brief reverse's the iterator by Dis codepoint's. */
	LWUnicodeIterator<Type> &operator-=(uint32_t Dis) {
		return rAdvance(Dis);
	}

	/*!< \brief returns a new iterator Dis prev codepoints, or start if encountered first. */
	LWUnicodeIterator<Type> operator-(uint32_t Dis) const {
		return Prev(Dis);
	}

	/*!< \brief returns the underlying position pointer. */
	const Type *operator ()(void) const {
		return m_Position;
	}

	/*!< \brief returns a non const underlying position pointer. */
	Type *operator ()(void) {
		return (Type*)m_Position;
	}

	/*!< \brief returns a 32 bit hash'd representation of this string.  This hashing is based on FNVA1A implementation as found in LWCrypto. */
	uint32_t Hash(void) const {
		return LWCrypto::HashFNV1A(*this);
	}

	/*!< \brief generates iterator's between each token, and end at next token(or end of Iterator). 
		 \param IterBuffer buffer of iterator's for each split.
		 \param IterBufferSize the number of buffer's available to be used.
		 \param Token the token to split on.
		 \return the number of iterator's needed to split the string completely.
	*/
	uint32_t SplitToken(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize, uint32_t Token) const {
		LWUnicodeIterator<Type> P = *this;
		LWUnicodeIterator<Type> C = NextToken(Token, false);
		uint32_t o = 0;
		for (; !C.AtEnd(); P=C.Next(), C.AdvanceToken(Token), o++) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeIterator<Type>(P, C);
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief generates iterator's between each token in the token list, and end at next token(or end of Iterator).
	*	 \param IterBuffer buffer of iterator's for each split.
	*    \param IterBufferSize the number of buffer's available to be used.
	*    \param TokenList the iterator to the list of token's.
	*    \return the number of iterator's needed to split the string completely.
	*/
	uint32_t SplitTokenList(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize, const LWUnicodeIterator<Type> &TokenList) const {
		LWUnicodeIterator<Type> P = *this;
		LWUnicodeIterator<Type> C = NextTokens(TokenList, false);
		uint32_t o = 0;
		for (; !C.AtEnd(); P = C.Next(), C.AdvanceTokens(TokenList), o++) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeIterator<Type>(P, C);
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief generates iterator's between each word, and end at end of word(or end of iterator).
	*	 \param IterBuffer buffer of iterator's for each split.
	*    \param IterBufferSize the number of IterBuffer's available to be used.
	*    \return the number of iterator's needed to split the string to each word.
	*/
	uint32_t SplitWords(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize) const {
		LWUnicodeIterator<Type> P = (*this).NextWord(true);
		LWUnicodeIterator<Type> C = P;
		uint32_t o = 0;
		for (; !C.AtEnd();) {
			if (C.isWhitespace()) {
				if (o++ < IterBufferSize) IterBuffer[o-1] = LWUnicodeIterator<Type>(P, C);
				P = C.AdvanceWord(true);
			} else ++C;
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief generates iterator's between each line, and end at end of line(or end of iterator).
	*	 \param IterBuffer buffer of iterator's for each split.
	*    \param IterBufferSize the number of IterBuffer's available to be used.
	*	 \return the number of iterator's needed to split the string to each line.
	*/
	uint32_t SplitLine(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize) const {
		LWUnicodeIterator<Type> P = (*this).NextLine(true);
		LWUnicodeIterator<Type> C = P;
		uint32_t o = 0;
		for (; !C.AtEnd();) {
			if (C.isLineEnd()) {
				if (o++ < IterBufferSize) IterBuffer[o - 1] = LWUnicodeIterator<Type>(P, C);
				P = C.AdvanceLine();
			} else ++C;
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief iterates to the next first whitespace codepoint, then to the first non-whitespace codepoint.
	*	 \param First will skip iterating to the next first whitespace codepoint, will return when finds first non-whitespace codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeIterator<Type> &AdvanceWord(bool First = false) {
		for (; !AtEnd(); Advance()) {
			if (isWhitespace()) First = true;
			else if (First) return *this;
		}
		return *this;
	}

	/*!< \brief advance's over Len word if possible. */
	LWUnicodeIterator<Type> &AdvanceWord(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) AdvanceWord(First && i == 0);
		return *this;
	}

	/*!< \brief iterates to the next first linebreak codepoint, then to the first non-linebreak codepoint.
	*	 \param First will skip iterating to the next first linebreak codepoint, will return when finds first non-linebreak codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeIterator<Type> &AdvanceLine(bool First = false) {
		for (; !AtEnd(); Advance()) {
			if (First) return *this;
			First = isLineBreak();
		}
		return *this;
	}

	/*!< \brief advance's over Len line's if possible. */
	LWUnicodeIterator<Type> &AdvanceLine(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) AdvanceLine(First && i==0);
		return *this;
	}

	/*!< \brief Advance's position by one codepoint. */
	LWUnicodeIterator<Type> &Advance(void) {
		if (AtEnd()) return *this;
		m_Position += CodePointUnitSize(m_Position);
		++m_Index;
		return *this;
	}

	/*!< \brief Advance's position by Len codepoints. */
	LWUnicodeIterator<Type> &Advance(uint32_t Len) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) Advance();
		return *this;
	}

	/*!< \brief Advance's position to the end of stream by iterating to it(since Last may not be the true end to this iterator). */
	LWUnicodeIterator<Type> &AdvanceToEnd() {
		while (!AtEnd()) Advance();
		return *this;
	}

	/*!< \brief set's position to first. */
	LWUnicodeIterator<Type> &rAdvanceToStart() {
		*this = LWUnicodeIterator<Type>(0, m_First, m_First, m_Last);
		return *this;
	}

	/*!< \brief Advance's to the first occurrence of token, or to the end. 
		 \param SkipCurrentToken Advances stream(use if iterating over token, set to false on first iteration).
	*/
	LWUnicodeIterator<Type> &AdvanceToken(uint32_t Token, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) Advance();
		for (; !AtEnd(); Advance()) {
			if(isToken(Token)) return *this;
		}
		return *this;
	}

	/*!< \brief advance's to the next Len of token's occurrence, or to the end. */
	LWUnicodeIterator<Type> &AdvanceToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) AdvanceToken(Token, i!=0 || SkipCurrentToken);
		return *this;
	}

	/*!< \brief Advance's to the start of the first substring occurrence, or to the end. */
	LWUnicodeIterator<Type> &AdvanceSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if(SkipCurrentSubString) Advance();
		for (; !AtEnd(); Advance()) {
			if (isSubString(SubStringIter)) return *this;
		}
		return *this;
	}

	/*!< \brief Advance's to the end of the first substring occurrence, or to the end. */
	LWUnicodeIterator<Type> &AdvancerSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) Advance();
		//Slight optimization can be made here instead of using isrSubString().
		for (; !AtEnd(); Advance()) {
			LWUnicodeIterator<Type> C = *this;
			LWUnicodeIterator<Type> N = SubStringIter;
			bool isEqual = true;
			while (isEqual && !N.AtEnd()) isEqual = *C++ == *N++;
			if (isEqual) return *this = --C;
		}
		return *this;
	}

	/*!< \brief Advance's to the end of the next Len substring occurence, or to end. */
	LWUnicodeIterator<Type> &AdvancerSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) AdvancerSubString(SubStringIter, SkipCurrentSubString || i != 0);
		return *this;
	}

	/*!< \brief Advance's to the start of the first Len substring occurrence's, or to the end. */
	LWUnicodeIterator<Type> &AdvanceSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubstring = true) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) AdvanceSubString(SubStringIter, SkipCurrentSubstring || i!=0);
		return *this;
	}

	/*!< \brief Advance's till encountering any of the tokens in the TokenIter list. */
	LWUnicodeIterator<Type> &AdvanceTokens(const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) Advance();
		for (; !AtEnd(); Advance()) {
			if (isTokens(TokenListIter)) return *this;
		}
		return *this;
	}

	/*!< \brief Advance's till encountering len number of any of the tokens in the TokenIter list. */
	LWUnicodeIterator<Type> &AdvanceTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) AdvanceTokens(TokenListIter, SkipCurrentToken || i != 0);
		return *this;
	}

	/*!< \brief iterates to the prev first whitespace codepoint, then to the next non-whitespace codepoint, then advances to the very start of the word(finds the next whitespace codepoint, then stops).
	*	 \param First will skip iterating to the prev first whitespace codepoint, will return when finds first whitespace codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeIterator<Type> &rAdvanceWord(bool First = false) {
		bool atWord = First;
		for (; !AtStart(); rAdvance()) {
			if (isWhitespace()) {
				if (atWord) return Advance();
				First = true;
			} else atWord = First;
		}
		return *this;
	}

	/*!< \brief reverse advance's over Len word's if possible. */
	LWUnicodeIterator<Type> &rAdvanceWord(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !AtStart(); i++) rAdvanceWord(First && i==0);
		return *this;
	}

	/*!< \brief iterates to the prev first linebreak codepoint, then to the next non-linebreak codepoint, then advances to the very start of the line(finds the next linebreak codepoint, then stops).
	*	 \param First will skip iterating to the prev first linebreak codepoint, will return when finds first linebreak codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeIterator<Type> &rAdvanceLine(bool First = false) {
		bool atLine = First;
		for (; !AtStart(); rAdvance()) {
			if (isLineBreak()) {
				if (atLine) return Advance();
				First = true;
			} else atLine = First;
		}
		return *this;
	}

	/*!< \brief reverse advance's over Len line's if possible. */
	LWUnicodeIterator<Type> &rAdvanceLine(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !AtStart(); i++) rAdvanceLine(First && i==0);
		return *this;
	}

	/*!< \brief iterates to the previous token, or First. */
	LWUnicodeIterator<Type> &rAdvanceToken(uint32_t Token, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) rAdvance();
		for (; !AtStart(); rAdvance()) {
			if (isToken(Token)) return *this;
		}
		return *this;
	}

	/*!< \brief iterates to the previous len of token's, or first. */
	LWUnicodeIterator<Type> &rAdvanceToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !AtStart(); i++) rAdvanceToken(Token, i != 0 || SkipCurrentToken);
		return *this;
	}

	/*!< \brief iterates to the first codepoint of the previous substring, or first. */
	LWUnicodeIterator<Type> &rAdvanceSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) rAdvance();
		for (; !AtStart(); rAdvance()) {
			if (isSubString(SubStringIter)) return *this;
		}
		return *this;
	}

	/*!< \brief iterates to the last codepoint of the previous substring, or first. */
	LWUnicodeIterator<Type> &rAdvancerSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) rAdvance();
		LWUnicodeIterator<Type> rSubStringIter = --(SubStringIter.NextEnd());
		do {
			LWUnicodeIterator<Type> C = *this;
			LWUnicodeIterator<Type> N = rSubStringIter;
			bool isEqual = true;
			for (; isEqual && !C.AtStart() && !N.AtStart();) isEqual = *C-- == *N--;
			if (isEqual && N.AtStart() && *C==*N) return *this;
			if (AtStart()) break;
			rAdvance();
		} while (true);
		return *this;
	}

	/*!< \brief iterates to the last codepoint of the previous Len substrings, or first. */
	LWUnicodeIterator<Type> &rAdvancerSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		for (uint32_t i = 0; i < Len; i++) rAdvancerSubString(SubStringIter, i != 0 || SkipCurrentSubString);
		return *this;
	}

	/*!< \brief iterates to the previous Len substrings, or first. */
	LWUnicodeIterator<Type> &rAdvanceSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		for (uint32_t i = 0; i < Len && !AtStart(); i++) rAdvanceSubString(SubStringIter, i != 0 || SkipCurrentSubString);
		return *this;
	}

	/*!< \brief rAdvance's till encountering any of the tokens in the TokenIter list. */
	LWUnicodeIterator<Type> &rAdvanceTokens(const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) rAdvance();
		for (; !AtStart(); rAdvance()) {
			if (isTokens(TokenListIter)) return *this;
		}
		return *this;
	}

	/*!< \brief rAdvance's till encountering len number of any of the tokens in the TokenIter list. */
	LWUnicodeIterator<Type> &rAdvanceTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !AtEnd(); i++) rAdvanceTokens(TokenListIter, SkipCurrentToken || i != 0);
		return *this;
	}

	/*!< \brief reverse's position by one codepoint. */
	LWUnicodeIterator<Type> &rAdvance(void) {
		if (AtStart()) return *this;
		while (isTrailingCodeUnit(--m_Position)) {}
		--m_Index;
		return *this;
	}

	/*!< \brief reverse's position by len codepoint. */
	LWUnicodeIterator<Type> &rAdvance(uint32_t Len) {
		for (uint32_t i = 0; i < Len && !AtStart(); i++) rAdvance();
		return *this;
	}

	/*!< \brief gets a new iterator to the next codepoint(or current if already at last.) */
	LWUnicodeIterator<Type> Next(void) const {
		return LWUnicodeIterator<Type>(*this).Advance();
	}

	/*!< \brief returns an iterator at the end of the iterator. */
	LWUnicodeIterator<Type> NextEnd(void) const {
		return LWUnicodeIterator<Type>(*this).AdvanceToEnd();
	}

	/*!< \brief get's a new iterator to the next word codepoint(or current if already at last.) */
	LWUnicodeIterator<Type> NextWord(bool First = false) const {
		return LWUnicodeIterator<Type>(*this).AdvanceWord(First);
	}

	/*!< \brief get's a new iterator to the next Len word's(or current if already at last.) */
	LWUnicodeIterator<Type> NextWord(uint32_t Len, bool First = false) const {
		return LWUnicodeIterator<Type>(*this).AdvanceWord(Len, First);
	}

	/*!< \brief get's a new iterator to the next line codepoint(or current if already at last.) */
	LWUnicodeIterator<Type> NextLine(bool First = false) const {
		return LWUnicodeIterator<Type>(*this).AdvanceLine(First);
	}

	/*!< \brief get's a new iterator to the next Len line's(or current if already at last.) */
	LWUnicodeIterator<Type> NextLine(uint32_t Len, bool First = false) const {
		return LWUnicodeIterator<Type>(*this).AdvanceLine(Len, First);
	}

	/*!< \brief get's a new iterator to the next token(or current if already at Token/last.) */
	LWUnicodeIterator<Type> NextToken(uint32_t Token, bool SkipCurrentToken=true) const {
		return LWUnicodeIterator<Type>(*this).AdvanceToken(Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of token's(or current if already at Token/last.) */
	LWUnicodeIterator<Type> NextToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvanceToken(Len, Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next substring(or current if already at Substring/last.) */
	LWUnicodeIterator<Type> NextSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvanceSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of substring's(or current if already at Substring/last.) */
	LWUnicodeIterator<Type> NextSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvanceSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next rsubstring(or current if already substring/last.) */
	LWUnicodeIterator<Type> NextrSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvancerSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the len next rsubstring's. */
	LWUnicodeIterator<Type> NextrSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvancerSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next of any of the tokens in TokenIter(or current if already at substring/last.) */
	LWUnicodeIterator<Type> NextTokens(const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvanceTokens(TokenIterList, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of any of the tokens in TokenIter(or current if already at substring/last.) */
	LWUnicodeIterator<Type> NextTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).AdvanceTokens(Len, TokenIterList, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next Len codepoint's(or to last if Len exceeds amount of codepoint's left.) */
	LWUnicodeIterator<Type> Next(uint32_t Len) const {
		return LWUnicodeIterator<Type>(*this).Advance(Len);
	}

	/*!< \brief get's a new iterator to the prev codepoint(or current if already at first.) */
	LWUnicodeIterator<Type> Prev(void) const {
		return LWUnicodeIterator<Type>(*this).rAdvance();
	}

	/*!< \brief get's a new iterator to the prev Len codepoints(or to first if Len exceeds amount of codepoint's left.) */
	LWUnicodeIterator<Type> Prev(uint32_t Len) const {
		return LWUnicodeIterator<Type>(*this).rAdvance(Len);
	}

	/*!< \brief get's a new iterator to the first codepoint. */
	LWUnicodeIterator<Type> PrevStart(void) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceToStart();
	}

	/*!< \brief get's a new iterator to the prev word codepoint(or current if already at first.) */
	LWUnicodeIterator<Type> PrevWord(bool First = false) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceWord(First);
	}

	/*!< \brief get's a new iterator to the prev Len word's codepoint(or current if already at first.) */
	LWUnicodeIterator<Type> PrevWord(uint32_t Len, bool First = false) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceWord(Len, First);
	}

	/*!< \brief get's a new iterator to the prev line codepoint(or current if already at first.) */
	LWUnicodeIterator<Type> PrevLine(bool First = false) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceLine(First);
	}

	/*!< \brief get's a new iterator to the prev Len line's codepoint(or current if already at first.) */
	LWUnicodeIterator<Type> PrevLine(uint32_t Len, bool First = false) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceLine(First);
	}

	/*!< \brief get's a new iterator to the prev token(or current if already at Token/first.) */
	LWUnicodeIterator<Type> PrevToken(uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceToken(Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev len of token's(or current if already at Token/first.) */
	LWUnicodeIterator<Type> PrevToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceToken(Len, Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev substring(or current if already at Substring/first.) */
	LWUnicodeIterator<Type> PrevSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev len of substring's(or current if already at Substring/first.) */
	LWUnicodeIterator<Type> PrevSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev rsubstring(or current if already substring/first.) */
	LWUnicodeIterator<Type> PrevrSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvancerSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the len next rsubstring's. */
	LWUnicodeIterator<Type> PrevrSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvancerSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next of any of the tokens in TokenIter(or current if already at substring/first.) */
	LWUnicodeIterator<Type> PrevTokens(const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceTokens(TokenIterList, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of any of the tokens in TokenIter(or current if already at substring/first.) */
	LWUnicodeIterator<Type> PrevTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeIterator<Type>(*this).rAdvanceTokens(Len, TokenIterList, SkipCurrentToken);
	}

	/*!< \brief validates it is safe to iterate to the next codepoint, returns false if it can't advance any further, or has detected malformed utf-8, this function is only used when Create or Validating a raw buffer. */
	bool ValidateAdvance(void) const {
		if (m_Position == m_Last) return false;
		if (!isLeadCodeUnit(m_Position)) return false;
		if (!*m_Position) return false;
		uintptr_t Remaining = RawRemaining();
		uint32_t Size = CodePointUnitSize(m_Position);
		if (Size > Remaining) return false;
		const Type *L = m_Position + Size;
		if (!isLeadCodeUnit(L)) return false;
		for (const Type *P = m_Position + 1; P != L; P++) {
			if (!isTrailingCodeUnit(P)) return false;
		}
		return ValidateCodePoint(DecodeCodePoint(m_Position));
	}

	/*!< \brief returns true if position is at last, or position is a nul terminated character. */
	bool AtEnd(void) const {
		return m_Position == m_Last || *m_Position == '\0';
	}

	/*!< \brief returns true if position is at start. */
	bool AtStart(void) const {
		return m_Position == m_First;
	}

	/*!< \brief returns true if the iterator has been initialized or not(if position is null or not.) */
	bool isInitialized(void) const {
		return m_Position != nullptr;
	}

	/*!< \brief returns true if the codepoint's upto Count are the same(or end is reached by both this and Iter at the same time.) */
	bool Compare(const LWUnicodeIterator<Type> &Iter, uint32_t Count) const {
		LWUnicodeIterator<Type> A = *this;
		LWUnicodeIterator<Type> B = Iter;
		bool isEqual = true;
		while (isEqual && !A.AtEnd() && !B.AtEnd() && Count--) isEqual = *A++ == *B++;
		return isEqual && (Count == 0 || (A.AtEnd() && B.AtEnd()));
	}

	/*!< \brief returns true if Value match's upto ValueCount(or both reach end first) .*/
	bool Compare(const Type *Value, uint32_t ValueCount) const {
		return Compare(LWUnicodeIterator<Type>(Value, ValueCount), ValueCount);
	}
	
	/*!< \brief returns true if Iter match's this to the end of both streams in text. */
	bool Compare(const LWUnicodeIterator<Type> &Iter) const {
		return Compare(Iter, std::numeric_limits<uint32_t>::max());
	}

	/*!< \brief returns true if Value match's this to the end of both streams in text. */
	bool Compare(const Type *Value) const {
		return Compare(LWUnicodeIterator(Value));
	}

	/*!< \brief variadic compareList base function. */
	template<uint32_t N=0, class T>
	uint32_t CompareList(const T &Arg) const {
		return Compare(Arg) ? N : -1;
	}

	/*!< \brief returns the index of any matching string in the list of strings. */
	template<uint32_t N=0, class T, typename ...Args>
	uint32_t CompareList(const T &Arg, Args... Pack) const {
		if (Compare(Arg)) return N;
		return CompareList<N+1>(Pack...);
	}

	/*!< \brief base variadic template that returns the index of any matching string upto Count length. */
	template<uint32_t N=0, class T>
	uint32_t CompareListn(uint32_t Count, const T &Arg) const {
		return Compare(Arg, Count) ? N : -1;
	}

	/*!< \brief variadic template that returns the index of any matching string upto codepoint Count length. */
	template<uint32_t N=0, class T, typename ...Args>
	uint32_t CompareListn(uint32_t Count, const T &Arg, Args... Pack) const {
		if (Compare(Arg, Count)) return N;
		return CompareListn<N+1>(Count, Pack...);
	}

	/*!< \brief base variadic template for compare N items where each item also has a leading count value. */
	template<uint32_t N=0, class T>
	uint32_t CompareListnc(uint32_t Count, const T &Arg) const {
		return Compare(Arg, Count) ? N : -1;
	}

	/*!< \brief alternative varidaic template where each item being compared to also has a unique count length(example parameters: 5, "Hello", 10, "HelloWorld"). */
	template<uint32_t N = 0, class T, typename ...Args>
	uint32_t CompareListnc(uint32_t Count, const T &Arg, Args... Pack) const {
		if (Compare(Arg, Count)) return N;
		return CompareListnc<N + 1>(Pack...);
	}

	
	/*!< \brief compares to an array of list of comparable items, returning the index of that matching List item, or -1 if no match is found. */
	template<class T>
	uint32_t CompareLista(uint32_t Count, const T *List) const {
		for (uint32_t i = 0; i < Count; i++)
			if (Compare(List[i])) return i;
		return -1;
	}

	/*!< \brief compares to an array of list of comparable items, upto n codepoints.  returning the index of that matching list item, or -1 if no match is found. */
	template<class T>
	uint32_t CompareLista(uint32_t Count, const T *List, uint32_t CodePointCount) {
		for (uint32_t i = 0; i < Count; i++)
			if (Compare(List[i], CodePointCount)) return i;
		return -1;
	}
	
	/*!< \brief search's string for the first occurrent of SubString, returning true if found, or false if not found. */
	bool HasSubString(const LWUnicodeIterator<Type> &SubString) const {
		return !NextSubString(SubString, false).AtEnd();
	}

	/*!< \breif base method when search's string for any of the listed substring's, returning the index of the matched substring, or -1 if non were matched. */
	template<uint32_t N = 0, class T>
	uint32_t HasSubStringList(const T &Arg) const {
		return HasSubString(Arg) ? N : -1;
	}

	/*!< \breif search's string for any of the listed substring's, returning the index of the matched substring, or -1 if non were matched. */
	template<uint32_t N = 0, class T, typename ...Args>
	uint32_t HasSubStringList(const T &Arg, Args... Pack) const {
		if (HasSubString(Arg)) return N;
		return HasSubStringList<N + 1>(Pack...);
	}

	/*!< \brief search's string for any of the listed substring in the specified array, returning the index of the matched substring, or -1 if non were found. */
	uint32_t HasSubStringLista(uint32_t Count, const LWUnicodeIterator<Type> *List) const {
		for (uint32_t i = 0; i < Count; i++) {
			if (HasSubString(List[i])) return i;
		}
		return -1;
	}

	/*!< \brief returns true if the position iterator of this == position iterator of Iter. */
	bool operator == (const LWUnicodeIterator<Type> &Iter) const {
		return m_Position == Iter.m_Position;
	}

	/*!< \brief returns true if the two position iterator's of this are not the same. */
	bool operator != (const LWUnicodeIterator<Type> &Iter) const {
		return m_Position != Iter.m_Position;
	}

	/*!< \brief returns the current position of the iterator. */
	const Type *GetPosition(void) const {
		return m_Position;
	}

	/*!< \brief returns the last position of the iterator(may not be nul terminated, or even apart of the string's content). */
	const Type *GetLast(void) const {
		return m_Last;
	}

	/*!< \brief returns the first position of the iterator(may not be nul terminated, or even apart of the string's content). */
	const Type *GetFirst(void) const {
		return m_First;
	}

	/*!< \brief returns the raw index of position in the buffer. */
	uint32_t RawIndex(void) const {
		return (uint32_t)(uintptr_t)(m_Position - m_First);
	}

	/*!< \brief creates a C_View with Len of space to pass to c api's. */
	template<std::size_t Len>
	LWUTFC_View<Type,Len> c_str(void) const {
		return LWUTFC_View<Type,Len>(*this);
	}

	/*!< \brief returns a c_str view of this iterator, note that using this method the program must guarantee the iterator is null terminated, as well the position is not checked against Last so the contents will be read upto the end of the buffer that iterator is pointing to. */
	const char *c_str(void) const {
		return (const char*)m_Position;
	}

	/*!< \brief returns the raw length remaining from position to Last. does not indicate actual number of characters the iterator has to end. */
	uintptr_t RawRemaining(void) const {
		return (uintptr_t)(m_Last - m_Position);
	}

	/*!< \brief returns the number of raw characters left of the string, either encountering a '\0' or last, whichever comes first. */
	uintptr_t RawLength(void) const {
		return (uintptr_t)RawDistance(NextEnd());
	}

	/*!< \brief returns the number of codepoint's left of the string, this function should not be used if calculating the size of a buffer to contain the string, instead use RawLength. */
	uintptr_t Length(void) const {
		return (uintptr_t)Distance(NextEnd());
	}

	/*!< \brief returns the remaining length from position to the c_str length(i.e: looking for only the '\0' character. does not include '\0' itself.) */
	uintptr_t c_strLength(void) const {
		const char *P = m_Position;
		while (*P) ++P;
		return (uintptr_t)(P-m_Position);
	}

	/*!< \brief constructs an iterator where position is at First, and Last is First+Length. applications should use create when possible unless it can be guaranteed the string being used is not malformed. */
	LWUnicodeIterator(const Type *First, uint32_t Length) : m_Position(First), m_First(First), m_Last(First + Length) {}

	/*!< \brief constructs an iterator where position is at first, and last is unknown(thus set to highest pointer value), it is imperative that the application ensures the string is properly null terminated if using this constructor. */
	LWUnicodeIterator(const Type *First) : m_Position(First), m_First(First), m_Last(First ? (const Type*)(uintptr_t)-1 : First) {}

	/*!< \brief constructs an iterator to a c_view of a c string created by another iterator, this constructor is provided for implict conversions to an interator when a c_view is returned(such as what Format does). */
	template<size_t Len>
	LWUnicodeIterator(const LWUTFC_View<Type, Len> &CStr) : LWUnicodeIterator<Type>(CStr.m_Data) {}

	/*!< \brief constructs an iterator to stack allocated array. */
	template<size_t Len>
	LWUnicodeIterator(const Type (&First)[Len]) : m_Position(First), m_First(First), m_Last(First + Len) {}

	/*!< \brief constructs an iterator between the two iterators. */
	LWUnicodeIterator(const LWUnicodeIterator<Type> &Begin, const LWUnicodeIterator<Type> &End) : m_Index(Begin.m_Index), m_Position(Begin()), m_First(Begin()), m_Last(End()) {}

	/*!< \brief constructs an iterator between the two char arrays. (note the application must ensure the char string is valid. */
	LWUnicodeIterator(const Type *Begin, const Type *End, uint32_t Index = 0) : m_Index(Index), m_Position(Begin), m_First(Begin), m_Last(End) {}

	/*!< \brief fills out all the parameters for the iterator, application's need to gurantee the utf-8 is not malformed, Position is at the start of a code point, and Index is at the correct codepoint, otherwise UB will occur. */
	LWUnicodeIterator(int32_t Index, const Type *Position, const Type *First, const Type *Last) : m_Index(Index), m_Position(Position), m_First(First), m_Last(Last) {}
	
	/*!< \brief default construct. */
	LWUnicodeIterator() = default;

	int32_t m_Index = 0; /*!< \brief the current codepoint Index of the iterator(relative to first). */
protected:
	const Type *m_Position = nullptr; /*!< \brief Position of the iterator. */
	const Type *m_First = nullptr; /*!< \brief start of the buffer position is iterating over. */
	const Type *m_Last = nullptr; /*!< \brief End of the buffer position is iterating over. */
};

template<>
inline uint32_t LWUnicodeIterator<char8_t>::CodePointUnitSize(uint32_t CodePoint){
	if (CodePoint > 0xFFFF) return 4;
	if (CodePoint > 0x7FF) return 3;
	if (CodePoint > 0x7F) return 2;
	return 1;
}

template<>
inline uint32_t LWUnicodeIterator<char16_t>::CodePointUnitSize(uint32_t CodePoint) {
	if (CodePoint > 0xFFFF) return 2;
	return 1;
}

template<>
inline uint32_t LWUnicodeIterator<char32_t>::CodePointUnitSize(uint32_t CodePoint) {
	return 1;
}

template<>
inline uint32_t LWUnicodeIterator<char8_t>::CodePointUnitSize(const char8_t *Pos) {
	if ((*Pos & 0xF0) == 0xF0) return 4;
	if ((*Pos & 0xE0) == 0xE0) return 3;
	if ((*Pos & 0xC0) == 0xC0) return 2;  
	return 1;
}

template<>
inline uint32_t LWUnicodeIterator<char16_t>::CodePointUnitSize(const char16_t *Pos) {
	if ((*Pos & 0xD800) == 0xD800) return 2;
	return 1;
}

template<>
inline uint32_t LWUnicodeIterator<char32_t>::CodePointUnitSize(const char32_t *Pos) {
	return 1;
}

template<>
inline uint32_t LWUnicodeIterator<char8_t>::DecodeCodePoint(const char8_t *Pos) {
	if ((*Pos & 0xF0) == 0xF0) return (*Pos & 0x7) << 18 | (*(Pos + 1) & 0x3F) << 12 | (*(Pos + 2) & 0x3F) << 6 | (*(Pos + 3) & 0x3F);
	else if ((*Pos & 0xE0) == 0xE0) return (*Pos & 0xF) << 12 | (*(Pos + 1) & 0x3F) << 6 | (*(Pos + 2) & 0x3F);
	else if ((*Pos & 0xC0) == 0xC0) return (*Pos & 0x1F) << 6 | (*(Pos + 1) & 0x3F);
	return *Pos;
}

template<>
inline uint32_t LWUnicodeIterator<char16_t>::DecodeCodePoint(const char16_t *Pos) {
	if ((*Pos & 0xD800) == 0xD800) return ((*Pos & 0x3FF) << 10) | ((*(Pos + 1) & 0x3FF) + 0x10000);
	return *Pos;
}

template<>
inline uint32_t LWUnicodeIterator<char32_t>::DecodeCodePoint(const char32_t *Pos) {
	return *Pos;
}

template<>
inline uint32_t LWUnicodeIterator<char8_t>::EncodeCodePoint(char8_t *Buffer, uint32_t BufferSize, uint32_t CodePoint) {
	if (CodePoint <= 0x7F) {
		if (BufferSize>0) Buffer[0] = (char8_t)CodePoint;
		return 1;
	} else if (CodePoint <= 0x7FF) {
		if (BufferSize>1) {
			Buffer[0] = (char8_t)(((CodePoint >> 6) & 0x1F) | 0xC0);
			Buffer[1] = (char8_t)((CodePoint & 0x3F) | 0x80);
		}
		return 2;
	} else if (CodePoint <= 0xFFFF) {
		if (BufferSize>2) {
			Buffer[0] = (char8_t)(((CodePoint >> 12) & 0xF) | 0xE0);
			Buffer[1] = (char8_t)(((CodePoint >> 6) & 0x3F) | 0x80);
			Buffer[2] = (char8_t)((CodePoint & 0x3F) | 0x80);
		}
		return 3;
	} else if (CodePoint <= 0x1FFFFF) {
		if(BufferSize>3){
			Buffer[0] = (char8_t)(((CodePoint >> 18) & 0x7) | 0xF0);
			Buffer[1] = (char8_t)(((CodePoint >> 12) & 0x3F) | 0x80);
			Buffer[2] = (char8_t)(((CodePoint >> 6) & 0x3F) | 0x80);
			Buffer[3] = (char8_t)((CodePoint & 0x3F) | 0x80);
		}
		return 4;
	}
	return 0;
}

template<>
inline uint32_t LWUnicodeIterator<char16_t>::EncodeCodePoint(char16_t *Buffer, uint32_t BufferSize, uint32_t CodePoint) {
	if (CodePoint <= 0xD7FF) { //We ignore the range of 
		if (BufferSize > 0) Buffer[0] = (char16_t)CodePoint;
		return 1;
	} else if (CodePoint >= 0xE000 && CodePoint <= 0xFFFF) {
		if (BufferSize > 0) Buffer[0] = (char16_t)CodePoint;
		return 1;
	} else if (CodePoint >= 0x10000 && CodePoint <= 0x10FFFF) {
		if (BufferSize > 1) {
			uint32_t C = CodePoint - 0x10000;
			Buffer[0] = (char16_t)(((C >> 10) & 0x3FF) | 0xD800);
			Buffer[1] = (char16_t)((C & 0x3FF) | 0xDC00);
		}
		return 2;
	}
	return 0;
}

template<>
inline uint32_t LWUnicodeIterator<char32_t>::EncodeCodePoint(char32_t *Buffer, uint32_t BufferSize, uint32_t CodePoint) {
	if (BufferSize > 0) *Buffer = CodePoint;
	return 1;
}

template<>
inline bool LWUnicodeIterator<char8_t>::isLeadCodeUnit(const char8_t *Pos) {
	return (*Pos & 0xC0) != 0x80;
}

template<>
inline bool LWUnicodeIterator<char16_t>::isLeadCodeUnit(const char16_t *Pos) {
	return (*Pos & 0xFC00) != 0xDC00;
}

template<>
inline bool LWUnicodeIterator<char32_t>::isLeadCodeUnit(const char32_t *Pos) {
	return true;
}

template<>
inline bool LWUnicodeIterator<char8_t>::isTrailingCodeUnit(const char8_t *Pos) {
	return (*Pos & 0xC0) == 0x80;
}

template<>
inline bool LWUnicodeIterator<char16_t>::isTrailingCodeUnit(const char16_t *Pos) {
	return (*Pos & 0xFC00) == 0xDC00;
}

template<>
inline bool LWUnicodeIterator<char32_t>::isTrailingCodeUnit(const char32_t *Pos) {
	return false;
}

std::ostream &operator << (std::ostream &o, const LWUTF8Iterator &Iter);
std::ostream &operator << (std::ostream &o, const LWUTF16Iterator &Iter);
std::ostream &operator << (std::ostream &o, const LWUTF32Iterator &Iter);

template<std::size_t Len>
using LWUTF8C_View = LWUTFC_View<char8_t, Len>;

template<std::size_t Len>
using LWUTF16C_View = LWUTFC_View<char16_t, Len>;

template<std::size_t Len>
using LWUTF32C_View = LWUTFC_View<char32_t, Len>;

#endif