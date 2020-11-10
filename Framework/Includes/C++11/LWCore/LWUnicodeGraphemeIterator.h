#ifndef LWUNICODEGRAPHEMEITERATOR_H
#define LWUNICODEGRAPHEMEITERATOR_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWUnicodeIterator.h"
#include "LWCore/LWCrypto.h"

#define LWGRAPHEME_ANY 0 
#define LWGRAPHEME_CR 1
#define LWGRAPHEME_LF 2
#define LWGRAPHEME_CONTROL 3
#define LWGRAPHEME_EXTEND 4
#define LWGRAPHEME_ZWJ 5
#define LWGRAPHEME_RI 6
#define LWGRAPHEME_PREPEND 7
#define LWGRAPHEME_SPACING 8
#define LWGRAPHEME_L 9
#define LWGRAPHEME_V 10
#define LWGRAPHEME_T 11
#define LWGRAPHEME_LV 12
#define LWGRAPHEME_LVT 13
#define LWGRAPHEME_TYPECOUNT 14

/*!< \brief Block Table, tells how many code point ranges are in the block, and the offset to that range. */
struct LWGraphemeBlockTable {
	uint32_t m_BlockCount = 0;
	uint32_t m_BlockOffset = 0;
};

/*!< \brief Grapheme Block, returns the type if the codepoint is >=Min && <= Max. */
struct LWGraphemeBlock {
	uint32_t m_Min = 0;
	uint32_t m_Max = 0;
	uint32_t m_Type = 0;

	/*!< \brief searching GraphemeBlock use's std::lower_bound to find most likely sub block. */
	friend bool operator < (const LWGraphemeBlock &B, uint32_t CodePoint);

	bool operator < (const LWGraphemeBlock &B);

	/*!< \brief returns LWGRAPHEME_ANY if not inside range, otherwise returns m_Type. */
	uint32_t InRange(uint32_t CodePoint) const;

	/*!< \brief construct's the grapheme block. */
	LWGraphemeBlock(uint32_t MinCodePoint, uint32_t MaxCodePoint, uint32_t Type);

	/*!< \brief construct's the grapheme block where min==max. */
	LWGraphemeBlock(uint32_t CodePoint, uint32_t Type);
};

struct LWGraphemeTable {
	static const uint32_t BlockSize = 256; /*!< \brief divide codepoint by blocksize to determine the block range's to search.  this significantly narrows the number of sub blocks a codepoint will have to search. */
	static const uint32_t TableBlocks = (LWUTF8Iterator::MaxCodePoints + BlockSize - 1) / BlockSize;
	//The Table's used by this class are defined in auto generated LWGraphemeTable.h(linked to from LWGrapheme.cpp)

	/*!< \brief evaluates if LeftGraphemeType x RightGraphemeType should break, or be joined.  true to break, false if joined. */
	static bool EvaluateGraphemeRules(uint32_t LeftGraphemeType, uint32_t RightGraphemeType);

	/*!< \brief Function takes in https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakProperty.txt file, and spits out a header file that can replace LWGraphemeTable.h
		 \return the number of bytes for the resulting file.
		 \note The sample project LWUnicode will generate this file, and pulls the above file from the internet.
	*/
	static uint32_t GenerateTable(char *Buffer, uint32_t BufferSize, LWUTF8Iterator &FileIter);

	/*!< \brief searches over tables to find the type for the codepoint, returns LWGRAPHEME_ANY if no codepoint is found.. */
	static uint32_t GetCodepointType(uint32_t CodePoint);
};

/*!< \brief GraphemeIterator iterates over a utfX(where X is 8, 16, or 32 encodings) and presents how many codepoints a grapheme cluster represents. iteration is done from grapheme cluster to grapheme cluster. */
template<class Type>
class LWUnicodeGraphemeIterator : public LWUnicodeIterator<Type> {
public:

	/*!< \brief iterate's over each codepoint and validate's no malformed utf is detected. */
	static bool ValidateIterator(const LWUnicodeIterator<Type> &Iter, uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		LWUnicodeIterator<Type> C = Iter;
		Characters = 0;
		uint32_t pType = LWGRAPHEME_CONTROL;
		while (C.ValidateAdvance()) {
			uint32_t cType = LWGraphemeTable::GetCodepointType(*C);
			if (LWGraphemeTable::EvaluateGraphemeRules(pType, cType)) Characters++;
			C.Advance();
			pType = cType;
		}
		Length = Iter.Distance(C);
		RawLength = Iter.RawDistance(C) + 1;
		return C.AtEnd();
	}

