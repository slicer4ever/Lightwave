#ifndef LWCORE_H
#define LWCORE_H
#include <cstring>
#include "LWCore/LWTypes.h"
#include "LWCore/LWAllocator.h"
#include <iostream>
/*! \addtogroup LWCore
	@{
*/

/*! \brief a UTF-8 string container. LWText can also accept const strings to act as a temporary container for passing to functions without allocating additional memory, if the LWText object is functioning like this then be careful that you only read back the LWText object, and not modify it.
*/
class LWText{
public:
	/*! \brief generates a hash for the supplied utf-8 text. this uses the fnv-1a algorithmn 
		\note 2166136261 is the default hash value of fnv-1a.
	*/
	static uint32_t MakeHash(const uint8_t *Text, uint32_t Hash = 2166136261) {
		const uint32_t Prime = 16777619;
		for (; *Text; ++Text) Hash = (Hash ^ (((uint32_t)*Text & 0xFF)))*Prime;
		return Hash;
	}

	/*! \overload uint32_t MakeHash(const char*, uint32_t) */
	static uint32_t MakeHash(const char *Text, uint32_t Hash = 2166136261) {
		const uint32_t Prime = 16777619;
		for (; *Text; ++Text) Hash = (Hash ^ (((uint32_t)*Text&0xFF)))*Prime;
		return Hash;
	}

	/*!< \brief generates a hash for the supplied buffer, this uses the fnv-1a algorithmn */
	static uint32_t MakeHashb(const uint8_t *Buffer, uint32_t BufferLen, uint32_t Hash = 2166136261) {
		const uint32_t Prime = 16777619;
		for (uint32_t i = 0; i < BufferLen; i++) Hash = (Hash ^ (((uint32_t)Buffer[i] & 0xFF)))*Prime;
		return Hash;
	}

	/*!< \overload uint32_t MakeHash(const char*, uint32_t, uint32_t) */
	static uint32_t MakeHashb(const char *Buffer, uint32_t BufferLen, uint32_t Hash = 2166136261) {
		const uint32_t Prime = 16777619;
		for (uint32_t i = 0; i < BufferLen; i++) Hash = (Hash ^ (((uint32_t)Buffer[i]&0xFF)))*Prime;
		return Hash;
	}

	/*!< \brief returns true if the character is considered a whitespace character. */
	static bool isWhitespace(uint32_t Character);

	/*!< \brief returns the number of characters the utf-8 character takes up. */
	static uint32_t UTF8ByteSize(uint32_t Character);

	/*!< \brief returns the number of characters that the utf-8 character at position P takes up. */
	static uint32_t UTF8ByteSize(const uint8_t *P);

	/*!< \overload uint32_t UTF8ByteSize(const char*) */
	static uint32_t UTF8ByteSize(const char *P);

	/*!< \brief returns the next first non white-space character in the utf-8 text, if none exists, returns null. 
		 \param Position the current location in the utf-8 character stream.
		 \param First if this is the first word to test, then we ignore leading whitespaces to the first non-whitespace character.
		 \return null if no word found, or the first character to the next word.
	*/
	static const uint8_t *NextWord(const uint8_t *Position, bool First = false);

	/*!< \overload const uint8_t * NextWord(const uint8_t*) */
	static const char *NextWord(const char *Position, bool First = false);

	/*!< \overload const uint8_t * NextWord(const uint8_t*) */
	static uint8_t *NextWord(uint8_t *Position, bool First = false);

	/*!< \overload const uint8_t * NextWord(const uint8_t*) */
	static char *NextWord(char *Position, bool First = false);

	/*!< \brief returns the prev first non white-space character in the uft-8 text, if none exists, return null.
		 \param Position the current location in the utf-8 character stream.
		 \param Start the beginning of the stream, necessary to know so we don't pass into garbage data.
		 \param First if this is the first word to test, then we find the first character in the current word.
		 \return null if no other word is found, otherwise returns the beginning of the prev word that is a non-white space character.
	*/
	static const uint8_t *PrevWord(const uint8_t *Position, const uint8_t *Start, bool First = false);
	
	/*!< \overload const uint8_t * PrevWord(const uint8_t *, const uint8_t *, bool) */
	static const char *PrevWord(const char *Position, const char *Start, bool First);

	/*!< \overload const uint8_t * PrevWord(const uint8_t *, const uint8_t *, bool) */
	static uint8_t *PrevWord(uint8_t *Position, const uint8_t *Start, bool First);

