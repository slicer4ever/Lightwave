#ifndef LWUNICODE_H
#define LWUNICODE_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWUnicodeIterator.h"
#include "LWCore/LWUnicodeGraphemeIterator.h"
#include "LWCore/LWAllocator.h"
#include <cassert>



/*!< \brief Unicode container which automatically increases size for new text. */
template<class Type>
class LWUnicode {
public:

	/*!< \brief validates the text for malformed text, returning false if malformed is detected, otherwise create's the Unicode object. */
	static bool Create(LWUnicode<Type> &UTF, const Type *Text, LWAllocator &Allocator) {
		uint32_t Len, RawLen;
		if (!LWUnicodeIterator<Type>::ValidateString(Text, &Len, &RawLen)) return false;
		UTF = LWUnicode<Type>(Text, Len, RawLen, Allocator);
		return true;
	}

	/*!< \brief validates the text upto count codepoints, returning false if malformed is detected, otherwise create's the Unicode object. */
	static bool Create(LWUnicode<Type> &UTF, const Type *Text, uint32_t Count, LWAllocator &Allocator) {
		uint32_t Len, RawLen;
		if (!LWUnicodeIterator<Type>::ValidateString(Text, Count, Len, RawLen)) return false;
		UTF = LWUnicode<Type>(Text, Len, RawLen, Allocator);
		return true;
	}

	/*!< \brief validates the text from the static string, returninf false if malformed utf is detected, otherwise create's the Unicode object. */
	template<size_t Len>
	static bool Create(LWUnicode<Type> &UTF, const Type(&Text)[Len], LWAllocator &Allocator) {
		uint32_t Length, RawLen;
		if (!LWUnicodeIterator<Type>::ValidateString(Text, Length, RawLen)) return false;
		UTF = LWUnicode<Type>(Text, Length, RawLen, Allocator);
		return true;
	}

	/*!< \brief insert's text at the specified position, returns a new iterator at the same pos. */
	template<std::size_t Len>
	LWUnicodeIterator<Type> Insert(LWUnicodeIterator<Type> &Pos, const Type(&Text)[Len]) {
		return Insert(Pos, Text, Len);
	}

	/*!< \brief insert's text at the specified position, from the source iterator, text is validated before copying in. 
		 \return a new iterator at the same pos. */
	LWUnicodeIterator<Type> Insert(const LWUnicodeIterator<Type> &Pos, const LWUnicodeIterator<Type> &Source) {
		LWAllocator *Alloc = LWAllocator::GetAllocator(m_Buffer);
		uint32_t NLen, NRawLen;
		if (!LWUnicodeIterator<Type>::ValidateIterator(Source, NLen, NRawLen)) return Pos;
		Type *oBuffer = pReserve(m_RawLength + --NRawLen);
		uint32_t rIndex = Pos.RawIndex();
		std::copy_backward(m_Buffer + rIndex, m_Buffer + m_RawLength, m_Buffer + NRawLen + m_RawLength);
		std::copy(Source(), Source() + NRawLen, m_Buffer + rIndex);
		m_RawLength += NRawLen;
		m_Length += NLen;
		LWAllocator::Destroy(oBuffer);
		return LWUnicodeIterator<Type>(m_Buffer + rIndex, m_Buffer + m_RawLength, Pos.m_Index);
	}

	/*!< \brief insert's text at the specified position, returns a new iterator at the same pos. */
	LWUnicodeIterator<Type> Insert(const LWUnicodeIterator<Type> &Pos, const Type *Text, uint32_t TextBufferLen) {
		return Insert(Pos, { Text, TextBufferLen });
	}

	/*!< \brief insert's text at the specified position, returns a new iterator at the same pos. */
	LWUnicodeIterator<Type> Insert(const LWUnicodeIterator<Type> &Pos, const Type *Text) {
		return Insert(Pos, { Text });
	}

