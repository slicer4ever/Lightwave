#ifndef LWCORETYPES_H
#define LWCORETYPES_H
#include <cstdint>
#include <cassert>

//Add libfmt support.
#ifdef _MSC_VER
#ifndef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0 //Because subsystem:Windows only supports 4096 characters, libfmt will throw an exception when such an issue is encountered, this ideally prevents it during release(and debug builds should be set to console mode, however if not then a debug build will assert).
#endif
#include "../../../../Dependency/libfmt/include/fmt/format.h"
#include "../../../../Dependency/libfmt/include/fmt/chrono.h"
#include "../../../../Dependency/libfmt/include/fmt/ostream.h"
#else
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/ostream.h>
#endif

/*!< \brief assert's in debug builds, but still keeps expression in non debug builds for simpler one-liners used by lightwave. */
#ifndef NDEBUG
#define LWVerify(x) assert(x)
#else
#define LWVerify(x) ((void)(x))
#endif

/*!< \brief used to define bit+bitsoffset of Name.  Name##Bits is the bit pattern representing the field, Name##BitsOffset represents the bit offset to the patten. */
#define LWBitField(Type, Name, BitCount, BitOffset) \
	Type Name = ((1u<<(BitCount))-1u)<<(BitOffset); \
	Type Name##Offset = (BitOffset);

/*!< \brief used to define 64 bit+bitsoffset of Name.  Name is the bit pattern representing the field, Name##Offset represents the bit offset to the patten. */
#define LWBitField64_(Type, Name, BitCount, BitOffset) \
	Type Name = ((1ull<<(BitCount))-1ull)<<(BitOffset); \
	Type Name##Offset = (BitOffset);

#define LWBitFieldMulti2(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitField(Type, __VA_ARGS__, (BaseBitOffset+BitCount))

#define LWBitFieldMulti264_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitField64_(Type, __VA_ARGS__, (BaseBitOffset+BitCount))

/*!< \brief helper function used to define 32 bit bit+bitsoffset of Name.  Name##Bits is the bit pattern representing the field, Name##BitsOffset represents the bit offset to the patten. */
#define LWBitField32(Name, BitCount, BitOffset) LWBitField(static const uint32_t, Name, (BitCount), (BitOffset))
#define LWBitField32Multi2(InitialBitOffset, ...) LWBitFieldMulti2(static const uint32_t, InitialBitOffset, __VA_ARGS__)

/*!< \brief helper function used to define 16 bit bit+bitsoffset of Name.  Name##Bits is the bit pattern representing the field, Name##BitsOffset represents the bit offset to the patten. */
#define LWBitField16(Name, BitCount, BitOffset) LWBitField(static const uint16_t, Name, (BitCount), (BitOffset))
#define LWBitField16Multi2(InitialBitOffset, ...) LWBitFieldMulti2(static const uint16_t, InitialBitOffset, __VA_ARGS__)

/*!< \brief helper function used to define 64 bit bit+bitsoffset of Name.  Name##Bits is the bit pattern representing the field, Name##BitsOffset represents the bit offset to the patten. */
#define LWBitField64(Name, BitCount, BitOffset) LWBitField64_(static const uint64_t, Name, (BitCount), (BitOffset))
#define LWBitField64Multi2(InitialBitOffset, ...) LWBitFieldMulti264_(static const uint64_t, InitialBitOffset, __VA_ARGS__)

/*!< \brief helper function used to define Name and Name##Offset in the translation unit. */
#define LWBitFieldDefine(Type, Name) const Type Name; \
	const Type Name##Offset;

#define LWBitFieldDefine2(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine(__VA_ARGS__); \

/*!< \brief helper function used to define 32 bit fields in the translation units. */
#define LWBitField32Define(Name) LWBitFieldDefine(uint32_t, Name)
#define LWBitField32Define2(Name, ...) LWBitFieldDefine2(uint32_t, Name, __VA_ARGS__)