	/*!< \overload const uint8_t * PrevWord(const uint8_t *, const uint8_t *, bool) */
	static char *PrevWord(char *Position, const char *Start, bool First);

	/*!< \brief returns the first occurrence of token in the utf-8 text, if none exists, returns null. */
	static const uint8_t *FirstToken(const uint8_t *Position, uint32_t Token);

	/*!< \overload const char * FirstToken(const char *, uint32_t) */
	static const char *FirstToken(const char *Position, uint32_t Token);

	/*!< \overload uint8_t *FirstToken(uint8_t *, uint32_t) */
	static uint8_t *FirstToken(uint8_t *Position, uint32_t Token);

	/*!< \overload char *FirstToken(char *, uint32_t) */
	static char *FirstToken(char *Position, uint32_t Token);

	/*!< \brief returns the first occurrence of any token specified in the token list. otherwise returns null if non exists.
		 \note The token list is expected to be a utf-8 string. */
	static const uint8_t *FirstTokens(const uint8_t *Position, const uint8_t *Tokens);

	/*!< \overload const char *FirstTokens(const char *, const char *); */
	static const char *FirstTokens(const char *Position, const char *Tokens);

	/*!< \overload uint8_t *FirstTokens(uint8_t *, const uint8_t *); */
	static uint8_t *FirstTokens(uint8_t *Position, const uint8_t *Tokens);

	/*!< \overload char *FirstTokens(char *Position, const char *); */
	static char *FirstTokens(char *Position, const char *Tokens);

	/*!< \brief returns the first occurrence of a substring, otherwise returns null if the sub string is not found.
		 \note the sub string is expected to be a utf-8 string. */
	static const uint8_t *FirstString(const uint8_t *Position, const uint8_t *SubString);

	/*!< \overload const char *FirstString(const char *, const char *); */
	static const char *FirstString(const char *Position, const char *SubString);

	/*!< \overload uint8_t *FirstString(uint8_t *, const uint8_t *); */
	static uint8_t *FirstString(uint8_t *Position, const uint8_t *SubString);

	/*!< \overload char *FirstString(char *, const char *); */
	static char *FirstString(char *Position, const char *SubString);

	/*!< \brief copies into buffer up until any of the specified tokens are encountered(or the end of the string, or if the end of the buffer is reached.)
		 \note the token list is expected to be a utf-8 string. 
		 \return the new position either at the end of stream, or when encountering the first token.
	*/
	static const uint8_t *CopyToTokens(const uint8_t *Position, uint8_t *Buffer, uint32_t BufferLen, const uint8_t *Tokens);

	/*!< \overload const char *CopyToTokens(const char *, char *, uint32_t, const char *); */
	static const char *CopyToTokens(const char *Position, char *Buffer, uint32_t BufferLen, const char *Tokens);

	/*!< \overload uint8_t *CopyToTokens(uint8_t *, uint8_t *, uint32_t, const char *); */
	static uint8_t *CopyToTokens(uint8_t *Position, uint8_t *Buffer, uint32_t BufferLen, const uint8_t *Tokens);

	/*!< \overload char *CopyToTokens(char *, char *, uint32_t, const char *); */
	static char *CopyToTokens(char *Position, char *Buffer, uint32_t BufferLen, const char *Tokens);

	/*!< \brief copies into BufferList a list of strings split by Token.
		 \param String the string to split.
		 \param BufferList array of pointers to buffers for each string(pass null if not writing anything out).
		 \param BufferLen the length of each buffer for writing to.
		 \param LongestBuffer the length of the longest buffer that was written to.
		 \param Token the token to split on.
		 \return the number of buffers written to.
	*/
	static uint32_t SplitToken(const uint8_t *String, uint8_t **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, uint32_t Token);
	
	/*!< \overload uint32_t SplitToken(const char*, char**, uint32_t, uint32_t, uint32_t &, uint32_t) */
	static uint32_t SplitToken(const char *String, char **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, uint32_t Token);
	
	/*!< \brief copies into BufferList a list of strings split by Tokens.
		 \param String the string to split.
		 \param BufferList array of pointers to buffers for each string(pass null if not writing anything out).
		 \param BufferLen the length of each buffer for writing to.
		 \param LongestBuffer the length of the longest buffer that was/will be written to.
		 \param Tokens the string of tokens to split on.
		 \return the number of buffers written to.
	*/
	static uint32_t SplitTokens(const uint8_t *String, uint8_t **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, const uint8_t *Tokens);