	/*!< \brief insert's text at the specified position, returns a new iterator at the same pos, upto Count characters. */
	LWUnicodeIterator<Type> Insert(const LWUnicodeIterator<Type> &Pos, const LWUnicodeIterator<Type> &SourceStart, const LWUnicodeIterator<Type> &SourceEnd) {
		return Insert(Pos, { SourceStart, SourceEnd });
	}

	/*!< \brief generates a list of grapheme iterator's, split for each token. 
		 \note if Token is inside a grapheme cluster(and not at the start), it will be skipped.
	*/
	uint32_t SplitTokenGrapheme(LWUnicodeGraphemeIterator<Type> *IterBuffer, uint32_t IterBufferSize, uint32_t Token) const {
		return beginGrapheme().SplitToken(IterBuffer, IterBufferSize, Token);
	}

	/*!< \brief generates a list of grapheme iterator's, split for each token in the token list. */
	uint32_t SplitTokenGrapheme(LWUnicodeGraphemeIterator<Type> *IterBuffer, uint32_t IterBufferSize, const LWUnicodeIterator<Type> &TokenList) const {
		return beginGrapheme().SplitTokenList(IterBuffer, IterBufferSize, TokenList);
	}

	/*!< \brief generates a list of iterator's split for each token. */
	uint32_t SplitToken(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize, uint32_t Token) const {
		return begin().SplitToken(IterBuffer, IterBufferSize, Token);
	}

	/*!< \brief generates a list of iterator's split for each token in the token list. */
	uint32_t SplitTokens(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize, const LWUnicodeIterator<Type> &TokenList) const {
		return begin().SplitTokens(IterBuffer, IterBufferSize, TokenList);
	}

	/*!< \brief generates a list of iterator's split between each word in the string. */
	uint32_t SplitWords(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize) const {
		return begin().SplitWords(IterBuffer, IterBufferSize);
	}

	/*!< \brief generates a list of iterator's split between each line in the string. */
	uint32_t SplitLines(LWUnicodeIterator<Type> *IterBuffer, uint32_t IterBufferSize) const {
		return begin().SplitLines(IterBuffer, IterBufferSize);
	}

	/*!< \brief clear's all text from this Unicode object. */
	LWUnicode<Type> &Clear(void) {
		Erase(begin(), end());
		return *this;
	}

	/*!< \brief removes text between the two iterator locations. 
		 \return iterator at begin.
	*/
	LWUnicodeIterator<Type> Erase(const LWUnicodeIterator<Type> &Begin, const LWUnicodeIterator<Type> &End) {
		const Type *bP = Begin();
		const Type *eP = End();
		assert(bP >= m_Buffer && bP <= m_Buffer + m_RawLength && eP >= m_Buffer && eP <= m_Buffer + m_RawLength);
		if (eP < bP) return Erase(End, Begin);
		uint32_t bIndex = Begin.RawIndex();
		uint32_t eIndex = End.RawIndex();
		std::copy(m_Buffer + eIndex, m_Buffer + m_RawLength, m_Buffer + bIndex);
		m_RawLength -= (eIndex - bIndex);
		m_Length -= Begin.Distance(End);
		return Begin;
	}

	/*!< \brief remove's text at begin upto n codepoints(or end if encountered first).
		 \return iterator at begin.
	*/
	LWUnicodeIterator<Type> Erase(const LWUnicodeIterator<Type> &Begin, uint32_t Count) {
		return Erase(Begin, Begin.Next(Count));
	}

	/*!< \brief construct's an grapheme iterator between start to end characters. */
	LWUnicodeGraphemeIterator<Type> RangeGrapheme(uint32_t Start, uint32_t End) const {
		LWUnicodeGraphemeIterator<Type> C = AtGrapheme(Start);
		LWUnicodeGraphemeIterator<Type> E = C.Next(End - Start);
		return LWUnicodeGraphemeIterator<Type>(C, E);
	}

