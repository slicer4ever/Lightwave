#ifndef LWBYTESTREAM_H
#define LWBYTESTREAM_H
#include <functional>
#include <LWCore/LWByteBuffer.h>

/*!<
	\brief class which parses an active binary data stream, rather that's a continuous TCP network, or reading from file(LWFileByteStream).  shares near all read functions that LWByteBuffer offers, this is for read-only formats.
*/
class LWByteStream {
public:
	enum {
		Network = 0x1, /*!< \brief Flag to indicate data should be treated as network ordered. */
		AutoSize = 0x2 /*!< \brief Flag to indicate the data buffer should auto resize if it's not large enough to hold the requested data-> */
	};


	/*!< \brief Returns true if no data remains in the buffer and no new data is read in. */
	bool EndOfStream(void);

	/*!< \brief Checks if the number of bytes can be read, loads the cache back up if necessary, otherwise returns false. */
	bool CanReadBytes(uint32_t Bytes);

	/*!< \brief resizes the cached buffer. if the CacheLen is less then the remaining stream then the buffer is set to the remaining length.
		 \param MakeSmaller the buffer well not shrink to a smaller size unless this boolean is set to true.
	*/
	bool ResizeCacheBuffer(uint32_t NewBufferCacheLen, LWAllocator &Allocator, bool MakeSmaller=false);

	/*!< \brief move operator. */
	LWByteStream &operator=(LWByteStream &&O);

	/*! \brief reads a variable of type from the internal buffer.
		\return the value of the object of type.
	*/
	template<class Type>
	Type Read(void) {
		typedef int32_t(*Func_T)(Type*, const int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		Type Value = Type();
		if (!CanReadBytes(sizeof(Type))) return Value;
		m_Position += Funcs[m_SelectedFunc](&Value, m_DataBuffer + m_Position);
		return Value;
	}

	/*! \brief reads an array of variables of type from the internal buffer.
		\param Values an array to store the values from.
		\param Len the length of elements to write into Values.
		\return the number of bytes read.
	*/
	template<class Type>
	int32_t Read(Type *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(Type *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type)*Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a vec2 of type from the internal buffer.*/
	template<class Type>
	LWVector2<Type> ReadVec2(void) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector2<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 2)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of vec2 of type from the internal buffer. */
	template<class Type>
	int32_t ReadVec2(LWVector2<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 2 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a vec3 of type from the internal buffer.*/
	template<class Type>
	LWVector3<Type> ReadVec3(void) {
		typedef int32_t(*Func_T)(LWVector3<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector3<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 3)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of vec3 of type from the internal buffer. */
	template<class Type>
	int32_t ReadVec3(LWVector3<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWVector3<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 3 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a vec4 of type from the internal buffer.*/
	template<class Type>
	LWVector4<Type> ReadVec4(void) {
		typedef int32_t(*Func_T)(LWVector4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector4<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 4)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of vec4 of type from the internal buffer. */
	template<class Type>
	int32_t ReadVec4(LWVector4<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWVector4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 4 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}


	/*!< \brief reads a Quaternion of type from the internal buffer.*/
	template<class Type>
	LWQuaternion<Type> ReadQuaternion(void) {
		typedef int32_t(*Func_T)(LWQuaternion<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWQuaternion<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 4)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of Quaternion of type from the internal buffer. */
	template<class Type>
	int32_t ReadQuaternion(LWQuaternion<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWQuaternion<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 4 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a matrix2 of type from the internal buffer.*/
	template<class Type>
	LWMatrix2<Type> ReadMat2(void) {
		typedef int32_t(*Func_T)(LWMatrix2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix2<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 4)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of matrix2 of type from the internal buffer. */
	template<class Type>
	int32_t ReadMat2(LWMatrix2<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWMatrix2<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 4 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a matrix3 of type from the internal buffer.*/
	template<class Type>
	LWMatrix3<Type> ReadMat3(void) {
		typedef int32_t(*Func_T)(LWMatrix3<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix3<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 9)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of matrix3 of type from the internal buffer. */
	template<class Type>
	int32_t ReadMat3(LWMatrix3<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWMatrix3<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 9 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a matrix4 of type from the internal buffer.*/
	template<class Type>
	LWMatrix4<Type> ReadMat4(void) {
		typedef int32_t(*Func_T)(LWMatrix4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix4<Type> Res;
		if (!CanReadBytes(sizeof(Type) * 16)) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_DataBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads an array of matrix4 of type from the internal buffer. */
	template<class Type>
	int32_t ReadMat4(LWMatrix4<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWMatrix4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (!CanReadBytes(sizeof(Type) * 16 * Len)) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_DataBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*! \brief reads an utf8 string from the buffer.
		\param Out the buffer to receive the text.
		\param OutLen the length of the buffer to receive the text.
		\sa int32_t ReadUTF8(int8_t *, uint32_t, const int8_t *)
	*/
	int32_t ReadUTF8(uint8_t *Out, uint32_t OutLen);

	/*! \overload int32_t ReadUTF8(char *, uint32_t) */
	int32_t ReadUTF8(char *Out, uint32_t OutLen);

	/*!< \brief reads a null terminated string from the buffer.
		 \param Out the buffer to receive the text.
		 \param OutLen the length of the buffer to receive the text.
	*/
	int32_t ReadText(uint8_t *Out, uint32_t OutLen);

	/*!< \overload int32_t ReadText(char*, uint32_t) */
	int32_t ReadText(char *Out, uint32_t OutLen);

	/*!< \brief offset's the stream by n bytes.  returns true if success, false on failure(not enough data remains) */
	bool OffsetStream(uint32_t Offset);

	/*!< \brief returns true stream was created with the flag Network. */
	bool IsNetworkStream(void) const;

	/*!< \brief returns the target cached buffer length. */
	uint32_t GetCachedBufferLength(void) const;

	/*!< \brief returns the remaining amount of data in the cache. */
	uint32_t GetRemainingCache(void) const;

	/*!< \brief Constructor for byte stream.
		 \param DataBufferLength the size of the internal cached amount of data to read at any time.
		 \param DataReadCallback function called when new data is needed(Buffer to write data to, length of data requested, user data, return total bytes read).
		 \param Flag flag for data stream, indicating how the data should be read.
		 \param Allocator allocator for creating the data buffer.
	*/
	LWByteStream(uint32_t CachedBufferLength, std::function<int32_t(int8_t*, uint32_t, void*)> DataReadCallback, uint32_t Flag, void *UserData, LWAllocator &Allocator);

	/*!< \brief default constructor. */
	LWByteStream() = default;

	/*!< \brief move constructor. */
	LWByteStream(LWByteStream &&O);

	/*!< \brief destructor for byte stream. */
	~LWByteStream();
private:
	LWAllocator *m_Allocator = nullptr;
	std::function<int32_t(int8_t*, uint32_t, void*)> m_ReadCallback = nullptr;
	int8_t *m_DataBuffer = nullptr;
	void *m_UserData = nullptr;
	uint32_t m_TargetCachedLength = 0;
	uint32_t m_CachedBufferLength = 0;
	uint32_t m_Position = 0;
	uint32_t m_SelectedFunc = 0;
	uint32_t m_Flag = 0;
};

#endif