	/*!< \overload uint32_t SplitTokens(const char *, char **, uint32_t, uint32_t, uint32_t &, const char *) */
	static uint32_t SplitTokens(const char *String, char **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer, const char *Tokens);

	/*!< \brief copies into BufferList a list of each non whitespace word. 
		 \param String the string to split.
		 \param BufferList array of pointers to buffers for each string(pass null if not writing anything out).
		 \param BufferLen the length of each buffer for writing to.
		 \param LongestBuffer the length of the longest buffer that was/will be written to.
	*/
	static uint32_t SplitWords(const uint8_t *String, uint8_t **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer);

	/*!< \overload uint32_t SplitWords(const char*, char **, uint32_t, uint32_t, uint32_t &) */
	static uint32_t SplitWords(const char *String, char **BufferList, uint32_t BufferLen, uint32_t BufferCount, uint32_t &LongestBuffer);

	/*!< \brief copys into buffer n of characters, returns the number of bytes written. */
	static uint32_t Copy(const uint8_t *Pos, uint32_t n, uint8_t *Buffer, uint32_t BufferLen);

	/*!< \overload uint32_t Copy(const char *, uint32_t, char*, uint32_t); */
	static uint32_t Copy(const char *Pos, uint32_t n, char *Buffer, uint32_t BufferLen);

	/*! \brief returns the next character of the UTF-8 string. 
		\note returns null if at the end of the string.
	*/
	static const uint8_t *Next(const uint8_t *Position);

	/*! \overload const char * Next(const char *) */
	static const char *Next(const char *Position);

	/*! \overload uint8_t *Next(uint8_t *) */
	static uint8_t *Next(uint8_t *Position);

	/*!< \overload char *Next(char *) */
	static char *Next(char *Position);

	/*! \brief returns the previous character of the utf-8 string. requires knowing the first character in the string.
		\note returns null if at the beginning of the string. requires passing in the start of the string. */
	static const uint8_t *Prev(const uint8_t *Position, const uint8_t *String);

	/*!< \overload const uint8_t *Prev(const uint8_t *, const uint8_t *) */
	static const char *Prev(const char *Position, const char *String);

	/*!< \overload const uint8_t *Prev(const uint8_t *, const uint8_t *) */
	static uint8_t *Prev(uint8_t *position, const uint8_t *String);

	/*!< \overload const uint8_t *Prev(const uint8_t *, const uint8_t *) */
	static char *Prev(char *Position, const char *String);

	/*!< \brief returns the pointer to the uft-8 character at index. returns null if the end of the string is encountered. */
	static const uint8_t *At(const uint8_t *Text, uint32_t Index);

	/*!< \overload const uint8_t *At(const uint8_t *, uint32_t) */
	static const char *At(const char *Text, uint32_t Index);

	/*!< \overload const uint8_t *At(const uint8_t *, uint32_t) */
	static uint8_t *At(uint8_t *Text, uint32_t Index);

	/*!< \overload const uint8_t *At(const uint8_t *, uint32_t) */
	static char *At(char *Text, uint32_t Index);

	/*! \brief returns the actual UTF-32 character that the current Position is at. */
	static uint32_t GetCharacter(const uint8_t *Position);

	/*! \overload uint32_t GetCharacter(const char *) */
	static uint32_t GetCharacter(const char *Position);

	/*! \brief returns the text length of the provided utf-8 text. */
	static uint32_t TextLength(const uint8_t *Text);

	/*! \overload uint32_t GetTextLength(const char *) */
	static uint32_t TextLength(const char *Position);

	/*! \brief returns the number of utf-8 chars to contain the character. */
	static uint32_t GetUTF8Size(uint32_t Char);

	/*!< \brief takes a UTF-8 string, and makes a UTF-16 string.
		 \param Text the utf-8 string.
		 \param Buffer the uint16_t buffer to receive the converted string. passing null will not write to buffer.
		 \param BufferLen the size of the Buffer(note this is sizeof(uint16_t), and not bytes.
		 \return the number of characters that the string is.
		 \note if a character is to be larger than UTF-16, the value is truncated to fit into the 16 bit type.
	*/
	static uint32_t MakeUTF8To16(const uint8_t *Text, uint16_t *Buffer, uint32_t BufferLen);

	/*!< \brief takes a UTF-8 string, and makes a UTF-32 string.
		 \param Text the utf-8 string.
		 \param Buffer the uint32_t buffer to receive the converted string. passing null will not write to buffer.
		 \param BufferLen the size of the Buffer(note this is sizeof(uint32_t), and not bytes.
		 \return the number of characters that the string is.
	*/
	static uint32_t MakeUTF8To32(const uint8_t *Text, uint32_t *Buffer, uint32_t BufferLen);