	/*!< \brief construct's an iterator between start to End codepoint's. */
	LWUnicodeIterator<Type> Range(uint32_t Start, uint32_t End) const {
		LWUnicodeIterator<Type> C = At(Start);
		LWUnicodeIterator<Type> E = C.Next(End - Start);
		return LWUnicodeIterator<Type>(C, E);
	}

	/*!< \brief returns an iterator at index, or end if reaching the end before reaching Index. */
	LWUnicodeIterator<Type> At(uint32_t Index) const {
		return begin().Next(Index);
	}

	/*!< \brief returns an grapheme iterator at the specified character, or end if reaching the end before reaching character. */
	LWUnicodeGraphemeIterator<Type> AtGrapheme(uint32_t Character) const {
		return beginGrapheme().Next(Character);
	}

	/*!< \brief update's buffer capcity to contain upto Length characters(if bufferSize is already >= Length then nothing is done.)
		 \return true if the buffer was resized, false if it wasn't.
	*/
	bool Reserve(uint32_t Length) {
		LWAllocator::Destroy(pReserve(Length));
	}

	/*!< \brief returns an iterator to the first grapheme in the unicode string. */
	LWUnicodeGraphemeIterator<Type> beginGrapheme(void) const {
		return LWUnicodeGraphemeIterator<Type>(m_Buffer, m_Buffer + m_RawLength - 1);
	}

	/*!< \brief returns an iterator the the last grapheme in the unicode string.
	*	 \note the character is set to m_Length, as the grapheme clusters are not tracked by LWUnicode, an application that wants to know the number of characters will need to count them manually from begin.
	*/
	LWUnicodeGraphemeIterator<Type> endGrapheme(void) const {
		return LWUnicodeGraphemeIterator<Type>(m_Length, m_Length, m_Buffer + m_RawLength - 1, m_Buffer, m_Buffer + m_RawLength - 1);
	}

	/*!< \brief returns an iterator to the first codepoint in the Unicode string. */
	LWUnicodeIterator<Type> begin(void) const {
		return LWUnicodeIterator<Type>(m_Buffer, m_Buffer + m_RawLength-1);
	}

	/*!< \brief returns an iterator to the end of the Unicode string. */
	LWUnicodeIterator<Type> end(void) const {
		return LWUnicodeIterator<Type>(m_Length, m_Buffer+m_RawLength-1, m_Buffer, m_Buffer+m_RawLength-1);
	}

	friend std::ostream &operator << (std::ostream &o, const LWUnicode<Type> &U) {
		o << U.begin();
		return o;
	}

	/*!< \brief returns a pointer to the underlying data. */
	const Type *data(void) const {
		return m_Buffer;
	}

	/*!< \brief returns a 32 bit hash'd representation of this string.  This hashing is based on FNVA1A implementation as found in LWCrypto. */
	uint32_t Hash(void) const {
		return LWCrypto::HashFNV1A(begin());
	}

	/*!< \brief returns a pointer to the underlying data. */
	const Type *operator()(void) const {
		return m_Buffer;
	}

	/*!< \brief quick way to get beginning of iterator to underlying string. */
	LWUnicodeIterator<Type> operator*(void) const {
		return begin();
	}

	/*!< \brief append's text to the end of the string. */
	template<size_t Len>
	LWUnicode<Type> &operator+=(const Type (&Text)[Len]) {
		Insert(end(), Text);
		return *this;
	}

	/*!< \brief append's text to the end of the string. */
	LWUnicode<Type> &operator+=(const Type *Text) {
		Insert(end(), Text);
		return *this;
	}

	/*!< \brief append's text to the end of the string. */
	LWUnicode<Type> &operator+=(const LWUnicodeIterator<Type> &Source) {
		Insert(end(), Source);
		return *this;
	}

	/*!< \brief set's the string to specified text. */
	template<size_t Len>
	LWUnicode<Type> &operator=(const Type(&Text)[Len]) {
		Erase(begin(), end());
		Insert(begin(), Text);
		return *this;
	}

