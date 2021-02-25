#ifndef LWBYTEBUFFER_H
#define LWBYTEBUFFER_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWSVector.h"
#include "LWCore/LWSMatrix.h"
#include "LWCore/LWSQuaternion.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMatrix.h"
#include "LWCore/LWQuaternion.h"
#include <cstdarg>
#include <memory>

/*! \addtogroup LWCore LWCore
	@{
*/

/*! \brief A universal byte buffer for writing and reading streams of bytes.
	The byte buffer class facilitates in reading and writing arrays of bytes in either network, 
	or in host order depending on the requirements, as well as has a numerous suite of types for encoding LW types for encoding.
	
*/

class LWByteBuffer {
public:

	/*! \defgroup LWByteBufferFlag LWByteBufferFlag
		\brief Flags to pass to LWByteBuffer when constructing the buffer.
		\ingroup LWCore
	@{ */

	static const uint8_t Network = 1; /*!< \brief Network specifies that the byte buffer is to encode data for network or portable transmission. */
	static const uint8_t ReadOnly = 2; /*!< \brief Specifies if the byte buffer is a read only class. */
	static const uint8_t BufferNotOwned = 4; /*!< \brief specifies if the internal buffer is to not be considered owned by the Buffer. */
	/*! @} */

	/*! \brief converts the number from host order to network order.
		\param Value the host order number to be changed to network order.
		\return the network order resultant number.
	*/
	static int8_t MakeNetwork(int8_t Value);

	/*!
		\overload uint8_t LWByteBuffer::MakeNetwork(uint8_t)
	*/
	static uint8_t MakeNetwork(uint8_t Value);

	/*!
	\overload char LWByteBuffer::MakeNetwork(char)
	*/
	static char MakeNetwork(char Value);

	/*!
		\overload int16_t LWByteBuffer::MakeNetwork(int16_t)
	*/
	static int16_t MakeNetwork(int16_t Value);

	/*!
		\overload uint16_t LWByteBuffer::MakeNetwork(uint16_t)
	*/
	static uint16_t MakeNetwork(uint16_t Value);

	/*!
		\overload int32_t LWByteBuffer::MakeNetwork(int32_t)
	*/
	static int32_t MakeNetwork(int32_t Value);

	/*!
		\overload uint32_t LWByteBuffer::MakeNetwork(uint32_t)
	*/
	static uint32_t MakeNetwork(uint32_t Value);

	/*!
		\overload int64_t LWByteBuffer::MakeNetwork(int64_t)
	*/
	static int64_t MakeNetwork(int64_t Value);

	/*!
		\overload uint64_t LWByteBuffer::MakeNetwork(uint64_t)
	*/
	static uint64_t MakeNetwork(uint64_t Value);

	/*!
		\brief Encodes a float into an network order uint32_t.
		\overload uint32_t LWByteBuffer::MakeNetwork(float);
	*/
	static uint32_t MakeNetwork(float Value);

	/*!
		\brief Encodes a double into an network order uint64_t.
		\overload uint64_t LWByteBuffer::MakeNetwork(double);
	*/
	static uint64_t MakeNetwork(double Value);


	/*! \brief converts the number from network order to host order.
		\note Template specializations are used for generic programming in read/write methods.
		\param Value the number to be changed from network order to host order.
		\return the host order version of the same number.
		*/

	static int8_t MakeHost(int8_t Value);

	/*!
		\overload uint8_t LWByteBuffer::MakeHost(uint8_t);
	*/
	static uint8_t MakeHost(uint8_t Value);

	/*!
	\overload char LWByteBuffer::MakeHost(char);
	*/
	static char MakeHost(char Value);

	/*!
		\overload int16_t LWByteBuffer::MakeHost(int16_t)
	*/
	static int16_t MakeHost(int16_t Value);

	/*!
		\overload uint16_t LWByteBuffer::MakeHost(uint16_t)
	*/
	static uint16_t MakeHost(uint16_t Value);

	/*!
		\overload int32_t LWByteBuffer::MakeHost(int32_t)
	*/
	static int32_t MakeHost(int32_t Value);

	/*!
		\overload uint32_t LWByteBuffer::MakeHost(uint32_t)
	*/
	static uint32_t MakeHost(uint32_t Value);

	/*!
		\overload int64_t LWByteBuffer::MakeHost(int64_t)
	*/
	static int64_t MakeHost(int64_t Value);

	/*!
		\overload uint64_t LWByteBuffer::MakeHost(uint64_t)
	*/
	static uint64_t MakeHost(uint64_t Value);

	/*!
		\brief Decodes a uint32_t from network order to host order float.
	*/
	static float MakeHostf(uint32_t Value);

	/*!
		\brief Decodes a uint64_t from network order to host order double.
		\overload double LWByteBuffer::MakeHostf(uint64_t)
	*/
	static double MakeHostf(uint64_t Value);

	/*! \brief converts the number from host order to big endian.
	\param Value the host order number to be changed to big endian order.
	\return the network order resultant number.
	*/
	static int8_t MakeBig(int8_t Value);

	/*!
	\overload uint8_t LWByteBuffer::MakeBig(uint8_t)
	*/
	static uint8_t MakeBig(uint8_t Value);
	/*!
	\overload char LWByteBuffer::MakeBig(char)
	*/
	static char MakeBig(char Value);

	/*!
	\overload int16_t LWByteBuffer::MakeBig(int16_t)
	*/
	static int16_t MakeBig(int16_t Value);

	/*!
	\overload uint16_t LWByteBuffer::MakeBig(uint16_t)
	*/
	static uint16_t MakeBig(uint16_t Value);

	/*!
	\overload int32_t LWByteBuffer::MakeBig(int32_t)
	*/
	static int32_t MakeBig(int32_t Value);

	/*!
	\overload uint32_t LWByteBuffer::MakeBig(uint32_t)
	*/
	static uint32_t MakeBig(uint32_t Value);

	/*!
	\overload int64_t LWByteBuffer::MakeBig(int64_t)
	*/
	static int64_t MakeBig(int64_t Value);

	/*!
	\overload uint64_t LWByteBuffer::MakeBig(uint64_t)
	*/
	static uint64_t MakeBig(uint64_t Value);

	/*!
	\brief converts a float into an big endian order float.
	\overload float LWByteBuffer::MakeBig(float);
	*/
	static float MakeBig(float Value);

	/*!
	\brief converts a double into an big endian order double.
	\overload double LWByteBuffer::MakeBig(double);
	*/
	static double MakeBig(double Value);


	/*! \brief converts the number from host order to little endian order.
	\note Template specializations are used for generic programming in read/write methods.
	\param Value the number to be changed from network order to host order.
	\return the host order version of the same number.
	*/

	static int8_t MakeLittle(int8_t Value);

	/*!
	\overload uint8_t LWByteBuffer::MakeLittle(uint8_t);
	*/
	static uint8_t MakeLittle(uint8_t Value);

	/*!
	\overload char LWByteBuffer::MakeLittle(char);
	*/
	static char MakeLittle(char Value);

	/*!
	\overload int16_t LWByteBuffer::MakeLittle(int16_t)
	*/
	static int16_t MakeLittle(int16_t Value);

	/*!
	\overload uint16_t LWByteBuffer::MakeLittle(uint16_t)
	*/
	static uint16_t MakeLittle(uint16_t Value);

	/*!
	\overload int32_t LWByteBuffer::MakeLittle(int32_t)
	*/
	static int32_t MakeLittle(int32_t Value);

	/*!
	\overload uint32_t LWByteBuffer::MakeLittle(uint32_t)
	*/
	static uint32_t MakeLittle(uint32_t Value);

	/*!
	\overload int64_t LWByteBuffer::MakeLittle(int64_t)
	*/
	static int64_t MakeLittle(int64_t Value);

	/*!
	\overload uint64_t LWByteBuffer::MakeLittle(uint64_t)
	*/
	static uint64_t MakeLittle(uint64_t Value);

	/*!
	\brief converts a float from host order to little endian order float.
	*/
	static float MakeLittle(float Value);

	/*!
	\brief converts a double from host order to little order double.
	\overload double LWByteBuffer::MakeLittle(double)
	*/
	static double MakeLittle(double Value);

	/*! \brief writes the value of the pointer itself into the buffer for later.
		Warning this function is only here for convenience, you should have a strong understanding of pointers before using!
		\param Value the pointer to be written.
		\param Buffer the buffer to be written to.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	static int32_t WritePointer(void *Value, int8_t *Buffer);

	/*! \brief writes the value into the buffer. Type must be a POD type to be written properly.
		\param Value the number to be written into Buffer
		\param Buffer the buffer which is to be written into.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	template<class Type>
	static int32_t Write(const Type Value, int8_t *Buffer) {
		if (Buffer) *(Type*)Buffer = Value;
		return sizeof(Type);
	}
	/*! \overload int32_t Write(const LWSQuaternion<Type> &, int8_t *) */
	template<class Type>
	static int32_t Write(const LWSQuaternion<Type> &Value, int8_t *Buffer) {
		return Write(Value.AsQuaternion(), Buffer);
	}


	/*! \overload int32_t Write(const LWQuaternion<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t Write(const LWQuaternion<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.x;
			*((Type*)Buffer + 1) = Value.y;
			*((Type*)Buffer + 2) = Value.z;
			*((Type*)Buffer + 3) = Value.w;
		}
		return sizeof(Type) * 4;
	}

	/*! \overload int32_t Write(const LWSVector4<Type> &, int8_t *) */
	template<class Type>
	static int32_t Write(const LWSVector4<Type> &Value, int8_t *Buffer) {
		return Write(Value.AsVec4(), Buffer);
	}

	/*!
		\overload int32_t Write(const LWVector4<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t Write(const LWVector4<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.x;
			*((Type*)Buffer + 1) = Value.y;
			*((Type*)Buffer + 2) = Value.z;
			*((Type*)Buffer + 3) = Value.w;
		}
		return sizeof(Type) * 4;
	}

	/*!
		\overload int32_t Write(const LWVector3<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t Write(const LWVector3<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.x;
			*((Type*)Buffer + 1) = Value.y;
			*((Type*)Buffer + 2) = Value.z;
		}
		return sizeof(Type) * 3;
	}

	/*!
		\overload int32_t Write(const LWVector2<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t Write(const LWVector2<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.x;
			*((Type*)Buffer + 1) = Value.y;
		}
		return sizeof(Type) * 2;
	}

	/*! \overload int32_t Write(const LWSMatrix4<Type> &, int8_t *) */
	template<class Type>
	static int32_t Write(const LWSMatrix4<Type> &Value, int8_t *Buffer) {
		return Write(Value.AsMat4(), Buffer);
	}