	/*!< \brief iterates over buffer and validates the utf-8 string is not malformed in anyway, if it detects malformed false is return.  also counts the number of Grapheme clusters(characters), codepoints(Length), and the raw buffer length(+1) that is needed to store the string Buffer points to.
		 \note BufferSize should be the space allocated for buffer, it does not need to be the strlen(Buffer), ValidateString will automatically detect a null terminated string.  if a non null terminated string is passed then the iterator will stop when it reach's BufferLength instead.
	*/
	static bool ValidateString(const Type *Buffer, uint32_t BufferSize, uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		if (!BufferSize) return true;
		return ValidateIterator({ Buffer, BufferSize }, Characters, Length, RawLength);
	}

	/*!< \brief iterates over a nul teminated buffer and validates the utf-8 string is not malformed in anyway, if it detects malformed false is returned.  also counts the number of Grapheme clusters(characters), codepoints(length), and raw buffer length(+1) that is needed to store the string buffer points to. */
	template<size_t Len>
	static bool ValidateString(const Type(&Buffer)[Len], uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		return ValidateIterator({ Buffer }, Characters, Length, RawLength);
	}

	/*!< \brief iterates over a nul teminated buffer and validates the utf-8 string is not malformed in anyway, if it detects malformed false is returned.  also counts the number of Grapheme clusters(characters), codepoints(length), and raw buffer length(+1) that is needed to store the string buffer points to. */
	static bool ValidateString(const Type *Buffer, uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		return ValidateIterator({ Buffer }, Characters, Length, RawLength);
	}

	/*!< \brief safe way to create an grapheme iterator from a c_str(must be casted from char to char8_t, or converted to relevant type).  */
	static bool Create(LWUnicodeGraphemeIterator<Type> &Res, const Type *Buffer, uint32_t BufferSize, uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		if (!ValidateString(Buffer, BufferSize, Characters, Length, RawLength)) return false;
		Res = LWUnicodeGraphemeIterator<Type>(Buffer, RawLength);
		return true;
	}

	/*!< \brief safe way to create an iterator from a c_str(must be casted from char to char8_t, or converted to relevant type).  */
	template<size_t Len>
	static bool Create(LWUnicodeGraphemeIterator<Type> &Res, const Type(&Buffer)[Len], uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		if (!ValidateString(Buffer, Len, Characters, Length, RawLength)) return false;
		Res = LWUnicodeGraphemeIterator<Type>(Buffer, RawLength);
		return true;
	}

	/*!< \brief safe way to create an iterator from a c_str(must be casted from char to char8_t, or converted to relevant type).  */
	static bool Create(LWUnicodeGraphemeIterator<Type> &Res, const Type *Buffer, uint32_t &Characters, uint32_t &Length, uint32_t &RawLength) {
		if (!ValidateString(Buffer, Characters, Length, RawLength)) return false;
		Res = LWUnicodeGraphemeIterator<Type>(Buffer, RawLength);
		return true;
	}

	/*!< \brief counts number of codepoint's at pos to next Grapheme. */
	static uint32_t CountGraphemeCluster(const LWUnicodeIterator<Type> &Pos) {
		LWUnicodeIterator<Type> C = Pos;
		uint32_t pType = LWGraphemeTable::GetCodepointType(*C);
		for (C.Advance(); !C.AtEnd(); ++C) {
			uint32_t cType = LWGraphemeTable::GetCodepointType(*C);
			if (LWGraphemeTable::EvaluateGraphemeRules(pType, cType)) break;
			pType = cType;
		}
		return C.m_Index - Pos.m_Index;
	}


	/*!< \brief calculates the Character distance between two iterators, if O is to the left of this, then the result will be negative. */
	int32_t CharacterDistance(const LWUnicodeGraphemeIterator<Type> &O) const {
		return O.m_Character - m_Character;
	}