	/*!< \brief takes a UTF-16 string, and makes a UTF-8 string.
		 \param Text the utf-16 string.
		 \param Buffer the uint8_t buffer to receive the converted string. pass null will not write to buffer.
		 \param BufferLen the size of the Buffer.
		 \return the number of bytes to be written to buffer.
	*/
	static uint32_t MakeUTF16To8(const uint16_t *Text, uint8_t *Buffer, uint32_t BufferLen);

	/*!< \brief takes a UTF-32 string, and makes a UTF-8 string.
		 \param Text the utf-32 string.
		 \param Buffer the uint8_t Buffer to receive the converted string. pass null will not write to buffer.
		 \param BufferLen the size of the buffer.
		 \return the number of bytes to be written to buffer.
	*/
	static uint32_t MakeUTF32To8(const uint32_t *Text, uint8_t *Buffer, uint32_t BufferLen);

	/*!< \brief takes a UTF-32 string, and makes a UTF-8 string.
		\param Text the utf-32 string.
		\param CharCnt the number of utf-32 characters to write into buffer.
		\param Buffer the uint8_t Buffer to receive the converted string. pass null will not write to buffer.
		\param BufferLen the size of the buffer.
		\return the number of bytes to be written to buffer.
	*/
	static uint32_t MakeUTF32To8(const uint32_t *Text, uint32_t CharCnt, uint8_t *Buffer, uint32_t BufferLen);

	/*! \brief returns the utf-8 character of the text. 
		\note this will return null if Text is null, or if the first character of text is null.
	*/
	static const uint8_t *FirstCharacter(const uint8_t *Text);

	/*! \overload uint8_t *FirstCharacter(const uint8_t *Text)*/
	static const char *FirstCharacter(const char *Text);

	/*! \overload uint8_t *FirstCharacter(const uint8_t *) */
	static uint8_t *FirstCharacter(uint8_t *Text);

	/*! \overload char *FirstCharacter(char*) */
	static char *FirstCharacter(char *Text);

	/*!< \brief compares two strings. */
	static bool Compare(const char *StrA, const char *StrB);

	static bool Compare(const LWText &StrA, const LWText &StrB);

	/*!< \brief compares two strings upto a certain length. */
	static bool Compare(const char *StrA, const char *StrB, uint32_t Count);

	/*!< \brief compares multiple strings and returns the index of the compared string, otherwise returns 0xFFFFFFFF if non compare. */
	static uint32_t CompareMultiple(const char *StrA, uint32_t CompareCount, ...);

	/*!< \brief compares multiple strings upto n characters, and returns the index of the compared string, otherwise returns 0xFFFFFFFF if non compare. */
	static uint32_t CompareMultiplen(const char *StrA, uint32_t CharCount, uint32_t CompareCount, ...);

	/*!< \brief compares multiple strings in an array of strings.
		 \return the index of the string compared, or 0xFFFFFFFF if no string was found.
	*/
	static uint32_t CompareMultiplea(const char *Str, uint32_t CompareCount, const char **Strings);

	/*!< \brief compares multiple strings in an array upto n characters.
		 \return the index of the string compared, or 0xFFFFFFFF if no string was found.
	*/
	static uint32_t CompareMultiplena(const char *Str, uint32_t CharCount, uint32_t CompareCount, const char **Strings);

	/*! \brief Sets the text object to a new string.  allocated using the LWAllocator passed in the constructor. if no allocator is preset, this is treated like a const wrapped constructor. */
	LWText &Set(const uint8_t *Text);

	/*! \overload LWText &Set(const char *) */
	LWText &Set(const char *Text);

	/*! \brief Sets the text object to a new string, allocated with the new allocator provided. */
	LWText &Set(const uint8_t *Text, LWAllocator &Allocator);

	/*! \overload LWText &Set(const char *Text, LWAllocator &Allocator); */
	LWText &Set(const char *Text, LWAllocator &Allocator);

	/*! \brief Sets the text object to a new string where Text is a variably formated utf-8 string. */
	LWText &Setf(const uint8_t *Text, ...);

	/*! \overload LWText &Set(const char *, ...) */
	LWText &Setf(const char *Text, ...);

	/*! \brief Sets the text object to a new string where Text is a variably formated utf-8 string. */
	LWText &Setfa(LWAllocator &Allocator, const uint8_t *Text, ...);