	/*! \overload int32_t Write(const LWMatrix4<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t Write(const LWMatrix4<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.m_Rows[0].x;
			*((Type*)Buffer + 1) = Value.m_Rows[0].y;
			*((Type*)Buffer + 2) = Value.m_Rows[0].z;
			*((Type*)Buffer + 3) = Value.m_Rows[0].w;
			*((Type*)Buffer + 4) = Value.m_Rows[1].x;
			*((Type*)Buffer + 5) = Value.m_Rows[1].y;
			*((Type*)Buffer + 6) = Value.m_Rows[1].z;
			*((Type*)Buffer + 7) = Value.m_Rows[1].w;
			*((Type*)Buffer + 8) = Value.m_Rows[2].x;
			*((Type*)Buffer + 9) = Value.m_Rows[2].y;
			*((Type*)Buffer + 10) = Value.m_Rows[2].z;
			*((Type*)Buffer + 11) = Value.m_Rows[2].w;
			*((Type*)Buffer + 12) = Value.m_Rows[3].x;
			*((Type*)Buffer + 13) = Value.m_Rows[3].y;
			*((Type*)Buffer + 14) = Value.m_Rows[3].z;
			*((Type*)Buffer + 15) = Value.m_Rows[3].w;
		}
		return sizeof(Type) * 16;
	}

	/*! \overload int32_t Write(const LWMatrix3<Type> &, int8_t*)
	*/
	template<class Type>
	static int32_t Write(const LWMatrix3<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.m_Rows[0].x;
			*((Type*)Buffer + 1) = Value.m_Rows[0].y;
			*((Type*)Buffer + 2) = Value.m_Rows[0].z;
			*((Type*)Buffer + 3) = Value.m_Rows[1].x;
			*((Type*)Buffer + 4) = Value.m_Rows[1].y;
			*((Type*)Buffer + 5) = Value.m_Rows[1].z;
			*((Type*)Buffer + 6) = Value.m_Rows[2].x;
			*((Type*)Buffer + 7) = Value.m_Rows[2].y;
			*((Type*)Buffer + 8) = Value.m_Rows[2].z;
		}
		return sizeof(Type) * 9;
	}

	/*! \overload int32_t Write(const LWMatrix2<Type> &, int8_t *) */
	template<class Type>
	static int32_t Write(const LWMatrix2<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = Value.m_Rows[0].x;
			*((Type*)Buffer + 1) = Value.m_Rows[0].y;
			*((Type*)Buffer + 2) = Value.m_Rows[1].x;
			*((Type*)Buffer + 3) = Value.m_Rows[1].y;
		}
		return sizeof(Type) * 4;
	}

	/*! \brief writes a utf-X encoded text to the buffer stream. */
	template<class Type>
	static int32_t WriteUTF(const LWUnicodeIterator<Type> &Iter, int8_t *Buffer) {
		const Type *P = Iter();
		const Type *L = Iter.GetLast();
		int32_t o = 0;
		uint32_t l = 0;
		for (; *P && P != L; ++P) {
			l = Write<Type>(*P, Buffer);
			if (Buffer) Buffer += l;
			o += l;
		}
		l = Write<Type>(0, Buffer);
		if (Buffer) Buffer += l;
		o += l;
		if(o&1) o += Write<char>(0, Buffer);
		return o;
	}

	/*!< \brief writes a null terminated string to buffer(including the null termination). */
	static int32_t WriteText(const uint8_t *Text, int8_t *Buffer);

	/*!< \overload int32_t WriteText(const char*Text, int8_t *Buffer); */
	static int32_t WriteText(const char *Text, int8_t *Buffer);

	/*! \brief writes a series of values of Type into the buffer. Type must be a POD type to be written properly.
		\param Len the number of values to be written into buffer.
		\param Values an array of values at least equal to Len
		\param Buffer the buffer which is to be written into.
		\return the number of bytes written into Buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const Type *Values, int8_t *Buffer) {
		if (Buffer)	for (uint32_t i = 0; i < Len; i++) *(((Type*)Buffer) + i) = Values[i];
		return sizeof(Type) * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWSQuaternion<Type> *, int8_t *) */
	template<class Type>
	static int32_t Write(uint32_t Len, const LWSQuaternion<Type> *Values, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				LWQuaternion<Type> V = Values[i].AsQuaternion();
				*((Type*)Buffer + i * 4 + 0) = V.x;
				*((Type*)Buffer + i * 4 + 1) = V.y;
				*((Type*)Buffer + i * 4 + 2) = V.z;
				*((Type*)Buffer + i * 4 + 3) = V.w;
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWQuaternion<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWQuaternion<Type> *Value, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				*((Type*)Buffer + i * 4 + 0) = Value[i].x;
				*((Type*)Buffer + i * 4 + 1) = Value[i].y;
				*((Type*)Buffer + i * 4 + 2) = Value[i].z;
				*((Type*)Buffer + i * 4 + 3) = Value[i].w;
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWSVector4<Type> *, int8_t *)*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWSVector4<Type> *Values, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				LWVector4<Type> V = Values[i].AsVec4();
				*((Type*)Buffer + i * 4 + 0) = V.x;
				*((Type*)Buffer + i * 4 + 1) = V.y;
				*((Type*)Buffer + i * 4 + 2) = V.z;
				*((Type*)Buffer + i * 4 + 3) = V.w;
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*!
		\overload int32_t Write(uint32_t, const LWVector4<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWVector4<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 4 + 0) = Values[i].x;
				*((Type*)Buffer + i * 4 + 1) = Values[i].y;
				*((Type*)Buffer + i * 4 + 2) = Values[i].z;
				*((Type*)Buffer + i * 4 + 3) = Values[i].w;
			}
		}
		return sizeof(Type)* 4 * Len;
	}

	/*!
		\overload int32_t Write(uint32_t, const LWVector3<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWVector3<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 3 + 0) = Values[i].x;
				*((Type*)Buffer + i * 3 + 1) = Values[i].y;
				*((Type*)Buffer + i * 3 + 2) = Values[i].z;
			}
		}
		return sizeof(Type)* 3*Len;
	}

	/*!
		\overload int32_t Write(uint32_t, const LWVector2<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWVector2<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 2 + 0) = Values[i].x;
				*((Type*)Buffer + i * 2 + 1) = Values[i].y;
			}
		}
		return sizeof(Type)* 2 * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWSMatrix4<Type> *, int8_t *) */
	template<class Type>
	static int32_t Write(uint32_t Len, const LWSMatrix4<Type> *Values, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				LWMatrix4<Type> V = Values[i].AsMat4();
				*((Type*)Buffer + i * 16 + 0) = V.m_Rows[0].x;
				*((Type*)Buffer + i * 16 + 1) = V.m_Rows[0].y;
				*((Type*)Buffer + i * 16 + 2) = V.m_Rows[0].z;
				*((Type*)Buffer + i * 16 + 3) = V.m_Rows[0].w;
				*((Type*)Buffer + i * 16 + 4) = V.m_Rows[1].x;
				*((Type*)Buffer + i * 16 + 5) = V.m_Rows[1].y;
				*((Type*)Buffer + i * 16 + 6) = V.m_Rows[1].z;
				*((Type*)Buffer + i * 16 + 7) = V.m_Rows[1].w;
				*((Type*)Buffer + i * 16 + 8) = V.m_Rows[2].x;
				*((Type*)Buffer + i * 16 + 9) = V.m_Rows[2].y;
				*((Type*)Buffer + i * 16 + 10) = V.m_Rows[2].z;
				*((Type*)Buffer + i * 16 + 11) = V.m_Rows[2].w;
				*((Type*)Buffer + i * 16 + 12) = V.m_Rows[3].x;
				*((Type*)Buffer + i * 16 + 13) = V.m_Rows[3].y;
				*((Type*)Buffer + i * 16 + 14) = V.m_Rows[3].z;
				*((Type*)Buffer + i * 16 + 15) = V.m_Rows[3].w;
			}
		}
		return sizeof(Type) * 16 * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWMatrix4<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWMatrix4<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 16 + 0) = Values[i].m_Rows[0].x;
				*((Type*)Buffer + i * 16 + 1) = Values[i].m_Rows[0].y;
				*((Type*)Buffer + i * 16 + 2) = Values[i].m_Rows[0].z;
				*((Type*)Buffer + i * 16 + 3) = Values[i].m_Rows[0].w;
				*((Type*)Buffer + i * 16 + 4) = Values[i].m_Rows[1].x;
				*((Type*)Buffer + i * 16 + 5) = Values[i].m_Rows[1].y;
				*((Type*)Buffer + i * 16 + 6) = Values[i].m_Rows[1].z;
				*((Type*)Buffer + i * 16 + 7) = Values[i].m_Rows[1].w;
				*((Type*)Buffer + i * 16 + 8) = Values[i].m_Rows[2].x;
				*((Type*)Buffer + i * 16 + 9) = Values[i].m_Rows[2].y;
				*((Type*)Buffer + i * 16 + 10) = Values[i].m_Rows[2].z;
				*((Type*)Buffer + i * 16 + 11) = Values[i].m_Rows[2].w;
				*((Type*)Buffer + i * 16 + 12) = Values[i].m_Rows[3].x;
				*((Type*)Buffer + i * 16 + 13) = Values[i].m_Rows[3].y;
				*((Type*)Buffer + i * 16 + 14) = Values[i].m_Rows[3].z;
				*((Type*)Buffer + i * 16 + 15) = Values[i].m_Rows[3].w;
			}
		}
		return sizeof(Type)* 16 * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWMatrix3<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWMatrix3<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 9 + 0) = Values[i].m_Rows[0].x;
				*((Type*)Buffer + i * 9 + 1) = Values[i].m_Rows[0].y;
				*((Type*)Buffer + i * 9 + 2) = Values[i].m_Rows[0].z;
				*((Type*)Buffer + i * 9 + 3) = Values[i].m_Rows[1].x;
				*((Type*)Buffer + i * 9 + 4) = Values[i].m_Rows[1].y;
				*((Type*)Buffer + i * 9 + 5) = Values[i].m_Rows[1].z;
				*((Type*)Buffer + i * 9 + 6) = Values[i].m_Rows[2].x;
				*((Type*)Buffer + i * 9 + 7) = Values[i].m_Rows[2].y;
				*((Type*)Buffer + i * 9 + 8) = Values[i].m_Rows[2].z;
			}
		}
		return sizeof(Type)* 9 * Len;
	}

	/*! \overload int32_t Write(uint32_t, const LWMatrix2<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t Write(uint32_t Len, const LWMatrix2<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 4 + 0) = Values[i].m_Rows[0].x;
				*((Type*)Buffer + i * 4 + 1) = Values[i].m_Rows[0].y;
				*((Type*)Buffer + i * 4 + 2) = Values[i].m_Rows[1].x;
				*((Type*)Buffer + i * 4 + 3) = Values[i].m_Rows[1].y;
			}
		}
		return sizeof(Type)* 4 * Len;
	}
	
	/*! \brief writes an variable list of int32 values into buffer. Type must be a POD type to be written properly.
		\param Buffer the buffer to write to.
		\param Len the number of elements expected from lst.
		\param lst a variable argument list of elements.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	template<class Type>
	static int32_t Write(int8_t *Buffer, uint32_t Len, va_list lst){
		if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((Type*)Buffer) + i) = va_arg(lst, Type);
		return sizeof(Type)*Len;
	}

	/*! \brief writes an variable list of values into buffer. Type must be a POD type to be written properly.
		\param Buffer the buffer to write to.
		\param Len the number of variable elements expected.
		\param ... the arguments expected to be at least Len in number.
		\return the number of bytes written into buffer
	*/
	template<class Type>
	static int32_t Write(int8_t *Buffer, uint32_t Len, ...){
		va_list lst;
		va_start(lst, Len);
		int32_t Result = Write<Type>(Buffer, Len, lst);
		va_end(lst);
		return Result;
	}

	/*! \brief writes an host encoded value into Buffer in network order.
		\param Value the host ordered value to be written.
		\param Buffer the buffer to write to.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	template<class Type>
	static int32_t WriteNetwork(const Type Value, int8_t *Buffer){
		if (Buffer) *(Type*)Buffer = MakeNetwork(Value);
		return sizeof(Type);
	}

	/*! \overload int32_t WriteNetwork(const LWSQuaternion<Type> &, int8_t *) */
	template<class Type>
	static int32_t WriteNetwork(const LWSQuaternion<Type> &Value, int8_t *Buffer) {
		return WriteNetwork(Value.AsSQuaternion(), Buffer);
	}

	/*!
	\overload int32_t WriteNetwork(const LWQuaternion<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWQuaternion<Type> &Value, int8_t *Buffer) {
		if (Buffer) {
			*((Type*)Buffer + 0) = MakeNetwork(Value.x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.y);
			*((Type*)Buffer + 2) = MakeNetwork(Value.z);
			*((Type*)Buffer + 3) = MakeNetwork(Value.w);
		}
		return sizeof(Type) * 4;
	}

	/*! \overload int32_t WriteNetwork(const LWSVector4<Type> &, int8_t *) */
	template<class Type>
	static int32_t WriteNetwork(const LWSVector4<Type> &Value, int8_t *Buffer) {
		return WriteNetwork(Value.AsVec4(), Buffer);
	}

	/*!
		\overload int32_t WriteNetwork(const LWVector4<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWVector4<Type> &Value, int8_t *Buffer){
		if (Buffer){
			*((Type*)Buffer + 0) = MakeNetwork(Value.x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.y);
			*((Type*)Buffer + 2) = MakeNetwork(Value.z);
			*((Type*)Buffer + 3) = MakeNetwork(Value.w);
		}
		return sizeof(Type)* 4;
	}

	/*!
		\overload int32_t WriteNetwork(const LWVector3<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWVector3<Type> &Value, int8_t *Buffer){
		if (Buffer){
			*((Type*)Buffer + 0) = MakeNetwork(Value.x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.y);
			*((Type*)Buffer + 2) = MakeNetwork(Value.z);
		}
		return sizeof(Type)* 3;
	}

	/*!
		\overload int32_t WriteNetwork(const LWVector2<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWVector2<Type> &Value, int8_t *Buffer){
		if (Buffer){
			*((Type*)Buffer + 0) = MakeNetwork(Value.x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.y);
		}
		return sizeof(Type)* 2;
	}

	/*! \overload int32_t WriteNetwork(const LWSMatrix4<Type> &, int8_t *) */
	template<class Type>
	static int32_t WriteNetwork(const LWSMatrix4<Type> &Value, int8_t *Buffer) {
		return WriteNetwork(Value.AsMat4(), Buffer);
	}

	/*!
		\overload int32_t WriteNetwork(const LWMatrix4<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWMatrix4<Type> &Value, int8_t *Buffer){
		if (Buffer){
			*((Type*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
			*((Type*)Buffer + 2) = MakeNetwork(Value.m_Rows[0].z);
			*((Type*)Buffer + 3) = MakeNetwork(Value.m_Rows[0].w);
			*((Type*)Buffer + 4) = MakeNetwork(Value.m_Rows[1].x);
			*((Type*)Buffer + 5) = MakeNetwork(Value.m_Rows[1].y);
			*((Type*)Buffer + 6) = MakeNetwork(Value.m_Rows[1].z);
			*((Type*)Buffer + 7) = MakeNetwork(Value.m_Rows[1].w);
			*((Type*)Buffer + 8) = MakeNetwork(Value.m_Rows[2].x);
			*((Type*)Buffer + 9) = MakeNetwork(Value.m_Rows[2].y);
			*((Type*)Buffer + 10) = MakeNetwork(Value.m_Rows[2].z);
			*((Type*)Buffer + 11) = MakeNetwork(Value.m_Rows[2].w);
			*((Type*)Buffer + 12) = MakeNetwork(Value.m_Rows[3].x);
			*((Type*)Buffer + 13) = MakeNetwork(Value.m_Rows[3].y);
			*((Type*)Buffer + 14) = MakeNetwork(Value.m_Rows[3].z);
			*((Type*)Buffer + 15) = MakeNetwork(Value.m_Rows[3].w);
		}
		return sizeof(Type)* 16;
	}

	/*! \overload int32_t WriteNetwork(const LWMatrix3<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWMatrix3<Type> &Value, int8_t *Buffer){
		if (Buffer){
			*((Type*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
			*((Type*)Buffer + 2) = MakeNetwork(Value.m_Rows[0].z);
			*((Type*)Buffer + 3) = MakeNetwork(Value.m_Rows[1].x);
			*((Type*)Buffer + 4) = MakeNetwork(Value.m_Rows[1].y);
			*((Type*)Buffer + 5) = MakeNetwork(Value.m_Rows[1].z);
			*((Type*)Buffer + 6) = MakeNetwork(Value.m_Rows[2].x);
			*((Type*)Buffer + 7) = MakeNetwork(Value.m_Rows[2].y);
			*((Type*)Buffer + 8) = MakeNetwork(Value.m_Rows[2].z);
		}
		return sizeof(Type)* 9;
	}

	/*! \overload int32_t WriteNetwork(const LWMatrix2<Type> &, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(const LWMatrix2<Type> &Value, int8_t *Buffer){
		if (Buffer){
			*((Type*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
			*((Type*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
			*((Type*)Buffer + 2) = MakeNetwork(Value.m_Rows[1].x);
			*((Type*)Buffer + 3) = MakeNetwork(Value.m_Rows[1].y);
		}
		return sizeof(Type)* 4;
	}

	/*! \brief writes a network encoded utfX string. */
	template<class Type>
	static int32_t WriteNetworkUTF(const LWUnicodeIterator<Type> &Iter, int8_t *Buffer) {
		const Type *P = Iter();
		const Type *L = Iter.GetLast();
		uint32_t l = 0;
		int32_t o = 0;
		for (; *P && P != L; ++P) {
			l = WriteNetwork<Type>(*P, Buffer);
			if (Buffer) Buffer += l;
			o += l;
		}
		l = WriteNetwork<Type>(0, Buffer);
		o += l;
		if (o & 1) {
			if (Buffer) Buffer += l;
			o += WriteNetwork<char>(0, Buffer);
		}
		return o;
	}

	/** \brief writes an host encoded number of values into buffer in network order.
		\param Len the number of values expected to be written.
		\param Values an array of values in host order expected to be at least Len in length.
		\param Buffer the buffer to write to.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const Type *Values, int8_t *Buffer){
		if (Buffer)	for (uint32_t i = 0; i < Len; i++) *(((Type*)Buffer) + i) = MakeNetwork(Values[i]);
		return sizeof(Type)*Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWSQuaternion<Type> *, int8_t *) */
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWSQuaternion<Type> *Value, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				LWQuaternion<Type> V = Value[i].AsQuaternion();
				*((Type*)Buffer + i * 4 + 0) = MakeNetwork(V.x);
				*((Type*)Buffer + i * 4 + 1) = MakeNetwork(V.y);
				*((Type*)Buffer + i * 4 + 2) = MakeNetwork(V.z);
				*((Type*)Buffer + i * 4 + 3) = MakeNetwork(V.w);
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWQuaternion<Type> *, int8_t *)
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWQuaternion<Type> *Value, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				*((Type*)Buffer + i * 4 + 0) = MakeNetwork(Value[i].x);
				*((Type*)Buffer + i * 4 + 1) = MakeNetwork(Value[i].y);
				*((Type*)Buffer + i * 4 + 2) = MakeNetwork(Value[i].z);
				*((Type*)Buffer + i * 4 + 3) = MakeNetwork(Value[i].w);
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWSVector4<Type> *, int8_t*) */
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWSVector4<Type> *Values, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				LWVector4<Type> V = Values[i].AsVec4();
				*((Type*)Buffer + i * 4 + 0) = MakeNetwork(V.x);
				*((Type*)Buffer + i * 4 + 1) = MakeNetwork(V.y);
				*((Type*)Buffer + i * 4 + 2) = MakeNetwork(V.z);
				*((Type*)Buffer + i * 4 + 3) = MakeNetwork(V.w);
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*!
		\overload int32_t WriteNetwork(uint32_t, const LWVector4<Type> *, int8_t*)
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWVector4<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].x);
				*((Type*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].y);
				*((Type*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].z);
				*((Type*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].w);
			}
		}
		return sizeof(Type)* 4 * Len;
	}

	/*!
		\overload int32_t WriteNetwork(uint32_t, const LWVector3<Type> *, int8_t*)
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWVector3<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 3 + 0) = MakeNetwork(Values[i].x);
				*((Type*)Buffer + i * 3 + 1) = MakeNetwork(Values[i].y);
				*((Type*)Buffer + i * 3 + 2) = MakeNetwork(Values[i].z);
			}
		}
		return sizeof(Type)* 3 * Len;
	}

	/*!
		\overload int32_t WriteNetwork(uint32_t, const LWVector2<Type> *, int8_t*)
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWVector2<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 2 + 0) = MakeNetwork(Values[i].x);
				*((Type*)Buffer + i * 2 + 1) = MakeNetwork(Values[i].y);
			}
		}
		return sizeof(Type)* 2 * Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWSMatrix4<Type> *, int8_t*) */
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWSMatrix4<Type> *Values, int8_t *Buffer) {
		if (Buffer) {
			for (uint32_t i = 0; i < Len; i++) {
				LWSMatrix4<Type> V = Values[i].AsMat4();
				*((Type*)Buffer + i * 16 + 0) = MakeNetwork(V.m_Rows[0].x);
				*((Type*)Buffer + i * 16 + 1) = MakeNetwork(V.m_Rows[0].y);
				*((Type*)Buffer + i * 16 + 2) = MakeNetwork(V.m_Rows[0].z);
				*((Type*)Buffer + i * 16 + 3) = MakeNetwork(V.m_Rows[0].w);
				*((Type*)Buffer + i * 16 + 4) = MakeNetwork(V.m_Rows[1].x);
				*((Type*)Buffer + i * 16 + 5) = MakeNetwork(V.m_Rows[1].y);
				*((Type*)Buffer + i * 16 + 6) = MakeNetwork(V.m_Rows[1].z);
				*((Type*)Buffer + i * 16 + 7) = MakeNetwork(V.m_Rows[1].w);
				*((Type*)Buffer + i * 16 + 8) = MakeNetwork(V.m_Rows[2].x);
				*((Type*)Buffer + i * 16 + 9) = MakeNetwork(V.m_Rows[2].y);
				*((Type*)Buffer + i * 16 + 10) = MakeNetwork(V.m_Rows[2].z);
				*((Type*)Buffer + i * 16 + 11) = MakeNetwork(V.m_Rows[2].w);
				*((Type*)Buffer + i * 16 + 12) = MakeNetwork(V.m_Rows[3].x);
				*((Type*)Buffer + i * 16 + 13) = MakeNetwork(V.m_Rows[3].y);
				*((Type*)Buffer + i * 16 + 14) = MakeNetwork(V.m_Rows[3].z);
				*((Type*)Buffer + i * 16 + 15) = MakeNetwork(V.m_Rows[3].w);
			}
		}
		return sizeof(Type) * 16 * Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWMatrix4<Type> *, int8_t*)
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWMatrix4<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 16 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
				*((Type*)Buffer + i * 16 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
				*((Type*)Buffer + i * 16 + 2) = MakeNetwork(Values[i].m_Rows[0].z);
				*((Type*)Buffer + i * 16 + 3) = MakeNetwork(Values[i].m_Rows[0].w);
				*((Type*)Buffer + i * 16 + 4) = MakeNetwork(Values[i].m_Rows[1].x);
				*((Type*)Buffer + i * 16 + 5) = MakeNetwork(Values[i].m_Rows[1].y);
				*((Type*)Buffer + i * 16 + 6) = MakeNetwork(Values[i].m_Rows[1].z);
				*((Type*)Buffer + i * 16 + 7) = MakeNetwork(Values[i].m_Rows[1].w);
				*((Type*)Buffer + i * 16 + 8) = MakeNetwork(Values[i].m_Rows[2].x);
				*((Type*)Buffer + i * 16 + 9) = MakeNetwork(Values[i].m_Rows[2].y);
				*((Type*)Buffer + i * 16 + 10) = MakeNetwork(Values[i].m_Rows[2].z);
				*((Type*)Buffer + i * 16 + 11) = MakeNetwork(Values[i].m_Rows[2].w);
				*((Type*)Buffer + i * 16 + 12) = MakeNetwork(Values[i].m_Rows[3].x);
				*((Type*)Buffer + i * 16 + 13) = MakeNetwork(Values[i].m_Rows[3].y);
				*((Type*)Buffer + i * 16 + 14) = MakeNetwork(Values[i].m_Rows[3].z);
				*((Type*)Buffer + i * 16 + 15) = MakeNetwork(Values[i].m_Rows[3].w);
			}
		}
		return sizeof(Type)* 16 * Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWMatrix3<Type> *, int8_t*)
	*/
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWMatrix3<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 9 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
				*((Type*)Buffer + i * 9 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
				*((Type*)Buffer + i * 9 + 2) = MakeNetwork(Values[i].m_Rows[0].z);
				*((Type*)Buffer + i * 9 + 3) = MakeNetwork(Values[i].m_Rows[1].x);
				*((Type*)Buffer + i * 9 + 4) = MakeNetwork(Values[i].m_Rows[1].y);
				*((Type*)Buffer + i * 9 + 5) = MakeNetwork(Values[i].m_Rows[1].z);
				*((Type*)Buffer + i * 9 + 6) = MakeNetwork(Values[i].m_Rows[2].x);
				*((Type*)Buffer + i * 9 + 7) = MakeNetwork(Values[i].m_Rows[2].y);
				*((Type*)Buffer + i * 9 + 8) = MakeNetwork(Values[i].m_Rows[2].z);
			}
		}
		return sizeof(Type)* 9 * Len;
	}

	/*! \overload int32_t WriteNetwork(uint32_t, const LWMatrix2<Type> *, int8_t *) */
	template<class Type>
	static int32_t WriteNetwork(uint32_t Len, const LWMatrix2<Type> *Values, int8_t *Buffer){
		if (Buffer){
			for (uint32_t i = 0; i < Len; i++){
				*((Type*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
				*((Type*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
				*((Type*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].m_Rows[1].x);
				*((Type*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].m_Rows[1].y);
			}
		}
		return sizeof(Type)* 4 * Len;
	}



	/** \brief writes an host encoded variable number of values into buffer in network order.
		\param Buffer the buffer to write to.
		\param Len the number of arguments to expect.
		\param lst the variable argument object expected.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	template<class Type>
	static int32_t WriteNetwork(int8_t *Buffer, uint32_t Len, va_list lst){
		if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((Type*)Buffer) + i) = MakeNetwork(va_arg(lst, Type));
		return sizeof(Type)*Len;
	}

	/** \brief writes an host encoded variable number of values into buffer in network order.
		\param Buffer the buffer to write to.
		\param Len the number of arguments to expect.
		\param ... the list of values to expect.
		\return the number of bytes written into buffer.
		\note Buffer can be null, in which case the total number of bytes that would have been written is returned.
	*/
	
	template<class Type>
	static int32_t WriteNetworkL(int8_t *Buffer, uint32_t Len, ...){
		va_list lst;
		va_start(lst, Len);
		int32_t Result = WriteNetwork<Type>(Buffer, Len, lst);
		va_end(lst);
		return Result;
	}

	/*! \brief Reads back a pointer from the buffer.
		\param Out the value to write out to.
		\param Buffer the buffer to read from.
		\return the number of bytes read. 
		\note Out can be null, in which case nothing is actually read. but the number of bytes is still reported.
	*/
	static int32_t ReadPointer(void **Out, int8_t *Buffer);

	/*! \brief reads back type from the buffer.
		\param Out the value to write out to.
		\param Buffer the buffer to read from.
		\return the number of bytes read.
		\note Out can be null, in which case nothing is actually read. but the number of bytes is still reported.
	*/
	template<class Type>
	static int32_t Read(Type *Out, const int8_t *Buffer){
		if (Out) *Out = *(Type*)Buffer;
		return sizeof(Type);
	}

	/*! \overload int32_t Read(LWSQuaternion<Type> *Out, const int8_t *Buffer) */

	template<class Type>
	static int32_t Read(LWSQuaternion<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			Type v[4];
			v[0] = *((Type*)Buffer + 0);
			v[1] = *((Type*)Buffer + 1);
			v[2] = *((Type*)Buffer + 2);
			v[3] = *((Type*)Buffer + 3);
			*Out = LWSQuaternion<Type>(v[3], v[0], v[1], v[2]);
		}
		return sizeof(Type) * 4;
	}

	/*! \overload int32_t Read(LWSVector4<Type> *Out, const int8_t *Buffer) */
	template<class Type>
	static int32_t Read(LWSVector4<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			Type v[4];
			v[0] = *((Type*)Buffer + 0);
			v[1] = *((Type*)Buffer + 1);
			v[2] = *((Type*)Buffer + 2);
			v[3] = *((Type*)Buffer + 3);
			*Out = LWSVector4<Type>(v[0], v[1], v[2], v[3]);
		}
		return sizeof(Type) * 4;
	}

	/*! \overload int32_t Read(LWSMatrix4<Type> *Out, const int8_t *Buffer) */
	template<class Type>
	static int32_t Read(LWSMatrix4<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			Type v[16];
			v[0] = *((Type*)Buffer + 0);
			v[1] = *((Type*)Buffer + 1);
			v[2] = *((Type*)Buffer + 2);
			v[3] = *((Type*)Buffer + 3);
			v[4] = *((Type*)Buffer + 4);
			v[5] = *((Type*)Buffer + 5);
			v[6] = *((Type*)Buffer + 6);
			v[7] = *((Type*)Buffer + 7);
			v[8] = *((Type*)Buffer + 8);
			v[9] = *((Type*)Buffer + 9);
			v[10] = *((Type*)Buffer + 10);
			v[11] = *((Type*)Buffer + 11);
			v[12] = *((Type*)Buffer + 12);
			v[13] = *((Type*)Buffer + 13);
			v[14] = *((Type*)Buffer + 14);
			v[15] = *((Type*)Buffer + 15);
			*Out = LWSMatrix4<Type>(LWSVector4<Type>(v[0], v[1], v[2], v[3]), LWSVector4<Type>(v[4], v[5], v[6], v[7]), LWSVector4<Type>(v[8], v[9], v[10], v[11]), LWSVector4<Type>(v[12], v[13], v[14], v[15]));
		}
		return sizeof(Type) * 16;
	}

	/*! \brief reads back an utf-X string from the buffer, and stores it into Out.
		\param Out the buffer to receive the utf-X text.
		\param OutLen the length of the out buffer(in Type) for storage.
		\param Buffer the buffer to read back from.
		\param BufferLen the length of the buffer, so that a buffer overflow on reading can't occur.
		\return the raw byte length of the utfX string(plus the null character)+possible padding byte.
		\note to calculate the length Out needs to be to store the entire string, simply allocate an array of length return/sizeof(Type).
	*/
	template<class Type>
	static int32_t ReadUTF(Type *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen) {
		int8_t o = 0;
		Type *oP = Out;
		Type *oL = oP + std::min<uint32_t>(OutLen - 1, OutLen);
		const Type *bP = (const Type*)Buffer;
		const Type *bL = bP + BufferLen;
		for (; bP != bL && *bP; ++bP) {
			if (oP < oL) *oP++ = *bP;
			o += sizeof(Type);
		}
		if (OutLen) *oP = 0;
		o += sizeof(Type);
		if (o & 1) o += 1; //skip padding if it was added.
		return o;
	}

	/*!< \brief reads back an null terminated string from the buffer, and stores it into Out.
		 \param Out the buffer to receive the null terminated text.
		 \param OutLen the length of the out buffer for storage.
		 \param Buffer the buffer to read back from.
		 \param BufferLen the length of the buffer to be read from to prevent buffer overflows.
		 \return the raw length of the text, including the null terminated character.
	*/
	static int32_t ReadText(uint8_t *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen);

	/*!< \overload int32_t ReadText(const char*, uint32_t, const int8_t *, const uint32_t) */
	static int32_t ReadText(char *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen);

	/*! \brief reads back Len number of type from the buffer.
		\param Out the array to receive the values to be written to.
		\param Len the number of elements expected.
		\param Buffer the buffer to read from.
		\return the number of bytes read.
		\note Out can be null, in which case nothing is actually read. but the number of bytes is still reported.
	*/
	template<class Type>
	static int32_t Read(Type *Out, uint32_t Len, const int8_t *Buffer){
		if (Out) for (uint32_t i = 0; i < Len; i++) Out[i] = *((Type*)Buffer + i);
		return sizeof(Type)*Len;
	}

	/*! \overload int32_t Read(LWSQuaternion<Type> *Out, uint32_t Len, const int8_t *Buffer) */
	template<class Type>
	static int32_t Read(LWSQuaternion<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Type v[4];
				v[0] = *((Type*)Buffer + i * 4 + 0);
				v[1] = *((Type*)Buffer + i * 4 + 1);
				v[2] = *((Type*)Buffer + i * 4 + 2);
				v[3] = *((Type*)Buffer + i * 4 + 3);
				Out[i] = LWSQuaternion<Type>(v[3], v[0], v[1], v[2]);
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t Read(LWSVector4<Type> *Out, uint32_t Len, const int8_t *Buffer) */
	template<class Type>
	static int32_t Read(LWSVector4<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Type v[4];
				v[0] = *((Type*)Buffer + i * 4 + 0);
				v[1] = *((Type*)Buffer + i * 4 + 1);
				v[2] = *((Type*)Buffer + i * 4 + 2);
				v[3] = *((Type*)Buffer + i * 4 + 3);
				Out[i] = LWSVector4<Type>(v[0], v[1], v[2], v[3]);
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t Read(LWSMatrix4<Type> *Out, uint32_t Len, const int8_t *Buffer) */
	template<class Type>
	static int32_t Read(LWSMatrix4<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Type v[16];
				v[0] = *((Type*)Buffer + i * 16 + 0);
				v[1] = *((Type*)Buffer + i * 16 + 1);
				v[2] = *((Type*)Buffer + i * 16 + 2);
				v[3] = *((Type*)Buffer + i * 16 + 3);
				v[4] = *((Type*)Buffer + i * 16 + 4);
				v[5] = *((Type*)Buffer + i * 16 + 5);
				v[6] = *((Type*)Buffer + i * 16 + 6);
				v[7] = *((Type*)Buffer + i * 16 + 7);
				v[8] = *((Type*)Buffer + i * 16 + 8);
				v[9] = *((Type*)Buffer + i * 16 + 9);
				v[10] = *((Type*)Buffer + i * 16 + 10);
				v[11] = *((Type*)Buffer + i * 16 + 11);
				v[12] = *((Type*)Buffer + i * 16 + 12);
				v[13] = *((Type*)Buffer + i * 16 + 13);
				v[14] = *((Type*)Buffer + i * 16 + 14);
				v[15] = *((Type*)Buffer + i * 16 + 15);
				Out[i] = LWSMatrix4<Type>(LWSVector4<Type>(v[0], v[1], v[2], v[3]), LWSVector4<Type>(v[4], v[5], v[6], v[7]), LWSVector4<Type>(v[8], v[9], v[10], v[11]), LWSVector4<Type>(v[12], v[13], v[14], v[15]));
			}
		}
		return sizeof(Type) * 16 * Len;
	}

	/*! \brief reads back a network encoded value from buffer and transforms it to host form.
		\param Out the value to write to.
		\param Buffer the buffer to read from.
		\return the number of bytes read.
		\note Out can be null, in which case nothing is actually read. but the number of bytes is still reported.
	*/
	template<class Type>
	static int32_t ReadNetwork(Type *Out, const int8_t *Buffer){
		if (Out) *Out = MakeHost(*((Type*)Buffer));
		return sizeof(Type);
	}

	/*! \overload int32_t ReadNetwork(LWSQuaternion<Type> *, const int8_t *) */
	template<class Type>
	static int32_t ReadNetwork(LWSQuaternion<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			float v[3];
			v[0] = MakeHost(*((Type*)Buffer + 0));
			v[1] = MakeHost(*((Type*)Buffer + 1));
			v[2] = MakeHost(*((Type*)Buffer + 2));
			v[3] = MakeHost(*((Type*)Buffer + 3));
			*Out = LWSQuaternion<Type>(v[3], v[0], v[1], v[2]);
		}
		return sizeof(Type) * 4;
	}

	/*
		\overload int32_t ReadNetwork(LWQuaternion<Type> *, const int8_t *)
	*/

	template<class Type>
	static int32_t ReadNetwork(LWQuaternion<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			Out->x = MakeHost(*((Type*)Buffer + 0));
			Out->y = MakeHost(*((Type*)Buffer + 1));
			Out->z = MakeHost(*((Type*)Buffer + 2));
			Out->w = MakeHost(*((Type*)Buffer + 3));
		}
		return sizeof(Type) * 4;
	}

	/*! \overload int32_t ReadNetwork(LWSVector4<Type> *, const int8_t *) */
	template<class Type>
	static int32_t ReadNetwork(LWSVector4<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			Type v[4];
			v[0] = MakeHost(*((Type*)Buffer + 0));
			v[1] = MakeHost(*((Type*)Buffer + 1));
			v[2] = MakeHost(*((Type*)Buffer + 2));
			v[3] = MakeHost(*((Type*)Buffer + 3));
		}
		return sizeof(Type) * 4;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector4<Type> *, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWVector4<Type> *Out, const int8_t *Buffer){
		if (Out){
			Out->x = MakeHost(*((Type*)Buffer + 0));
			Out->y = MakeHost(*((Type*)Buffer + 1));
			Out->z = MakeHost(*((Type*)Buffer + 2));
			Out->w = MakeHost(*((Type*)Buffer + 3));
		}
		return sizeof(Type)* 4;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector3<Type> *, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWVector3<Type> *Out, const int8_t *Buffer){
		if (Out){
			Out->x = MakeHost(*((Type*)Buffer + 0));
			Out->y = MakeHost(*((Type*)Buffer + 1));
			Out->z = MakeHost(*((Type*)Buffer + 2));
		}
		return sizeof(Type)* 3;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector2<Type> *, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWVector2<Type> *Out, const int8_t *Buffer){
		if (Out){
			Out->x = MakeHost(*((Type*)Buffer + 0));
			Out->y = MakeHost(*((Type*)Buffer + 1));
		}
		return sizeof(Type)* 2;
	}

	/*! \overload int32_t ReadNetwork(LWSMatrix4<Type> *, const int8_t *) */
	template<class Type>
	static int32_t ReadNetwork(LWSMatrix4<Type> *Out, const int8_t *Buffer) {
		if (Out) {
			Type v[16];
			v[0] = MakeHost(*((Type*)Buffer + 0));
			v[1] = MakeHost(*((Type*)Buffer + 1));
			v[2] = MakeHost(*((Type*)Buffer + 2));
			v[3] = MakeHost(*((Type*)Buffer + 3));
			v[4] = MakeHost(*((Type*)Buffer + 4));
			v[5] = MakeHost(*((Type*)Buffer + 5));
			v[6] = MakeHost(*((Type*)Buffer + 6));
			v[7] = MakeHost(*((Type*)Buffer + 7));
			v[8] = MakeHost(*((Type*)Buffer + 8));
			v[9] = MakeHost(*((Type*)Buffer + 9));
			v[10] = MakeHost(*((Type*)Buffer + 10));
			v[11] = MakeHost(*((Type*)Buffer + 11));
			v[12] = MakeHost(*((Type*)Buffer + 12));
			v[13] = MakeHost(*((Type*)Buffer + 13));
			v[14] = MakeHost(*((Type*)Buffer + 14));
			v[15] = MakeHost(*((Type*)Buffer + 15));
			*Out = LWSMatrix4<Type>(LWSVector4<Type>(v[0], v[1], v[2], v[3]), LWSVector4<Type>(v[4], v[5], v[6], v[7]), LWSVector4<Type>(v[8], v[9], v[10], v[11]), LWSVector4<Type>(v[12], v[13], v[14], v[15]));
		}
		return sizeof(Type) * 16;
	}

	/*! \overload int32_t ReadNetwork(LWMatrix4<Type> *, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWMatrix4<Type> *Out, const int8_t *Buffer){
		if (Out){
			Out->m_Rows[0].x = MakeHost(*((Type*)Buffer + 0));
			Out->m_Rows[0].y = MakeHost(*((Type*)Buffer + 1));
			Out->m_Rows[0].z = MakeHost(*((Type*)Buffer + 2));
			Out->m_Rows[0].w = MakeHost(*((Type*)Buffer + 3));
			Out->m_Rows[1].x = MakeHost(*((Type*)Buffer + 4));
			Out->m_Rows[1].y = MakeHost(*((Type*)Buffer + 5));
			Out->m_Rows[1].z = MakeHost(*((Type*)Buffer + 6));
			Out->m_Rows[1].w = MakeHost(*((Type*)Buffer + 7));
			Out->m_Rows[2].x = MakeHost(*((Type*)Buffer + 8));
			Out->m_Rows[2].y = MakeHost(*((Type*)Buffer + 9));
			Out->m_Rows[2].z = MakeHost(*((Type*)Buffer + 10));
			Out->m_Rows[2].w = MakeHost(*((Type*)Buffer + 11));
			Out->m_Rows[3].x = MakeHost(*((Type*)Buffer + 12));
			Out->m_Rows[3].y = MakeHost(*((Type*)Buffer + 13));
			Out->m_Rows[3].z = MakeHost(*((Type*)Buffer + 14));
			Out->m_Rows[4].w = MakeHost(*((Type*)Buffer + 15));
		}
		return sizeof(Type)* 16;
	}

	/*! \overload int32_t ReadNetwork(LWMatrix3<Type> *, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWMatrix3<Type> *Out, const int8_t *Buffer){
		if (Out){
			Out->m_Rows[0].x = MakeHost(*((Type*)Buffer + 0));
			Out->m_Rows[0].y = MakeHost(*((Type*)Buffer + 1));
			Out->m_Rows[0].z = MakeHost(*((Type*)Buffer + 2));
			Out->m_Rows[1].x = MakeHost(*((Type*)Buffer + 3));
			Out->m_Rows[1].y = MakeHost(*((Type*)Buffer + 4));
			Out->m_Rows[1].z = MakeHost(*((Type*)Buffer + 5));
			Out->m_Rows[2].x = MakeHost(*((Type*)Buffer + 6));
			Out->m_Rows[2].y = MakeHost(*((Type*)Buffer + 7));
			Out->m_Rows[2].z = MakeHost(*((Type*)Buffer + 8));
		}
		return sizeof(Type)* 9;
	}

	/*! \overload int32_t ReadNetwork(LWMatrix2<Type> *, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWMatrix2<Type> *Out, const int8_t *Buffer){
		if (Out){
			Out->m_Rows[0].x = MakeHost(*((Type*)Buffer + 0));
			Out->m_Rows[0].y = MakeHost(*((Type*)Buffer + 1));
			Out->m_Rows[1].x = MakeHost(*((Type*)Buffer + 2));
			Out->m_Rows[1].y = MakeHost(*((Type*)Buffer + 3));
		}
		return sizeof(Type)* 4;
	}

	/*! \brief reads back an network utfX string from the buffer, and stores it into Out.
		\param Out the buffer to receive the utfX text.
		\param OutLen the length of the out buffer for storage.
		\param Buffer the buffer to read back from.
		\param BufferLen the length of the buffer, to prevent reading past the buffer.
		\return the raw length of the utfX string(plus the null character)+possible padding byte.
		\note to calculate the length Out needs to be to store the entire string, simply allocate an array of length return.
	*/
	template<class Type>
	static int32_t ReadNetworkUTF(Type *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen) {
		int8_t o = 0;
		Type *oP = Out;
		Type *oL = oP + std::min<uint32_t>(OutLen - 1, OutLen);
		const Type *bP = (const Type*)Buffer;
		const Type *bL = bP + BufferLen;
		for (; bP != bL && *bP; ++bP) {
			if (oP < oL) *oP++ = *bP;
			o += sizeof(Type);
		}
		if (OutLen) *oP = 0;
		o += sizeof(Type);
		if (o & 1) o += 1; //skip padding if it was added.
		return o;
	}

	/*! \brief reads back an array of network encoded value from buffer and transforms it to host form.
		\param Out the values to write to.
		\param Len the number of elements to read.
		\param Buffer the buffer to read from.
		\return the number of bytes read.
		\note Out can be null, in which case nothing is actually read.
	*/
	template<class Type>
	static int32_t ReadNetwork(Type *Out, uint32_t Len, const int8_t *Buffer){
		if (Out) for (uint32_t i = 0; i < Len; i++) Out[i] = MakeHost(*(((Type*)Buffer) + i));
		return sizeof(Type)*Len;
	}

	/*! \overload int32_t ReadNetwork(LWSVector4<Type>*, uint32_t, const int8_t *) */
	template<class Type>
	static int32_t ReadNetwork(LWSQuaternion<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Type v[4];
				v[0] = MakeHost(*((Type*)Buffer + i * 4 + 0));
				v[1] = MakeHost(*((Type*)Buffer + i * 4 + 1));
				v[2] = MakeHost(*((Type*)Buffer + i * 4 + 2));
				v[3] = MakeHost(*((Type*)Buffer + i * 4 + 3));
				Out[i] = LWSQuaternion<Type>(v[3], v[0], v[1], v[2]);
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector4<Type>*, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWQuaternion<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Out[i].x = MakeHost(*((Type*)Buffer + i * 4 + 0));
				Out[i].y = MakeHost(*((Type*)Buffer + i * 4 + 1));
				Out[i].z = MakeHost(*((Type*)Buffer + i * 4 + 2));
				Out[i].w = MakeHost(*((Type*)Buffer + i * 4 + 3));
			}
		}
		return sizeof(Type) * 4 * Len;
	}

	/*! \overload int32_t ReadNetwork(LWSVector4<Type>*, uint32_t, const int8_t *) */
	template<class Type>
	static int32_t ReadNetwork(LWSVector4<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Type v[4];
				v[0] = MakeHost(*((Type*)Buffer + i * 4 + 0));
				v[1] = MakeHost(*((Type*)Buffer + i * 4 + 1));
				v[2] = MakeHost(*((Type*)Buffer + i * 4 + 2));
				v[3] = MakeHost(*((Type*)Buffer + i * 4 + 3));
				Out[i] = LWSVector4<Type>(v[0], v[1], v[2], v[3]);
			}
		}
		return sizeof(uint32_t) * 4 * Len;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector4<Type>*, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWVector4<Type> *Out, uint32_t Len, const int8_t *Buffer){
		if (Out){
			for (uint32_t i = 0; i < Len; i++){
				Out[i].x = MakeHost(*((Type*)Buffer + i * 4 + 0));
				Out[i].y = MakeHost(*((Type*)Buffer + i * 4 + 1));
				Out[i].z = MakeHost(*((Type*)Buffer + i * 4 + 2));
				Out[i].w = MakeHost(*((Type*)Buffer + i * 4 + 3));
			}
		}
		return sizeof(Type)* 4 * Len;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector3<Type>*, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWVector3<Type> *Out, uint32_t Len, const int8_t *Buffer){
		if (Out){
			for (uint32_t i = 0; i < Len; i++){
				Out[i].x = MakeHost(*((Type*)Buffer + i * 3 + 0));
				Out[i].y = MakeHost(*((Type*)Buffer + i * 3 + 1));
				Out[i].z = MakeHost(*((Type*)Buffer + i * 3 + 2));
			}
		}
		return sizeof(Type)* 3 * Len;
	}

	/*!
		\overload int32_t ReadNetwork(LWVector2<Type>*, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWVector2<Type> *Out, uint32_t Len, const int8_t *Buffer){
		if (Out){
			for (uint32_t i = 0; i < Len; i++){
				Out[i].x = MakeHost(*((Type*)Buffer + i * 2 + 0));
				Out[i].y = MakeHost(*((Type*)Buffer + i * 2 + 1));
			}
		}
		return sizeof(Type)* 2 * Len;
	}

	/*! \overload int32_t ReadNetwork(LWSMatrix4<Type> *, uint32_t, const int8_t *) */
	template<class Type>
	static int32_t ReadNetwork(LWSMatrix4<Type> *Out, uint32_t Len, const int8_t *Buffer) {
		if (Out) {
			for (uint32_t i = 0; i < Len; i++) {
				Type v[16];
				v[0] = MakeHost(*((Type*)Buffer + i * 16 + 0));
				v[1] = MakeHost(*((Type*)Buffer + i * 16 + 1));
				v[2] = MakeHost(*((Type*)Buffer + i * 16 + 2));
				v[3] = MakeHost(*((Type*)Buffer + i * 16 + 3));
				v[4] = MakeHost(*((Type*)Buffer + i * 16 + 4));
				v[5] = MakeHost(*((Type*)Buffer + i * 16 + 5));
				v[6] = MakeHost(*((Type*)Buffer + i * 16 + 6));
				v[7] = MakeHost(*((Type*)Buffer + i * 16 + 7));
				v[8] = MakeHost(*((Type*)Buffer + i * 16 + 8));
				v[9] = MakeHost(*((Type*)Buffer + i * 16 + 9));
				v[10] = MakeHost(*((Type*)Buffer + i * 16 + 10));
				v[11] = MakeHost(*((Type*)Buffer + i * 16 + 11));
				v[12] = MakeHost(*((Type*)Buffer + i * 16 + 12));
				v[13] = MakeHost(*((Type*)Buffer + i * 16 + 13));
				v[14] = MakeHost(*((Type*)Buffer + i * 16 + 14));
				v[15] = MakeHost(*((Type*)Buffer + i * 16 + 15));
				Out[i] = LWSMatrix4<Type>(LWSVector4<Type>(v[0], v[1], v[2], v[3]), LWSVector4<Type>(v[4], v[5], v[6], v[7]), LWSVector4<Type>(v[8], v[9], v[10], v[11]), LWSVector4<Type>(v[12], v[13], v[14], v[15]));
			}
		}
		return sizeof(Type) * 16 * Len;
	}

	/*! \overload int32_t ReadNetwork(LWMatrix4<Type> *, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWMatrix4<Type> *Out, uint32_t Len, const int8_t *Buffer){
		if (Out){
			for (uint32_t i = 0; i < Len; i++){
				Out[i].m_Rows[0].x = MakeHost(*((Type*)Buffer + i * 16 + 0));
				Out[i].m_Rows[0].y = MakeHost(*((Type*)Buffer + i * 16 + 1));
				Out[i].m_Rows[0].z = MakeHost(*((Type*)Buffer + i * 16 + 2));
				Out[i].m_Rows[0].w = MakeHost(*((Type*)Buffer + i * 16 + 3));
				Out[i].m_Rows[1].x = MakeHost(*((Type*)Buffer + i * 16 + 4));
				Out[i].m_Rows[1].y = MakeHost(*((Type*)Buffer + i * 16 + 5));
				Out[i].m_Rows[1].z = MakeHost(*((Type*)Buffer + i * 16 + 6));
				Out[i].m_Rows[1].w = MakeHost(*((Type*)Buffer + i * 16 + 7));
				Out[i].m_Rows[2].x = MakeHost(*((Type*)Buffer + i * 16 + 8));
				Out[i].m_Rows[2].y = MakeHost(*((Type*)Buffer + i * 16 + 9));
				Out[i].m_Rows[2].z = MakeHost(*((Type*)Buffer + i * 16 + 10));
				Out[i].m_Rows[2].w = MakeHost(*((Type*)Buffer + i * 16 + 11));
				Out[i].m_Rows[3].x = MakeHost(*((Type*)Buffer + i * 16 + 12));
				Out[i].m_Rows[3].y = MakeHost(*((Type*)Buffer + i * 16 + 13));
				Out[i].m_Rows[3].z = MakeHost(*((Type*)Buffer + i * 16 + 14));
				Out[i].m_Rows[3].w = MakeHost(*((Type*)Buffer + i * 16 + 15));
			}
		}
		return sizeof(Type)* 16 * Len;
	}

	/*! \overload int32_t ReadNetwork(LWMatrix3<Type> *, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWMatrix3<Type> *Out, uint32_t Len, const int8_t *Buffer){
		if (Out){
			for (uint32_t i = 0; i < Len; i++){
				Out[i].m_Rows[0].x = MakeHost(*((Type*)Buffer + i * 9 + 0));
				Out[i].m_Rows[0].y = MakeHost(*((Type*)Buffer + i * 9 + 1));
				Out[i].m_Rows[0].z = MakeHost(*((Type*)Buffer + i * 9 + 2));
				Out[i].m_Rows[1].x = MakeHost(*((Type*)Buffer + i * 9 + 3));
				Out[i].m_Rows[1].y = MakeHost(*((Type*)Buffer + i * 9 + 4));
				Out[i].m_Rows[1].z = MakeHost(*((Type*)Buffer + i * 9 + 5));
				Out[i].m_Rows[2].x = MakeHost(*((Type*)Buffer + i * 9 + 6));
				Out[i].m_Rows[2].y = MakeHost(*((Type*)Buffer + i * 9 + 7));
				Out[i].m_Rows[2].z = MakeHost(*((Type*)Buffer + i * 9 + 8));
			}
		}
		return sizeof(Type)* 9 * Len;
	}

	/*! \overload int32_t ReadNetwork(LWMatrix2<Type> *, uint32_t, const int8_t *)
	*/
	template<class Type>
	static int32_t ReadNetwork(LWMatrix2<Type> *Out, uint32_t Len, const int8_t *Buffer){
		if (Out){
			for (uint32_t i = 0; i < Len; i++){
				Out[i].m_Rows[0].x = MakeHost(*((Type*)Buffer + i * 4 + 0));
				Out[i].m_Rows[0].y = MakeHost(*((Type*)Buffer + i * 4 + 1));
				Out[i].m_Rows[1].x = MakeHost(*((Type*)Buffer + i * 4 + 2));
				Out[i].m_Rows[1].y = MakeHost(*((Type*)Buffer + i * 4 + 3));
			}
		}
		return sizeof(Type)* 4 * Len;
	}

	/*! \brief Writes a pointer to the internal buffer.
		\note this method should only be used by someone who deeply understands pointers.
	*/
	int32_t WritePointer(void *Value);

	/*! \brief writes a value of type into the internal buffer.
		\return the number of bytes written.
	*/

	template<class Type>
	int32_t Write(const Type Value){
		typedef int32_t (*Func_T)(const Type, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type);
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*! \overload int32_t Write(const LWSQuaternion<Type> &) */
	template<class Type>
	int32_t Write(const LWSQuaternion<Type> &Value) {
		typedef int32_t(*Func_T)(const LWSQuaternion<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type) * 4;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*!
		\overload int32_t Write(const LWQuaternion<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWQuaternion<Type> &Value) {
		typedef int32_t(*Func_T)(const LWQuaternion<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type) * 4;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*! \overload int32_t Write(const LWSVector4<Type> &) */
	template<class Type>
	int32_t Write(const LWSVector4<Type> &Value) {
		typedef int32_t(*Func_T)(const LWSVector4<Type> &, int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type) * 4;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*!
		\overload int32_t Write(const LWVector4<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWVector4<Type> &Value){
		typedef int32_t(*Func_T)(const LWVector4<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type)* 4;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*!
		\overload int32_t Write(const LWVector3<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWVector3<Type> &Value){
		typedef int32_t(*Func_T)(const LWVector3<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type)* 3;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*!
		\overload int32_t Write(const LWVector2<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWVector2<Type> &Value){
		typedef int32_t(*Func_T)(const LWVector2<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type)* 2;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*! \overload int32_t Write(const LWSMatrix4<Type> &) */
	template<class Type>
	int32_t Write(const LWSMatrix4<Type> &Value) {
		typedef int32_t(*Func_T)(const LWSMatrix4<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type) * 16;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*! \overload int32_t Write(const LWMatrix4<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWMatrix4<Type> &Value){
		typedef int32_t(*Func_T)(const LWMatrix4<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type)* 16;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*! \overload int32_t Write(const LWMatrix3<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWMatrix3<Type> &Value){
		typedef int32_t(*Func_T)(const LWMatrix3<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type)* 9;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*! \overload int32_t Write(const LWMatrix2<Type> &)
	*/
	template<class Type>
	int32_t Write(const LWMatrix2<Type> &Value){
		typedef int32_t(*Func_T)(const LWMatrix2<Type> &, int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Len = sizeof(Type)* 4;
		if (m_Position + Len > m_BufferSize) return Len;
		m_Position += Funcs[m_SelectedFunc](Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Len;
		return Len;
	}

	/*!< \brief write's a UTFx string to the buffer. */
	template<class Type>
	int32_t WriteUTF(const LWUnicodeIterator<Type> &Iter) {
		typedef int32_t(*Func_T)(const LWUnicodeIterator<Type> &, int8_t*);
		Func_T Funcs[] = { LWByteBuffer::WriteUTF<Type>, LWByteBuffer::WriteNetworkUTF<Type> };
		int32_t Length = Funcs[m_SelectedFunc](Iter, nullptr);
		if (m_Position + Length > m_BufferSize) return Length;
		m_Position += Funcs[m_SelectedFunc](Iter, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Length;
		return Length;
	}

	/*!< \brief writes an null terminated text string into the buffer. */
	int32_t WriteText(const uint8_t *Text);

	/*! \overload int32_t WriteText(const char*) */
	int32_t WriteText(const char *Text);

	/*! \brief writes an array of values into the internal buffer.
		\param Len the number of elements to be written.
		\param Values The array of values of length at least equal to Len
	*/
	template<class Type>
	int32_t Write(uint32_t Len, const Type *Values) {
		typedef int32_t (*Func_T)(uint32_t, const Type *, int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Length = sizeof(Type)*Len;
		if (m_Position + Length > m_BufferSize) return Length;
		m_Position += Funcs[m_SelectedFunc](Len, Values, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Length;
		return Length;
	}

	/*! \overload int32_t Write(uint32_t Len, const LWSQuaternion<Type> *Values) */
	template<class Type>
	int32_t Write(uint32_t Len, const LWSQuaternion<Type> *Values) {
		typedef int32_t(*Func_T)(uint32_t, const LWSQuaternion<Type> *, int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Length = sizeof(Type) * 4 * Len;
		if (m_Position + Length > m_BufferSize) return Length;
		m_Position += Funcs[m_SelectedFunc](Len, Values, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Length;
		return Length;
	}

	/*! \overload int32_t Write(uint32_t Len, const LWSVector4<Type> *Values) */
	template<class Type>
	int32_t Write(uint32_t Len, const LWSVector4<Type> *Values) {
		typedef int32_t(*Func_T)(uint32_t, const LWSVector4<Type>*, int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Length = sizeof(Type) * 4 * Len;
		if (m_Position + Length > m_BufferSize) return Length;
		m_Position += Funcs[m_SelectedFunc](Len, Values, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Length;
		return Length;
	}

	/*! \overload int32_t Write(uint32_t Len, const LWSMatrix4<Type> *Values) */
	template<class Type>
	int32_t Write(uint32_t Len, const LWSMatrix4<Type> *Values) {
		typedef int32_t(*Func_T)(uint32_t, const LWSMatrix4<Type>*, int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Write, LWByteBuffer::WriteNetwork };
		int32_t Length = sizeof(Type) * 16 * Len;
		if (m_Position + Length > m_BufferSize) return Length;
		m_Position += Funcs[m_SelectedFunc](Len, Values, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
		m_BytesWritten += Length;
		return Length;
	}
	
	/*! \brief writes an variable number of values into the internal buffer
		\param Len the number of variable arguments to be written.
		\param lst the variable list object.
	*/
	template<class Type>
	int32_t WriteLst(uint32_t Len, va_list lst) {
		typedef int32_t (*Func_T)(int8_t *, uint32_t, va_list);
		Func_T Funcs[] = { LWByteBuffer::Write<Type>, LWByteBuffer::WriteNetwork<Type>};
		int32_t Length = sizeof(Type)*Len;
		if (m_Position + Length > m_BufferSize) return Length;
		m_Position += Funcs[m_SelectedFunc](m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr, Len, lst);
		m_BytesWritten += Length;
		return Length;
	}

	/*! \brief writes an variable number of values into the internal buffer.
		\param Len the number of variable arguments to be written.
		\param ... the arguments.
	*/
	template<class Type>
	int32_t WriteL(uint32_t Len, ...){
		va_list lst;
		va_start(lst, Len);
		int32_t Result = WriteLst<Type>(Len, lst);
		va_end(lst);
		return Result;
	}

	/*! \brief reads a variable of type from the internal buffer.
		\return the value of the object of type.
	*/
	template<class Type>
	Type Read(void){
		typedef int32_t (*Func_T)(Type*, const int8_t *);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		Type Value = Type();
		if (m_Position >= m_BufferSize) return Value;
		m_Position += Funcs[m_SelectedFunc](&Value, m_ReadBuffer + m_Position);
		return Value;
	}

	/*! \brief reads a variable of type from the internal buffer at position.
		\param Position the position in the internal buffer to read from.
		\return the value of the object of type.
	*/
	template<class Type>
	Type Read(int32_t Position){
		typedef int32_t (*Func_T)(Type*, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		Type Value = Type();
		if (Position >= m_BufferSize) return Value;
		Funcs[m_SelectedFunc](&Value, m_ReadBuffer + Position);
		return Value;
	}

	/*! \brief reads an array of variables of type from the internal buffer.
		\param Values an array to store the values from.
		\param Len the length of elements to write into Values.
		\return the number of bytes read.
	*/
	template<class Type>
	int32_t Read(Type *Values, uint32_t Len){
		typedef int32_t (*Func_T)(Type *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads a vec2 of type from the internal buffer.*/
	template<class Type>
	LWVector2<Type> ReadVec2(void) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector2<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a vec2 of type from the internal buffer at the specified location. */
	template<class Type>
	LWVector2<Type> ReadVec2(int32_t Position) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector2<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer+Position);
		return Res;
	}

	/*!< \brief reads an array of vec2 of type from the internal buffer. */
	template<class Type>
	int32_t ReadVec2(LWVector2<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of vec2 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadVec2(LWVector2<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a vec3 of type from the internal buffer.*/
	template<class Type>
	LWVector3<Type> ReadVec3(void) {
		typedef int32_t(*Func_T)(LWVector3<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector3<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a vec3 of type from the internal buffer at the specified location. */
	template<class Type>
	LWVector3<Type> ReadVec3(int32_t Position) {
		typedef int32_t(*Func_T)(LWVector2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector3<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of vec3 of type from the internal buffer. */
	template<class Type>
	int32_t ReadVec3(LWVector3<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWVector3<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of vec3 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadVec3(LWVector3<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWVector3<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a vec4 of type from the internal buffer.*/
	template<class Type>
	LWVector4<Type> ReadVec4(void) {
		typedef int32_t(*Func_T)(LWVector4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector4<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a vec4 of type from the internal buffer at the specified location. */
	template<class Type>
	LWVector4<Type> ReadVec4(int32_t Position) {
		typedef int32_t(*Func_T)(LWVector4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWVector4<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of vec4 of type from the internal buffer. */
	template<class Type>
	int32_t ReadVec4(LWVector4<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWVector4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of vec4 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadVec4(LWVector4<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWVector4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a svec4 of type from the internal buffer.*/
	template<class Type>
	LWSVector4<Type> ReadSVec4(void) {
		typedef int32_t(*Func_T)(LWSVector4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWSVector4<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a SVec4 of type from the internal buffer at the specified location. */
	template<class Type>
	LWSVector4<Type> ReadSVec4(int32_t Position) {
		typedef int32_t(*Func_T)(LWSVector4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWSVector4<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of vec4 of type from the internal buffer. */
	template<class Type>
	int32_t ReadSVec4(LWSVector4<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWSVector4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of vec4 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadSVec4(LWSVector4<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWSVector4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a Quaternion of type from the internal buffer.*/
	template<class Type>
	LWQuaternion<Type> ReadQuaternion(void) {
		typedef int32_t(*Func_T)(LWQuaternion<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWQuaternion<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a Quaternion of type from the internal buffer at the specified location. */
	template<class Type>
	LWQuaternion<Type> ReadQuaternion(int32_t Position) {
		typedef int32_t(*Func_T)(LWQuaternion<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWQuaternion<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of Quaternion of type from the internal buffer. */
	template<class Type>
	int32_t ReadQuaternion(LWQuaternion<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWQuaternion<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of Quaternion of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadQuaternion(LWQuaternion<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWQuaternion<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}


	/*!< \brief reads a SIMD Quaternion of type from the internal buffer.*/
	template<class Type>
	LWSQuaternion<Type> ReadSQuaternion(void) {
		typedef int32_t(*Func_T)(LWSQuaternion<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWSQuaternion<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a SIMD Quaternion of type from the internal buffer at the specified location. */
	template<class Type>
	LWSQuaternion<Type> ReadSQuaternion(int32_t Position) {
		typedef int32_t(*Func_T)(LWSQuaternion<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWSQuaternion<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of SIMD Quaternion of type from the internal buffer. */
	template<class Type>
	int32_t ReadSQuaternion(LWSQuaternion<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWSQuaternion<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of Quaternion of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadSQuaternion(LWSQuaternion<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWSQuaternion<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a matrix2 of type from the internal buffer.*/
	template<class Type>
	LWMatrix2<Type> ReadMat2(void) {
		typedef int32_t(*Func_T)(LWMatrix2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix2<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a matrix2 of type from the internal buffer at the specified location. */
	template<class Type>
	LWMatrix2<Type> ReadMat2(int32_t Position) {
		typedef int32_t(*Func_T)(LWMatrix2<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix2<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of matrix2 of type from the internal buffer. */
	template<class Type>
	int32_t ReadMat2(LWMatrix2<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWMatrix2<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of matrix2 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadMat2(LWMatrix2<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWMatrix2<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a matrix3 of type from the internal buffer.*/
	template<class Type>
	LWMatrix3<Type> ReadMat3(void) {
		typedef int32_t(*Func_T)(LWMatrix3<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix3<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a matrix3 of type from the internal buffer at the specified location. */
	template<class Type>
	LWMatrix3<Type> ReadMat3(int32_t Position) {
		typedef int32_t(*Func_T)(LWMatrix3<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix3<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of matrix3 of type from the internal buffer. */
	template<class Type>
	int32_t ReadMat3(LWMatrix3<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWMatrix3<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of matrix3 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadMat3(LWMatrix3<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWMatrix3<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*!< \brief reads a matrix4 of type from the internal buffer.*/
	template<class Type>
	LWMatrix4<Type> ReadMat4(void) {
		typedef int32_t(*Func_T)(LWMatrix4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix4<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a matrix4 of type from the internal buffer at the specified location. */
	template<class Type>
	LWMatrix4<Type> ReadMat4(int32_t Position) {
		typedef int32_t(*Func_T)(LWMatrix4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWMatrix4<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of matrix4 of type from the internal buffer. */
	template<class Type>
	int32_t ReadMat4(LWMatrix4<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWMatrix4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of matrix4 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadMat4(LWMatrix4<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWMatrix4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}


	/*!< \brief reads a SIMD matrix4 of type from the internal buffer.*/
	template<class Type>
	LWSMatrix4<Type> ReadSMat4(void) {
		typedef int32_t(*Func_T)(LWSMatrix4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWSMatrix4<Type> Res;
		if (m_Position >= m_BufferSize) return Res;
		int32_t Length = Funcs[m_SelectedFunc](&Res, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Res;
	}

	/*!< \brief reads a SIMD matrix4 of type from the internal buffer at the specified location. */
	template<class Type>
	LWSMatrix4<Type> ReadSMat4(int32_t Position) {
		typedef int32_t(*Func_T)(LWSMatrix4<Type> *, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		LWSMatrix4<Type> Res;
		Funcs[m_SelectedFunc](&Res, m_ReadBuffer + Position);
		return Res;
	}

	/*!< \brief reads an array of SIMD matrix4 of type from the internal buffer. */
	template<class Type>
	int32_t ReadSMat4(LWSMatrix4<Type> *Values, uint32_t Len) {
		typedef int32_t(*Func_T)(LWSMatrix4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (m_Position >= m_BufferSize) return 0;
		int32_t Length = Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + m_Position);
		m_Position += Length;
		return Length;
	}

	/*!< \brief reads an array of SIMD matrix4 of type from the internal buffer at the specified location. */
	template<class Type>
	int32_t ReadSMat4(LWSMatrix4<Type> *Values, uint32_t Len, int32_t Position) {
		typedef int32_t(*Func_T)(LWSMatrix4<Type> *, uint32_t, const int8_t*);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*! \brief reads an array of variables of type from the internal buffer at position.
		\param Values an array to store the values from.
		\param Len the length of elements to write into values.
		\param Position the position in the internal buffer to read from.
		\return the number of bytes read.
	*/
	template<class Type>
	int32_t Read(Type *Values, uint32_t Len, int32_t Position){
		typedef int32_t (*Func_T)(Type *, uint32_t, const int8_t);
		Func_T Funcs[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
		if (Position >= m_BufferSize) return 0;
		return Funcs[m_SelectedFunc](Values, Len, m_ReadBuffer + Position);
	}

	/*! \brief reads an utfX string from the buffer.
		\param Out the buffer to receive the text.
		\param OutLen the length of the buffer to receive the text.
		\sa int32_t ReadUTF(int8_t *, uint32_t, const int8_t *)
	*/
	template<class Type>
	int32_t ReadUTF(Type *Out, uint32_t OutLen) {
		typedef int32_t(*Func_T)(Type *, uint32_t, const int8_t*, const uint32_t);
		Func_T Funcs[] = { LWByteBuffer::ReadUTF<Type>, LWByteBuffer::ReadNetworkUTF<Type> };
		int32_t Length = Funcs[m_SelectedFunc](Out, OutLen, m_ReadBuffer + m_Position, (uint32_t)(m_BufferSize - m_Position));
		m_Position += Length;
		return Length;
	}

	/*! \brief reads an utfX string from the buffer at position.
		\param Out the buffer to receive the text.
		\param OutLen the length of the buffer to receive the text.
		\param Position the position of the buffer to read from.
		\sa int32_t ReadUTF(int8_t *, uint32_t, const int8_t *)
	*/
	template<class Type>
	int32_t ReadUTF(Type *Out, uint32_t OutLen, int32_t Position) {
		typedef int32_t(*Func_T)(Type *, uint32_t, const int8_t*, const uint32_t);
		Func_T Funcs[] = { LWByteBuffer::ReadUTF<Type>, LWByteBuffer::ReadNetworkUTF<Type> };
		if (Position >= m_BufferSize) return 0;
		return Funcs[m_SelectedFunc](Out, OutLen, m_ReadBuffer + Position, (uint32_t)(m_BufferSize - Position));
	}

	/*!< \brief reads a null terminated string from the buffer.
	\param Out the buffer to receive the text.
	\param OutLen the length of the buffer to receive the text.
	*/
	int32_t ReadText(uint8_t *Out, uint32_t OutLen);

	/*!< \overload int32_t ReadText(char*, uint32_t) */
	int32_t ReadText(char *Out, uint32_t OutLen);

	/*!< \brief reads a null terminated string from the buffer.
		 \param Out the buffer to receive the text.
		 \param OutLen the length of the buffer to receive the text.
		 \param Position the position of the buffer to read from.
	*/
	int32_t ReadText(uint8_t *Out, uint32_t OutLen, int32_t Position);

	/*!< \overload int32_t ReadText(char *, uint32_t, uint32_t) */
	int32_t ReadText(char *Out, uint32_t OutLen, int32_t Position);

	/*! \brief Set's the number of bytes written.
		\return Returns the LWByteBuffer object.
	*/
	LWByteBuffer &SetBytesWritten(uint32_t BytesWritten);

	/*! \brief Set's the position of the buffer stream.
		\return Returns the LWByteBuffer object.
	*/
	LWByteBuffer &SetPosition(int32_t Position);

	/*! \brief offset's the position to the next alignment.
		\param Write, add the offset to bytesWritten.
		\return the number of bytes that need to be added for alignment.
		\note Alignment must be a power 2.
	*/
	int32_t AlignPosition(uint32_t Alignment, bool Write=false);

	/*! \brief Offset's the position of the buffer stream.
		\param Offset the offset to apply to position.
		\return Returns the LWByteBuffer object.
	*/
	LWByteBuffer &OffsetPosition(int32_t Offset);

	/*! \brief returns size of the buffer.
	*/
	int32_t GetBufferSize(void);

	/*! \brief returns the number of bytes written.
	*/
	int32_t GetBytesWritten(void);

	/*! \brief returns the position of the buffer for read/writing.
	*/
	int32_t GetPosition(void);

	/*! \brief returns if we've reached the end of readable data.
	*/
	bool EndOfData(void);
	
	/*! \brief returns if we've reached the end of writable data.
	*/
	bool EndOfBuffer(void);

	/*! \brief returns the read buffer of the object.
		\return the read buffer.
	*/
	const int8_t *GetReadBuffer(void);

	/*! \brief Constructs a LWByteBuffer object where Buffer can be read/written to.
		\param Buffer the buffer object that can be read/written to.
		\param BufferSize the size of the buffer.
		\param Flag the flags for the buffer to use.
	*/
	LWByteBuffer(int8_t *Buffer, uint32_t BufferSize, uint8_t Flag = 0);

	/*! \brief Constructs a LWByteBuffer object when buffer is read only, as such the buffer is not owned by the LWByteBuffer object, and cannot destroy it.
		\param ReadBuffer the buffer object that can be read from.
		\param BufferSize the size of the buffer for writing to.
		\param Flag the flags for the buffer to use.
	*/
	LWByteBuffer(const int8_t *ReadBuffer, uint32_t BufferSize, uint8_t Flag = 0);

	/*! \brief Deconstruct that cleans up write buffer if the BufferNotOwned flag wasn't set. */
	~LWByteBuffer();
private:
	int8_t *m_WriteBuffer = nullptr;
	const int8_t *m_ReadBuffer = nullptr;
	int32_t m_BufferSize = 0;
	int32_t m_Position = 0;
	int32_t m_BytesWritten = 0;
	int32_t m_SelectedFunc = 0;
	uint8_t m_Flag = 0;
};

/*! \cond */

template<>
inline int32_t LWByteBuffer::Write<float>(int8_t *Buffer, uint32_t Len, va_list lst){
	if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((float*)Buffer) + i) = (float)va_arg(lst, double);
	return sizeof(float)*Len;
}

template<>
inline int32_t LWByteBuffer::Write<double>(int8_t *Buffer, uint32_t Len, va_list lst){
	if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((double*)Buffer) + i) = va_arg(lst, double);
	return sizeof(double)*Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const float Value, int8_t *Buffer){
	if (Buffer) *(uint32_t*)Buffer = MakeNetwork(Value);
	return sizeof(float);
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const double Value, int8_t *Buffer){
	if (Buffer) *(uint64_t*)Buffer = MakeNetwork(Value);
	return sizeof(double);
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWQuaternion<float> &Value, int8_t *Buffer) {
	if (Buffer) {
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(Value.z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(Value.w);
	}
	return sizeof(uint32_t) * 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWQuaternion<double> &Value, int8_t *Buffer) {
	if (Buffer) {
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(Value.z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(Value.w);
	}
	return sizeof(uint64_t) * 4;
}


template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWSQuaternion<float> &Value, int8_t *Buffer) {
	if (Buffer) {
		LWQuaternion<float> v = Value.AsQuaternion();
		*((uint32_t*)Buffer + 0) = MakeNetwork(v.x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(v.y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(v.z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(v.w);
	}
	return sizeof(uint32_t) * 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWSQuaternion<double> &Value, int8_t *Buffer) {
	if (Buffer) {
		LWQuaternion<double> v = Value.AsQuaternion();
		*((uint64_t*)Buffer + 0) = MakeNetwork(v.x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(v.y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(v.z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(v.w);
	}
	return sizeof(uint64_t) * 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWSVector4<float> &Value, int8_t *Buffer) {
	if (Buffer) {
		LWVector4<float> v = Value.AsVec4();
		*((uint32_t*)Buffer + 0) = MakeNetwork(v.x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(v.y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(v.z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(v.w);
	}
	return sizeof(uint32_t) * 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWSVector4<double> &Value, int8_t *Buffer) {
	if (Buffer) {
		LWVector4<double> v = Value.AsVec4();
		*((uint64_t*)Buffer + 0) = MakeNetwork(v.x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(v.y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(v.z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(v.w);
	}
	return sizeof(uint64_t) * 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWVector4<float> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(Value.z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(Value.w);
	}
	return sizeof(uint32_t)* 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWVector4<double> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(Value.z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(Value.w);
	}
	return sizeof(uint64_t)* 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWVector3<float> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(Value.z);
	}
	return sizeof(uint32_t)* 3;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWVector3<double> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(Value.z);
	}
	return sizeof(uint64_t)* 3;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWVector2<float> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.y);
	}
	return sizeof(uint32_t)* 2;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWVector2<double> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.y);
	}
	return sizeof(uint64_t)* 2;
}


template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWSMatrix4<float> &Value, int8_t *Buffer) {
	if (Buffer) {
		LWMatrix4<float> v = Value.AsMat4();
		*((uint32_t*)Buffer + 0) = MakeNetwork(v.m_Rows[0].x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(v.m_Rows[0].y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(v.m_Rows[0].z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(v.m_Rows[0].w);
		*((uint32_t*)Buffer + 4) = MakeNetwork(v.m_Rows[1].x);
		*((uint32_t*)Buffer + 5) = MakeNetwork(v.m_Rows[1].y);
		*((uint32_t*)Buffer + 6) = MakeNetwork(v.m_Rows[1].z);
		*((uint32_t*)Buffer + 7) = MakeNetwork(v.m_Rows[1].w);
		*((uint32_t*)Buffer + 8) = MakeNetwork(v.m_Rows[2].x);
		*((uint32_t*)Buffer + 9) = MakeNetwork(v.m_Rows[2].y);
		*((uint32_t*)Buffer + 10) = MakeNetwork(v.m_Rows[2].z);
		*((uint32_t*)Buffer + 11) = MakeNetwork(v.m_Rows[2].w);
		*((uint32_t*)Buffer + 12) = MakeNetwork(v.m_Rows[3].x);
		*((uint32_t*)Buffer + 13) = MakeNetwork(v.m_Rows[3].y);
		*((uint32_t*)Buffer + 14) = MakeNetwork(v.m_Rows[3].z);
		*((uint32_t*)Buffer + 15) = MakeNetwork(v.m_Rows[3].w);
	}
	return sizeof(uint32_t) * 16;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWSMatrix4<double> &Value, int8_t *Buffer) {
	if (Buffer) {
		LWMatrix4<double> v = Value.AsMat4();
		*((uint64_t*)Buffer + 0) = MakeNetwork(v.m_Rows[0].x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(v.m_Rows[0].y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(v.m_Rows[0].z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(v.m_Rows[0].w);
		*((uint64_t*)Buffer + 4) = MakeNetwork(v.m_Rows[1].x);
		*((uint64_t*)Buffer + 5) = MakeNetwork(v.m_Rows[1].y);
		*((uint64_t*)Buffer + 6) = MakeNetwork(v.m_Rows[1].z);
		*((uint64_t*)Buffer + 7) = MakeNetwork(v.m_Rows[1].w);
		*((uint64_t*)Buffer + 8) = MakeNetwork(v.m_Rows[2].x);
		*((uint64_t*)Buffer + 9) = MakeNetwork(v.m_Rows[2].y);
		*((uint64_t*)Buffer + 10) = MakeNetwork(v.m_Rows[2].z);
		*((uint64_t*)Buffer + 11) = MakeNetwork(v.m_Rows[2].w);
		*((uint64_t*)Buffer + 12) = MakeNetwork(v.m_Rows[3].x);
		*((uint64_t*)Buffer + 13) = MakeNetwork(v.m_Rows[3].y);
		*((uint64_t*)Buffer + 14) = MakeNetwork(v.m_Rows[3].z);
		*((uint64_t*)Buffer + 15) = MakeNetwork(v.m_Rows[3].w);
	}
	return sizeof(uint64_t) * 16;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWMatrix4<float> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(Value.m_Rows[0].z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(Value.m_Rows[0].w);
		*((uint32_t*)Buffer + 4) = MakeNetwork(Value.m_Rows[1].x);
		*((uint32_t*)Buffer + 5) = MakeNetwork(Value.m_Rows[1].y);
		*((uint32_t*)Buffer + 6) = MakeNetwork(Value.m_Rows[1].z);
		*((uint32_t*)Buffer + 7) = MakeNetwork(Value.m_Rows[1].w);
		*((uint32_t*)Buffer + 8) = MakeNetwork(Value.m_Rows[2].x);
		*((uint32_t*)Buffer + 9) = MakeNetwork(Value.m_Rows[2].y);
		*((uint32_t*)Buffer + 10) = MakeNetwork(Value.m_Rows[2].z);
		*((uint32_t*)Buffer + 11) = MakeNetwork(Value.m_Rows[2].w);
		*((uint32_t*)Buffer + 12) = MakeNetwork(Value.m_Rows[3].x);
		*((uint32_t*)Buffer + 13) = MakeNetwork(Value.m_Rows[3].y);
		*((uint32_t*)Buffer + 14) = MakeNetwork(Value.m_Rows[3].z);
		*((uint32_t*)Buffer + 15) = MakeNetwork(Value.m_Rows[3].w);
	}
	return sizeof(uint32_t)* 16;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWMatrix4<double> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(Value.m_Rows[0].z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(Value.m_Rows[0].w);
		*((uint64_t*)Buffer + 4) = MakeNetwork(Value.m_Rows[1].x);
		*((uint64_t*)Buffer + 5) = MakeNetwork(Value.m_Rows[1].y);
		*((uint64_t*)Buffer + 6) = MakeNetwork(Value.m_Rows[1].z);
		*((uint64_t*)Buffer + 7) = MakeNetwork(Value.m_Rows[1].w);
		*((uint64_t*)Buffer + 8) = MakeNetwork(Value.m_Rows[2].x);
		*((uint64_t*)Buffer + 9) = MakeNetwork(Value.m_Rows[2].y);
		*((uint64_t*)Buffer + 10) = MakeNetwork(Value.m_Rows[2].z);
		*((uint64_t*)Buffer + 11) = MakeNetwork(Value.m_Rows[2].w);
		*((uint64_t*)Buffer + 12) = MakeNetwork(Value.m_Rows[3].x);
		*((uint64_t*)Buffer + 13) = MakeNetwork(Value.m_Rows[3].y);
		*((uint64_t*)Buffer + 14) = MakeNetwork(Value.m_Rows[3].z);
		*((uint64_t*)Buffer + 15) = MakeNetwork(Value.m_Rows[3].w);
	}
	return sizeof(uint64_t)* 16;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWMatrix3<float> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(Value.m_Rows[0].z);
		*((uint32_t*)Buffer + 3) = MakeNetwork(Value.m_Rows[1].x);
		*((uint32_t*)Buffer + 4) = MakeNetwork(Value.m_Rows[1].y);
		*((uint32_t*)Buffer + 5) = MakeNetwork(Value.m_Rows[1].z);
		*((uint32_t*)Buffer + 6) = MakeNetwork(Value.m_Rows[2].x);
		*((uint32_t*)Buffer + 7) = MakeNetwork(Value.m_Rows[2].y);
		*((uint32_t*)Buffer + 8) = MakeNetwork(Value.m_Rows[2].z);
	}
	return sizeof(uint32_t)* 9;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWMatrix3<double> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(Value.m_Rows[0].z);
		*((uint64_t*)Buffer + 3) = MakeNetwork(Value.m_Rows[1].x);
		*((uint64_t*)Buffer + 4) = MakeNetwork(Value.m_Rows[1].y);
		*((uint64_t*)Buffer + 5) = MakeNetwork(Value.m_Rows[1].z);
		*((uint64_t*)Buffer + 6) = MakeNetwork(Value.m_Rows[2].x);
		*((uint64_t*)Buffer + 7) = MakeNetwork(Value.m_Rows[2].y);
		*((uint64_t*)Buffer + 8) = MakeNetwork(Value.m_Rows[2].z);
	}
	return sizeof(uint64_t)* 9;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(const LWMatrix2<float> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint32_t*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
		*((uint32_t*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
		*((uint32_t*)Buffer + 2) = MakeNetwork(Value.m_Rows[1].x);
		*((uint32_t*)Buffer + 3) = MakeNetwork(Value.m_Rows[1].y);
	}
	return sizeof(uint32_t)* 4;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(const LWMatrix2<double> &Value, int8_t *Buffer){
	if (Buffer){
		*((uint64_t*)Buffer + 0) = MakeNetwork(Value.m_Rows[0].x);
		*((uint64_t*)Buffer + 1) = MakeNetwork(Value.m_Rows[0].y);
		*((uint64_t*)Buffer + 2) = MakeNetwork(Value.m_Rows[1].x);
		*((uint64_t*)Buffer + 3) = MakeNetwork(Value.m_Rows[1].y);
	}
	return sizeof(uint64_t)* 4;
}

/*! \cond */

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const float *Values, int8_t *Buffer){
	if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((int32_t*)Buffer) + i) = MakeNetwork(Values[i]);
	return sizeof(float)*Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const double *Values, int8_t *Buffer){
	if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((int64_t*)Buffer) + i) = MakeNetwork(Values[i]);
	return sizeof(double)*Len;
}


template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWSQuaternion<float> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			LWQuaternion<float> v = Values[i].AsQuaternion();
			*((uint32_t*)Buffer + i * 4 + 0) = MakeNetwork(v.x);
			*((uint32_t*)Buffer + i * 4 + 1) = MakeNetwork(v.y);
			*((uint32_t*)Buffer + i * 4 + 2) = MakeNetwork(v.z);
			*((uint32_t*)Buffer + i * 4 + 3) = MakeNetwork(v.w);
		}
	}
	return sizeof(uint32_t) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWSQuaternion<double> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			LWQuaternion<double> v = Values[i].AsQuaternion();
			*((uint64_t*)Buffer + i * 4 + 0) = MakeNetwork(v.x);
			*((uint64_t*)Buffer + i * 4 + 1) = MakeNetwork(v.y);
			*((uint64_t*)Buffer + i * 4 + 2) = MakeNetwork(v.z);
			*((uint64_t*)Buffer + i * 4 + 3) = MakeNetwork(v.w);
		}
	}
	return sizeof(uint64_t) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWQuaternion<float> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			*((uint32_t*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].x);
			*((uint32_t*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].y);
			*((uint32_t*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].z);
			*((uint32_t*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].w);
		}
	}
	return sizeof(uint32_t) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWQuaternion<double> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			*((uint64_t*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].x);
			*((uint64_t*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].y);
			*((uint64_t*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].z);
			*((uint64_t*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].w);
		}
	}
	return sizeof(uint64_t) * 4 * Len;
}


template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWSVector4<float> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			LWVector4<float> v = Values[i].AsVec4();
			*((uint32_t*)Buffer + i * 4 + 0) = MakeNetwork(v.x);
			*((uint32_t*)Buffer + i * 4 + 1) = MakeNetwork(v.y);
			*((uint32_t*)Buffer + i * 4 + 2) = MakeNetwork(v.z);
			*((uint32_t*)Buffer + i * 4 + 3) = MakeNetwork(v.w);
		}
	}
	return sizeof(uint32_t) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWSVector4<double> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			LWVector4<double> v = Values[i].AsVec4();
			*((uint64_t*)Buffer + i * 4 + 0) = MakeNetwork(v.x);
			*((uint64_t*)Buffer + i * 4 + 1) = MakeNetwork(v.y);
			*((uint64_t*)Buffer + i * 4 + 2) = MakeNetwork(v.z);
			*((uint64_t*)Buffer + i * 4 + 3) = MakeNetwork(v.w);
		}
	}
	return sizeof(uint64_t) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWVector4<float> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint32_t*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].x);
			*((uint32_t*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].y);
			*((uint32_t*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].z);
			*((uint32_t*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].w);
		}
	}
	return sizeof(uint32_t)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWVector4<double> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint64_t*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].x);
			*((uint64_t*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].y);
			*((uint64_t*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].z);
			*((uint64_t*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].w);
		}
	}
	return sizeof(uint64_t)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWVector3<float> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint32_t*)Buffer + i * 3 + 0) = MakeNetwork(Values[i].x);
			*((uint32_t*)Buffer + i * 3 + 1) = MakeNetwork(Values[i].y);
			*((uint32_t*)Buffer + i * 3 + 2) = MakeNetwork(Values[i].z);
		}
	}
	return sizeof(uint32_t)* 3 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWVector3<double> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint64_t*)Buffer + i * 3 + 0) = MakeNetwork(Values[i].x);
			*((uint64_t*)Buffer + i * 3 + 1) = MakeNetwork(Values[i].y);
			*((uint64_t*)Buffer + i * 3 + 2) = MakeNetwork(Values[i].z);
		}
	}
	return sizeof(uint64_t)* 3 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWVector2<float> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint32_t*)Buffer + i * 2 + 0) = MakeNetwork(Values[i].x);
			*((uint32_t*)Buffer + i * 2 + 1) = MakeNetwork(Values[i].y);
		}
	}
	return sizeof(uint32_t)* 2 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWVector2<double> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint64_t*)Buffer + i * 2 + 0) = MakeNetwork(Values[i].x);
			*((uint64_t*)Buffer + i * 2 + 1) = MakeNetwork(Values[i].y);
		}
	}
	return sizeof(uint64_t)* 2 * Len;
}


template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWSMatrix4<float> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			LWMatrix4<float> v = Values[i].AsMat4();
			*((uint32_t*)Buffer + i * 16 + 0) = MakeNetwork(v.m_Rows[0].x);
			*((uint32_t*)Buffer + i * 16 + 1) = MakeNetwork(v.m_Rows[0].y);
			*((uint32_t*)Buffer + i * 16 + 2) = MakeNetwork(v.m_Rows[0].z);
			*((uint32_t*)Buffer + i * 16 + 3) = MakeNetwork(v.m_Rows[0].w);
			*((uint32_t*)Buffer + i * 16 + 4) = MakeNetwork(v.m_Rows[1].x);
			*((uint32_t*)Buffer + i * 16 + 5) = MakeNetwork(v.m_Rows[1].y);
			*((uint32_t*)Buffer + i * 16 + 6) = MakeNetwork(v.m_Rows[1].z);
			*((uint32_t*)Buffer + i * 16 + 7) = MakeNetwork(v.m_Rows[1].w);
			*((uint32_t*)Buffer + i * 16 + 8) = MakeNetwork(v.m_Rows[2].x);
			*((uint32_t*)Buffer + i * 16 + 9) = MakeNetwork(v.m_Rows[2].y);
			*((uint32_t*)Buffer + i * 16 + 10) = MakeNetwork(v.m_Rows[2].z);
			*((uint32_t*)Buffer + i * 16 + 11) = MakeNetwork(v.m_Rows[2].w);
			*((uint32_t*)Buffer + i * 16 + 12) = MakeNetwork(v.m_Rows[3].x);
			*((uint32_t*)Buffer + i * 16 + 13) = MakeNetwork(v.m_Rows[3].y);
			*((uint32_t*)Buffer + i * 16 + 14) = MakeNetwork(v.m_Rows[3].z);
			*((uint32_t*)Buffer + i * 16 + 15) = MakeNetwork(v.m_Rows[3].w);
		}
	}
	return sizeof(uint32_t) * 16 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWSMatrix4<double> *Values, int8_t *Buffer) {
	if (Buffer) {
		for (uint32_t i = 0; i < Len; i++) {
			LWMatrix4<double> v = Values[i].AsMat4();
			*((uint64_t*)Buffer + i * 16 + 0) = MakeNetwork(v.m_Rows[0].x);
			*((uint64_t*)Buffer + i * 16 + 1) = MakeNetwork(v.m_Rows[0].y);
			*((uint64_t*)Buffer + i * 16 + 2) = MakeNetwork(v.m_Rows[0].z);
			*((uint64_t*)Buffer + i * 16 + 3) = MakeNetwork(v.m_Rows[0].w);
			*((uint64_t*)Buffer + i * 16 + 4) = MakeNetwork(v.m_Rows[1].x);
			*((uint64_t*)Buffer + i * 16 + 5) = MakeNetwork(v.m_Rows[1].y);
			*((uint64_t*)Buffer + i * 16 + 6) = MakeNetwork(v.m_Rows[1].z);
			*((uint64_t*)Buffer + i * 16 + 7) = MakeNetwork(v.m_Rows[1].w);
			*((uint64_t*)Buffer + i * 16 + 8) = MakeNetwork(v.m_Rows[2].x);
			*((uint64_t*)Buffer + i * 16 + 9) = MakeNetwork(v.m_Rows[2].y);
			*((uint64_t*)Buffer + i * 16 + 10) = MakeNetwork(v.m_Rows[2].z);
			*((uint64_t*)Buffer + i * 16 + 11) = MakeNetwork(v.m_Rows[2].w);
			*((uint64_t*)Buffer + i * 16 + 12) = MakeNetwork(v.m_Rows[3].x);
			*((uint64_t*)Buffer + i * 16 + 13) = MakeNetwork(v.m_Rows[3].y);
			*((uint64_t*)Buffer + i * 16 + 14) = MakeNetwork(v.m_Rows[3].z);
			*((uint64_t*)Buffer + i * 16 + 15) = MakeNetwork(v.m_Rows[3].w);
		}
	}
	return sizeof(uint64_t) * 16 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWMatrix4<float> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint32_t*)Buffer + i * 16 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
			*((uint32_t*)Buffer + i * 16 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
			*((uint32_t*)Buffer + i * 16 + 2) = MakeNetwork(Values[i].m_Rows[0].z);
			*((uint32_t*)Buffer + i * 16 + 3) = MakeNetwork(Values[i].m_Rows[0].w);
			*((uint32_t*)Buffer + i * 16 + 4) = MakeNetwork(Values[i].m_Rows[1].x);
			*((uint32_t*)Buffer + i * 16 + 5) = MakeNetwork(Values[i].m_Rows[1].y);
			*((uint32_t*)Buffer + i * 16 + 6) = MakeNetwork(Values[i].m_Rows[1].z);
			*((uint32_t*)Buffer + i * 16 + 7) = MakeNetwork(Values[i].m_Rows[1].w);
			*((uint32_t*)Buffer + i * 16 + 8) = MakeNetwork(Values[i].m_Rows[2].x);
			*((uint32_t*)Buffer + i * 16 + 9) = MakeNetwork(Values[i].m_Rows[2].y);
			*((uint32_t*)Buffer + i * 16 + 10) = MakeNetwork(Values[i].m_Rows[2].z);
			*((uint32_t*)Buffer + i * 16 + 11) = MakeNetwork(Values[i].m_Rows[2].w);
			*((uint32_t*)Buffer + i * 16 + 12) = MakeNetwork(Values[i].m_Rows[3].x);
			*((uint32_t*)Buffer + i * 16 + 13) = MakeNetwork(Values[i].m_Rows[3].y);
			*((uint32_t*)Buffer + i * 16 + 14) = MakeNetwork(Values[i].m_Rows[3].z);
			*((uint32_t*)Buffer + i * 16 + 15) = MakeNetwork(Values[i].m_Rows[3].w);
		}
	}
	return sizeof(uint32_t)* 16 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWMatrix4<double> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint64_t*)Buffer + i * 16 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
			*((uint64_t*)Buffer + i * 16 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
			*((uint64_t*)Buffer + i * 16 + 2) = MakeNetwork(Values[i].m_Rows[0].z);
			*((uint64_t*)Buffer + i * 16 + 3) = MakeNetwork(Values[i].m_Rows[0].w);
			*((uint64_t*)Buffer + i * 16 + 4) = MakeNetwork(Values[i].m_Rows[1].x);
			*((uint64_t*)Buffer + i * 16 + 5) = MakeNetwork(Values[i].m_Rows[1].y);
			*((uint64_t*)Buffer + i * 16 + 6) = MakeNetwork(Values[i].m_Rows[1].z);
			*((uint64_t*)Buffer + i * 16 + 7) = MakeNetwork(Values[i].m_Rows[1].w);
			*((uint64_t*)Buffer + i * 16 + 8) = MakeNetwork(Values[i].m_Rows[2].x);
			*((uint64_t*)Buffer + i * 16 + 9) = MakeNetwork(Values[i].m_Rows[2].y);
			*((uint64_t*)Buffer + i * 16 + 10) = MakeNetwork(Values[i].m_Rows[2].z);
			*((uint64_t*)Buffer + i * 16 + 11) = MakeNetwork(Values[i].m_Rows[2].w);
			*((uint64_t*)Buffer + i * 16 + 12) = MakeNetwork(Values[i].m_Rows[3].x);
			*((uint64_t*)Buffer + i * 16 + 13) = MakeNetwork(Values[i].m_Rows[3].y);
			*((uint64_t*)Buffer + i * 16 + 14) = MakeNetwork(Values[i].m_Rows[3].z);
			*((uint64_t*)Buffer + i * 16 + 15) = MakeNetwork(Values[i].m_Rows[3].w);
		}
	}
	return sizeof(uint64_t)* 16 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWMatrix3<float> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint32_t*)Buffer + i * 9 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
			*((uint32_t*)Buffer + i * 9 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
			*((uint32_t*)Buffer + i * 9 + 2) = MakeNetwork(Values[i].m_Rows[0].z);
			*((uint32_t*)Buffer + i * 9 + 3) = MakeNetwork(Values[i].m_Rows[1].x);
			*((uint32_t*)Buffer + i * 9 + 4) = MakeNetwork(Values[i].m_Rows[1].y);
			*((uint32_t*)Buffer + i * 9 + 5) = MakeNetwork(Values[i].m_Rows[1].z);
			*((uint32_t*)Buffer + i * 9 + 6) = MakeNetwork(Values[i].m_Rows[2].x);
			*((uint32_t*)Buffer + i * 9 + 7) = MakeNetwork(Values[i].m_Rows[2].y);
			*((uint32_t*)Buffer + i * 9 + 8) = MakeNetwork(Values[i].m_Rows[2].z);
		}
	}
	return sizeof(uint32_t)* 9 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWMatrix3<double> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint64_t*)Buffer + i * 9 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
			*((uint64_t*)Buffer + i * 9 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
			*((uint64_t*)Buffer + i * 9 + 2) = MakeNetwork(Values[i].m_Rows[0].z);
			*((uint64_t*)Buffer + i * 9 + 3) = MakeNetwork(Values[i].m_Rows[1].x);
			*((uint64_t*)Buffer + i * 9 + 4) = MakeNetwork(Values[i].m_Rows[1].y);
			*((uint64_t*)Buffer + i * 9 + 5) = MakeNetwork(Values[i].m_Rows[1].z);
			*((uint64_t*)Buffer + i * 9 + 6) = MakeNetwork(Values[i].m_Rows[2].x);
			*((uint64_t*)Buffer + i * 9 + 7) = MakeNetwork(Values[i].m_Rows[2].y);
			*((uint64_t*)Buffer + i * 9 + 8) = MakeNetwork(Values[i].m_Rows[2].z);
		}
	}
	return sizeof(uint64_t)* 9 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(uint32_t Len, const LWMatrix2<float> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint32_t*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
			*((uint32_t*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
			*((uint32_t*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].m_Rows[1].x);
			*((uint32_t*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].m_Rows[1].y);
		}
	}
	return sizeof(uint32_t)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(uint32_t Len, const LWMatrix2<double> *Values, int8_t *Buffer){
	if (Buffer){
		for (uint32_t i = 0; i < Len; i++){
			*((uint64_t*)Buffer + i * 4 + 0) = MakeNetwork(Values[i].m_Rows[0].x);
			*((uint64_t*)Buffer + i * 4 + 1) = MakeNetwork(Values[i].m_Rows[0].y);
			*((uint64_t*)Buffer + i * 4 + 2) = MakeNetwork(Values[i].m_Rows[1].x);
			*((uint64_t*)Buffer + i * 4 + 3) = MakeNetwork(Values[i].m_Rows[1].y);
		}
	}
	return sizeof(uint64_t)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<float>(int8_t *Buffer, uint32_t Len, va_list lst){
	if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((int32_t*)Buffer) + i) = MakeNetwork((float)va_arg(lst, double));
	return sizeof(float)*Len;
}

template<>
inline int32_t LWByteBuffer::WriteNetwork<double>(int8_t *Buffer, uint32_t Len, va_list lst){
	if (Buffer) for (uint32_t i = 0; i < Len; i++) *(((int64_t*)Buffer) + i) = MakeNetwork(va_arg(lst, double));
	return sizeof(double)*Len;
}


template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(float *Out, const int8_t *Buffer){
	if (Out) *Out = MakeHostf(*((uint32_t*)Buffer));
	return sizeof(float);
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(double *Out, const int8_t *Buffer){
	if (Out) *Out = MakeHostf(*((uint64_t*)Buffer));
	return sizeof(double);
}


template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWSQuaternion<float> *Out, const int8_t *Buffer) {
	if (Out) {
		float v[4];
		v[0] = MakeHostf(*((uint32_t*)Buffer + 0));
		v[1] = MakeHostf(*((uint32_t*)Buffer + 1));
		v[2] = MakeHostf(*((uint32_t*)Buffer + 2));
		v[3] = MakeHostf(*((uint32_t*)Buffer + 3));
		*Out = LWSQuaternion<float>(v[3], v[0], v[1], v[2]);
	}
	return sizeof(float) * 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWSQuaternion<double> *Out, const int8_t *Buffer) {
	if (Out) {
		double v[4];
		v[0] = MakeHostf(*((uint64_t*)Buffer + 0));
		v[1] = MakeHostf(*((uint64_t*)Buffer + 1));
		v[2] = MakeHostf(*((uint64_t*)Buffer + 2));
		v[3] = MakeHostf(*((uint64_t*)Buffer + 3));
		*Out = LWSQuaternion<double>(v[3], v[0], v[1], v[2]);
	}
	return sizeof(double) * 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWQuaternion<float> *Out, const int8_t *Buffer) {
	if (Out) {
		Out->x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint32_t*)Buffer + 1));
		Out->z = MakeHostf(*((uint32_t*)Buffer + 2));
		Out->w = MakeHostf(*((uint32_t*)Buffer + 3));
	}
	return sizeof(float) * 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWQuaternion<double> *Out, const int8_t *Buffer) {
	if (Out) {
		Out->x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint64_t*)Buffer + 1));
		Out->z = MakeHostf(*((uint64_t*)Buffer + 2));
		Out->w = MakeHostf(*((uint64_t*)Buffer + 3));
	}
	return sizeof(double) * 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWSVector4<float> *Out, const int8_t *Buffer) {
	if (Out) {
		float v[4];
		v[0] = MakeHostf(*((uint32_t*)Buffer + 0));
		v[1] = MakeHostf(*((uint32_t*)Buffer + 1));
		v[2] = MakeHostf(*((uint32_t*)Buffer + 2));
		v[3] = MakeHostf(*((uint32_t*)Buffer + 3));
		*Out = LWSVector4<float>(v[0], v[1], v[2], v[3]);
	}
	return sizeof(float) * 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWSVector4<double> *Out, const int8_t *Buffer) {
	if (Out) {
		double v[4];
		v[0] = MakeHostf(*((uint64_t*)Buffer + 0));
		v[1] = MakeHostf(*((uint64_t*)Buffer + 1));
		v[2] = MakeHostf(*((uint64_t*)Buffer + 2));
		v[3] = MakeHostf(*((uint64_t*)Buffer + 3));
		*Out = LWSVector4<double>(v[0], v[1], v[2], v[3]);
	}
	return sizeof(double) * 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWVector4<float> *Out, const int8_t *Buffer){
	if (Out){
		Out->x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint32_t*)Buffer + 1));
		Out->z = MakeHostf(*((uint32_t*)Buffer + 2));
		Out->w = MakeHostf(*((uint32_t*)Buffer + 3));
	}
	return sizeof(float)* 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWVector4<double> *Out, const int8_t *Buffer){
	if (Out){
		Out->x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint64_t*)Buffer + 1));
		Out->z = MakeHostf(*((uint64_t*)Buffer + 2));
		Out->w = MakeHostf(*((uint64_t*)Buffer + 3));
	}
	return sizeof(double)* 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWVector3<float> *Out, const int8_t *Buffer){
	if (Out){
		Out->x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint32_t*)Buffer + 1));
		Out->z = MakeHostf(*((uint32_t*)Buffer + 2));
	}
	return sizeof(float)* 3;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWVector3<double> *Out, const int8_t *Buffer){
	if (Out){
		Out->x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint64_t*)Buffer + 1));
		Out->z = MakeHostf(*((uint64_t*)Buffer + 2));
	}
	return sizeof(double)* 3;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWVector2<float> *Out, const int8_t *Buffer){
	if (Out){
		Out->x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint32_t*)Buffer + 1));
	}
	return sizeof(float)* 2;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWVector2<double> *Out, const int8_t *Buffer){
	if (Out){
		Out->x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->y = MakeHostf(*((uint64_t*)Buffer + 1));
	}
	return sizeof(double)* 2;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWSMatrix4<float> *Out, const int8_t *Buffer) {
	if (Out) {
		float v[16];
		v[0] = MakeHostf(*((uint32_t*)Buffer + 0));
		v[1] = MakeHostf(*((uint32_t*)Buffer + 1));
		v[2] = MakeHostf(*((uint32_t*)Buffer + 2));
		v[3] = MakeHostf(*((uint32_t*)Buffer + 3));
		v[4] = MakeHostf(*((uint32_t*)Buffer + 4));
		v[5] = MakeHostf(*((uint32_t*)Buffer + 5));
		v[6] = MakeHostf(*((uint32_t*)Buffer + 6));
		v[7] = MakeHostf(*((uint32_t*)Buffer + 7));
		v[8] = MakeHostf(*((uint32_t*)Buffer + 8));
		v[9] = MakeHostf(*((uint32_t*)Buffer + 9));
		v[10] = MakeHostf(*((uint32_t*)Buffer + 10));
		v[11] = MakeHostf(*((uint32_t*)Buffer + 11));
		v[12] = MakeHostf(*((uint32_t*)Buffer + 12));
		v[13] = MakeHostf(*((uint32_t*)Buffer + 13));
		v[14] = MakeHostf(*((uint32_t*)Buffer + 14));
		v[15] = MakeHostf(*((uint32_t*)Buffer + 15));
		*Out = LWSMatrix4<float>(LWSVector4<float>(v[0], v[1], v[2], v[3]), LWSVector4<float>(v[4], v[5], v[6], v[7]), LWSVector4<float>(v[8], v[9], v[10], v[11]), LWSVector4<float>(v[12], v[13], v[14], v[15]));
	}
	return sizeof(float) * 16;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWSMatrix4<double> *Out, const int8_t *Buffer) {
	if (Out) {
		double v[16];
		v[0] = MakeHostf(*((uint64_t*)Buffer + 0));
		v[1] = MakeHostf(*((uint64_t*)Buffer + 1));
		v[2] = MakeHostf(*((uint64_t*)Buffer + 2));
		v[3] = MakeHostf(*((uint64_t*)Buffer + 3));
		v[4] = MakeHostf(*((uint64_t*)Buffer + 4));
		v[5] = MakeHostf(*((uint64_t*)Buffer + 5));
		v[6] = MakeHostf(*((uint64_t*)Buffer + 6));
		v[7] = MakeHostf(*((uint64_t*)Buffer + 7));
		v[8] = MakeHostf(*((uint64_t*)Buffer + 8));
		v[9] = MakeHostf(*((uint64_t*)Buffer + 9));
		v[10] = MakeHostf(*((uint64_t*)Buffer + 10));
		v[11] = MakeHostf(*((uint64_t*)Buffer + 11));
		v[12] = MakeHostf(*((uint64_t*)Buffer + 12));
		v[13] = MakeHostf(*((uint64_t*)Buffer + 13));
		v[14] = MakeHostf(*((uint64_t*)Buffer + 14));
		v[15] = MakeHostf(*((uint64_t*)Buffer + 15));
		*Out = LWSMatrix4<double>(LWSVector4<double>(v[0], v[1], v[2], v[3]), LWSVector4<double>(v[4], v[5], v[6], v[7]), LWSVector4<double>(v[8], v[9], v[10], v[11]), LWSVector4<double>(v[12], v[13], v[14], v[15]));
	}
	return sizeof(double) * 16;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWMatrix4<float> *Out, const int8_t *Buffer){
	if (Out){
		Out->m_Rows[0].x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->m_Rows[0].y = MakeHostf(*((uint32_t*)Buffer + 1));
		Out->m_Rows[0].z = MakeHostf(*((uint32_t*)Buffer + 2));
		Out->m_Rows[0].w = MakeHostf(*((uint32_t*)Buffer + 3));
		Out->m_Rows[1].x = MakeHostf(*((uint32_t*)Buffer + 4));
		Out->m_Rows[1].y = MakeHostf(*((uint32_t*)Buffer + 5));
		Out->m_Rows[1].z = MakeHostf(*((uint32_t*)Buffer + 6));
		Out->m_Rows[1].w = MakeHostf(*((uint32_t*)Buffer + 7));
		Out->m_Rows[2].x = MakeHostf(*((uint32_t*)Buffer + 8));
		Out->m_Rows[2].y = MakeHostf(*((uint32_t*)Buffer + 9));
		Out->m_Rows[2].z = MakeHostf(*((uint32_t*)Buffer + 10));
		Out->m_Rows[2].w = MakeHostf(*((uint32_t*)Buffer + 11));
		Out->m_Rows[3].x = MakeHostf(*((uint32_t*)Buffer + 12));
		Out->m_Rows[3].y = MakeHostf(*((uint32_t*)Buffer + 13));
		Out->m_Rows[3].z = MakeHostf(*((uint32_t*)Buffer + 14));
		Out->m_Rows[3].w = MakeHostf(*((uint32_t*)Buffer + 15));
	}
	return sizeof(float)* 16;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWMatrix4<double> *Out, const int8_t *Buffer){
	if (Out){
		Out->m_Rows[0].x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->m_Rows[0].y = MakeHostf(*((uint64_t*)Buffer + 1));
		Out->m_Rows[0].z = MakeHostf(*((uint64_t*)Buffer + 2));
		Out->m_Rows[0].w = MakeHostf(*((uint64_t*)Buffer + 3));
		Out->m_Rows[1].x = MakeHostf(*((uint64_t*)Buffer + 4));
		Out->m_Rows[1].y = MakeHostf(*((uint64_t*)Buffer + 5));
		Out->m_Rows[1].z = MakeHostf(*((uint64_t*)Buffer + 6));
		Out->m_Rows[1].w = MakeHostf(*((uint64_t*)Buffer + 7));
		Out->m_Rows[2].x = MakeHostf(*((uint64_t*)Buffer + 8));
		Out->m_Rows[2].y = MakeHostf(*((uint64_t*)Buffer + 9));
		Out->m_Rows[2].z = MakeHostf(*((uint64_t*)Buffer + 10));
		Out->m_Rows[2].w = MakeHostf(*((uint64_t*)Buffer + 11));
		Out->m_Rows[3].x = MakeHostf(*((uint64_t*)Buffer + 12));
		Out->m_Rows[3].y = MakeHostf(*((uint64_t*)Buffer + 13));
		Out->m_Rows[3].z = MakeHostf(*((uint64_t*)Buffer + 14));
		Out->m_Rows[3].w = MakeHostf(*((uint64_t*)Buffer + 15));
	}
	return sizeof(double)* 16;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWMatrix3<float> *Out, const int8_t *Buffer){
	if (Out){
		Out->m_Rows[0].x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->m_Rows[0].y = MakeHostf(*((uint32_t*)Buffer + 1));
		Out->m_Rows[0].z = MakeHostf(*((uint32_t*)Buffer + 2));
		Out->m_Rows[1].x = MakeHostf(*((uint32_t*)Buffer + 3));
		Out->m_Rows[1].y = MakeHostf(*((uint32_t*)Buffer + 4));
		Out->m_Rows[1].z = MakeHostf(*((uint32_t*)Buffer + 5));
		Out->m_Rows[2].x = MakeHostf(*((uint32_t*)Buffer + 6));
		Out->m_Rows[2].y = MakeHostf(*((uint32_t*)Buffer + 7));
		Out->m_Rows[2].z = MakeHostf(*((uint32_t*)Buffer + 8));
	}
	return sizeof(float)* 9;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWMatrix3<double> *Out, const int8_t *Buffer){
	if (Out){
		Out->m_Rows[0].x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->m_Rows[0].y = MakeHostf(*((uint64_t*)Buffer + 1));
		Out->m_Rows[0].z = MakeHostf(*((uint64_t*)Buffer + 2));
		Out->m_Rows[1].x = MakeHostf(*((uint64_t*)Buffer + 3));
		Out->m_Rows[1].y = MakeHostf(*((uint64_t*)Buffer + 4));
		Out->m_Rows[1].z = MakeHostf(*((uint64_t*)Buffer + 5));
		Out->m_Rows[2].x = MakeHostf(*((uint64_t*)Buffer + 6));
		Out->m_Rows[2].y = MakeHostf(*((uint64_t*)Buffer + 7));
		Out->m_Rows[2].z = MakeHostf(*((uint64_t*)Buffer + 8));
	}
	return sizeof(double)* 9;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWMatrix2<float> *Out, const int8_t *Buffer){
	if (Out){
		Out->m_Rows[0].x = MakeHostf(*((uint32_t*)Buffer + 0));
		Out->m_Rows[0].y = MakeHostf(*((uint32_t*)Buffer + 1));
		Out->m_Rows[1].x = MakeHostf(*((uint32_t*)Buffer + 2));
		Out->m_Rows[1].y = MakeHostf(*((uint32_t*)Buffer + 3));
	}
	return sizeof(float)* 4;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWMatrix2<double> *Out, const int8_t *Buffer){
	if (Out){
		Out->m_Rows[0].x = MakeHostf(*((uint64_t*)Buffer + 0));
		Out->m_Rows[0].y = MakeHostf(*((uint64_t*)Buffer + 1));
		Out->m_Rows[1].x = MakeHostf(*((uint64_t*)Buffer + 2));
		Out->m_Rows[1].y = MakeHostf(*((uint64_t*)Buffer + 3));
	}
	return sizeof(double)* 4;
}


template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(float *Out, uint32_t Len, const int8_t *Buffer){
	if (Out) for (uint32_t i = 0; i < Len; i++) Out[i] = MakeHostf(*(((uint32_t*)Buffer) + i));
	return sizeof(float)*Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(double *Out, uint32_t Len, const int8_t *Buffer){
	if (Out) for (uint32_t i = 0; i < Len; i++) Out[i] = MakeHostf(*(((uint64_t*)Buffer) + i));
	return sizeof(double)*Len;
}


template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWSQuaternion<float> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			float v[4];
			v[0] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 0));
			v[1] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 1));
			v[2] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 2));
			v[3] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 3));
			Out[i] = LWSQuaternion<float>(v[3], v[0], v[1], v[2]);
		}
	}
	return sizeof(float) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWSQuaternion<double> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			double v[4];
			v[0] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 0));
			v[1] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 1));
			v[2] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 2));
			v[3] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 3));
			Out[i] = LWSQuaternion<double>(v[3], v[0], v[1], v[2]);
		}
	}
	return sizeof(double) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWQuaternion<float> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			Out[i].x = MakeHostf(*((uint32_t*)Buffer + i * 4 + 0));
			Out[i].y = MakeHostf(*((uint32_t*)Buffer + i * 4 + 1));
			Out[i].z = MakeHostf(*((uint32_t*)Buffer + i * 4 + 2));
			Out[i].w = MakeHostf(*((uint32_t*)Buffer + i * 4 + 3));
		}
	}
	return sizeof(float) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWQuaternion<double> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			Out[i].x = MakeHostf(*((uint64_t*)Buffer + i * 4 + 0));
			Out[i].y = MakeHostf(*((uint64_t*)Buffer + i * 4 + 1));
			Out[i].z = MakeHostf(*((uint64_t*)Buffer + i * 4 + 2));
			Out[i].w = MakeHostf(*((uint64_t*)Buffer + i * 4 + 3));
		}
	}
	return sizeof(double) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWSVector4<float> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			float v[4];
			v[0] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 0));
			v[1] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 1));
			v[2] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 2));
			v[3] = MakeHostf(*((uint32_t*)Buffer + i * 4 + 3));
			Out[i] = LWSVector4<float>(v[0], v[1], v[2], v[3]);
		}
	}
	return sizeof(float) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWSVector4<double> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			double v[4];
			v[0] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 0));
			v[1] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 1));
			v[2] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 2));
			v[3] = MakeHostf(*((uint64_t*)Buffer + i * 4 + 3));
			Out[i] = LWSVector4<double>(v[0], v[1], v[2], v[3]);
		}
	}
	return sizeof(double) * 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWVector4<float> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].x = MakeHostf(*((uint32_t*)Buffer + i * 4 + 0));
			Out[i].y = MakeHostf(*((uint32_t*)Buffer + i * 4 + 1));
			Out[i].z = MakeHostf(*((uint32_t*)Buffer + i * 4 + 2));
			Out[i].w = MakeHostf(*((uint32_t*)Buffer + i * 4 + 3));
		}
	}
	return sizeof(float)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWVector4<double> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].x = MakeHostf(*((uint64_t*)Buffer + i * 4 + 0));
			Out[i].y = MakeHostf(*((uint64_t*)Buffer + i * 4 + 1));
			Out[i].z = MakeHostf(*((uint64_t*)Buffer + i * 4 + 2));
			Out[i].w = MakeHostf(*((uint64_t*)Buffer + i * 4 + 3));
		}
	}
	return sizeof(double)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWVector3<float> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].x = MakeHostf(*((uint32_t*)Buffer + i * 3 + 0));
			Out[i].y = MakeHostf(*((uint32_t*)Buffer + i * 3 + 1));
			Out[i].z = MakeHostf(*((uint32_t*)Buffer + i * 3 + 2));
		}
	}
	return sizeof(float)* 3 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWVector3<double> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].x = MakeHostf(*((uint64_t*)Buffer + i * 3 + 0));
			Out[i].y = MakeHostf(*((uint64_t*)Buffer + i * 3 + 1));
			Out[i].z = MakeHostf(*((uint64_t*)Buffer + i * 3 + 2));
		}
	}
	return sizeof(double)* 3 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWVector2<float> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].x = MakeHostf(*((uint32_t*)Buffer + i * 2 + 0));
			Out[i].y = MakeHostf(*((uint32_t*)Buffer + i * 2 + 1));
		}
	}
	return sizeof(float)* 2 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWVector2<double> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].x = MakeHostf(*((uint64_t*)Buffer + i * 2 + 0));
			Out[i].y = MakeHostf(*((uint64_t*)Buffer + i * 2 + 1));
		}
	}
	return sizeof(double)* 2 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWSMatrix4<float> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			float v[16];
			v[0] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 0));
			v[1] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 1));
			v[2] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 2));
			v[3] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 3));
			v[4] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 4));
			v[5] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 5));
			v[6] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 6));
			v[7] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 7));
			v[8] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 8));
			v[9] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 9));
			v[10] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 10));
			v[11] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 11));
			v[12] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 12));
			v[13] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 13));
			v[14] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 14));
			v[15] = MakeHostf(*((uint32_t*)Buffer + i * 16 + 15));
			Out[i] = LWSMatrix4<float>(LWSVector4<float>(v[0], v[1], v[2], v[3]), LWSVector4<float>(v[4], v[5], v[6], v[7]), LWSVector4<float>(v[8], v[9], v[10], v[11]), LWSVector4<float>(v[12], v[13], v[14], v[15]));
		}
	}
	return sizeof(float) * 16 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWSMatrix4<double> *Out, uint32_t Len, const int8_t *Buffer) {
	if (Out) {
		for (uint32_t i = 0; i < Len; i++) {
			double v[16];
			v[0] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 0));
			v[1] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 1));
			v[2] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 2));
			v[3] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 3));
			v[4] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 4));
			v[5] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 5));
			v[6] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 6));
			v[7] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 7));
			v[8] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 8));
			v[9] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 9));
			v[10] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 10));
			v[11] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 11));
			v[12] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 12));
			v[13] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 13));
			v[14] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 14));
			v[15] = MakeHostf(*((uint64_t*)Buffer + i * 16 + 15));
			Out[i] = LWSMatrix4<double>(LWSVector4<double>(v[0], v[1], v[2], v[3]), LWSVector4<double>(v[4], v[5], v[6], v[7]), LWSVector4<double>(v[8], v[9], v[10], v[11]), LWSVector4<double>(v[12], v[13], v[14], v[15]));
		}
	}
	return sizeof(double) * 16 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWMatrix4<float> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].m_Rows[0].x = MakeHostf(*((uint32_t*)Buffer + i * 16 + 0));
			Out[i].m_Rows[0].y = MakeHostf(*((uint32_t*)Buffer + i * 16 + 1));
			Out[i].m_Rows[0].z = MakeHostf(*((uint32_t*)Buffer + i * 16 + 2));
			Out[i].m_Rows[0].w = MakeHostf(*((uint32_t*)Buffer + i * 16 + 3));
			Out[i].m_Rows[1].x = MakeHostf(*((uint32_t*)Buffer + i * 16 + 4));
			Out[i].m_Rows[1].y = MakeHostf(*((uint32_t*)Buffer + i * 16 + 5));
			Out[i].m_Rows[1].z = MakeHostf(*((uint32_t*)Buffer + i * 16 + 6));
			Out[i].m_Rows[1].w = MakeHostf(*((uint32_t*)Buffer + i * 16 + 7));
			Out[i].m_Rows[2].x = MakeHostf(*((uint32_t*)Buffer + i * 16 + 8));
			Out[i].m_Rows[2].y = MakeHostf(*((uint32_t*)Buffer + i * 16 + 9));
			Out[i].m_Rows[2].z = MakeHostf(*((uint32_t*)Buffer + i * 16 + 10));
			Out[i].m_Rows[2].w = MakeHostf(*((uint32_t*)Buffer + i * 16 + 11));
			Out[i].m_Rows[3].x = MakeHostf(*((uint32_t*)Buffer + i * 16 + 12));
			Out[i].m_Rows[3].y = MakeHostf(*((uint32_t*)Buffer + i * 16 + 13));
			Out[i].m_Rows[3].z = MakeHostf(*((uint32_t*)Buffer + i * 16 + 14));
			Out[i].m_Rows[3].w = MakeHostf(*((uint32_t*)Buffer + i * 16 + 15));
		}
	}
	return sizeof(float)* 16 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWMatrix4<double> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].m_Rows[0].x = MakeHostf(*((uint64_t*)Buffer + i * 16 + 0));
			Out[i].m_Rows[0].y = MakeHostf(*((uint64_t*)Buffer + i * 16 + 1));
			Out[i].m_Rows[0].z = MakeHostf(*((uint64_t*)Buffer + i * 16 + 2));
			Out[i].m_Rows[0].w = MakeHostf(*((uint64_t*)Buffer + i * 16 + 3));
			Out[i].m_Rows[1].x = MakeHostf(*((uint64_t*)Buffer + i * 16 + 4));
			Out[i].m_Rows[1].y = MakeHostf(*((uint64_t*)Buffer + i * 16 + 5));
			Out[i].m_Rows[1].z = MakeHostf(*((uint64_t*)Buffer + i * 16 + 6));
			Out[i].m_Rows[1].w = MakeHostf(*((uint64_t*)Buffer + i * 16 + 7));
			Out[i].m_Rows[2].x = MakeHostf(*((uint64_t*)Buffer + i * 16 + 8));
			Out[i].m_Rows[2].y = MakeHostf(*((uint64_t*)Buffer + i * 16 + 9));
			Out[i].m_Rows[2].z = MakeHostf(*((uint64_t*)Buffer + i * 16 + 10));
			Out[i].m_Rows[2].w = MakeHostf(*((uint64_t*)Buffer + i * 16 + 11));
			Out[i].m_Rows[3].x = MakeHostf(*((uint64_t*)Buffer + i * 16 + 12));
			Out[i].m_Rows[3].y = MakeHostf(*((uint64_t*)Buffer + i * 16 + 13));
			Out[i].m_Rows[3].z = MakeHostf(*((uint64_t*)Buffer + i * 16 + 14));
			Out[i].m_Rows[3].w = MakeHostf(*((uint64_t*)Buffer + i * 16 + 15));
		}
	}
	return sizeof(double)* 16 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWMatrix3<float> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].m_Rows[0].x = MakeHostf(*((uint32_t*)Buffer + i * 9 + 0));
			Out[i].m_Rows[0].y = MakeHostf(*((uint32_t*)Buffer + i * 9 + 1));
			Out[i].m_Rows[0].z = MakeHostf(*((uint32_t*)Buffer + i * 9 + 2));
			Out[i].m_Rows[1].x = MakeHostf(*((uint32_t*)Buffer + i * 9 + 3));
			Out[i].m_Rows[1].y = MakeHostf(*((uint32_t*)Buffer + i * 9 + 4));
			Out[i].m_Rows[1].z = MakeHostf(*((uint32_t*)Buffer + i * 9 + 5));
			Out[i].m_Rows[2].x = MakeHostf(*((uint32_t*)Buffer + i * 9 + 6));
			Out[i].m_Rows[2].y = MakeHostf(*((uint32_t*)Buffer + i * 9 + 7));
			Out[i].m_Rows[2].z = MakeHostf(*((uint32_t*)Buffer + i * 9 + 8));
		}
	}
	return sizeof(float)* 9 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWMatrix3<double> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].m_Rows[0].x = MakeHostf(*((uint64_t*)Buffer + i * 9 + 0));
			Out[i].m_Rows[0].y = MakeHostf(*((uint64_t*)Buffer + i * 9 + 1));
			Out[i].m_Rows[0].z = MakeHostf(*((uint64_t*)Buffer + i * 9 + 2));
			Out[i].m_Rows[1].x = MakeHostf(*((uint64_t*)Buffer + i * 9 + 3));
			Out[i].m_Rows[1].y = MakeHostf(*((uint64_t*)Buffer + i * 9 + 4));
			Out[i].m_Rows[1].z = MakeHostf(*((uint64_t*)Buffer + i * 9 + 5));
			Out[i].m_Rows[2].x = MakeHostf(*((uint64_t*)Buffer + i * 9 + 6));
			Out[i].m_Rows[2].y = MakeHostf(*((uint64_t*)Buffer + i * 9 + 7));
			Out[i].m_Rows[2].z = MakeHostf(*((uint64_t*)Buffer + i * 9 + 8));
		}
	}
	return sizeof(double)* 9 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<float>(LWMatrix2<float> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].m_Rows[0].x = MakeHostf(*((uint32_t*)Buffer + i * 4 + 0));
			Out[i].m_Rows[0].y = MakeHostf(*((uint32_t*)Buffer + i * 4 + 1));
			Out[i].m_Rows[1].x = MakeHostf(*((uint32_t*)Buffer + i * 4 + 2));
			Out[i].m_Rows[1].y = MakeHostf(*((uint32_t*)Buffer + i * 4 + 3));
		}
	}
	return sizeof(float)* 4 * Len;
}

template<>
inline int32_t LWByteBuffer::ReadNetwork<double>(LWMatrix2<double> *Out, uint32_t Len, const int8_t *Buffer){
	if (Out){
		for (uint32_t i = 0; i < Len; i++){
			Out[i].m_Rows[0].x = MakeHostf(*((uint64_t*)Buffer + i * 4 + 0));
			Out[i].m_Rows[0].y = MakeHostf(*((uint64_t*)Buffer + i * 4 + 1));
			Out[i].m_Rows[1].x = MakeHostf(*((uint64_t*)Buffer + i * 4 + 2));
			Out[i].m_Rows[1].y = MakeHostf(*((uint64_t*)Buffer + i * 4 + 3));
		}
	}
	return sizeof(double)* 4 * Len;
}
/*! \endcond */
/* @} */

#endif