	/*!< \brief we need to account for us being a grapheme character, and advance iter to the end codepoint of this iterator. */
	bool isrSubString(const LWUnicodeIterator<Type> &SubString) const {
		LWUnicodeIterator<Type> C = LWUnicodeIterator<Type>(*this).Advance(m_CodePointCount-1);
		LWUnicodeIterator<Type> N = --(SubString.NextEnd());
		bool isEqual = true;
		for (; isEqual && !N.AtStart() && !C.AtStart();) isEqual = *C-- == *N--;
		return isEqual && *C == *N && N.AtStart();
	}

	/*!< \brief advance's the iterator by one Grapheme. */
	LWUnicodeGraphemeIterator<Type> &operator++() {
		return Advance();
	}

	/*!< \brief advance's the iterator by one Grapheme, returns the previous position. */
	LWUnicodeGraphemeIterator<Type> operator++(int) {
		LWUnicodeGraphemeIterator<Type> C = *this;
		Advance();
		return C;
	}

	/*!< \brief advance's the iterator by Dis Grapheme.  Iterator will not advance past Last. */
	LWUnicodeGraphemeIterator<Type> &operator+=(uint32_t Dis) {
		return Advance(Dis);
	}

	/*!< \brief returns a new iterator Dis Grapheme's ahead, Iterator will not advance past Last. */
	LWUnicodeGraphemeIterator<Type> operator+(uint32_t Dis) const {
		return Next(Dis);
	}

	/*!< \brief reverse's the iterator by one Grapheme, will not go behind First. */
	LWUnicodeGraphemeIterator<Type> &operator--() {
		return rAdvance();
	}

	/*!< \brief reverse's the iterator by one Grapheme, returns the previous position. */
	LWUnicodeGraphemeIterator<Type> operator--(int) {
		LWUnicodeGraphemeIterator<Type> N = *this;
		rAdvance();
		return N;
	}

	/*!< \brief reverse's the iterator by Dis Grapheme's. */
	LWUnicodeGraphemeIterator<Type> &operator-=(uint32_t Dis) {
		return rAdvance(Dis);
	}

	/*!< \brief returns a new iterator Dis Prev grapheme's, will not go before First. */
	LWUnicodeGraphemeIterator<Type> operator-(uint32_t Dis) const {
		return Prev(Dis);
	}