	/*!< \brief set's the string to specified text. */
	LWUnicode<Type> &operator=(const Type *Text) {
		Erase(begin(), end());
		Insert(begin(), Text);
		return *this;
	}

	/*!< \brief set's the string to specified text. */
	LWUnicode<Type> &operator=(const LWUnicodeIterator<Type> &TextIter) {
		Erase(begin(), end());
		Insert(begin(), TextIter);
		return *this;
	}

	/*!< \brief move operator for LWUnicode. */
	LWUnicode<Type> &operator=(LWUnicode<Type> &&O) {
		Type *oBuffer = m_Buffer;
		m_Buffer = O.m_Buffer;
		m_RawLength = O.m_RawLength;
		m_Length = O.m_Length;
		m_Capacity = O.m_Capacity;
		O.m_RawLength = O.m_Length = O.m_Capacity = 0;
		O.m_Buffer = nullptr;
		LWAllocator::Destroy(oBuffer);
		return *this;
	}

	/*!< \brief copy operator for LWUnicode. */
	LWUnicode<Type> &operator=(const LWUnicode<Type> &O) {
		assert(O.m_Buffer != nullptr);
		LWAllocator *Alloc = LWAllocator::GetAllocator(O.m_Buffer);
		Type *oBuffer = m_Buffer;
		Type *nBuffer = Alloc.AllocateA<Type>(O.m_Capacity);
		std::copy(O.m_Buffer, O.m_Buffer + O.m_RawLength, nBuffer);
		m_Length = O.m_Length;
		m_RawLength = O.m_RawLength;
		m_Capacity = O.m_Capacity;
		LWAllocator::Destroy(oBuffer);
		return *this;
	}

	/*!< \brief compare's the text of this string to the text of other, returning true if they match. */
	bool Compare(const LWUnicode<Type> &Other) const {
		return begin().Compare(Other.begin());
	}

	/*!< \brief compare's the text of this string to the text of other, upto n codepoint's.  returning true if they match. */
	bool Compare(const LWUnicode<Type> &Other, uint32_t Count) const {
		return begin().Compare(Other.begin(), Count);
	}

	/*!< \brief compare's the text of this string to the text of iterator. */
	bool Compare(const LWUnicodeIterator<Type> &Iter) const {
		return begin().Compare(Iter);
	}

	/*!< \brief compare's the text of this string upto Count codepoints to the text of iterator. */
	bool Compare(const LWUnicodeIterator<Type> &Iter, uint32_t Count) const {
		return begin().Compare(Iter, Count);
	}

	/*!< \brief compare's the text of this string to text. */
	template<size_t Len>
	bool Compare(const Type(&Text)[Len]) const {
		return begin().Compare(Text);
	}

	/*!< \brief compare's the text of this string to text upto Count codepoints. */
	template<size_t Len>
	bool Compare(const Type (&Text)[Len], uint32_t Count) const {
		return begin().Compare(Text, Count);
	}

	/*!< \brief compare's the text of this string to Text. */
	bool Compare(const Type *Text) const {
		return begin().Compare(Text);
	}

	/*!< \brief compare's the text of this string to Text upto Count codepoints. */
	bool Compare(const Type *Text, uint32_t Count) const {
		return begin().Compare(Text, Count);
	}

	/*!< \brief base list compare function, if compare's, then returns N, otherwise returns -1. */
	template<uint32_t N=0, class T>
	uint32_t CompareList(const T &Arg) const {
		return Compare(Arg) ? N : -1;
	}

	/*!< \brief base list compare upto Count codepoints.  if compare is successful then returns N, otherwise returns -1. */
	template<uint32_t N=0, class T>
	uint32_t CompareListn(uint32_t Count, const T &Arg) const {
		return Compare(Arg, Count) ? N : -1;
	}

	/*!< \brief base list compare where each item also has a char count. */
	template<uint32_t N = 0, class T>
	uint32_t CompareListnc(uint32_t Count, const T &Arg) const {
		return Compare(Arg, Count) ? N : -1;
	}