	/*! \overload LWText &Set(const char *, LWAllocator &, ...) */
	LWText &Setfa(LWAllocator &Allocator, const char *Text, ...);

	/*! \brief Appends text after the current string. allocated using the LWAllocator passed in the constructor. */
	LWText &Append(const uint8_t *Text);

	/*! \overload LWText &Append(const char *); */
	LWText &Append(const char *Text);

	/*! \brief Appends a variable formated utf-8 string. */
	LWText &Appendf(const uint8_t *Text, ...);

	/*! \overload LWText &Appendf(const char *); */
	LWText &Appendf(const char *Text, ...);

	/*! \brief Pre-Appends text before the current string. allocated using the LWAllocator passed in the constructor. */
	LWText &PreAppend(const uint8_t *Text);

	/*! \overload LWText &PreAppend(const char *) */
	LWText &PreAppend(const char *Text);

	/*! \brief Pre-Appends a variable formated utf-8 string. */
	LWText &PreAppendf(const uint8_t *Text, ...);

	/*! \overload LWText &PreAppendf(const char *Text, ...) */
	LWText &PreAppendf(const char *Text, ...);

	/*! \brief move's the temporary into the other object. */
	LWText &operator = (LWText &&Other);

	/*! \brief returns if two strings are the same. */
	bool operator == (const LWText &Rhs) const;

	/*! \overload bool operator == (const uint8_t *Rhs) */
	bool operator == (const uint8_t *Rhs) const;

	/*! \overload bool operator == (const char *Rhs) */
	bool operator == (const char *Rhs) const;

	/*! \brief returns if two strings are not the same. */
	bool operator != (const LWText &Rhs) const;

	/*! \overload bool operator != (const uint8_t *Rhs) */
	bool operator != (const uint8_t *Rhs) const;

	/*! \overload bool operator != (const char *Rhs) */
	bool operator != (const char *Rhs) const;

	/*! \brief appends the right hand side string into this string. */
	LWText &operator+=(const LWText &Rhs);

	/*! \overload LWText &operator+=(const uint8_t *) */
	LWText &operator+=(const uint8_t *Rhs);

	/*! \overload LWText &operator+=(const char *) */
	LWText &operator+=(const char *Rhs);

	/*! \brief copys the other text object's text into this text, and uses the allocator used by other for allocation. */
	LWText &operator=(const LWText &Other);

	/*!< \brief adds LWText to be writable to std::ostream's(such as std::cout). */
	friend std::ostream &operator<<(std::ostream &o, const LWText &Text);
	
	/*! \brief returns the text length of the string. */
	uint32_t GetLength(void) const;

	/*! \brief returns the raw buffer length of the string. */
	uint32_t GetBufferLength(void) const;

	/*! \brief returns the generated 32 bit hash of the string. */
	uint32_t GetHash(void) const;

	/*! \brief returns an pointer to the internal text buffer of the first character, use the static methods to get characters, and advance the pointer.
		\note this pointer is invalid if the text object is changed.
	*/
	const uint8_t *GetCharacters(void) const;
		
	/*! \brief returns the allocator used in the creation of this text object. */
	LWAllocator *GetAllocator(void) const;

	/*! \brief copy constructor that copys the text and uses the Allocator specified from the original object, if it exists, otherwise it treats it as a const type object for readback only.	*/
	LWText(const LWText &Other);

	/*! \brief creates a read only LWText object that wraps the const uint8_t *, any attempts to modify the LWText object will result in nothing happenning. */
	LWText(const uint8_t *Text);

	/*! \overload LWText(const char *) */
	LWText(const char *Text);

	/*! \brief move's the object into this object. */
	LWText(LWText &&Other);

	/*! \brief constructor that accepts a utf-8 character string and copys it into the LWText object.
		\param Text the utf-8 encoded text.
		\param Allocator the allocator object to allocate the memory into, all future allocations will use this allocator.  
	*/
	LWText(const uint8_t *Text, LWAllocator &Allocator);

	/*! \overload LWText(const char *Text, LWAllocator &Allocator) */
	LWText(const char *Text, LWAllocator &Allocator);

	/*! \brief constructs an empty LWText object. */
	LWText();

	/*! \brief destroys the TextBuffer object. */
	~LWText();
private:
	uint8_t *m_TextBuffer;
	const uint8_t *m_ReadBuffer;
	uint32_t m_TextLength;
	uint32_t m_BufferLength;
	uint32_t m_Hash;
};

/*! @} */
#endif