/*!< \brief helper function used to define 16 bit fields in the translation units. */
#define LWBitField16Define(Name) LWBitFieldDefine(uint16_t, Name)
#define LWBitField16Define2(Name, ...) LWBitFieldDefine2(uint16_t, Name, __VA_ARGS__)

/*!< \brief helper function used to define 64 bit fields in the translation units. */
#define LWBitField64Define(Name) LWBitFieldDefine(uint64_t, Name)
#define LWBitField64Define2(Name, ...) LWBitFieldDefine2(uint64_t, Name, __VA_ARGS__)

/*!< \brief helper macro which get's the stored value from flag of the Named bitfield(as declared by LWBitField). */
#define LWBitFieldGet(Name, Flag) \
	(((Flag)&(Name)) >> (Name##Offset))

/*!< \brief helper macro which returns a new value of flag with the value set in the named bits for the bit's defined by Name, note that this function does not prevent bit's from overflowing if value is outside the bit range of Name, Use the strict variant if that is a requirment. */
#define LWBitFieldSet(Name, Flag, Value) \
	(((Flag)&~(Name)) | ((Value) << (Name##Offset)))

/*!< \brief helper macro which return's a new value of flag with the value set in the named bits, but truncates any bit's which fall outside the range of Name's bits. */
#define LWBitFieldSetStrict(Name, Flag, Value) \
	(((Flag)&~(Name)) | (((Value) << (Name##Offset))&(Name)))

/*!< \brief returns conditional comparison that Value is within the named bits size. */
#define LWBitFieldCheckValue(Name, Value) \
	Value<=(Name>>Name##Offset)

/*!< \brief asserts that value fits within bits assigned to Name, this allows us to test multiple values at once. */
#define LWBitFieldCheckAssert(Name, Value) \
    assert(LWBitFieldCheckValue(Name, Value) && (#Name " Value exceeds supported bits."));

#define LWBitFieldCheckAssert2(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert(__VA_ARGS__)

#define LWBFEncode(ResultType, BitName, Value) \
    ((ResultType)Value << BitName##Offset)

#define LWBFEncode2(ResultType, BitName, Value, ...) \
    LWBFEncode(ResultType,  __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

//Asserts if value spills outside of bitname's space, this macro(and it's X variants) should only ever be called on it's own, as it produces several assert lines for each BitName being set.
#define LWBFEncodeAssert(ResultType, BitName, Value) \
    LWBFEncode(ResultType, BitName, Value); \
    LWBitFieldCheckAssert(BitName, Value);

#define LWBFEncodeAssert2(ResultType, BitName, Value, ...) \
    LWBFEncode2(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert2(BitName, Value, __VA_ARGS__);

#define LWBF16Encode(BitName, Value) \
    LWBFEncode(uint16_t, BitName, Value)

#define LWBF16Encode2(BitName, Value, ...) \
    LWBF16Encode(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16EncodeAssert(BitName, Value) \
    LWBFEncodeAssert(uint16_t, BitName, Value)

#define LWBF16EncodeAssert2(BitName, Value, ...) \
    LWBFEncodeAssert2(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF32Encode(BitName, Value) \
    LWBFEncode(uint32_t, BitName, Value)

#define LWBF32Encode2(BitName, Value, ...) \
    LWBF32Encode(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32EncodeAssert(BitName, Value) \
    LWBFEncodeAssert(uint32_t, BitName, Value)

#define LWBF32EncodeAssert2(BitName, Value, ...) \
    LWBFEncodeAssert2(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF64Encode(BitName, Value) \
    LWBFEncode(uint64_t, BitName, Value)

#define LWBF64EncodeAssert(BitName, Value) \
    LWBFEncodeAssert(uint64_t, BitName, Value)

#define LWBF64EncodeAssert2(BitName, Value, ...) \
    LWBFEncodeAssert2(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBFDecode1(Input, BitName, Output) \
    (Output) = static_cast<std::remove_reference_t<decltype((Output))>>(LWBitFieldGet(BitName, (Input)));

#define LWBFDecode2(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode1(Input, __VA_ARGS__)



/*! \defgroup LWCore LWCore
	\brief the core of the entire framework that is built upon these classes.
	@{
*/

#ifndef __cpp_char8_t
typedef char char8_t;
#endif

class LWAllocator;

class LWByteBuffer;

class LWByteStream;

class LWFileStream;

template<class Type>
class LWUnicode;

template<class Type>
class LWUnicodeIterator;

template<class Type>
class LWUnicodeGraphemeIterator;

class LWTimer;

template<class Type>
struct LWVector4;

template<class Type>
struct LWVector3;

template<class Type>
struct LWVector2;

template<class Type>
struct LWMatrix4;

template<class Type>
struct LWMatrix3;

template<class Type>
struct LWMatrix2;

template<class Type>
struct LWQuaternion;

template<class Type>
struct LWSVector4;

template<class Type>
struct LWSMatrix4;

template<class Type>
struct LWSQuaternion;

class LWAllocator;

class LWAllocator_Default;

class LWAllocator_LocalCircular;

class LWAllocator_ConcurrentCircular;

class LWAllocator_LocalHeap;

struct LWRef_Counter;

template<class Type>
class LWRef;

template<class Type>
class LWFIFO;

template<class Type, uint32_t MaxElements>
class LWConcurrentFIFO;

typedef LWUnicode<char8_t> LWUTF8;
typedef LWUnicode<char16_t> LWUTF16;
typedef LWUnicode<char32_t> LWUTF32;

typedef LWUnicodeIterator<char8_t> LWUTF8Iterator;
typedef LWUnicodeIterator<char16_t> LWUTF16Iterator;
typedef LWUnicodeIterator<char32_t> LWUTF32Iterator;

typedef LWUnicodeGraphemeIterator<char8_t> LWUTF8GraphemeIterator;
typedef LWUnicodeGraphemeIterator<char16_t> LWUTF16GraphemeIterator;
typedef LWUnicodeGraphemeIterator<char32_t> LWUTF32GraphemeIterator;

typedef LWUTF8Iterator LWUTF8I; //Shorthand for utf8 iterator.
typedef LWUTF16Iterator LWUTF16I;
typedef LWUTF32Iterator LWUTF32I;

typedef LWUTF8GraphemeIterator LWUTF8GI; //Shorthand for utf8 grapheme iterator;
typedef LWUTF16GraphemeIterator LWUTF16GI;
typedef LWUTF32GraphemeIterator LWUTF32GI;

/*!< \brief defined double version of the quaternion class. */
typedef LWQuaternion<double> LWQuaterniond;
/*!< \brief defined float version of the quaternion class. */
typedef LWQuaternion<float> LWQuaternionf;

/*! \brief defined double version of the Vector4 class. */
typedef LWVector4<double> LWVector4d;
/*! \brief defined float version of the Vector4 class. */
typedef LWVector4<float> LWVector4f;
/*! \brief defined int32 version of the Vector4 class. */
typedef LWVector4<int32_t> LWVector4i;
/*! \brief defined uint32 version of the Vector4 class. */
typedef LWVector4<uint32_t> LWVector4ui;

/*! \brief defined double version of the Vector3 class. */
typedef LWVector3<double> LWVector3d;
/*! \brief defined float version of the Vector3 class. */
typedef LWVector3<float> LWVector3f;
/*! \brief defined int32 version of the Vector3 class. */
typedef LWVector3<int32_t> LWVector3i;
/*! \brief defined uint32 version of the Vector3 class. */
typedef LWVector3<uint32_t> LWVector3ui;

/*! \brief defined double version of the Vector2 class. */
typedef LWVector2<double> LWVector2d;
/*! \brief defined float version of the Vector2 class. */
typedef LWVector2<float> LWVector2f;
/*! \brief defined int32 version of the Vector2 class. */
typedef LWVector2<int32_t> LWVector2i;
/*!< \brief defined uint32 version of the vector2 class. */
typedef LWVector2<uint32_t> LWVector2ui;

/*! \brief defined double version of the Matrix4 class. */
typedef LWMatrix4<double> LWMatrix4d;

/*! \brief defined float version of the Matrix4 class. */
typedef LWMatrix4<float> LWMatrix4f;

/*! \brief defined int32 version of the Matrix4 class. */
typedef LWMatrix4<int32_t> LWMatrix4i;

/*! \brief defined double version of the Matrix3 class. */
typedef LWMatrix3<double> LWMatrix3d;

/*! \brief defined float version of the Matrix3 class. */
typedef LWMatrix3<float> LWMatrix3f;

/*! \brief defined int32 version of the Matrix3 class. */
typedef LWMatrix3<int32_t> LWMatrix3i;

/*! \brief defined double version of the Matrix2 class. */
typedef LWMatrix2<double> LWMatrix2d;

/*! \brief defined float version of the Matrix2 class. */
typedef LWMatrix2<float> LWMatrix2f;

/*! \brief defined int32 version of the Matrix2 class. */
typedef LWMatrix2<int32_t> LWMatrix2i;

/*!< \brief defined float variant of simd vector4 class. */
typedef LWSVector4<float> LWSVector4f;

/*!< \brief defined double variant of simd vector4 class. */
typedef LWSVector4<double> LWSVector4d;

/*!< \brief defined int32 version of the simd vector4 class. */
typedef LWSVector4<int32_t> LWSVector4i;

/*!< \brief defined float variant of simd matrix4 class. */
typedef LWSMatrix4<float> LWSMatrix4f;

/*!< \brief defined double variant of simd matrix4 class. */
typedef LWSMatrix4<double> LWSMatrix4d;

/*!< \brief defined int32 version of the simd matrix4 class. */
typedef LWSMatrix4<int32_t> LWSMatrix4i;

/*!< \brief defined float variant of simd quaternion class. */
typedef LWSQuaternion<float> LWSQuaternionf;

/*!< \brief defined double variant of the simd quaternion class. */
typedef LWSQuaternion<double> LWSQuaterniond;

/*!< \brief internal variable for Matrix4's ortho function, this will automatically be set depending on which video driver object is last created.  Side note: i really hate that i had to implement this, but I could not think of any reasonable solution since OpenGL and directX use different depth NDC ranges.) */
extern bool LWMatrix4_UseDXOrtho;

/* @} */
/*! \mainpage The Lightwave Framework Index.
	\section intro Introduction
	The lightwave framework is designed to be a relatively light weight, module based system that can be built upon.
*/


//All variants of bitfield functions, we go upto 8 by default, but can be expanded if needed:
#define LWBitFieldMulti3(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti2(Type, (BaseBitOffset+BitCount),  __VA_ARGS__) \

#define LWBitFieldMulti4(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti3(Type, (BaseBitOffset+BitCount),  __VA_ARGS__) \

#define LWBitFieldMulti5(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti4(Type, (BaseBitOffset+BitCount),  __VA_ARGS__) \

#define LWBitFieldMulti6(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti5(Type, (BaseBitOffset+BitCount),  __VA_ARGS__) \

#define LWBitFieldMulti7(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti6(Type, (BaseBitOffset+BitCount),  __VA_ARGS__) \

#define LWBitFieldMulti8(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti7(Type, (BaseBitOffset+BitCount),  __VA_ARGS__) \

#define LWBitFieldMulti364_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti264_(Type, (BaseBitOffset+BitCount), __VA_ARGS__)

#define LWBitFieldMulti464_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti364_(Type, (BaseBitOffset+BitCount), __VA_ARGS__)

#define LWBitFieldMulti564_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti464_(Type, (BaseBitOffset+BitCount), __VA_ARGS__)

#define LWBitFieldMulti664_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti564_(Type, (BaseBitOffset+BitCount), __VA_ARGS__)

#define LWBitFieldMulti764_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti664_(Type, (BaseBitOffset+BitCount), __VA_ARGS__)

#define LWBitFieldMulti864_(Type, BaseBitOffset, Name, BitCount, ...) \
    LWBitField64_(Type, Name, BitCount, BaseBitOffset) \
    LWBitFieldMulti764_(Type, (BaseBitOffset+BitCount), __VA_ARGS__)

#define LWBitField32Multi3(InitialBitOffset, ...) LWBitFieldMulti3(static const uint32_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField32Multi4(InitialBitOffset, ...) LWBitFieldMulti4(static const uint32_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField32Multi5(InitialBitOffset, ...) LWBitFieldMulti5(static const uint32_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField32Multi6(InitialBitOffset, ...) LWBitFieldMulti6(static const uint32_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField32Multi7(InitialBitOffset, ...) LWBitFieldMulti7(static const uint32_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField32Multi8(InitialBitOffset, ...) LWBitFieldMulti8(static const uint32_t, InitialBitOffset, __VA_ARGS__)

#define LWBitField16Multi3(InitialBitOffset, ...) LWBitFieldMulti3(static const uint16_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField16Multi4(InitialBitOffset, ...) LWBitFieldMulti4(static const uint16_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField16Multi5(InitialBitOffset, ...) LWBitFieldMulti5(static const uint16_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField16Multi6(InitialBitOffset, ...) LWBitFieldMulti6(static const uint16_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField16Multi7(InitialBitOffset, ...) LWBitFieldMulti7(static const uint16_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField16Multi8(InitialBitOffset, ...) LWBitFieldMulti8(static const uint16_t, InitialBitOffset, __VA_ARGS__)

#define LWBitField64Multi3(InitialBitOffset, ...) LWBitFieldMulti364_(static const uint64_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField64Multi4(InitialBitOffset, ...) LWBitFieldMulti464_(static const uint64_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField64Multi5(InitialBitOffset, ...) LWBitFieldMulti564_(static const uint64_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField64Multi6(InitialBitOffset, ...) LWBitFieldMulti664_(static const uint64_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField64Multi7(InitialBitOffset, ...) LWBitFieldMulti764_(static const uint64_t, InitialBitOffset, __VA_ARGS__)
#define LWBitField64Multi8(InitialBitOffset, ...) LWBitFieldMulti864_(static const uint64_t, InitialBitOffset, __VA_ARGS__)

#define LWBitFieldDefine3(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine2(__VA_ARGS__); \

#define LWBitFieldDefine4(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine3(__VA_ARGS__); \

#define LWBitFieldDefine5(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine4(__VA_ARGS__); \

#define LWBitFieldDefine6(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine5(__VA_ARGS__); \

#define LWBitFieldDefine7(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine6(__VA_ARGS__); \

#define LWBitFieldDefine8(Type, Name, ...) \
    LWBitFieldDefine(Type, Name); \
    LWBitFieldDefine7(__VA_ARGS__); \

#define LWBitField32Define3(Name, ...) LWBitFieldDefine3(uint32_t, Name, __VA_ARGS__)
#define LWBitField32Define4(Name, ...) LWBitFieldDefine4(uint32_t, Name, __VA_ARGS__)
#define LWBitField32Define5(Name, ...) LWBitFieldDefine5(uint32_t, Name, __VA_ARGS__)
#define LWBitField32Define6(Name, ...) LWBitFieldDefine6(uint32_t, Name, __VA_ARGS__)
#define LWBitField32Define7(Name, ...) LWBitFieldDefine7(uint32_t, Name, __VA_ARGS__)
#define LWBitField32Define8(Name, ...) LWBitFieldDefine8(uint32_t, Name, __VA_ARGS__)

#define LWBitField16Define3(Name, ...) LWBitFieldDefine3(uint16_t, Name, __VA_ARGS__)
#define LWBitField16Define4(Name, ...) LWBitFieldDefine4(uint16_t, Name, __VA_ARGS__)
#define LWBitField16Define5(Name, ...) LWBitFieldDefine5(uint16_t, Name, __VA_ARGS__)
#define LWBitField16Define6(Name, ...) LWBitFieldDefine6(uint16_t, Name, __VA_ARGS__)
#define LWBitField16Define7(Name, ...) LWBitFieldDefine7(uint16_t, Name, __VA_ARGS__)
#define LWBitField16Define8(Name, ...) LWBitFieldDefine8(uint16_t, Name, __VA_ARGS__)

#define LWBitField64Define3(Name, ...) LWBitFieldDefine3(uint64_t, Name, __VA_ARGS__)
#define LWBitField64Define4(Name, ...) LWBitFieldDefine4(uint64_t, Name, __VA_ARGS__)
#define LWBitField64Define5(Name, ...) LWBitFieldDefine5(uint64_t, Name, __VA_ARGS__)
#define LWBitField64Define6(Name, ...) LWBitFieldDefine6(uint64_t, Name, __VA_ARGS__)
#define LWBitField64Define7(Name, ...) LWBitFieldDefine7(uint64_t, Name, __VA_ARGS__)
#define LWBitField64Define8(Name, ...) LWBitFieldDefine8(uint64_t, Name, __VA_ARGS__)

#define LWBitFieldCheckAssert3(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert2(__VA_ARGS__)

#define LWBitFieldCheckAssert4(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert3(__VA_ARGS__)

#define LWBitFieldCheckAssert5(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert4(__VA_ARGS__)

#define LWBitFieldCheckAssert6(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert5(__VA_ARGS__)

#define LWBitFieldCheckAssert7(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert6(__VA_ARGS__)

#define LWBitFieldCheckAssert8(Name, Value, ...) \
    LWBitFieldCheckAssert(Name, Value) \
    LWBitFieldCheckAssert7(__VA_ARGS__)

#define LWBFEncode3(ResultType, BitName, Value, ...) \
    LWBFEncode2(ResultType, __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

#define LWBFEncode4(ResultType, BitName, Value, ...) \
    LWBFEncode3(ResultType, __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

#define LWBFEncode5(ResultType, BitName, Value, ...) \
    LWBFEncode4(ResultType, __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

#define LWBFEncode6(ResultType, BitName, Value, ...) \
    LWBFEncode5(ResultType, __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

#define LWBFEncode7(ResultType, BitName, Value, ...) \
    LWBFEncode6(ResultType, __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

#define LWBFEncode8(ResultType, BitName, Value, ...) \
    LWBFEncode7(ResultType, __VA_ARGS__) | LWBFEncode(ResultType, BitName, Value)

#define LWBFEncodeAssert3(ResultType, BitName, Value, ...) \
    LWBFEncode3(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert3(BitName, Value, __VA_ARGS__);

#define LWBFEncodeAssert4(ResultType, BitName, Value, ...) \
    LWBFEncode4(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert4(BitName, Value, __VA_ARGS__);

#define LWBFEncodeAssert5(ResultType, BitName, Value, ...) \
    LWBFEncode5(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert5(BitName, Value, __VA_ARGS__);

#define LWBFEncodeAssert6(ResultType, BitName, Value, ...) \
    LWBFEncode6(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert6(BitName, Value, __VA_ARGS__);

#define LWBFEncodeAssert7(ResultType, BitName, Value, ...) \
    LWBFEncode7(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert7(BitName, Value, __VA_ARGS__);

#define LWBFEncodeAssert8(ResultType, BitName, Value, ...) \
    LWBFEncode8(ResultType, BitName, Value, __VA_ARGS__); \
    LWBitFieldCheckAssert8(BitName, Value, __VA_ARGS__);

#define LWBF16Encode3(BitName, Value, ...) \
    LWBF16Encode2(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16Encode4(BitName, Value, ...) \
    LWBF16Encode3(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16Encode5(BitName, Value, ...) \
    LWBF16Encode4(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16Encode6(BitName, Value, ...) \
    LWBF16Encode5(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16Encode7(BitName, Value, ...) \
    LWBF16Encode6(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16Encode8(BitName, Value, ...) \
    LWBF16Encode7(__VA_ARGS__) | LWBF16Encode(BitName, Value)

#define LWBF16EncodeAssert3(BitName, Value, ...) \
    LWBFEncodeAssert3(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF16EncodeAssert4(BitName, Value, ...) \
    LWBFEncodeAssert4(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF16EncodeAssert5(BitName, Value, ...) \
    LWBFEncodeAssert5(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF16EncodeAssert6(BitName, Value, ...) \
    LWBFEncodeAssert6(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF16EncodeAssert7(BitName, Value, ...) \
    LWBFEncodeAssert7(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF16EncodeAssert8(BitName, Value, ...) \
    LWBFEncodeAssert8(uint16_t, BitName, Value, __VA_ARGS__)

#define LWBF64Encode2(BitName, Value, ...) \
    LWBF64Encode(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF32Encode3(BitName, Value, ...) \
    LWBF32Encode2(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32Encode4(BitName, Value, ...) \
    LWBF32Encode3(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32Encode5(BitName, Value, ...) \
    LWBF32Encode4(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32Encode6(BitName, Value, ...) \
    LWBF32Encode5(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32Encode7(BitName, Value, ...) \
    LWBF32Encode6(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32Encode8(BitName, Value, ...) \
    LWBF32Encode7(__VA_ARGS__) | LWBF32Encode(BitName, Value)

#define LWBF32EncodeAssert3(BitName, Value, ...) \
    LWBFEncodeAssert3(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF32EncodeAssert4(BitName, Value, ...) \
    LWBFEncodeAssert4(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF32EncodeAssert5(BitName, Value, ...) \
    LWBFEncodeAssert5(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF32EncodeAssert6(BitName, Value, ...) \
    LWBFEncodeAssert6(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF32EncodeAssert7(BitName, Value, ...) \
    LWBFEncodeAssert7(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF32EncodeAssert8(BitName, Value, ...) \
    LWBFEncodeAssert8(uint32_t, BitName, Value, __VA_ARGS__)

#define LWBF64Encode3(BitName, Value, ...) \
    LWBF64Encode2(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF64Encode4(BitName, Value, ...) \
    LWBF64Encode3(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF64Encode5(BitName, Value, ...) \
    LWBF64Encode4(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF64Encode6(BitName, Value, ...) \
    LWBF64Encode5(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF64Encode7(BitName, Value, ...) \
    LWBF64Encode6(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF64Encode8(BitName, Value, ...) \
    LWBF64Encode7(__VA_ARGS__) | LWBF64Encode(BitName, Value)

#define LWBF64EncodeAssert3(BitName, Value, ...) \
    LWBFEncodeAssert3(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBF64EncodeAssert4(BitName, Value, ...) \
    LWBFEncodeAssert4(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBF64EncodeAssert5(BitName, Value, ...) \
    LWBFEncodeAssert5(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBF64EncodeAssert6(BitName, Value, ...) \
    LWBFEncodeAssert6(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBF64EncodeAssert7(BitName, Value, ...) \
    LWBFEncodeAssert7(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBF64EncodeAssert8(BitName, Value, ...) \
    LWBFEncodeAssert8(uint64_t, BitName, Value, __VA_ARGS__)

#define LWBFDecode3(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode2(Input, __VA_ARGS__)

#define LWBFDecode4(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode3(Input, __VA_ARGS__)

#define LWBFDecode5(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode4(Input, __VA_ARGS__)

#define LWBFDecode6(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode5(Input, __VA_ARGS__)

#define LWBFDecode7(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode6(Input, __VA_ARGS__)

#define LWBFDecode8(Input, BitName, Output, ...) \
    LWBFDecode1(Input, BitName, Output) \
    LWBFDecode7(Input, __VA_ARGS__)


#endif