	/*!< \brief compare's a list of string arguments, returning the index of the item that was compared to, or -1 if does not match any. */
	template<uint32_t N=0, class T, typename ...Args>
	uint32_t CompareList(const T &Arg, Args... Pack) const {
		if (Compare(Arg)) return N;
		return CompareList<N + 1>(Pack...);
	}

	/*!< \brief compare's a list of string arguments upto Count code points, returning the index of the item that was compared to, or -1 if does not match any. */
	template<uint32_t N=0, class T, typename ...Args>
	uint32_t CompareListn(uint32_t Count, const T &Arg, Args... Pack) const {
		if (Compare(Count, Arg)) return N;
		return CompareListn<N + 1>(Count, Pack...);
	}

	/*!< \brief compare's a list of string arguments, each string is a pair of Count + String. */
	template<uint32_t N=0, class T, typename ...Args>
	uint32_t CompareListnc(uint32_t Count, const T &Arg, Args... Pack) const {
		if (Compare(Count, Arg)) return N;
		return CompareListnc<N + 1>(Pack...);
	}

	/*!< \brief compare's a list of comparable item's if they match this text, returns the index if one of the string's match, or -1 if none match. */
	template<class T>
	uint32_t CompareLista(uint32_t Count, const T *List) const {
		return begin().CompareLista(Count, List);
	}

	/*!< \brief compare's a list of comparable item's upto CodePointCount, returns the index if one of the string's match, or -1 if none match. */
	template<class T>
	uint32_t CompareLista(uint32_t Count, const T *List, uint32_t CodepointCount) const {
		return begin().CompareLista(Count, List, CodepointCount);
	}

	/*!< \brief compare's this string match's Other string. */
	bool operator==(const LWUnicode<Type> &Other) const {
		return Compare(Other);
	}

	/*!< \brief compare's this string to the iterator string text, returning true if the text match's, or false if it does not match. */
	bool operator==(const LWUnicodeIterator<Type> &Iter) const {
		return Compare(Iter);
	}

	/*!< \brief returns true if this string is not equal to the other string. */
	bool operator!=(const LWUnicode<Type> &Other) const {
		return !(*this == Other);
	}

	/*!< \brief returns true if this string is not equal to the iter string. */
	bool operator!=(const LWUnicodeIterator<Type> &Iter) const {
		return !(*this == Iter);
	}

	/*!< \brief returns an iterator to the index, or to the end of the string if that's encountered first. */
	LWUnicodeIterator<Type> operator[](uint32_t Index) const {
		return At(Index);
	}

	/*!< \brief returns the codepoint length's of the string. */
	uint32_t Length(void) const {
		return m_Length;
	}

	/*!< \brief returns a c_str const char * to pass to c api's, note that this is only applicable for utf-8 strings. */
	const char *c_str(void) const {
		return (const char*)m_Buffer;
	}

	/*!< \brief returns the raw length of the string(+1 for null character). */
	uint32_t RawLength(void) const {
		return m_RawLength;
	}

	uint32_t Capacity(void) const {
		return m_Capacity;
	}

	/*!< \brief default constructor. */
	LWUnicode() = default;

	/*!< \brief move constructor of this Unicode object. */
	LWUnicode(LWUnicode<Type> &&O) : m_Length(O.m_Length), m_RawLength(O.m_RawLength), m_Capacity(O.m_Capacity) {
		Type *oBuffer = m_Buffer;
		m_Buffer = O.m_Buffer;
		O.m_Buffer = nullptr;
		LWAllocator::Destroy(oBuffer);
	}

	/*!< \brief create's a copy of this Unicode object. */
	LWUnicode(const LWUnicode<Type> &O) : m_Length(O.m_Length), m_RawLength(O.m_RawLength), m_Capacity(O.m_Capacity) {
		assert(O.m_Buffer != nullptr);
		Type *oBuffer = m_Buffer;
		LWAllocator *Alloc = LWAllocator::GetAllocator(O.m_Buffer);
		m_Buffer = Alloc->AllocateA<Type>(m_Capacity);
		std::copy(O.m_Buffer, O.m_Buffer + m_RawLength, m_Buffer);
		LWAllocator::Destroy(oBuffer);
	}