	/*!< \brief generates iterator's between each token, and end at next token(or end of Iterator).
		 \param IterBuffer buffer of iterator's for each split.
		 \param IterBufferSize the number of buffer's available to be used.
		 \param Token the token to split on.
		 \return the number of iterator's needed to split the string completely.
		 \note If the token is part of a grapheme cluster(and not the first codepoint of the cluster), then it will be skipped.
	*/
	uint32_t SplitToken(LWUnicodeGraphemeIterator<Type> *IterBuffer, uint32_t IterBufferSize, uint32_t Token) const {
		LWUnicodeGraphemeIterator<Type> P = *this;
		LWUnicodeGraphemeIterator<Type> C = NextToken(Token, false);
		uint32_t o = 0;
		for (; !C.AtEnd(); P = C.Next(), C.AdvanceToken(Token), o++) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeGraphemeIterator<Type>(P, C);
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeGraphemeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief generates iterator's between each token in the token list, and end at next token(or end of Iterator).
	*	 \param IterBuffer buffer of iterator's for each split.
	*    \param IterBufferSize the number of buffer's avaiable to be used.
	*    \param TokenList the iterator to the list of token's.
	*    \return the number of iterator's needed to split the stirng completely.
	*	 \note if any of the token's is inside a grapheme cluster(and not at the start), it will get skipped by the GraphemeIterator.
	*/
	uint32_t SplitTokenList(LWUnicodeGraphemeIterator<Type> *IterBuffer, uint32_t IterBufferSize, const LWUnicodeIterator<Type> &TokenList) const {
		LWUnicodeGraphemeIterator<Type> P = *this;
		LWUnicodeGraphemeIterator<Type> C = NextTokens(TokenList, false);
		uint32_t o = 0;
		for (; !C.AtEnd(); P = C.Next(), C.AdvanceTokens(TokenList), o++) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeGraphemeIterator<Type>(P, C);
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeGraphemeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief generates iterator's between each word, and end at end of word(or end of iterator).
	*	 \param IterBuffer buffer of iterator's for each split.
	*    \param IterBufferSize the number of IterBuffer's available to be used.
	*    \return the number of iterator's needed to split the string to each word.
	*/
	uint32_t SplitWords(LWUnicodeGraphemeIterator<Type> *IterBuffer, uint32_t IterBufferSize) const {
		LWUnicodeGraphemeIterator<Type> P = (*this).NextWord(true);
		LWUnicodeGraphemeIterator<Type> C = P;
		uint32_t o = 0;
		for (; !C.AtEnd();) {
			if (C.isWhitespace()) {
				if (o++ < IterBufferSize) IterBuffer[o - 1] = LWUnicodeGraphemeIterator<Type>(P, C);
				P = C.AdvanceWord(true);
			} else ++C;
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeGraphemeIterator<Type>(P, C);
			o++;
		}
		return o;
	}

	/*!< \brief generates iterator's between each line, and end at end of line(or end of iterator).
	*	 \param IterBuffer buffer of iterator's for each split.
	*    \param IterBufferSize the number of IterBuffer's available to be used.
	*	 \return the number of iterator's needed to split the string to each line.
	*/
	uint32_t SplitLine(LWUnicodeGraphemeIterator<Type> *IterBuffer, uint32_t IterBufferSize) const {
		LWUnicodeGraphemeIterator<Type> P = (*this).NextLine(true);
		LWUnicodeGraphemeIterator<Type> C = P;
		uint32_t o = 0;
		for (; !C.AtEnd();) {
			if(C.isLineEnd()) {
				if (o++ < IterBufferSize) IterBuffer[o - 1] = LWUnicodeGraphemeIterator<Type>(P, C);
				P = C.AdvanceLine();
			} else ++C;
		}
		if (!P.AtEnd()) {
			if (o < IterBufferSize) IterBuffer[o] = LWUnicodeGraphemeIterator<Type>(P, C);
			o++;
		}
		return o;
	}


	/*!< \brief iterates to the next first whitespace codepoint, then to the first non-whitespace codepoint.
	*	 \param First will skip iterating to the next first whitespace codepoint, will return when finds first non-whitespace codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeGraphemeIterator<Type> &AdvanceWord(bool First = false) {
		for (; !this->AtEnd(); Advance()) {
			if (this->isWhitespace()) First = true;
			else if (First) return *this;
		}
		return *this;
	}

	/*!< \brief advances over Len word if possible. */
	LWUnicodeGraphemeIterator<Type> &AdvanceWord(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) AdvanceWord(First && i == 0);
		return *this;
	}

	/*!< \brief iterates to the next first linebreak codepoint, then to the first non-linebreak codepoint.
	*	 \param First will skip iterating to the next first linebreak codepoint, will return when finds first non-linebreak codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeGraphemeIterator<Type> &AdvanceLine(bool First = false) {
		for (; !this->AtEnd(); Advance()) {
			if (First) return *this;
			First = this->isLineBreak();
		}
		return *this;
	}

	/*!< \brief advance's over Len line's if possible. */
	LWUnicodeGraphemeIterator<Type> &AdvanceLine(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) AdvanceLine(First && i == 0);
		return *this;
	}


	/*!< \brief advance's forward 1 grapheme cluster. */
	LWUnicodeGraphemeIterator &Advance(void) {
		if (this->AtEnd()) return *this;
		LWUnicodeIterator<Type>::Advance(m_CodePointCount);
		m_Character++;
		m_CodePointCount = CountGraphemeCluster(*this);
		return *this;
	}