	/*!< \brief create's a copy of stack Text object. */
	template<size_t Len>
	LWUnicode(const Type(&Text)[Len], LWAllocator &Allocator) {
		if (!LWUnicodeIterator<Type>::ValidateString(Text, m_Length, m_RawLength)) return;
		m_Buffer = Allocator.AllocateA<Type>(m_RawLength);
		std::copy(Text, Text + m_RawLength, m_Buffer);
		m_Capacity = m_RawLength;
	}
	
	/*!< \brief Validates Text and allocates space for it. */
	LWUnicode(const Type *Text, LWAllocator &Allocator) {
		if (!LWUnicodeIterator<Type>::ValidateString(Text, m_Length, m_RawLength)) return; //If malformed text is detected, then no unicode string is created.
		m_Buffer = Allocator.AllocateA<Type>(m_RawLength);
		std::copy(Text, Text + m_RawLength, m_Buffer);
		m_Capacity = m_RawLength;
	}

	/*!< \brief constructs a unicode object, the application must ensure the text is not malformed. */
	LWUnicode(const Type *Text, uint32_t Length, uint32_t RawLength, LWAllocator &Allocator) : m_Length(Length), m_RawLength(RawLength), m_Capacity(RawLength) {
		m_Buffer = Allocator.AllocateA<Type>(m_Capacity);
		std::copy(Text, Text + m_RawLength, m_Buffer);
	}

	/*!< \brief constructs a unicode object from an iterator. */
	LWUnicode(const LWUnicodeIterator<Type> &Text, LWAllocator &Allocator) {
		if (!LWUnicodeIterator<Type>::ValidateString(Text(), m_Length, m_RawLength)) {
			m_Capacity = m_RawLength = 1;
			m_Buffer = Allocator.AllocateA<Type>(m_Capacity);
			*m_Buffer = '\0';
			return;
		}
		m_Buffer = Allocator.AllocateA<Type>(m_RawLength);
		std::copy(Text(), Text() + m_RawLength, m_Buffer);
		m_Capacity = m_RawLength;
	}

	/*!< \brief destructor for unicode object. */
	~LWUnicode() {
		m_Buffer = LWAllocator::Destroy(m_Buffer);
	}
private:
	/*!< \brief internal reserve that doesn't destroy old buffer, instead returning it, for certain functions to work correctly. */
	Type *pReserve(uint32_t Length) {
		assert(m_Buffer != nullptr);
		if (Length <= m_Capacity) return nullptr;
		LWAllocator *Alloc = LWAllocator::GetAllocator(m_Buffer);
		Type *oBuffer = m_Buffer;
		Type *nBuffer = Alloc->AllocateA<Type>(Length);
		std::copy(oBuffer, oBuffer + m_RawLength, nBuffer);
		m_Buffer = nBuffer;
		m_Capacity = Length;
		return oBuffer;
	}

	Type *m_Buffer = nullptr; /*!< \brief buffer that contain's the unicode string. */
	uint32_t m_Length = 0; /*!< \brief codepoint length of the unicode string. */
	uint32_t m_RawLength = 0; /*!< \brief raw length of the unicode string. */
	uint32_t m_Capacity = 0; /*!< \brief capcity of the buffer. */
};


//Implementations of strlcpy+strlcat if they aren't part of the platform's standard c library.
size_t LWstrlcpy(char *Dest, const char *Src, size_t dstSize);

size_t LWstrlcat(char *Dest, const char *Src, size_t dstSize);

#ifndef strlcpy
#define strlcpy LWstrlcpy
#endif

#ifndef strlcat
#define strlcat LWstrlcat
#endif

#endif