	/*!< \brief Advance's position by Len Grapheme's. */
	LWUnicodeGraphemeIterator<Type> &Advance(uint32_t Len) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) Advance();
		return *this;
	}

	/*!< \brief Advance's position to the end of stream by iterating to it(since Last is not guranteed to be the true end to the iterator). */
	LWUnicodeGraphemeIterator<Type> &AdvanceToEnd() {
		while (!this->AtEnd()) Advance();
		return *this;
	}

	/*!< \brief set's position to first. 
		 \note This overrides index+character to 0 as those values are not retained internally.
	*/
	LWUnicodeGraphemeIterator<Type> &rAdvanceToStart() {
		*this = LWUnicodeGraphemeIterator<Type>(0, 0, this->m_First, this->m_First, this->m_Last);
		return *this;
	}

	/*!< \brief Advance's to the first occurrence of token, or to the end.
		 \param SkipCurrentToken Advances stream(use if iterating over token, set to false on first iteration).
		 \note if Token is inside a grapheme cluster(and not the start of the cluster), then this function will skip over it.
	*/
	LWUnicodeGraphemeIterator<Type> &AdvanceToken(uint32_t Token, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) Advance();
		for (; !this->AtEnd(); Advance()) {
			if (this->isToken(Token)) return *this;
		}
		return *this;
	}

	/*!< \brief advance's to the next Len of token's occurrence, or to the end. */
	LWUnicodeGraphemeIterator<Type> &AdvanceToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) AdvanceToken(Token, i != 0 || SkipCurrentToken);
		return *this;
	}

	/*!< \brief Advance's to the start of the first substring occurrence, or to the end. */
	LWUnicodeGraphemeIterator<Type> &AdvanceSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) Advance();
		for (; !this->AtEnd(); Advance()) {
			if (this->isSubString(SubStringIter)) return *this;
		}
		return *this;
	}

	/*!< \brief Advance's to the end of the first substring occurrence, or to the end. */
	LWUnicodeGraphemeIterator<Type> &AdvancerSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) Advance();
		//Slight optimization can be made here instead of using isrSubString().
		for (; !this->AtEnd(); Advance()) {
			LWUnicodeGraphemeIterator<Type> C = *this;
			LWUnicodeIterator<Type> N = SubStringIter;
			bool isEqual = true;
			while (isEqual && !N.AtEnd()) {
				LWUnicodeIterator<Type> SubC = C++.ClusterIterator();
				for (; isEqual && !SubC.AtEnd() && !N.AtEnd();) isEqual = *SubC++ == *N++;
			}
			if (isEqual) return *this = --C;
		}
		return *this;
	}

	/*!< \brief Advance's to the end of the next Len substring occurrence, or to end. */
	LWUnicodeGraphemeIterator<Type> &AdvancerSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) AdvancerSubString(SubStringIter, SkipCurrentSubString || i != 0);
		return *this;
	}

	/*!< \brief Advance's to the start of the first Len substring occurrence's, or to the end. */
	LWUnicodeGraphemeIterator<Type> &AdvanceSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubstring = true) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) AdvanceSubString(SubStringIter, SkipCurrentSubstring || i != 0);
		return *this;
	}

	/*!< \brief Advance's till encountering any of the tokens in the TokenIter list.
		 \note if any of the token list are tokens that are in the middle of a grapheme cluster, then it is likely to be skipped.
	*/
	LWUnicodeGraphemeIterator<Type> &AdvanceTokens(const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) Advance();
		for (; !this->AtEnd(); Advance()) {
			if (this->isTokens(TokenListIter)) return *this;
		}
		return *this;
	}

	/*!< \brief Advance's till encountering len number of any of the tokens in the TokenIter list. */
	LWUnicodeGraphemeIterator<Type> &AdvanceTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) AdvanceTokens(TokenListIter, SkipCurrentToken || i != 0);
		return *this;
	}

	/*!< \brief iterates to the prev first whitespace codepoint, then to the next non-whitespace codepoint, then advances to the very start of the word(finds the next whitespace codepoint, then stops).
	*	 \param First will skip iterating to the prev first whitespace codepoint, will return when finds first whitespace codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeGraphemeIterator<Type> &rAdvanceWord(bool First = false) {
		bool atWord = First;
		for (; !this->AtStart(); rAdvance()) {
			if (this->isWhitespace()) {
				if (atWord) return Advance();
				First = true;
			} else atWord = First;
		}
		return *this;
	}

	/*!< \brief reverse advance's over Len word's if possible. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceWord(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !this->AtStart(); i++) rAdvanceWord(First && i == 0);
		return *this;
	}

	/*!< \brief iterates to the prev first linebreak codepoint, then to the next non-linebreak codepoint, then advances to the very start of the line(finds the next linebreak codepoint, then stops).
	*	 \param First will skip iterating to the prev first linebreak codepoint, will return when finds first linebreak codepoint, even if it's on that codepoint right now.
	*/
	LWUnicodeGraphemeIterator<Type> &rAdvanceLine(bool First = false) {
		bool atLine = First;
		for (; !this->AtStart(); rAdvance()) {
			if (this->isLineBreak()) {
				if (atLine) return Advance();
				First = true;
			} else atLine = First;
		}
		return *this;
	}

	/*!< \brief reverse advance's over Len line's if possible. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceLine(uint32_t Len, bool First = false) {
		for (uint32_t i = 0; i < Len && !this->AtStart(); i++) rAdvanceLine(First && i == 0);
		return *this;
	}

	/*!< \brief iterates to the previous token, or First.
		 \note if Token is not the first codepoint in a grapheme cluster, it will be skipped.
	*/
	LWUnicodeGraphemeIterator<Type> &rAdvanceToken(uint32_t Token, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) rAdvance();
		for (; !this->AtStart(); rAdvance()) {
			if (this->isToken(Token)) return *this;
		}
		return *this;
	}

	/*!< \brief iterates to the previous len of token's, or first. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !this->AtStart(); i++) rAdvanceToken(Token, i != 0 || SkipCurrentToken);
		return *this;
	}

	/*!< \brief iterates to the first Grapheme of the previous substring, or first. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) rAdvance();
		for (; !this->AtStart(); rAdvance()) {
			if (this->isSubString(SubStringIter)) return *this;
		}
		return *this;
	}

	/*!< \brief iterates to the last Grapheme of the previous substring, or first. */
	LWUnicodeGraphemeIterator<Type> &rAdvancerSubString(const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		if (SkipCurrentSubString) rAdvance();
		LWUnicodeIterator<Type> rSubStringIter = --(SubStringIter.NextEnd());
		do {
			LWUnicodeGraphemeIterator<Type> C = *this;
			LWUnicodeIterator<Type> N = rSubStringIter;
			bool isEqual = true;
			do {
				LWUnicodeIterator<Type> SubC = --(C.ClusterIterator().NextEnd());
				for (; isEqual && !SubC.AtStart() && !N.AtStart();) isEqual = *SubC-- == *N--;
				if (!N.AtStart()) isEqual = isEqual && *SubC == *N--;
				if (C.AtStart()) break;
				--C;
			} while (isEqual && !N.AtStart());
			if (isEqual && N.AtStart() && *C == *N) return *this;
			if (this->AtStart()) break;
			rAdvance();
		} while (true);
		return *this;
	}

	/*!< \brief iterates to the last Grapheme of the previous Len substrings, or first. */
	LWUnicodeGraphemeIterator<Type> &rAdvancerSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		for (uint32_t i = 0; i < Len; i++) rAdvancerSubString(SubStringIter, i != 0 || SkipCurrentSubString);
		return *this;
	}

	/*!< \brief iterates to the previous Len substrings, or first. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubStringIter, bool SkipCurrentSubString = true) {
		for (uint32_t i = 0; i < Len && !this->AtStart(); i++) rAdvanceSubString(SubStringIter, i != 0 || SkipCurrentSubString);
		return *this;
	}

	/*!< \brief rAdvance's till encountering any of the tokens in the TokenIter list. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceTokens(const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		if (SkipCurrentToken) rAdvance();
		for (; !this->AtStart(); rAdvance()) {
			if (this->isTokens(TokenListIter)) return *this;
		}
		return *this;
	}

	/*!< \brief rAdvance's till encountering len number of any of the tokens in the TokenIter list. */
	LWUnicodeGraphemeIterator<Type> &rAdvanceTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenListIter, bool SkipCurrentToken = true) {
		for (uint32_t i = 0; i < Len && !this->AtEnd(); i++) rAdvanceTokens(TokenListIter, SkipCurrentToken || i != 0);
		return *this;
	}

	/*!< \brief advance's back 1 grapheme cluster(or stays at start). */
	LWUnicodeGraphemeIterator &rAdvance(void) {
		if (this->AtStart()) return *this;
		m_Character--;
		m_CodePointCount = 1;
		LWUnicodeIterator<Type>::rAdvance();
		uint32_t cType = LWGraphemeTable::GetCodepointType(this->CodePoint());
		for (; !this->AtStart();) {
			LWUnicodeIterator<Type>::rAdvance();
			uint32_t pType = LWGraphemeTable::GetCodepointType(this->CodePoint());
			if (LWGraphemeTable::EvaluateGraphemeRules(pType, cType)) {
				LWUnicodeIterator<Type>::Advance();
				break;
			}
			cType = pType;
			m_CodePointCount++;
		}
		return *this;
	}

	/*!< \brief reverse's position by len Grapheme's. */
	LWUnicodeGraphemeIterator<Type> &rAdvance(uint32_t Len) {
		for (uint32_t i = 0; i < Len && !this->AtStart(); i++) rAdvance();
		return *this;
	}

	/*!< \brief gets a new iterator to the next Grapheme(or current if already at last.) */
	LWUnicodeGraphemeIterator<Type> Next(void) const {
		return LWUnicodeGraphemeIterator<Type>(*this).Advance();
	}

	/*!< \brief returns an iterator at the end of the iterator. */
	LWUnicodeGraphemeIterator<Type> NextEnd(void) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceToEnd();
	}

	/*!< \brief get's a new iterator to the next word Grapheme(or current if already at last.) */
	LWUnicodeGraphemeIterator<Type> NextWord(bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceWord(First);
	}

	/*!< \brief get's a new iterator to the next Len word's(or current if already at last.) */
	LWUnicodeGraphemeIterator<Type> NextWord(uint32_t Len, bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceWord(Len, First);
	}

	/*!< \brief get's a new iterator to the next line Grapheme(or current if already at last.) */
	LWUnicodeGraphemeIterator<Type> NextLine(bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceLine(First);
	}

	/*!< \brief get's a new iterator to the next Len line's(or current if already at last.) */
	LWUnicodeGraphemeIterator<Type> NextLine(uint32_t Len, bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceLine(Len, First);
	}

	/*!< \brief get's a new iterator to the next token(or current if already at Token/last.) 
		 \note if Token is a codepoint that would be inside a grapheme cluster and not at the start of the cluster, then it may get skipped.
	*/
	LWUnicodeGraphemeIterator<Type> NextToken(uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceToken(Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of token's(or current if already at Token/last.) */
	LWUnicodeGraphemeIterator<Type> NextToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceToken(Len, Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next substring(or current if already at Substring/last.) */
	LWUnicodeGraphemeIterator<Type> NextSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of substring's(or current if already at Substring/last.) */
	LWUnicodeGraphemeIterator<Type> NextSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next rsubstring(or current if already substring/last.) */
	LWUnicodeGraphemeIterator<Type> NextrSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvancerSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the len next rsubstring's. */
	LWUnicodeGraphemeIterator<Type> NextrSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvancerSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next of any of the tokens in TokenIter(or current if already at substring/last.) */
	LWUnicodeGraphemeIterator<Type> NextTokens(const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceTokens(TokenIterList, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of any of the tokens in TokenIter(or current if already at substring/last.) */
	LWUnicodeGraphemeIterator<Type> NextTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).AdvanceTokens(Len, TokenIterList, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next Len codepoint's(or to last if Len exceeds amount of codepoint's left.) */
	LWUnicodeGraphemeIterator<Type> Next(uint32_t Len) const {
		return LWUnicodeGraphemeIterator<Type>(*this).Advance(Len);
	}

	/*!< \brief get's a new iterator to the prev codepoint(or current if already at first.) */
	LWUnicodeGraphemeIterator<Type> Prev(void) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvance();
	}

	/*!< \brief get's a new iterator to the prev Len codepoints(or to first if Len exceeds amount of codepoint's left.) */
	LWUnicodeGraphemeIterator<Type> Prev(uint32_t Len) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvance(Len);
	}

	/*!< \brief get's a new iterator to the first codepoint. */
	LWUnicodeGraphemeIterator<Type> PrevStart(void) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceToStart();
	}

	/*!< \brief get's a new iterator to the prev word codepoint(or current if already at first.) */
	LWUnicodeGraphemeIterator<Type> PrevWord(bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceWord(First);
	}

	/*!< \brief get's a new iterator to the prev Len word's codepoint(or current if already at first.) */
	LWUnicodeGraphemeIterator<Type> PrevWord(uint32_t Len, bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceWord(Len, First);
	}

	/*!< \brief get's a new iterator to the prev line codepoint(or current if already at first.) */
	LWUnicodeGraphemeIterator<Type> PrevLine(bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceLine(First);
	}

	/*!< \brief get's a new iterator to the prev Len line's codepoint(or current if already at first.) */
	LWUnicodeGraphemeIterator<Type> PrevLine(uint32_t Len, bool First = false) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceLine(First);
	}

	/*!< \brief get's a new iterator to the prev token(or current if already at Token/first.) */
	LWUnicodeGraphemeIterator<Type> PrevToken(uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceToken(Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev len of token's(or current if already at Token/first.) */
	LWUnicodeGraphemeIterator<Type> PrevToken(uint32_t Len, uint32_t Token, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceToken(Len, Token, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev substring(or current if already at Substring/first.) */
	LWUnicodeGraphemeIterator<Type> PrevSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev len of substring's(or current if already at Substring/first.) */
	LWUnicodeGraphemeIterator<Type> PrevSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the prev rsubstring(or current if already substring/first.) */
	LWUnicodeGraphemeIterator<Type> PrevrSubString(const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvancerSubString(SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the len next rsubstring's. */
	LWUnicodeGraphemeIterator<Type> PrevrSubString(uint32_t Len, const LWUnicodeIterator<Type> &SubString, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvancerSubString(Len, SubString, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next of any of the tokens in TokenIter(or current if already at substring/first.) */
	LWUnicodeGraphemeIterator<Type> PrevTokens(const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceTokens(TokenIterList, SkipCurrentToken);
	}

	/*!< \brief get's a new iterator to the next len of any of the tokens in TokenIter(or current if already at substring/first.) */
	LWUnicodeGraphemeIterator<Type> PrevTokens(uint32_t Len, const LWUnicodeIterator<Type> &TokenIterList, bool SkipCurrentToken = true) const {
		return LWUnicodeGraphemeIterator<Type>(*this).rAdvanceTokens(Len, TokenIterList, SkipCurrentToken);
	}

	/*!< \brief returns a Unicode iterator between the start and end of the current cluster. */
	LWUnicodeIterator<Type> ClusterIterator(void) {
		return LWUnicodeIterator<Type>(*this, LWUnicodeIterator<Type>::Next(m_CodePointCount));
	}

	template<size_t Len>
	LWUnicodeGraphemeIterator(const Type(&Buffer)[Len]) : LWUnicodeIterator<Type>(Buffer) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(const Type *Buffer, uint32_t BufferLen) : LWUnicodeIterator<Type>(Buffer, BufferLen) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(const Type *Buffer) : LWUnicodeIterator<Type>(Buffer) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	template<size_t Len>
	LWUnicodeGraphemeIterator(const LWUnicodeGraphemeIterator<Type>::C_View<Len> &CStr) : LWUnicodeGraphemeIterator(CStr.m_Data) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(const LWUnicodeGraphemeIterator<Type> &First, const LWUnicodeGraphemeIterator<Type> &End) : LWUnicodeIterator<Type>(First.m_Index, First(), First(), End()), m_Character(First.m_Character) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(const LWUnicodeIterator<Type> &First, const LWUnicodeIterator<Type> &End, int32_t Index = 0, int32_t Character = 0) : LWUnicodeIterator<Type>(First, End, Index), m_Character(Character) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(const Type *First, const Type *End, int32_t Index = 0, int32_t Character = 0) : LWUnicodeIterator<Type>(First, End, Index), m_Character(Character) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(const LWUnicodeIterator<Type> &Iterator, int32_t Character = 0) : LWUnicodeIterator<Type>(Iterator), m_Character(Character) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator(int32_t Character, uint32_t Index, const Type *Position, const Type *First, const Type *Last) : LWUnicodeIterator<Type>(Index, Position, First, Last), m_Character(Character) {
		m_CodePointCount = CountGraphemeCluster(*this);
	}

	LWUnicodeGraphemeIterator() = default;

	int32_t m_Character = 0; /*!< \brief the current grapheme cluster character. */
	uint32_t m_CodePointCount = 0; /*!< \brief the grapheme cluster codepoint's at current position. */
private:
};


#endif